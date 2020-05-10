#pragma once

#include <string>
#include <string_view>
#include <array>
#include <boost/algorithm/string.hpp>
#include <boost/assert.hpp>
#include <boost/multiprecision/cpp_int.hpp>

using uint128_t = __uint128_t;

namespace example {

    using namespace boost::multiprecision::literals;
    // using namespace boost::multiprecision;
    using boost::multiprecision::uint256_t;

    static const char decode_map[51] = ".-0123456789abcdefghijklmnopqrstuvwxyz_:<>[]{}()`~";
    static constexpr uint64_t char_to_symbol(char c) {
        if( c == '~' )
            return (c - '~') + 49;
        if( c == '`' )
            return (c - '`') + 48;
        if( c == ')' )
            return (c - ')') + 47;
        if( c == '(' )
            return (c - '(') + 46;
        if( c == '}' )
            return (c - '}') + 45;
        if( c == '{' )
            return (c - '{') + 44;
        if( c == ']' )
            return (c - ']') + 43;
        if( c == '[' )
            return (c - '[') + 42;
        if( c == '>' )
            return (c - '>') + 41;
        if( c == '<' )
            return (c - '<') + 40;
        if( c == ':' )
            return (c - ':') + 39;
        if( c == '_' )
            return (c - '_') + 38;
        if( c >= 'A' && c <= 'Z' )
            return (c - 'A') + 12;
        if( c >= 'a' && c <= 'z' )
            return (c - 'a') + 12;
        if( c >= '0' && c <= '9' )
            return (c - '0') + 2;
        if( c == '-' )
            return (c - '-') + 1;
        return 0;
    }

    class name {
    public:
//    typedef example::name_t name_t;
        union 
        {
            /* data */
            std::array<uint8_t, 32> bytes;
            std::array<uint32_t, 8> dwords;
            std::array<uint64_t, 4> qwords;
            std::array<__uint128_t, 2> qtwords;
            uint256_t value = {0x00_cppui256};
        };
        

        constexpr name() {}

        constexpr name(::uint128_t l, ::uint128_t r) {
            // ((uint128_t*)&value)[0] = l;
            // ((uint128_t*)&value)[1] = r;
            value = (uint256_t(l) << 128) | r;
        }

        constexpr name(std::string &str) { set(str); }
        constexpr name(std::string_view str) { set(str); }

        constexpr name(uint256_t& v) { value = v; }
        constexpr name(const uint256_t& v): value(v) {}

        constexpr name(name& v) { value = v.value; }
        constexpr name(const name& v): value(v.value) {}
        // constexpr name(name_t v) {
        //     value[0] = v[0];
        //     value[1] = v[1];
        // }
        // constexpr name(const name_t v) {
        //     value[0] = v[0];
        //     value[1] = v[1];
        // }

        void set(const std::string_view str);

        std::string to_string() const { return std::string(*this); }

        uint32_t length() const;

        name suffix();

        operator const std::string() const;

//    operator const name_t() const { return value; }
        name &operator=(const name &v) {
            if (&v == this) return *this;
            // value[0] = v.value[0];
            // value[1] = v.value[1];
            value = v.value;
            return *this;
        }

//    const name_t operator&() const { return value; }
//    const name operator&() const { return this; }

        friend bool operator==(const name &a, const name &b) { return a.qwords == b.qwords; }
        friend bool operator!=(const name &a, const name &b) { return a.qwords != b.qwords; }

        // friend bool operator==(const name &a, name_t b) { return a.value == b; }
        // friend bool operator!=(const name &a, name_t b) { return a.value != b; }
    };

    void name::set(const std::string_view str) {
        const auto len = str.size(); //strnlen(str, 44);
        if (len == 0) {
            return;
        }
        value = 0x00_cppui256;
        int i = 0;
        for ( ; str[i] && i < 42; ++i) {
            uint256_t c = (char_to_symbol(str[i]) & 0x3f);
            c <<= (4 + (6 * i));
            value |= c;
        }
    }

    name::operator const std::string() const {
        std::string str(44, '\0');

        uint256_t tmp = value;
        uint8_t i = 0, l = 0;
        tmp >>= 4;
        for( ; i < 42; ++i ) {
            char c = decode_map[static_cast<uint8_t >(tmp & 0x3f)];
            if (c != '.') l = i;
            str[i] = c;
            tmp >>= 6;
        }

        // boost::algorithm::trim_right_if( str, []( char c ){ return c == '.'; } );
        return str.substr(0, l + 1);
    }

    uint32_t name::length() const {
        uint256_t tmp = value;
        tmp >>= 4;
        uint8_t i = 0;
        uint8_t l = 0;
        for (; i < 42; ++i, tmp >>= 6) {
            if ((tmp & 0x3f) > 0) l = i;
        }

        return l + 1;
    }

    name name::suffix() {
        uint32_t remaining_bits_after_last_actual_dot = 0;
        uint32_t tmp = 0;
        for( int32_t remaining_bits = 4; remaining_bits < 252; remaining_bits += 6 ) { // Note: remaining_bits must remain signed integer
            // Get characters one-by-one in name in order from left to right
            auto c = (value >> remaining_bits) & 0x3Full;
            if( !c ) { // if this character is a dot
                tmp = static_cast<uint32_t>(remaining_bits);
            } else { // if this character is not a dot
                remaining_bits_after_last_actual_dot = tmp;
            }
        }

        // uint256_t thirteenth_character = static_cast<uint8_t>(value & 0x0Full);
        // if (thirteenth_character) { // if 13th character is not a dot
        //     remaining_bits_after_last_actual_dot = 41;
        // }

        if (remaining_bits_after_last_actual_dot ==
            0) // there is no actual dot in the %name other than potentially leading dots
            return name{value};

        // At this point remaining_bits_after_last_actual_dot has to be within the range of 4 to 59 (and restricted to increments of 5).

//        // Mask for remaining bits corresponding to characters after last actual dot, except for 4 least significant bits (corresponds to 13th character).
//        uint64_t mask = (1ull << remaining_bits_after_last_actual_dot) - 16;
//        uint32_t shift = 64 - remaining_bits_after_last_actual_dot;
//
//        return name{((value & mask) << shift) + (thirteenth_character << (shift - 1))};
        remaining_bits_after_last_actual_dot = 256 - remaining_bits_after_last_actual_dot * 6;
        uint256_t mask = (uint256_t(0x01ull) << remaining_bits_after_last_actual_dot) - 0x10ull;
        uint32_t shift = 256 - remaining_bits_after_last_actual_dot;
        uint256_t suffix = (value & mask) << (shift);
        // suffix += (thirteenth_character << (shift - 2));
        
        return name{suffix};
    }

    namespace detail {
        template<char... Str>
        struct to_const_char_arr {
            static constexpr const char value[] = {Str...};
        };
    }

} // namespace example

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"

template<typename T, T... Str>
inline constexpr example::name operator ""_n() {
    auto x = example::name{std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)}};
    return x;
}

//template <typename T, T... Str>
//inline constexpr example::name_t operator""_n() {
//    auto x = example::name{std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)}};
//    return x.value;
//}

template <typename T, T... Str>
inline constexpr std::string_view operator""_str() {
   return std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)};
}

template<typename T, T... Str>
inline constexpr uint128_t operator ""_l() {
    auto x = example::name{std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)}};
    return x.qtwords[0];
}

template<typename T, T... Str>
inline constexpr uint128_t operator ""_r() {
    auto x = example::name{std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)}};
    return x.qtwords[1];
}

#define T(table) table##_l, table##_r

// template<typename T, T... Str>
// inline constexpr hana::tuple<uint128_t, uint128_t> operator ""_tuple() {
//     auto x = example::name{std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)}};
//     return hana::make_tuple(x.value[0], x.value[1]);
// }

#pragma clang diagnostic pop
