#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/utils.hpp>
#include <beauty/url.hpp>

// --------------------------------------------------------------------------
TEST_CASE("Escape")
{
    std::string s("some text with % chars to escape Ã Ã©iÃ´u");

    CHECK_EQ(beauty::unescape(beauty::escape(s)), s);

    CHECK_EQ(beauty::escape("must be"), "must%20be");
}

// --------------------------------------------------------------------------
TEST_CASE("Unescape")
{
    CHECK_EQ(beauty::unescape("/tmp/srv"), "/tmp/srv");
    CHECK_EQ(beauty::unescape("%2ftmp%2fsrv"), "/tmp/srv");
    CHECK_EQ(beauty::unescape("some spaces in a%20line"), "some spaces in a line");

    CHECK_EQ(beauty::unescape("%252ftmp%252fsrv"), "%2ftmp%2fsrv");
}

// --------------------------------------------------------------------------
// Non-regression tests for escape / unescape (and the internal
// hexdigit_to_num helper, exercised through unescape).
// --------------------------------------------------------------------------
namespace {

std::string
to_hex2(unsigned char c, bool upper)
{
    const char* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    return std::string{digits[c >> 4], digits[c & 0x0F]};
}

bool
is_unreserved(unsigned char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' || c == '~';
}

}

// --------------------------------------------------------------------------
TEST_CASE("Escape - empty string")
{
    CHECK_EQ(beauty::escape(""), "");
}

// --------------------------------------------------------------------------
TEST_CASE("Escape - unreserved characters are kept as is")
{
    const std::string unreserved =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
    CHECK_EQ(beauty::escape(unreserved), unreserved);

    // Lowercase letters must not be uppercased
    CHECK_EQ(beauty::escape("hello"), "hello");
}

// --------------------------------------------------------------------------
TEST_CASE("Escape - reserved ASCII characters are percent-encoded")
{
    CHECK_EQ(beauty::escape(" "), "%20");
    CHECK_EQ(beauty::escape("%"), "%25");
    CHECK_EQ(beauty::escape("+"), "%2B");
    CHECK_EQ(beauty::escape("/"), "%2F");
    CHECK_EQ(beauty::escape("a/b c"), "a%2Fb%20c");
    CHECK_EQ(beauty::escape("!*'();:@&=+$,/?#[]"),
             "%21%2A%27%28%29%3B%3A%40%26%3D%2B%24%2C%2F%3F%23%5B%5D");
    CHECK_EQ(beauty::escape("\"<>\\^`{|}"), "%22%3C%3E%5C%5E%60%7B%7C%7D");
    CHECK_EQ(beauty::escape("key=value&other=1"), "key%3Dvalue%26other%3D1");
}

// --------------------------------------------------------------------------
TEST_CASE("Escape - control and non-ASCII bytes are percent-encoded")
{
    CHECK_EQ(beauty::escape(std::string(1, '\0')), "%00");
    CHECK_EQ(beauty::escape("\t\r\n"), "%09%0D%0A");
    CHECK_EQ(beauty::escape("\x7F"), "%7F");
    CHECK_EQ(beauty::escape("\x80"), "%80");
    CHECK_EQ(beauty::escape("\xFF"), "%FF");

    // UTF-8 multi-byte sequences are encoded byte per byte, uppercase hex
    CHECK_EQ(beauty::escape("\xC3\xA9"), "%C3%A9");         // é
    CHECK_EQ(beauty::escape("caf\xC3\xA9"), "caf%C3%A9");   // café
    CHECK_EQ(beauty::escape("\xE2\x82\xAC"), "%E2%82%AC");  // €
}

// --------------------------------------------------------------------------
TEST_CASE("Escape - every byte value")
{
    for (int i = 0; i < 256; ++i) {
        const auto c = static_cast<unsigned char>(i);
        const std::string input(1, static_cast<char>(c));
        const std::string expected = is_unreserved(c) ? input : "%" + to_hex2(c, true);
        CAPTURE(i);
        CHECK_EQ(beauty::escape(input), expected);
    }
}

// --------------------------------------------------------------------------
TEST_CASE("Unescape - empty and plain strings")
{
    CHECK_EQ(beauty::unescape(""), "");
    CHECK_EQ(beauty::unescape("abc-._~XYZ019"), "abc-._~XYZ019");
}

// --------------------------------------------------------------------------
TEST_CASE("Unescape - hex digits, upper and lower case")
{
    // Digits
    CHECK_EQ(beauty::unescape("%30%39"), "09");
    // Uppercase A-F
    CHECK_EQ(beauty::unescape("%4A%4F"), "JO");
    // Lowercase a-f
    CHECK_EQ(beauty::unescape("%4a%4f"), "JO");
    // Mixed case in the same sequence
    CHECK_EQ(beauty::unescape("%c3%A9"), "\xC3\xA9");
    CHECK_EQ(beauty::unescape("%C3%a9"), "\xC3\xA9");
    CHECK_EQ(beauty::unescape("%Ff%fF"), "\xFF\xFF");
    // NUL byte
    CHECK_EQ(beauty::unescape("a%00b"), std::string("a\0b", 3));
}

// --------------------------------------------------------------------------
TEST_CASE("Unescape - every %XX value")
{
    for (int i = 0; i < 256; ++i) {
        const auto c = static_cast<unsigned char>(i);
        const std::string expected(1, static_cast<char>(c));
        CAPTURE(i);
        CHECK_EQ(beauty::unescape("%" + to_hex2(c, true)), expected);
        CHECK_EQ(beauty::unescape("%" + to_hex2(c, false)), expected);
    }
}

// --------------------------------------------------------------------------
TEST_CASE("Unescape - plus sign")
{
    CHECK_EQ(beauty::unescape("+"), " ");
    CHECK_EQ(beauty::unescape("a+b+c"), "a b c");
    CHECK_EQ(beauty::unescape("++"), "  ");
    // An encoded '+' must stay a '+', not become a space
    CHECK_EQ(beauty::unescape("%2B"), "+");
    CHECK_EQ(beauty::unescape("1%2B1+%3D+2"), "1+1 = 2");
}

// --------------------------------------------------------------------------
TEST_CASE("Unescape - invalid or truncated sequences are kept verbatim")
{
    // Truncated
    CHECK_EQ(beauty::unescape("%"), "%");
    CHECK_EQ(beauty::unescape("%4"), "%4");
    CHECK_EQ(beauty::unescape("100%"), "100%");
    CHECK_EQ(beauty::unescape("abc%4"), "abc%4");

    // Non hex digits
    CHECK_EQ(beauty::unescape("%zz"), "%zz");
    CHECK_EQ(beauty::unescape("%G1"), "%G1");
    CHECK_EQ(beauty::unescape("%1G"), "%1G");
    CHECK_EQ(beauty::unescape("%g1"), "%g1");
    CHECK_EQ(beauty::unescape("%1g"), "%1g");

    // Characters right next to the hex ranges in the ASCII table
    CHECK_EQ(beauty::unescape("%/0"), "%/0");   // '/' < '0'
    CHECK_EQ(beauty::unescape("%:0"), "%:0");   // ':' > '9'
    CHECK_EQ(beauty::unescape("%@0"), "%@0");   // '@' < 'A'
    CHECK_EQ(beauty::unescape("%`0"), "%`0");   // '`' < 'a'
    CHECK_EQ(beauty::unescape("%0/"), "%0/");
    CHECK_EQ(beauty::unescape("%0:"), "%0:");
    CHECK_EQ(beauty::unescape("%0@"), "%0@");
    CHECK_EQ(beauty::unescape("%0`"), "%0`");

    // Non-ASCII bytes after '%'
    CHECK_EQ(beauty::unescape("%\xC3\xA9"), "%\xC3\xA9");

    // '+' after an invalid '%' is still decoded as a space
    CHECK_EQ(beauty::unescape("%+1"), "% 1");
    CHECK_EQ(beauty::unescape("%1+"), "%1 ");

    // A '%' that does not start a valid sequence does not swallow the next one
    CHECK_EQ(beauty::unescape("%%41"), "%A");
    CHECK_EQ(beauty::unescape("%4%41"), "%4A");
}

// --------------------------------------------------------------------------
TEST_CASE("Unescape - no double decoding")
{
    CHECK_EQ(beauty::unescape("%2541"), "%41");
    CHECK_EQ(beauty::unescape(beauty::unescape("%2541")), "A");
}

// --------------------------------------------------------------------------
TEST_CASE("Escape / Unescape - round trip on every byte value")
{
    std::string all;
    for (int i = 0; i < 256; ++i) {
        all += static_cast<char>(i);
    }
    CHECK_EQ(beauty::unescape(beauty::escape(all)), all);

    for (int i = 0; i < 256; ++i) {
        const std::string s{'a', static_cast<char>(i), '%', '+', static_cast<char>(i)};
        CAPTURE(i);
        CHECK_EQ(beauty::unescape(beauty::escape(s)), s);
    }
}
