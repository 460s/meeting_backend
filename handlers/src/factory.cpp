#include <Poco/Net/HTTPServerRequest.h>
#include <handlers.hpp>
#include <handlers/error.hpp>
#include <handlers/factory.hpp>
<<<<<<< HEAD
#include <nlohmann/json.hpp>
=======
>>>>>>> upstream/develop
#include <regex>

namespace handlers {

HTTPRequestHandler *Factory::GetMethodHandlers(const std::string &uri) const {
	if (uri == "/user/meeting") {
		return new UserMeetingList();
<<<<<<< HEAD
	} else if (std::regex_match(uri, std::regex("/user/meeting/\\d+"))) {
		return new UserMeeting();
=======
	} else if (std::smatch m; std::regex_match(uri, m, std::regex{R"(/user/meeting/(\d+))"})) {
		return new UserMeetingRead(std::stoi(m[1]));
>>>>>>> upstream/develop
	}
	return nullptr;
}

HTTPRequestHandler *Factory::PostMethodHandlers(const std::string &uri) const {
	if (uri == "/user/meeting") {
		return new UserMeetingCreate();
	}
	return nullptr;
}

HTTPRequestHandler *Factory::PatchMethodHandlers(const std::string &uri) const {
<<<<<<< HEAD
	if (std::regex_match(uri, std::regex("/user/meeting/\\d+"))) {
		return new UserMeetingChange();
=======
	if (std::smatch m; std::regex_match(uri, m, std::regex{R"(/user/meeting/(\d+))"})) {
		return new UserMeetingUpdate(std::stoi(m[1]));
>>>>>>> upstream/develop
	}
	return nullptr;
}

HTTPRequestHandler *Factory::DeleteMethodHandlers(const std::string &uri) const {
<<<<<<< HEAD
	if (std::regex_match(uri, std::regex("/user/meeting/\\d+"))) {
		return new UserMeetingDelete();
=======
	if (std::smatch m; std::regex_match(uri, m, std::regex{R"(/user/meeting/(\d+))"})) {
		return new UserMeetingDelete(std::stoi(m[1]));
>>>>>>> upstream/develop
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
