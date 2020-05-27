#include <appbase/application.hpp>

#include <eosio/http_plugin/http_plugin.hpp>
#include <eosio/net_plugin/net_plugin.hpp>
#include <eosio/producer_plugin/producer_plugin.hpp>
#include <potato/version/version.hpp>

#include <boost/exception/diagnostic_information.hpp>
#include <fc/filesystem.hpp>
#include <fc/exception/exception.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/compressed_pair.hpp>

#include <iostream>
#include "config.hpp"
// #include "wabt.hpp"

using namespace appbase;
using namespace eosio;

int main(int argc, char **argv) {
    try {

        app().set_version(GeneralServer::example::config::version);
        app().set_version_string(potato::version::version_client());
        app().set_full_version_string(potato::version::version_full());
        std::cout << u8"example " << app().version_string() << std::endl;

        http_plugin::set_defaults({
                .default_unix_socket_path = "",
                .default_http_port = 8888
            });
        if (!app().initialize<chain_plugin, net_plugin, producer_plugin>(argc, argv))
        {
            const auto& opts = app().get_options();
            if( opts.count("help") || opts.count("version") || opts.count("full-version") || opts.count("print-default-config") ) {
                return 0;
            }
            return -1;
        }
        app().startup();
        app().set_thread_priority_max();
        app().exec();
    } catch( const fc::exception& e ) {
        std::cerr << e.to_detail_string() << "\n";
    } catch (const boost::interprocess::bad_alloc &e) {
        std::cerr << "bad alloc" << "\n";
    } catch (const boost::exception &e) {
        std::cerr << boost::diagnostic_information(e) << "\n";
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
    } catch (...) {
        std::cerr << "unknown exception\n";
    }
    std::cout << "exited cleanly\n";
    return 0;
}