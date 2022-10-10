#pragma once

#include <string>

namespace beauty::base64 {

template <typename InputIt>
std::string encode(InputIt first, InputIt last)
{
    static constexpr char CHAR_TABLE[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
    };
    std::string res;
    size_t len = last - first;
    if (len == 0)
    {
        return res;
    }
    size_t r = len % 3;
    res.resize((len + 2) / 3 * 4);
    auto j = last - r;
    auto p = std::begin(res);
    while (first != j)
    {
        uint32_t n = static_cast<uint8_t>(*first++) << 16;
        n += static_cast<uint8_t>(*first++) << 8;
        n += static_cast<uint8_t>(*first++);
        *p++ = CHAR_TABLE[n >> 18];
        *p++ = CHAR_TABLE[(n >> 12) & 0x3fu];
        *p++ = CHAR_TABLE[(n >> 6) & 0x3fu];
        *p++ = CHAR_TABLE[n & 0x3fu];
    }

    if (r == 2)
    {
        uint32_t n = static_cast<uint8_t>(*first++) << 16;
        n += static_cast<uint8_t>(*first++) << 8;
        *p++ = CHAR_TABLE[n >> 18];
        *p++ = CHAR_TABLE[(n >> 12) & 0x3fu];
        *p++ = CHAR_TABLE[(n >> 6) & 0x3fu];
        *p++ = '=';
    }
    else if (r == 1)
    {
        uint32_t n = static_cast<uint8_t>(*first++) << 16;
        *p++ = CHAR_TABLE[n >> 18];
        *p++ = CHAR_TABLE[(n >> 12) & 0x3fu];
        *p++ = '=';
        *p++ = '=';
    }
    return res;
}

template <typename InputIt>
InputIt
next_decode_input(InputIt first, InputIt last, const int *tbl)
{
    for (; first != last; ++first)
    {
        if (tbl[static_cast<size_t>(*first)] != -1 || *first == '=')
        {
            break;
        }
    }
    return first;
}

template <typename InputIt>
std::string
decode(InputIt first, InputIt last)
{
    static constexpr int INDEX_TABLE[] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57,
        58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6,
        7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
        25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
        37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1};
    auto len = last - first;
    if (len % 4 != 0)
    {
        return "";
    }
    std::string res;
    res.resize(len / 4 * 3);

    auto p = std::begin(res);
    for (; first != last;)
    {
        uint32_t n = 0;
        for (int i = 1; i <= 4; ++i, ++first)
        {
            auto idx = INDEX_TABLE[static_cast<size_t>(*first)];
            if (idx == -1)
            {
                if (i <= 2)
                {
                    return "";
                }
                if (i == 3)
                {
                    if (*first == '=' && *(first + 1) == '=' && first + 2 == last)
                    {
                        *p++ = n >> 16;
                        res.resize(p - std::begin(res));
                        return res;
                    }
                    return "";
                }
                if (*first == '=' && first + 1 == last)
                {
                    *p++ = n >> 16;
                    *p++ = n >> 8 & 0xffu;
                    res.resize(p - std::begin(res));
                    return res;
                }
                return "";
            }

            n += idx << (24 - i * 6);
        }

        *p++ = n >> 16;
        *p++ = n >> 8 & 0xffu;
        *p++ = n & 0xffu;
    }

    return res;
}

// --------------------------------------------------------------------------
inline std::string
encode(const std::string& str) { return encode(str.begin(), str.end()); }

inline std::string
decode(const std::string& str) { return decode(str.begin(), str.end()); }

}   // namespace beauty::base64
