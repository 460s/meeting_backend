#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>
#include <iostream>
#include <user_meeting.hpp>

using nlohmann::json;

TEST_CASE("json_unmarshal") {
	//json json_sourse;
	handlers::Meeting source;
	source.id = 1;
	source.name = "test";
	source.address = "addr";
	source.description = "desc";
	source.signup_description = "sdesc";
	source.signup_from_date = 1;
	source.signup_to_date = 2;
	source.from_date = 3;
	source.to_date = 4;
	source.published = true;

	json js = R"(
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
	handlers::Meeting target = js;
	//std::cout << std::to_string(target.id.emplace());// << " ; " << std::to_string(source.id) << std::endl;
	CHECK(target.id.emplace() == source.id.emplace());
	CHECK(target.name == source.name);
	CHECK(target.address == source.address);
	CHECK(target.description == source.description);
	CHECK(target.signup_description == source.signup_description);
	CHECK(target.signup_from_date == source.signup_from_date);
	CHECK(target.signup_to_date == source.signup_to_date);
	CHECK(target.from_date == source.from_date);
	CHECK(target.to_date == source.to_date);
	CHECK(target.published == source.published);
}