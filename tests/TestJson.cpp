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
    Json object{R"(
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

    EXPECT_EQ(std::any_cast<std::string>(object["lastname"]), "Ivanov");
    EXPECT_EQ(std::any_cast<bool>(object["islegal"]), false);
    EXPECT_EQ(std::any_cast<double>(object["age"]), 25);

    auto marks = std::any_cast<Json>(object["marks"]);
    EXPECT_EQ(std::any_cast<double>(marks[0]), 4);
    EXPECT_EQ(std::any_cast<double>(marks[1]), 5);

    auto address = std::any_cast<Json>(object["address"]);
    EXPECT_EQ(std::any_cast<std::string>(address["city"]), "Moscow");
    EXPECT_EQ(std::any_cast<std::string>(address["street"]), "Vozdvijenka");
}

