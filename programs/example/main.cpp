#include <appbase/application.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <fc/filesystem.hpp>
#include <fc/exception/exception.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <iostream>
#include "config.hpp"
#include "wabt.hpp"
#include "name.hpp"
#include "muilt_index.hpp"

using namespace appbase;
using namespace example;
using namespace example::wabt;

std::ostream &operator<<(std::ostream &dest, __uint128_t value) {
    std::ostream::sentry s(dest);
    if (s) {
        __uint128_t tmp = value;
        char buffer[128];
        char *d = std::end(buffer);
        do {
            --d;
            *d = "0123456789ABCDEF"[tmp % 0x0F];
            tmp /= 0x0F;
        } while (tmp != 0);
        int len = std::end(buffer) - d;
        if (dest.rdbuf()->sputn(d, len) != len) {
            dest.setstate(std::ios_base::badbit);
        }
    }
    return dest;
}

//name test = "test"_n;
//name_t test_t = "test"_t;
//char tablename[] = "abcdefghijklmnopqrstuvwxyz\0";

int main(int argc, char **argv) {
    try {
        app().set_version(GeneralServer::example::config::version);
        std::cout << u8"example " << appbase::app().version_string() << std::endl;

        {
            using namespace boost::multiprecision::literals;
            using namespace boost::multiprecision;
            constexpr std::array<__uint128_t, 2UL> a = {0};
            constexpr __uint128_t b[2UL] = {0};
            constexpr boost::multiprecision::uint256_t c = 0x00_cppui256;
            constexpr std::array<__uint64_t, 4UL> d = {0};
            constexpr __uint64_t e[4UL] = {0};
            std::cout << sizeof(a) << ", " << sizeof(b) << ", " << sizeof(c) << ", " << sizeof(d) << ", " << sizeof(e) << std::endl;
        }
        
        // std::vector<uint8_t> data = {
        //     0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x14, 0x04, 0x60,
        //     0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x60, 0x02, 0x7f, 0x7f, 0x00, 0x60, 0x01,
        //     0x7f, 0x01, 0x7f, 0x60, 0x00, 0x00, 0x02, 0x2d, 0x03, 0x04, 0x68, 0x6f,
        //     0x73, 0x74, 0x03, 0x6d, 0x65, 0x6d, 0x02, 0x00, 0x01, 0x04, 0x68, 0x6f,
        //     0x73, 0x74, 0x08, 0x66, 0x69, 0x6c, 0x6c, 0x5f, 0x62, 0x75, 0x66, 0x00,
        //     0x00, 0x04, 0x68, 0x6f, 0x73, 0x74, 0x08, 0x62, 0x75, 0x66, 0x5f, 0x64,
        //     0x6f, 0x6e, 0x65, 0x00, 0x01, 0x03, 0x03, 0x02, 0x02, 0x03, 0x07, 0x09,
        //     0x01, 0x05, 0x72, 0x6f, 0x74, 0x31, 0x33, 0x00, 0x03, 0x0a, 0x74, 0x02,
        //     0x39, 0x01, 0x01, 0x7f, 0x20, 0x00, 0x41, 0xc1, 0x00, 0x49, 0x04, 0x40,
        //     0x20, 0x00, 0x0f, 0x0b, 0x20, 0x00, 0x41, 0xdf, 0x01, 0x71, 0x21, 0x01,
        //     0x20, 0x01, 0x41, 0xcd, 0x00, 0x4d, 0x04, 0x40, 0x20, 0x00, 0x41, 0x0d,
        //     0x6a, 0x0f, 0x0b, 0x20, 0x01, 0x41, 0xda, 0x00, 0x4d, 0x04, 0x40, 0x20,
        //     0x00, 0x41, 0x0d, 0x6b, 0x0f, 0x0b, 0x20, 0x00, 0x0f, 0x0b, 0x38, 0x01,
        //     0x02, 0x7f, 0x41, 0x00, 0x41, 0x80, 0x08, 0x10, 0x00, 0x21, 0x00, 0x02,
        //     0x40, 0x03, 0x40, 0x20, 0x01, 0x20, 0x00, 0x4f, 0x04, 0x40, 0x0c, 0x02,
        //     0x0b, 0x20, 0x01, 0x20, 0x01, 0x2d, 0x00, 0x00, 0x10, 0x02, 0x3a, 0x00,
        //     0x00, 0x20, 0x01, 0x41, 0x01, 0x6a, 0x21, 0x01, 0x0c, 0x00, 0x0b, 0x0b,
        //     0x41, 0x00, 0x20, 0x00, 0x10, 0x01, 0x0b,
        // };
        // wabt_test test;
        // test.string_data = "Hello, WebAssembly!";
        // test.Init();
        // test.LoadModule(data);
        // test.Run();
        // std::cout << test.string_data << std::endl;
        // test.Run();
        // std::cout << test.string_data << std::endl;

        // constexpr uint128_t mask = uint128_t(0xFC00000000000000ull) << 64;
        // std::cout << mask << std::endl;

//         name test1 = "test1"_n;
        // test1.set("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
        // std::cout << std::string(test1) << ": " << test1.length() << std::endl;
        // std::string test1_str = test1;
        // std::cout << test1_str << ": " << test1_str.length() << std::endl;
        // name_t test2 = test1;
        // std::cout << std::string(test1) << ": " << (uint64_t)test1.length() << ": " << test2[0] << ", " << test2[1] << std::endl;
        // name test3("eosio.token");
        // name suffix;
        // std::cout << std::string(test3) << ": " << (uint64_t) test3.length() << ": " << test3.value[0] << ", "
        //           << test3.value[1] << std::endl;
        // suffix = test3.suffix();
        // std::cout << std::string(suffix) << ": " << (uint64_t) suffix.length() << ": " << suffix.value[0] << ", "
        //           << suffix.value[1] << std::endl;
        // test3 = name("eosio.tokentokentokentokentokentokentoken");
        // std::cout << std::string(test3) << ": " << (uint64_t) test3.length() << ": " << test3.value[0] << ", "
        //           << test3.value[1] << std::endl;
        // suffix = test3.suffix();
        // std::cout << std::string(suffix) << ": " << (uint64_t) suffix.length() << ": " << suffix.value[0] << ", "
        //           << suffix.value[1] << std::endl;
        // test3 = name("eosioeosioeosioeosioeosio.token");
        // std::cout << std::string(test3) << ": " << (uint64_t) test3.length() << ": " << test3.value[0] << ", "
        //           << test3.value[1] << std::endl;
        // suffix = test3.suffix();
        // std::cout << std::string(suffix) << ": " << (uint64_t) suffix.length() << ": " << suffix.value[0] << ", "
        //           << suffix.value[1] << std::endl;
        // test3 = name("eosioeosioeosioeosioe.token");
        // std::cout << std::string(test3) << ": " << (uint64_t) test3.length() << ": " << test3.value[0] << ", "
        //           << test3.value[1] << std::endl;
        // suffix = test3.suffix();
        // std::cout << std::string(suffix) << ": " << (uint64_t) suffix.length() << ": " << suffix.value[0] << ", "
        //           << suffix.value[1] << std::endl;
        // test3 = name("eosioeosioeosioeosioeo.token");
        // std::cout << std::string(test3) << ": " << (uint64_t) test3.length() << ": " << test3.value[0] << ", "
        //           << test3.value[1] << std::endl;
        // suffix = test3.suffix();
        // std::cout << std::string(suffix) << ": " << (uint64_t) suffix.length() << ": " << suffix.value[0] << ", "
        //           << suffix.value[1] << std::endl;
        // test3 = name("eosioeosioeosioeosioeos.token");
        // std::cout << std::string(test3) << ": " << (uint64_t) test3.length() << ": " << test3.value[0] << ", "
        //           << test3.value[1] << std::endl;
        // suffix = test3.suffix();
        // std::cout << std::string(suffix) << ": " << (uint64_t) suffix.length() << ": " << suffix.value[0] << ", "
        //           << suffix.value[1] << std::endl;
        // test3 = name("eosio.tokentokentokentokentokentokentokentoken");
        // std::cout << std::string(test3) << ": " << (uint64_t) test3.length() << ": " << test3.value[0] << ", "
        //           << test3.value[1] << std::endl;
        // suffix = test3.suffix();
        // std::cout << std::string(suffix) << ": " << (uint64_t) suffix.length() << ": " << suffix.value[0] << ", "
        //           << suffix.value[1] << std::endl;

        struct record {
            uint64_t primary;

            uint64_t primary_key() const { return primary; }
        };

        name test = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopq"_n;
        std::cout << std::hex << std::showbase << test.value << std::endl;
        example::uint256_t v = 0x00_cppui256;
        std::cout << std::hex << std::showbase << v << std::endl;
        // memcpy(&v, &test.qwords, sizeof(uint64_t)*4);
        // v.backend().resize(4, 1);
        import_bits(v, test.qwords.begin(), test.qwords.end(), 0, false);
        std::cout << std::hex << std::showbase << v << std::endl;
        std::cout << std::hex << std::showbase << test.qwords[0] << ", " << test.qwords[1] << ", " << test.qwords[2] << ", " << test.qwords[3] << std::endl;
        auto limbs = v.backend().limbs();
        std::cout << std::hex << std::showbase << limbs[0] << ", " << limbs[1] << ", " << limbs[2] << ", " << limbs[3] << std::endl;
        std::cout << std::string(test) << ": " << test.length() << std::endl;
        std::string test_str = test;
        std::cout << test_str << ": " << test_str.length() << std::endl;
//        muilt_index<test, record> table("root"_n, "root"_n);
//        muilt_index<test_t, record> table("root"_n, "root"_n);
//        muilt_index<"test"_t, record> table("root"_n, "root"_n);
//        muilt_index<name("test"), record> table("root"_n, "root"_n);
//        muilt_index_test<"test"_tuple, record> table;
//        typedef muilt_index<"abcdefghijklmnopqrstuvwxyzabcdefghijklmnop"_l, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnop"_r, record> Table;
//        typedef muilt_index<T("abcdefghijklmnopqrstuvwxyzabcdefghijklmnop"), record> Table;
//        Table table("test"_n, "root"_n);
//        std::cout << std::string(Table::TableName) << ", " << Table::TableName.length() << std::endl;
//        name_t temp = {T("abcdefghijklmnopqrstuvwxyzabcdefghijklmnop")};
//        name temp2(T("abcdefghijklmnopqrstuvwxyzabcdefghijklmnop"));
//        std::cout << std::string(temp2) << ", " << temp2.length() << std::endl;

//        muilt_index_test<("abcdefghijklmnopqrstuvwxyzabcdefghijklmnop"), record> table("test"_n, "root"_n);
//        muilt_index_test<("abcdefghijklmnopqrstuvwxyzabcdefghijklmnop"), record> table("test"_n, "root"_n);
//        constexpr static const char tablename[] = "abcdefghijklmnopqrstuvwxyz\0";
//        muilt_index_test<tablename, record> table("test"_n, "root"_n);
//        std::cout << std::string(table._code) << ", " << std::string(table._scope) << ", " << std::string(table._table)
//                  << std::endl;
//        std::cout << (table._code.length()) << ", " << (table._scope.length()) << ", " << (table._table.length())
//                  << std::endl;

        auto root = fc::app_path();
        app().set_default_data_dir(root / "example/data");
        app().set_default_config_dir(root / "example/config");
        if (!appbase::app().initialize(argc, argv))
            return -1;
        appbase::app().startup();
        appbase::app().exec();
    } catch (const fc::exception &e) {
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