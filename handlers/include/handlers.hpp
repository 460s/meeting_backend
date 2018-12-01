#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>

// #define REGISTER_HANDLER(name) \
//     class name: public Poco::Net::HTTPRequestHandler { \
//     private: \
//     void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override; \
// }

class RestHandler : public Poco::Net::HTTPRequestHandler {
	void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override {
		response.setContentType("application/json");
		HandleRestRequest(request, response);
	}

	virtual void HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) = 0;
};

#define REGISTER_HANDLER(name) \
    class name : public RestHandler { \
    void HandleRestRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override; \
}

#define REGISTER_HANDLER_WITH_ID(name) \
    class name : public RestHandler { \
    public: \
      name(int id) : m_id(id) {} \
      void HandleRestRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override; \
    private: \
      int m_id; \
}

namespace handlers {

REGISTER_HANDLER(UserMeetingList);

REGISTER_HANDLER(UserMeetingCreate);

REGISTER_HANDLER_WITH_ID(UserMeetingRead);

REGISTER_HANDLER_WITH_ID(UserMeetingUpdate);

REGISTER_HANDLER_WITH_ID(UserMeetingDelete);

}
