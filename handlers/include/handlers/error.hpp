#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPResponse.h>

namespace handlers {

class Error : public Poco::Net::HTTPRequestHandler {
public:
	Error(const Poco::Net::HTTPResponse::HTTPStatus error_code, std::string_view message = "");

private:
	void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

	const Poco::Net::HTTPResponse::HTTPStatus m_error_code;
	const std::string m_message;
};

} // namespace handlers
