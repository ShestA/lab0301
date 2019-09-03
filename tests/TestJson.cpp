// Copyright 2018 Your Name <your_email>

#include <gtest/gtest.h>

#include "Json.h"

TEST(Json, NullJson)
{
    Json json{};
    EXPECT_EQ(json.getSize(), 0);
    EXPECT_EQ(json.is_object(), false);
    EXPECT_EQ(json.is_array(), false);
    EXPECT_EQ(json.is_null(), true);
}

TEST(Json, ExampleJson)
{
    Json json{R"(
        {
            "lastname" : "Ivanov",
            "firstname" : "Ivan",
            "age" : 25,
            "islegal" : false,
            "marks" : [
                4,5,5,5,2,3
            ],
            "address" : {
                "city" : "Moscow",
                "street" : "Vozdvijenka"
            }
        }
    )"};
}

