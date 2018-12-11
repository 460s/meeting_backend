#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>
#include <user_meeting.hpp>
#include <iostream>

using nlohmann::json;

TEST_CASE("json_marshal") {
//json json_sourse;
handlers::Meeting source;
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


CHECK(json::diff(json(source),target).empty() == true);
}

TEST_CASE("json_unmarshal") {
	handlers::Meeting source;
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

	handlers::Meeting meeting = handlers::Meeting(target);

	CHECK((
		meeting.name == source.name &&
		meeting.address == source.address &&
		meeting.description == source.description &&
		meeting.signup_description == source.signup_description &&
		meeting.signup_from_date == source.signup_from_date &&
		meeting.signup_to_date == source.signup_to_date &&
		meeting.from_date == source.from_date &&
		meeting.to_date == source.to_date &&
		meeting.published == source.published));

}