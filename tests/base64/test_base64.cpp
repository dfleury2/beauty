#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/base64.hpp>

// --------------------------------------------------------------------------
TEST_CASE("Encode Decode")
{
    std::string source = "Hello world";
    std::string encoded = beauty::base64::encode(source);
    CHECK_EQ(encoded, "SGVsbG8gd29ybGQ=");

    std::string decoded = beauty::base64::decode(encoded);

    CHECK_EQ(decoded, source);
}

TEST_CASE("Encode suite")
{
    CHECK_EQ(beauty::base64::encode("") ,"");
    CHECK_EQ(beauty::base64::encode("f") ,"Zg==");
    CHECK_EQ(beauty::base64::encode("fo") ,"Zm8=");
    CHECK_EQ(beauty::base64::encode("foo") ,"Zm9v");
    CHECK_EQ(beauty::base64::encode("foob") ,"Zm9vYg==");
    CHECK_EQ(beauty::base64::encode("fooba") ,"Zm9vYmE=");
    CHECK_EQ(beauty::base64::encode("foobar") ,"Zm9vYmFy");
}
