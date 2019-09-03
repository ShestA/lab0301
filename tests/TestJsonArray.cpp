#include <gtest/gtest.h>

#include "Json.h"

TEST(JsonArray, EmptyArray)
{
    Json json{"[]"};
    EXPECT_EQ(json.getSize(), 0);
    EXPECT_EQ(json.is_object(), false);
    EXPECT_EQ(json.is_array(), true);
    EXPECT_EQ(json.is_null(), false);
}

TEST(JsonArray, SimpleArray)
{
    Json json{"[ 1 ]"};
    EXPECT_EQ(json.getSize(), 1);
    EXPECT_EQ(json.is_object(), false);
    EXPECT_EQ(json.is_array(), true);
    EXPECT_EQ(json.is_null(), false);

    EXPECT_EQ(std::any_cast<double>(json[0]), 1.);
}

TEST(JsonArray, MultitypeArray)
{
    Json json{R"([1.1, "I am a string", "another string", false])"};
    EXPECT_EQ(json.getSize(), 4);
    EXPECT_EQ(json.is_object(), false);
    EXPECT_EQ(json.is_array(), true);
    EXPECT_EQ(json.is_null(), false);

    EXPECT_EQ(std::any_cast<double>(json[0]), 1.1);
    EXPECT_EQ(std::any_cast<std::string>(json[1]), "I am a string");
    EXPECT_EQ(std::any_cast<std::string>(json[2]), "another string");
    EXPECT_EQ(std::any_cast<bool>(json[3]), false);
}

TEST(JsonArray, NestedArray)
{
    Json json{"[ [ 1 ] ]"};
    EXPECT_EQ(json.getSize(), 1);
    EXPECT_EQ(json.is_object(), false);
    EXPECT_EQ(json.is_array(), true);
    EXPECT_EQ(json.is_null(), false);

    Json &nested = *std::any_cast<Json *>(json[0]);
    EXPECT_EQ(nested.getSize(), 1);
    EXPECT_EQ(nested.is_object(), false);
    EXPECT_EQ(nested.is_array(), true);
    EXPECT_EQ(nested.is_null(), false);

    EXPECT_EQ(std::any_cast<double>(nested[0]), 1.);
}

TEST(JsonArray, NestedWithObject)
{
    Json json{R"([ {"k":false}])"};
    EXPECT_EQ(json.getSize(), 1);
    EXPECT_EQ(json.is_object(), false);
    EXPECT_EQ(json.is_array(), true);
    EXPECT_EQ(json.is_null(), false);

    Json &nested = *std::any_cast<Json *>(json[0]);
    EXPECT_EQ(nested.getSize(), 1);
    EXPECT_EQ(nested.is_object(), true);
    EXPECT_EQ(nested.is_array(), false);
    EXPECT_EQ(nested.is_null(), false);

    EXPECT_EQ(std::any_cast<bool>(nested["k"]), false);
}

TEST(JsonArray, NumberWithoutSpaceAfter)
{
    Json json{"[1]"};

    EXPECT_EQ(json.getSize(), 1);
    EXPECT_EQ(json.is_object(), false);
    EXPECT_EQ(json.is_array(), true);
    EXPECT_EQ(json.is_null(), false);

    EXPECT_EQ(std::any_cast<double>(json[0]), 1.);
}

TEST(JsonArray, WithObjects)
{
    Json json{R"([{"a": "b"}, {"b": true}, {"test": 123}])"};

    EXPECT_EQ(json.getSize(), 3);
    EXPECT_EQ(json.is_object(), false);
    EXPECT_EQ(json.is_array(), true);
    EXPECT_EQ(json.is_null(), false);

    Json &nested = *std::any_cast<Json *>(json[0]);
    EXPECT_EQ(nested.getSize(), 1);
    EXPECT_EQ(nested.is_object(), true);
    EXPECT_EQ(nested.is_array(), false);
    EXPECT_EQ(nested.is_null(), false);

    EXPECT_EQ(std::any_cast<std::string>(nested["a"]), "b");

    nested = *std::any_cast<Json *>(json[1]);
    EXPECT_EQ(nested.getSize(), 1);
    EXPECT_EQ(nested.is_object(), true);
    EXPECT_EQ(nested.is_array(), false);
    EXPECT_EQ(nested.is_null(), false);

    EXPECT_EQ(std::any_cast<bool>(nested["b"]), true);

    nested = *std::any_cast<Json *>(json[2]);
    EXPECT_EQ(nested.getSize(), 1);
    EXPECT_EQ(nested.is_object(), true);
    EXPECT_EQ(nested.is_array(), false);
    EXPECT_EQ(nested.is_null(), false);

    EXPECT_EQ(std::any_cast<double>(nested["test"]), 123);
}