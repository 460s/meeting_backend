#include <Poco/Net/HTTPServerResponse.h>
#include <handlers/error.hpp>

namespace handlers {

Error::Error(const Poco::Net::HTTPResponse::HTTPStatus error_code, std::string_view message)
    : m_error_code(error_code), m_message(message) {}

void Error::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatusAndReason(m_error_code, Poco::Net::HTTPResponse::HTTP_REASON_BAD_REQUEST);
	response.send() << m_message;
}

} // namespace handlers
