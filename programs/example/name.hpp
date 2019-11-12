#pragma once
#include <string>
#include <array>
#include <boost/algorithm/string.hpp>

namespace example
{
using uint128_t = __uint128_t;
typedef std::array<uint128_t, 2UL> key256_t;
using name_t = key256_t;

static constexpr uint128_t char_to_symbol(char c)
{
    if (c >= 'a' && c <= 'z')
        return (c - 'a') + 6;
    if (c >= '1' && c <= '5')
        return (c - '1') + 1;
    return 0;
}

// Each char of the string is encoded into 5-bit chunk and left-shifted
// to its 5-bit slot starting with the highest slot for the first char.
// The 13th char, if str is long enough, is encoded into 4-bit chunk
// and placed in the lowest 4 bits. 64 = 12 * 5 + 4
static constexpr name_t string_to_name(const char *str)
{
    name_t name = {0};
    int i = 0;
    for (; str[i] && i < 21; ++i)
    {
        name[0] |= (char_to_symbol(str[i]) & 0x3F) << (128 - 6 * (i + 1));
    }
    if (str[i] && i == 21)
    {
        uint128_t symbo = char_to_symbol(str[21]);
        name[0] |= (symbo & 0x30) >> 4;
        name[1] |= (symbo & 0x0F) << 124;

		for (i = 22; str[i] && i < 42; ++i)
		{
		    name[1] |= (char_to_symbol(str[i]) & 0x3F) << (124 - 6 * (i - 22 + 1));
		}
		if (str[i] && i == 42)
		{
		    name[1] |= char_to_symbol(str[42]) & 0x0F;
		}
    }
    return name;
}

class name
{
public:
    name_t value = {0};

    name() {}
    name(std::string str) { set(str.c_str()); }

    void set(const char *str);
    std::string to_string() const { return std::string(*this); }

    operator std::string() const;
    operator name_t() const { return value; }

    friend bool operator==(const name &a, const name &b) { return a.value == b.value; }
    friend bool operator!=(const name &a, const name &b) { return a.value != b.value; }

    friend bool operator==(const name &a, name_t b) { return a.value == b; }
    friend bool operator!=(const name &a, name_t b) { return a.value != b; }
};

void name::set(const char *str)
{
    const auto len = strnlen(str, 44);
    value = string_to_name(str);
}

name::operator std::string() const
{
    static const char *charmap = ".0123456789abcdefghijklmnopqrstuvwxyz...........................";

    std::string str(43, '.');

    uint128_t tmp = value[1];
    uint32_t i = 0;
    for (; i < 21; ++i)
    {
        char c = charmap[tmp & (i == 0 ? 0x0f : 0x3f)];
        str[42 - i] = c;
        tmp >>= (i == 0 ? 4 : 6);
    }
    {
        uint128_t index = 0;
        index |= tmp & 0x0F;
        tmp = value[0];
        index |=  (tmp & 0x03) << 4;
        str[42 - i] = charmap[index & 0x3f];
        tmp >>= 2;
    }
    for (i = 22; i < 42; ++i)
    {
        char c = charmap[tmp & 0x3f];
        str[42 - i] = c;
        tmp >>= 6;
    }
    if (i == 42)
    {
        str[42 - i] = charmap[tmp & 0x3f];
    }

    boost::algorithm::trim_right_if(str, [](char c) { return c == '.'; });
    return str;
}
} // namespace example
