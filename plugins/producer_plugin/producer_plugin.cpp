#include <potato/producer_plugin/producer_plugin.hpp>

#include <potato/chain_plugin/chain_plugin.hpp>
#include <potato/chain/exceptions.hpp>
#include <potato/chain/config.hpp>

#include <fc/network/message_buffer.hpp>
#include <fc/network/ip.hpp>
#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>
#include <fc/log/appender.hpp>
#include <fc/log/logger_config.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/crypto/rand.hpp>
#include <fc/exception/exception.hpp>
#include <fc/network/url.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/signals2/connection.hpp>

#include <atomic>
#include <shared_mutex>

const fc::string logger_name("producer_plugin");
fc::logger _log;

const fc::string trx_trace_logger_name("transaction_tracing");
fc::logger _trx_trace_log;

namespace potato
{
    using namespace potato::chain;

    class producer_plugin_impl : public std::enable_shared_from_this<producer_plugin_impl>
    {
    public:
        using signature_provider_type = std::function<chain::signature_type(chain::digest_type)>;
        std::map<chain::public_key_type, signature_provider_type> _signature_providers;

        bool _production_enabled = false;
        bool _pause_production = false;
    };
    static producer_plugin_impl *my_impl;

    static producer_plugin_impl::signature_provider_type make_key_signature_provider(const private_key_type& key) {
        return [key]( const chain::digest_type& digest ) {
            return key.sign(digest);
        };
    }

    // static producer_plugin_impl::signature_provider_type make_keosd_signature_provider(const std::shared_ptr<producer_plugin_impl>& impl, const string& url_str, const public_key_type pubkey) {
    //     fc::url keosd_url;
    //     if (boost::algorithm::starts_with(url_str, "unix://"))
    //         //send the entire string after unix:// to http_plugin. It'll auto-detect which part
    //         // is the unix socket path, and which part is the url to hit on the server
    //         keosd_url = fc::url("unix", url_str.substr(7), ostring(), ostring(), ostring(), ostring(), ovariant_object(), fc::optional<uint16_t>());
    //     else
    //         keosd_url = fc::url(url_str);
    //     std::weak_ptr<producer_plugin_impl> weak_impl = impl;
    //
    //     return [weak_impl, keosd_url, pubkey]( const chain::digest_type& digest ) {
    //         auto impl = weak_impl.lock();
    //         if (impl) {
    //             fc::variant params;
    //             fc::to_variant(std::make_pair(digest, pubkey), params);
    //             // auto deadline = impl->_keosd_provider_timeout_us.count() >= 0 ? fc::time_point::now() + impl->_keosd_provider_timeout_us : fc::time_point::maximum();
    //             auto deadline = fc::time_point::maximum();
    //             return app().get_plugin<http_client_plugin>().get_client().post_sync(keosd_url, params, deadline).as<chain::signature_type>();
    //         } else {
    //             return signature_type();
    //         }
    //     };
    // }

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
        auto default_priv_key = private_key_type::regenerate<fc::ecc::private_key_shim>(fc::sha256::hash(std::string("nathan")));
        auto private_key_default = std::make_pair(default_priv_key.get_public_key(), default_priv_key);

        boost::program_options::options_description producer_options;

        producer_options.add_options()
            ("enable-stale-production,e", boost::program_options::bool_switch()->notifier([this](bool e) { my->_production_enabled = e; }), "Enable block production, even if the chain is stale.")
            ("pause-on-startup,x", boost::program_options::bool_switch()->notifier([this](bool p) { my->_pause_production = p; }), "Start this node in a state where production is paused")
            ("producer-name,p", boost::program_options::value<vector<string>>()->composing()->multitoken(), "ID of producer controlled by this node (e.g. inita; may specify multiple times)")
            ("signature-provider", boost::program_options::value<vector<string>>()->composing()->multitoken()->default_value(
                {default_priv_key.get_public_key().to_string() + "=KEY:" + default_priv_key.to_string()}, 
                default_priv_key.get_public_key().to_string() + "=KEY:" + default_priv_key.to_string()),
                "Key=Value pairs in the form <public-key>=<provider-spec>\n"
                "Where:\n"
                "   <public-key>    \tis a string form of a vaild EOSIO public key\n\n"
                "   <provider-spec> \tis a string in the form <provider-type>:<data>\n\n"
                "   <provider-type> \tis KEY, or KEOSD\n\n"
                "   KEY:<data>      \tis a string form of a valid EOSIO private key which maps to the provided public key\n\n"
                "   KEOSD:<data>    \tis the URL where keosd is available and the approptiate wallet(s) are unlocked")
            ("producer-threads", bpo::value<uint16_t>()->default_value(config::default_controller_thread_pool_size), "Number of worker threads in producer thread pool");
        cfg.add(producer_options);
    }

    void producer_plugin::plugin_initialize(const variables_map &options)
    {
        if (options.count("signature-provider"))
        {
            const std::vector<std::string> key_spec_pairs = options["signature-provider"].as<std::vector<std::string>>();
            for (const auto &key_spec_pair : key_spec_pairs)
            {
                try
                {
                    auto delim = key_spec_pair.find("=");
                    EOS_ASSERT(delim != std::string::npos, plugin_config_exception, "Missing \"=\" in the key spec pair");
                    auto pub_key_str = key_spec_pair.substr(0, delim);
                    auto spec_str = key_spec_pair.substr(delim + 1);

                    auto spec_delim = spec_str.find(":");
                    EOS_ASSERT(spec_delim != std::string::npos, plugin_config_exception, "Missing \":\" in the key spec pair");
                    auto spec_type_str = spec_str.substr(0, spec_delim);
                    auto spec_data = spec_str.substr(spec_delim + 1);

                    auto pubkey = public_key_type(pub_key_str);

                    if (spec_type_str == "KEY")
                    {
                        my->_signature_providers[pubkey] = make_key_signature_provider(private_key_type(spec_data));
                    }
                    // else if (spec_type_str == "KEOSD")
                    // {
                    //     my->_signature_providers[pubkey] = make_keosd_signature_provider(my, spec_data, pubkey);
                    // }
                }
                catch (...)
                {
                    elog("Malformed signature provider: \"${val}\", ignoring!", ("val", key_spec_pair));
                }
            }
        }
    }

    void producer_plugin::plugin_startup()
    {
        try {
            handle_sighup();
            try {

            } catch( ... ) {
                // always call plugin_shutdown, even on exception
                plugin_shutdown();
                throw;
            } 
        } FC_CAPTURE_AND_RETHROW()
    }

    void producer_plugin::handle_sighup()
    {
        fc::logger::update( logger_name, _log );
        fc::logger::update( trx_trace_logger_name, _trx_trace_log );
    }

    void producer_plugin::plugin_shutdown()
    {
    }

    bool producer_plugin::is_producer_key(const chain::public_key_type &key) const
    {
        auto private_key_itr = my->_signature_providers.find(key);
        if (private_key_itr != my->_signature_providers.end())
            return true;
        return false;
    }

    chain::signature_type producer_plugin::sign_compact(const chain::public_key_type &key, const fc::sha256 &digest) const
    {
        if (key != chain::public_key_type())
        {
            auto private_key_itr = my->_signature_providers.find(key);
            EOS_ASSERT(private_key_itr != my->_signature_providers.end(), producer_priv_key_not_found, "Local producer has no private key in config.ini corresponding to public key ${key}", ("key", key));

            return private_key_itr->second(digest);
        }
        else
        {
            return chain::signature_type();
        }
    }

} // namespace potato