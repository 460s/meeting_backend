#include <Poco/Net/HTTPServerRequest.h>
#include <handlers.hpp>
#include <handlers/error.hpp>
#include <handlers/factory.hpp>
#include <Poco/URI.h>
#include <Poco/NumberParser.h>

namespace handlers {

HTTPRequestHandler *Factory::GetMethodHandlers(const std::string &uri) const {
	
	Poco::URI uri_parser(uri);
	std::vector < std::string > segments;
	uri_parser.getPathSegments(segments);
	int meeting_id;
	//  /user/*  route
	if (segments[0].compare("user") == 0) {
		if (segments[1].compare("meeting") == 0) { // /user/meeting route
			if (segments[2].length() == 0) { 
				return new UserMeetingList();
			} else if (Poco::NumberParser::tryParse(segments[2],meeting_id)) { // проверям, прислали ли нам id
				return new UserMeetingGet(meeting_id);
			}
		} 
	}
	return nullptr;
}

HTTPRequestHandler *Factory::PostMethodHandlers(const std::string &uri) const {
	
	Poco::URI uri_parser(uri);
	std::vector < std::string > segments;
	uri_parser.getPathSegments(segments);

	//  /user/*  route
	if (segments[0].compare("user") == 0) {
		if (segments[1].compare("meeting") == 0) { // /user/meeting route
			return new UserMeetingCreate();
		} 
	}
	return nullptr;
}

HTTPRequestHandler *Factory::PatchMethodHandlers(const std::string &uri) const {
	
	Poco::URI uri_parser(uri);
	std::vector < std::string > segments;
	uri_parser.getPathSegments(segments);
	int meeting_id;
	//  /user/*  route
	if (segments[0].compare("user") == 0) {
		if (segments[1].compare("meeting") == 0) { // /user/meeting route
			if (segments[2].length() != 0 && Poco::NumberParser::tryParse(segments[2],meeting_id)) { // проверям, прислали ли нам id
				return new UserMeetingUpdate(meeting_id);
			}
		} 
	}

	return nullptr;
}

HTTPRequestHandler *Factory::DeleteMethodHandlers(const std::string &uri) const {
	
	Poco::URI uri_parser(uri);
	std::vector < std::string > segments;
	uri_parser.getPathSegments(segments);
	int meeting_id;
	//  /user/*  route
	if (segments[0].compare("user") == 0) {
		if (segments[1].compare("meeting") == 0) { // /user/meeting route
			if (segments[2].length() != 0 && Poco::NumberParser::tryParse(segments[2],meeting_id)) { // проверям, прислали ли нам id
				return new UserMeetingDelete(meeting_id);
			}
		}
	}

	return nullptr;
}

Poco::Net::HTTPRequestHandler *Factory::createRequestHandler(const Poco::Net::HTTPServerRequest &request) {
	using Poco::Net::HTTPRequest;
	
	Poco::Net::HTTPRequestHandler *result = nullptr;
	const auto method = request.getMethod();
	const auto uri = request.getURI();
	if (method == HTTPRequest::HTTP_GET) {
		result = GetMethodHandlers(uri);
	} else if (method == HTTPRequest::HTTP_POST) {
		result = PostMethodHandlers(uri);
	} else if (method == HTTPRequest::HTTP_PATCH) {
		result = PatchMethodHandlers(uri);
	} else if (method == HTTPRequest::HTTP_DELETE) {
		result = DeleteMethodHandlers(uri);
	}

	if (result == nullptr) {
		return new Error(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "Wrong endpoint " + uri);
	}
	return result;
}

} // namespace handlers
