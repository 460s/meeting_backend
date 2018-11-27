#include <iostream>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/NumberParser.h>
#include <Poco/RegularExpression.h>
#include <handlers.hpp>
#include <handlers/error.hpp>
#include <handlers/factory.hpp>

namespace handlers {

Poco::RegularExpression regexp_user_meeting_id("/user/meeting/(\\d+)",
                                               Poco::RegularExpression::RE_ANCHORED);

HTTPRequestHandler *Factory::GetMethodHandlers(const std::string &uri) const {
	if (uri == "/user/meeting") {
		return new UserMeetingList();
	}
	if (regexp_user_meeting_id.match(uri)) {
		return new UserMeetingGet();
	}
	return nullptr;
}

HTTPRequestHandler *Factory::PostMethodHandlers(const std::string &uri) const {
	if (uri == "/user/meeting") {
		return new UserMeetingCreate();
	}
	return nullptr;
}

HTTPRequestHandler *Factory::DeleteMethodHandlers(const std::string &uri) const {
	if (regexp_user_meeting_id.match(uri)) {
		return new UserMeetingDelete();
	}
	return nullptr;
}

HTTPRequestHandler *Factory::PatchMethodHandlers(const std::string &uri) const {
	if (regexp_user_meeting_id.match(uri)) {
		return new UserMeetingPatch();
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
	} else if (method == HTTPRequest::HTTP_DELETE) {
		result = DeleteMethodHandlers(uri);
	} else if (method == HTTPRequest::HTTP_PATCH) {
		result = PatchMethodHandlers(uri);
	}
	if (result == nullptr) {
		return new Error(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "Wrong endpoint " + uri);
	}
	return result;
}

} // namespace handlers
