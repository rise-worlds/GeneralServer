#include <potato/chain_plugin/chain_plugin.hpp>

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

namespace potato
{
    class chain_plugin_impl : public std::enable_shared_from_this<chain_plugin_impl>
    {
        public:
            fc::optional<controller> chain;
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

    void chain_plugin::set_program_options(options_description & /*cli*/, options_description &cfg)
    {
    }

    void chain_plugin::plugin_initialize(const variables_map &options)
    {
        try {
            genesis_state gs;
            fc::optional<chain_id_type> chain_id = gs.compute_chain_id();

            my->chain.emplace( *chain_id );
            ilog("chain_id is ${id}", ("id", *chain_id));
        } FC_LOG_AND_RETHROW()
    }

    void chain_plugin::plugin_startup()
    {
    }

    void chain_plugin::handle_sighup()
    {
    }

    void chain_plugin::plugin_shutdown()
    {
    }

    chain::chain_id_type chain_plugin::get_chain_id() const {
        return my->chain->get_chain_id();
    }

} // namespace potato