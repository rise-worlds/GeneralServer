#include <potato/producer_plugin/producer_plugin.hpp>

#include <potato/chain_plugin/chain_plugin.hpp>

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

namespace potato
{
    class producer_plugin_impl : public std::enable_shared_from_this<producer_plugin_impl>{};
    static producer_plugin_impl *my_impl;

    producer_plugin::producer_plugin()
        : my(new producer_plugin_impl)
    {
        my_impl = my.get();
    }

    producer_plugin::~producer_plugin()
    {
    }

    void producer_plugin::set_program_options(options_description & /*cli*/, options_description &cfg)
    {
    }

    void producer_plugin::plugin_initialize(const variables_map &options)
    {
    }

    void producer_plugin::plugin_startup()
    {
    }

    void producer_plugin::handle_sighup()
    {
    }

    void producer_plugin::plugin_shutdown()
    {
    }

} // namespace potato