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

	CHECK(json::diff(json(source), target).empty());
}

TEST_CASE("json_unmarshal") {
	char source[] = R"(
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
	)";
	auto parsed_json = nlohmann::json::parse(source);
	handlers::Meeting target = parsed_json;
	// std::cout << std::setw(2) << parsed_json << "\n\n";	

	handlers::Meeting expected;
	expected.name = "test";
	expected.address = "addr";
	expected.description = "desc";
	expected.signup_description = "sdesc";
	expected.signup_from_date = 1;
	expected.signup_to_date = 2;
	expected.from_date = 3;
	expected.to_date = 4;
	expected.published = true;

	CHECK(
		(target.name == expected.name &&
		target.address == expected.address &&
		target.description == expected.description &&
		target.signup_description == expected.signup_description &&
		target.signup_from_date == expected.signup_from_date &&
		target.signup_to_date == expected.signup_to_date &&
		target.from_date == expected.from_date &&
		target.to_date == expected.to_date &&
		target.published == expected.published)
	);
}