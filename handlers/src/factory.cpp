#include <Poco/Net/HTTPServerRequest.h>
#include <handlers.hpp>
#include <handlers/error.hpp>
#include <handlers/factory.hpp>
#include <iostream>

namespace handlers {

HTTPRequestHandler *Factory::GetMethodHandlers(const std::string &uri) const {
	std::cout << uri.substr(0, uri.rfind("/"));
	if (uri == "/user/meeting") {
		return new UserMeetingList();
	} else if (uri.substr(0, uri.rfind("/")) == "/user/meeting") {
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

HTTPRequestHandler *Factory::PatchMethodHandlers(const std::string &uri) const {
	if (/*uri!="/user/meeting" &&*/ uri.substr(0, 13) == "/user/meeting") {
		return new UserMeetingPatch();
	}
}
HTTPRequestHandler *Factory::DeleteMethodHandlers(const std::string &uri) const {
	if (uri.substr(0, 13) == "/user/meeting") {
		return new UserMeetingDelete();
	}
}

//выполняется когда что-то пришло
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
