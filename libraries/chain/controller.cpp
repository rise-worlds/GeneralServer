#include <potato/chain/controller.hpp>
#include <potato/chain/types.hpp>
#include <potato/chain/global_property_object.hpp>
#include <potato/chain/thread_utils.hpp>
#include <potato/chain/platform_timer.hpp>
#include <potato/chain/database_header_object.hpp>
#include <chainbase/chainbase.hpp>

namespace potato
{
    using chainbase::database;

    namespace chain
    {
        struct controller_impl
        {
            // LLVM sets the new handler, we need to reset this to throw a bad_alloc exception so we can possibly exit cleanly
            // and not just abort.
            struct reset_new_handler {
                reset_new_handler() { std::set_new_handler([](){ throw std::bad_alloc(); }); }
            };

            reset_new_handler              rnh; // placed here to allow for this to be set before constructing the other fields
            controller& self;
            controller::config conf;
            const chain_id_type chain_id;
            chainbase::database db;

            named_thread_pool              thread_pool;
            platform_timer                 timer;

            controller_impl(const controller::config &cfg, controller& s, const chain_id_type &chain_id)
                : rnh()
                , self(s)
                , conf(cfg)
                , chain_id(chain_id)
                , db(cfg.state_dir, database::read_write, cfg.state_size, true)
                , thread_pool("chain", cfg.thread_pool_size)
            {
            }

            void startup(std::function<bool()> shutdown, const genesis_state& genesis)
            {
                const auto& genesis_chain_id = genesis.compute_chain_id();
                EOS_ASSERT( genesis_chain_id == chain_id, chain_id_type_exception,
                            "genesis state provided to startup corresponds to a chain ID (${genesis_chain_id}) that does not match the chain ID that controller was constructed with (${controller_chain_id})",
                            ("genesis_chain_id", genesis_chain_id)("controller_chain_id", chain_id)
                );

                initialize_blockchain_state(genesis); // sets head to genesis state

                init(shutdown);
            }
            void startup(std::function<bool()> shutdown)
            {
                init(shutdown);
            }

            static auto validate_db_version( const chainbase::database& db ) {
                // check database version
                const auto& header_idx = db.get_index<database_header_multi_index>().indices().get<by_id>();

                EOS_ASSERT(header_idx.begin() != header_idx.end(), bad_database_version_exception,
                            "state database version pre-dates versioning, please restore from a compatible snapshot or replay!");

                auto header_itr = header_idx.begin();
                header_itr->validate();

                return header_itr;
            }
            void init(std::function<bool()> shutdown)
            {
                {
                    const auto& state_chain_id = db.get<global_property_object>().chain_id;
                    EOS_ASSERT( state_chain_id == chain_id, chain_id_type_exception,
                                "chain ID in state (${state_chain_id}) does not match the chain ID that controller was constructed with (${controller_chain_id})",
                                ("state_chain_id", state_chain_id)("controller_chain_id", chain_id)
                    );
                }

                auto header_itr = validate_db_version( db );
                // upgrade to the latest compatible version
                if (header_itr->version != database_header_object::current_version) {
                    db.modify(*header_itr, [](auto& header) {
                        header.version = database_header_object::current_version;
                    });
                }

                if (shutdown()) return;
            }

            ~controller_impl()
            {
                thread_pool.stop();
            }

            /**
             *  Sets fork database head to the genesis state.
             */
            void initialize_blockchain_state(const genesis_state& genesis)
            {
                wlog("Initializinng new blockchain with genesis state");
                initialize_database(genesis);
            }

            void initialize_database(const genesis_state& genesis)
            {
                // create the database header sigil
                db.create<database_header_object>([&]( auto& header ){
                    // nothing to do for now
                });

                genesis.initial_configuration.validate();
                db.create<global_property_object>([&genesis,&chain_id=this->chain_id](auto& gpo ){
                    gpo.configuration = genesis.initial_configuration;
                    gpo.chain_id = chain_id;
                });
                db.create<dynamic_global_property_object>([](auto&){});
            }
        };

        controller::controller(const controller::config &config, const chain_id_type &chain_id)
            : my(new controller_impl(config, *this, chain_id))
        {
        }

        controller::~controller()
        {
        }

        void controller::startup( std::function<bool()> shutdown, const genesis_state& genesis )
        {
            my->startup(shutdown, genesis);
        }

        void controller::startup( std::function<bool()> shutdown )
        {
            my->startup(shutdown);
        }

        const chainbase::database& controller::db()const { return my->db; }

        chainbase::database& controller::mutable_db()const { return my->db; }

        chain_id_type controller::get_chain_id() const
        {
            return my->chain_id;
        }

        boost::asio::io_context& controller::get_thread_pool()
        {
            return my->thread_pool.get_executor();
        }

        fc::optional<chain_id_type> controller::extract_chain_id_from_db( const path& state_dir ) {
            try {
                chainbase::database db( state_dir, chainbase::database::read_only );

                db.add_index<database_header_multi_index>();
                db.add_index<global_property_multi_index>();

                controller_impl::validate_db_version( db );

                if( db.revision() < 1 ) return {};

                return db.get<global_property_object>().chain_id;
            } catch( const bad_database_version_exception& ) {
                throw;
            } catch( ... ) {
            }

            return {};
        }

    } // namespace chain
} // namespace potato