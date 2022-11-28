#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/attributes.hpp>

// --------------------------------------------------------------------------
TEST_CASE("Empty value")
{
    beauty::attributes attributes;

    CHECK_EQ(attributes["empty"], "");
}

// --------------------------------------------------------------------------
TEST_CASE("Insert value")
{
    beauty::attributes attributes;
    attributes.insert("key", "value");

    CHECK_EQ(attributes["key"], "value");
}

// --------------------------------------------------------------------------
TEST_CASE("Target split constructor")
{
    beauty::attributes attributes("key=value");

    CHECK_EQ(attributes["key"], "value");
}

// --------------------------------------------------------------------------
TEST_CASE("Target multiple split constructor")
{
    beauty::attributes attributes("key1=value1&key2=value2");

    CHECK_EQ(attributes["key1"], "value1");
    CHECK_EQ(attributes["key2"], "value2");
}

// --------------------------------------------------------------------------
TEST_CASE("As integer/double/string")
{
    beauty::attributes attributes;
    attributes.insert("key1", "1");
    attributes.insert("key2", "123");
    attributes.insert("key3", "0");

    CHECK_EQ(attributes["key1"].as_integer(), 1);
    CHECK_EQ(attributes["key2"].as_integer(), 123);
    CHECK_EQ(attributes["key3"].as_integer(), 0);
    CHECK_EQ(attributes["key4"].as_integer(789), 789);

    attributes.insert("key11", "1.5");
    attributes.insert("key21", "123.8");
    attributes.insert("key31", "0.4");

    CHECK_EQ(attributes["key11"].as_double(), 1.5);
    CHECK_EQ(attributes["key21"].as_double(), 123.8);
    CHECK_EQ(attributes["key31"].as_double(), 0.4);

    CHECK_EQ(attributes["key11"].as_string(), "1.5");
    CHECK_EQ(attributes["key21"].as_string(), "123.8");
    CHECK_EQ(attributes["key31"].as_string(), "0.4");
}

// --------------------------------------------------------------------------
TEST_CASE("As boolean")
{
    beauty::attributes attributes;
    attributes.insert("key1", "1");
    attributes.insert("key2", "true");
    attributes.insert("key3", "yes");

    attributes.insert("key4", "0");
    attributes.insert("key5", "false");
    attributes.insert("key6", "no");

    attributes.insert("key7", "unknown");

    CHECK(attributes["key1"].as_boolean());
    CHECK(attributes["key2"].as_boolean());
    CHECK(attributes["key3"].as_boolean());

    CHECK_FALSE(attributes["key4"].as_boolean());
    CHECK_FALSE(attributes["key5"].as_boolean());
    CHECK_FALSE(attributes["key6"].as_boolean());

    CHECK_FALSE(attributes["key7"].as_boolean());

    CHECK_FALSE(attributes["not_set"].as_boolean());
    CHECK(attributes["still_not_set"].as_boolean(true));
}
