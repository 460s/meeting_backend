#pragma once
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <mutex>

namespace handlers {

using Poco::Net::HTTPRequestHandler;

class Factory : public Poco::Net::HTTPRequestHandlerFactory {
private:
	HTTPRequestHandler *GetMethodHandlers(const std::string &uri) const;
	HTTPRequestHandler *PostMethodHandlers(const std::string &uri) const;
	HTTPRequestHandler *PatchMethodHandlers(const std::string &uri) const;
	HTTPRequestHandler *DeleteMethodHandlers(const std::string &uri) const;

	HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;

	static std::mutex m_mutex;
public:
	static std::mutex &GetMutex();
};

} // namespace handlers
