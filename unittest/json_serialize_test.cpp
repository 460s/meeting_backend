#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>
#include <meeting.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

TEST_CASE("json_marshal") {
domain::Meeting source;
source.id = 1;
source.name = "test";
source.address = "addr";
source.description ="desc";
source.signup_description = "sdesc";
source.signup_from_date = 1;
source.signup_to_date = 2;
source.from_date = 3;
source.to_date = 4;
source.published = true;
json target = R"(
	{
		"id": 1,
		"name": "test",
		"address": "addr",
		"description": "desc",
		"signup_description": "sdesc",
		"signup_from_date": 1,
		"signup_to_date": 2,
		"from_date": 3,
		"to_date": 4,
		"published": true
	}
 )"_json;
CHECK(json::diff(json(source), target).empty());
}

TEST_CASE("json_unmarshal") {
    domain::Meeting target;
    target.name = "test";
    target.address = "addr";
    target.description ="desc";
    target.signup_description = "sdesc";
    target.signup_from_date = 1;
    target.signup_to_date = 2;
    target.from_date = 3;
    target.to_date = 4;
    target.published = true;
    json source = R"(
	{
		"name": "test",
		"address": "addr",
		"description": "desc",
		"signup_description": "sdesc",
		"signup_from_date": 1,
		"signup_to_date": 2,
		"from_date": 3,
		"to_date": 4,
		"published": true
	}
 )"_json;
    auto result = domain::Meeting(source);



    CHECK(static_cast<bool>(
            result.name == target.name && result.address == target.address &&
            result.description == target.description &&
            result.signup_description == target.signup_description &&
            result.signup_from_date == target.signup_from_date &&
            result.signup_to_date == target.signup_to_date &&
            result.from_date == target.from_date &&
            result.to_date == target.to_date &&
            result.published == target.published));
}
