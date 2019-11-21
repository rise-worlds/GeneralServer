#pragma once
#include <string>
#include <string_view>
#include <array>
#include <boost/algorithm/string.hpp>
#include <boost/assert.hpp>
#include <boost/hana/tuple.hpp>

namespace hana = boost::hana;
using uint128_t = __uint128_t;

namespace example
{
//typedef std::array<uint128_t, 2UL> key256_t;
//using name_t = key256_t;
typedef uint128_t name_t[2];
//struct name_t {
//    uint128_t value[2];
//    uint128_t& operator[](int index)
//    {
//        assert(index < 2 && index >= 0);
//        return value[index];
//    }
//    uint128_t operator [](int index) const {
//        assert(index < 2 && index >= 0);
//        return value[index];
//    }
//    friend bool operator==(const name_t &a, const name_t &b) { return a.value[0] == b.value[0] && a.value[1] == b.value[1]; }
//    friend bool operator!=(const name_t &a, const name_t &b) { return a.value[0] != b.value[0] || a.value[1] != b.value[1]; }
//    constexpr name_t& operator=(const name_t& v) {
//        if (&v == this) return *this;
//        value[0] = v.value[0];
//        value[1] = v.value[1];
//        return *this;
//    }
//};

static constexpr uint128_t char_to_symbol(char c)
{
    if( c >= 'a' && c <= 'z' )
        return (c - 'a') + 11;
    if( c >= '0' && c <= '9' )
        return (c - '0') + 1;
    return 0;
}

class name
{
public:
//    typedef example::name_t name_t;
    name_t value = {0};

    constexpr name() {}
    constexpr name(uint128_t l, uint128_t r) { value[0] = l; value[1] = r; }
    constexpr name(std::string& str) { set(str); }
    constexpr name(std::string_view str) { set(str); }
//    constexpr name(name_t& v) { value = v; }
//    constexpr name(const name_t& v): value(v) {}

    constexpr void set(const std::string_view str);
    std::string to_string() const { return std::string(*this); }
    constexpr uint32_t length()const ;

    operator const std::string() const;
//    operator const name_t() const { return value; }
    constexpr name& operator=(const name& v) {
        if (&v == this) return *this;
        value[0] = v.value[0];
        value[1] = v.value[1];
        return *this;
    }

//    const name_t operator&() const { return value; }
//    const name operator&() const { return this; }

    friend bool operator==(const name &a, const name &b) { return a.value == b.value; }
    friend bool operator!=(const name &a, const name &b) { return a.value != b.value; }

    friend bool operator==(const name &a, name_t b) { return a.value == b; }
    friend bool operator!=(const name &a, name_t b) { return a.value != b; }
};

//constexpr name::name(std::string_view str)
constexpr void name::set(const std::string_view str)
{
    const auto len = str.size(); //strnlen(str, 44);
    if( str.size() > 43 ) {
        BOOST_ASSERT_MSG( false, "string is too long to be a valid name" );
    }
    if( str.empty() ) {
        return;
    }
    int i = 0;
    for (; i < 21 && i < len && str[i]; ++i)
    {
        value[0] |= (char_to_symbol(str[i]) & 0x3F) << (128 - 6 * (i + 1));
    }
    if (i == 21 && i < len && str[i])
    {
        uint128_t symbol = char_to_symbol(str[21]);
        value[0] |= (symbol & 0x30) >> 4;
        value[1] |= (symbol & 0x0F) << 124;

        for (i = 22; i < 42 && i < len && str[i]; ++i)
        {
            value[1] |= (char_to_symbol(str[i]) & 0x3F) << (124 - 6 * (i - 22 + 1));
        }
        if (i == 42 && i < len && str[i])
        {
            value[1] |= char_to_symbol(str[42]) & 0x0F;
        }
    }
}

name::operator const std::string() const
{
    constexpr uint128_t mask = uint128_t(0xFC00000000000000ull) << 64;
    static const char *charmap = ".0123456789abcdefghijklmnopqrstuvwxyz...........................";

    std::string str(43, '\0');

    uint128_t tmp = value[0];
    uint8_t i = 0;
    for (; i < 21; ++i, tmp <<= 6)
    {
        if( tmp == 0 ) break;
        auto index = (tmp & mask) >> 122;
        str[i] = charmap[index & 0x3Full];
    }
    if (i == 21 && (tmp != 0 || value[1] != 0)) {
//        tmp = ((tmp & mask) >> 122) & 0x30;
        tmp = (value[0] << 4) & 0x30;
        tmp |= ((value[1] & mask) >> 124);
        str[i] = charmap[tmp & 0x3Full];
        tmp = value[1] << 4;
        i = 22;
        for (; i < 42; ++i, tmp <<= 6) {
            if( tmp == 0 ) break;
            auto index = (tmp & mask) >> 122;
            str[i] = charmap[index & 0x3Full];
        }
        if (i == 42 && tmp != 0)
        {
            auto index = (tmp & mask) >> 124;
            str[i] = charmap[index & 0x3Full];
        }
    }
    
    return str.substr(0, i);
}

constexpr uint32_t name::length()const {
    constexpr uint128_t mask = uint128_t(0xF800000000000000ull) << 64;

    uint128_t tmp = value[0];
    uint8_t i = 0;
    uint8_t l = 0;
    for (; i < 21; ++i, tmp <<= 6)
    {
        if ( (tmp & mask) > 0 ) l = i;
    }
    if (tmp != 0 || value[1] != 0) {
        tmp = ((tmp & mask) >> 122) & 0x30;
        tmp |= (value[1] & mask) >> 124;
        if ( (tmp & mask) > 0 ) l = i;
        tmp = value[1] << 4;
        i = 22;
        for (; i < 42; ++i, tmp <<= 6) {
            if ( (tmp & mask) > 0 ) l = i;
        }
        if (i == 42 && tmp != 0)
        {
            if ( (tmp & mask) > 0 ) l = i;
        }
    }
    return l + 1;
}

namespace detail {
    template <char... Str>
    struct to_const_char_arr {
        static constexpr const char value[] = {Str...};
    };
}

} // namespace example

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
template <typename T, T... Str>
inline constexpr example::name operator""_n() {
   auto x = example::name{std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)}};
   return x;
}
//template <typename T, T... Str>
//inline constexpr example::name_t operator""_t() {
//    auto x = example::name{std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)}};
//    return x.value;
//}
template <typename T, T... Str>
inline constexpr uint128_t operator""_l() {
    auto x = example::name{std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)}};
    return x.value[0];
}
template <typename T, T... Str>
inline constexpr uint128_t operator""_r() {
    auto x = example::name{std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)}};
    return x.value[1];
}
template <typename T, T... Str>
inline constexpr hana::tuple<uint128_t, uint128_t> operator""_tuple() {
    auto x = example::name{std::string_view{example::detail::to_const_char_arr<Str...>::value, sizeof...(Str)}};
    return hana::make_tuple(x.value[0], x.value[1]);
}
#pragma clang diagnostic pop
