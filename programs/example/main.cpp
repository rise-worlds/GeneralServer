#include <appbase/application.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <fc/filesystem.hpp>
#include <fc/exception/exception.hpp>

#include <iostream>
#include "config.hpp"
#include "wabt.hpp"

using namespace appbase;
using namespace example::wabt;

int main(int argc, char **argv) {
    try {
        app().set_version(GeneralServer::example::config::version);
        std::cout << u8"example " << appbase::app().version_string() << std::endl;

        std::vector<uint8_t> data = {
            0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x14, 0x04, 0x60,
            0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x60, 0x02, 0x7f, 0x7f, 0x00, 0x60, 0x01,
            0x7f, 0x01, 0x7f, 0x60, 0x00, 0x00, 0x02, 0x2d, 0x03, 0x04, 0x68, 0x6f,
            0x73, 0x74, 0x03, 0x6d, 0x65, 0x6d, 0x02, 0x00, 0x01, 0x04, 0x68, 0x6f,
            0x73, 0x74, 0x08, 0x66, 0x69, 0x6c, 0x6c, 0x5f, 0x62, 0x75, 0x66, 0x00,
            0x00, 0x04, 0x68, 0x6f, 0x73, 0x74, 0x08, 0x62, 0x75, 0x66, 0x5f, 0x64,
            0x6f, 0x6e, 0x65, 0x00, 0x01, 0x03, 0x03, 0x02, 0x02, 0x03, 0x07, 0x09,
            0x01, 0x05, 0x72, 0x6f, 0x74, 0x31, 0x33, 0x00, 0x03, 0x0a, 0x74, 0x02,
            0x39, 0x01, 0x01, 0x7f, 0x20, 0x00, 0x41, 0xc1, 0x00, 0x49, 0x04, 0x40,
            0x20, 0x00, 0x0f, 0x0b, 0x20, 0x00, 0x41, 0xdf, 0x01, 0x71, 0x21, 0x01,
            0x20, 0x01, 0x41, 0xcd, 0x00, 0x4d, 0x04, 0x40, 0x20, 0x00, 0x41, 0x0d,
            0x6a, 0x0f, 0x0b, 0x20, 0x01, 0x41, 0xda, 0x00, 0x4d, 0x04, 0x40, 0x20,
            0x00, 0x41, 0x0d, 0x6b, 0x0f, 0x0b, 0x20, 0x00, 0x0f, 0x0b, 0x38, 0x01,
            0x02, 0x7f, 0x41, 0x00, 0x41, 0x80, 0x08, 0x10, 0x00, 0x21, 0x00, 0x02,
            0x40, 0x03, 0x40, 0x20, 0x01, 0x20, 0x00, 0x4f, 0x04, 0x40, 0x0c, 0x02,
            0x0b, 0x20, 0x01, 0x20, 0x01, 0x2d, 0x00, 0x00, 0x10, 0x02, 0x3a, 0x00,
            0x00, 0x20, 0x01, 0x41, 0x01, 0x6a, 0x21, 0x01, 0x0c, 0x00, 0x0b, 0x0b,
            0x41, 0x00, 0x20, 0x00, 0x10, 0x01, 0x0b,
        };
        wabt_test test;
        test.string_data = "Hello, WebAssembly!";
        test.Init();
        test.LoadModule(data);
        test.Run();
        std::cout << test.string_data << std::endl;
        test.Run();
        std::cout << test.string_data << std::endl;

        auto root = fc::app_path();
        app().set_default_data_dir(root / "example/data" );
        app().set_default_config_dir(root / "example/config" );
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