#include <potato/chain_plugin/chain_plugin.hpp>

#include <chainbase/environment.hpp>

#include <potato/chain/controller.hpp>
#include <potato/chain/genesis_state.hpp>

#include <fc/network/message_buffer.hpp>
#include <fc/network/ip.hpp>
#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>
#include <fc/log/appender.hpp>
#include <fc/log/logger_config.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/crypto/rand.hpp>
#include <fc/exception/exception.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/steady_timer.hpp>

#include <atomic>
#include <shared_mutex>

using namespace potato;
using namespace potato::chain;
using boost::signals2::scoped_connection;

// reflect chainbase::environment for --print-build-info option
FC_REFLECT_ENUM( chainbase::environment::os_t,
                 (OS_LINUX)(OS_MACOS)(OS_WINDOWS)(OS_OTHER) )
FC_REFLECT_ENUM( chainbase::environment::arch_t,
                 (ARCH_X86_64)(ARCH_ARM)(ARCH_RISCV)(ARCH_OTHER) )
FC_REFLECT(chainbase::environment, (debug)(os)(arch)(boost_version)(compiler) )

namespace potato
{
    class chain_plugin_impl : public std::enable_shared_from_this<chain_plugin_impl>
    {
    public:
        chain_plugin_impl(){}

        bfs::path blocks_dir;

        fc::optional<controller> chain;
        fc::optional<controller::config> chain_config;
        fc::optional<genesis_state> genesis;
    };
    static chain_plugin_impl *my_impl;

    chain_plugin::chain_plugin()
        : my(new chain_plugin_impl)
    {
        my_impl = my.get();
    }

    chain_plugin::~chain_plugin()
    {
    }

    void chain_plugin::set_program_options(options_description &cli, options_description &cfg)
    {
        cfg.add_options()
            ("blocks-dir", bpo::value<bfs::path>()->default_value("blocks"), "the location of the blocks directory (absolute path or relative to application data dir)")
            ("chain-state-db-size-mb", bpo::value<uint64_t>()->default_value(config::default_state_size / (1024  * 1024)), "Maximum size (in MiB) of the chain state database")
            ("chain-state-db-guard-size-mb", bpo::value<uint64_t>()->default_value(config::default_state_guard_size / (1024  * 1024)), "Safely shut down node when free space remaining in the chain state database drops below this size (in MiB).")
            ("reversible-blocks-db-size-mb", bpo::value<uint64_t>()->default_value(config::default_reversible_cache_size / (1024  * 1024)), "Maximum size (in MiB) of the reversible blocks database")
            ("reversible-blocks-db-guard-size-mb", bpo::value<uint64_t>()->default_value(config::default_reversible_guard_size / (1024  * 1024)), "Safely shut down node when free space remaining in the reverseible blocks database drops below this size (in MiB).")
            ("signature-cpu-billable-pct", bpo::value<uint32_t>()->default_value(config::default_sig_cpu_bill_pct / config::percent_1), "Percentage of actual signature recovery cpu to bill. Whole number percentages, e.g. 50 for 50%")
            ("chain-threads", bpo::value<uint16_t>()->default_value(config::default_controller_thread_pool_size), "Number of worker threads in controller thread pool")
          ;
        cli.add_options()
            ("genesis-json", bpo::value<bfs::path>(), "File to read Genesis State from")
            ("genesis-timestamp", bpo::value<string>(), "override the initial timestamp in the Genesis State file")
            ("print-genesis-json", bpo::bool_switch()->default_value(false), "extract genesis_state from blocks.log as JSON, print to console, and exit")
            ("extract-genesis-json", bpo::value<bfs::path>(), "extract genesis_state from blocks.log as JSON, write into specified file, and exit")
            ("print-build-info", bpo::bool_switch()->default_value(false), "print build environment information to console as JSON and exit")
            ("extract-build-info", bpo::value<bfs::path>(), "extract build environment information as JSON, write into specified file, and exit")
            ("delete-all-blocks", bpo::bool_switch()->default_value(false), "clear chain state database and block log")
          ;
    }

    void clear_directory_contents( const fc::path& p ) {
        using boost::filesystem::directory_iterator;

        if( !fc::is_directory( p ) )
            return;

        for( directory_iterator enditr, itr{p}; itr != enditr; ++itr ) {
            fc::remove_all( itr->path() );
        }
    }

    void clear_chainbase_files( const fc::path& p ) {
        if( !fc::is_directory( p ) )
            return;

        fc::remove( p / "shared_memory.bin" );
        fc::remove( p / "shared_memory.meta" );
    }

    void chain_plugin::plugin_initialize(const variables_map &options)
    {
        try
        {
            try
            {
                genesis_state gs; // Check if POTATO_ROOT_KEY is bad
            }
            catch (const fc::exception &)
            {
                elog("POTATO_ROOT_KEY ('${root_key}') is invalid. Recompile with a valid public key.",
                     ("root_key", genesis_state::potato_root_key));
                throw;
            }

            my->chain_config = controller::config();

            if (options.at("print-build-info").as<bool>() || options.count("extract-build-info"))
            {
                if (options.at("print-build-info").as<bool>())
                {
                    ilog("Build environment JSON:\n${e}", ("e", fc::json::to_pretty_string(chainbase::environment())));
                }
                if (options.count("extract-build-info"))
                {
                    auto p = options.at("extract-build-info").as<bfs::path>();

                    if (p.is_relative())
                    {
                        p = bfs::current_path() / p;
                    }

                    EOS_ASSERT(fc::json::save_to_file(chainbase::environment(), p, true), misc_exception,
                               "Error occurred while writing build info JSON to '${path}'",
                               ("path", p.generic_string()));

                    ilog("Saved build info JSON to '${path}'", ("path", p.generic_string()));
                }

                EOS_THROW(node_management_success, "reported build environment information");
            }

            if (options.count("blocks-dir"))
            {
                auto bld = options.at("blocks-dir").as<bfs::path>();
                if (bld.is_relative())
                    my->blocks_dir = app().data_dir() / bld;
                else
                    my->blocks_dir = bld;
            }

            if (options.count("chain-state-db-size-mb"))
                my->chain_config->state_size = options.at("chain-state-db-size-mb").as<uint64_t>() * 1024 * 1024;

            if (options.count("chain-state-db-guard-size-mb"))
                my->chain_config->state_guard_size = options.at("chain-state-db-guard-size-mb").as<uint64_t>() * 1024 * 1024;

            if (options.count("reversible-blocks-db-size-mb"))
                my->chain_config->reversible_cache_size =
                    options.at("reversible-blocks-db-size-mb").as<uint64_t>() * 1024 * 1024;

            if (options.count("reversible-blocks-db-guard-size-mb"))
                my->chain_config->reversible_guard_size = options.at("reversible-blocks-db-guard-size-mb").as<uint64_t>() * 1024 * 1024;

            if (options.count("chain-threads"))
            {
                my->chain_config->thread_pool_size = options.at("chain-threads").as<uint16_t>();
                EOS_ASSERT(my->chain_config->thread_pool_size > 0, plugin_config_exception,
                           "chain-threads ${num} must be greater than 0", ("num", my->chain_config->thread_pool_size));
            }

            if (options.count("extract-genesis-json") || options.at("print-genesis-json").as<bool>())
            {
                fc::optional<genesis_state> gs;

                // if (fc::exists(my->blocks_dir / "blocks.log"))
                // {
                //     gs = block_log::extract_genesis_state(my->blocks_dir);
                //     EOS_ASSERT(gs,
                //                plugin_config_exception,
                //                "Block log at '${path}' does not contain a genesis state, it only has the chain-id.",
                //                ("path", (my->blocks_dir / "blocks.log").generic_string()));
                // }
                // else
                // {
                //     wlog("No blocks.log found at '${p}'. Using default genesis state.",
                //          ("p", (my->blocks_dir / "blocks.log").generic_string()));
                //     gs.emplace();
                // }

                if (options.at("print-genesis-json").as<bool>())
                {
                    ilog("Genesis JSON:\n${genesis}", ("genesis", fc::json::to_pretty_string(*gs)));
                }

                if (options.count("extract-genesis-json"))
                {
                    auto p = options.at("extract-genesis-json").as<bfs::path>();

                    if (p.is_relative())
                    {
                        p = bfs::current_path() / p;
                    }

                    EOS_ASSERT(fc::json::save_to_file(*gs, p, true),
                               misc_exception,
                               "Error occurred while writing genesis JSON to '${path}'",
                               ("path", p.generic_string()));

                    ilog("Saved genesis JSON to '${path}'", ("path", p.generic_string()));
                }

                EOS_THROW(extract_genesis_state_exception, "extracted genesis state from blocks.log");
            }

            if (options.at("delete-all-blocks").as<bool>())
            {
                ilog("Deleting state database and blocks");
                // if( options.at( "truncate-at-block" ).as<uint32_t>() > 0 )
                //     wlog( "The --truncate-at-block option does not make sense when deleting all blocks." );
                clear_directory_contents(my->chain_config->state_dir);
                clear_directory_contents(my->blocks_dir);
            }

            fc::optional<chain_id_type> chain_id;
            chain_id = controller::extract_chain_id_from_db(my->chain_config->state_dir);

            if (!chain_id)
            {
                if (my->genesis)
                {
                    // Uninitialized state database and genesis state extracted from block log
                    chain_id = my->genesis->compute_chain_id();
                }
                else
                {
                    // Uninitialized state database and no genesis state provided

                    // EOS_ASSERT(!block_log_chain_id, plugin_config_exception,
                    //            "Genesis state is necessary to initialize fresh blockchain state but genesis state could not be "
                    //            "found in the blocks log. Please either load from snapshot or find a blocks log that starts "
                    //            "from genesis.");

                    ilog("Starting fresh blockchain state using default genesis state.");
                    my->genesis.emplace();
                    chain_id = my->genesis->compute_chain_id();
                }
            }

            my->chain.emplace(*my->chain_config, *chain_id);
            ilog("chain_id is ${id}", ("id", *chain_id));
        }
        FC_LOG_AND_RETHROW()
    }

    void chain_plugin::plugin_startup()
    {
        try
        {
            try
            {
                auto shutdown = []() { return app().is_quiting(); };
                if (my->genesis)
                {
                    my->chain->startup(shutdown, *my->genesis);
                }
                else
                {
                    my->chain->startup(shutdown);
                }
            }
            catch (const database_guard_exception &e)
            {
                log_guard_exception(e);
                // make sure to properly close the db
                my->chain.reset();
                throw;
            }
        }
        FC_CAPTURE_AND_RETHROW()
    }

    void chain_plugin::handle_sighup()
    {
    }

    void chain_plugin::plugin_shutdown()
    {
    }

    void chain_plugin::log_guard_exception(const chain::guard_exception&e ) {
        if (e.code() == chain::database_guard_exception::code_value) {
            elog("Database has reached an unsafe level of usage, shutting down to avoid corrupting the database.  "
                "Please increase the value set for \"chain-state-db-size-mb\" and restart the process!");
        } else if (e.code() == chain::reversible_guard_exception::code_value) {
            elog("Reversible block database has reached an unsafe level of usage, shutting down to avoid corrupting the database.  "
                "Please increase the value set for \"reversible-blocks-db-size-mb\" and restart the process!");
        }

        dlog("Details: ${details}", ("details", e.to_detail_string()));
    }

    controller& chain_plugin::chain() { return *my->chain; }
    const controller& chain_plugin::chain() const { return *my->chain; }

    chain::chain_id_type chain_plugin::get_chain_id() const
    {
        return my->chain->get_chain_id();
    }

} // namespace potato