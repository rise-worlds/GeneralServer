#include <appbase/application.hpp>

#include <potato/http_plugin/http_plugin.hpp>

#include <iostream>
#include <boost/exception/diagnostic_information.hpp>
#include <fc/filesystem.hpp>
#include <fc/exception/exception.hpp>

using namespace appbase;
using namespace potato;

int main(int argc, char **argv) {
    try {
        std::cout << u8"example " << app().version_string() << std::endl;

        auto root = fc::app_path();

        http_plugin::set_defaults({
                .default_unix_socket_path = "",
                .default_http_port = 8888
            });
        if (!app().initialize(argc, argv))
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
    } catch( const boost::interprocess::bad_alloc& e ) {
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