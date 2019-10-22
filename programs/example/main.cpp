#include <appbase/application.hpp>
#include <iostream>
#include <boost/exception/diagnostic_information.hpp>
#include <fc/filesystem.hpp>
#include <fc/exception/exception.hpp>

int main(int argc, char **argv) {
    try {
        std::cout << u8"example " << appbase::app().version_string() << std::endl;

        auto root = fc::app_path();

        if (!appbase::app().initialize(argc, argv))
            return -1;
        appbase::app().startup();
        appbase::app().exec();
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