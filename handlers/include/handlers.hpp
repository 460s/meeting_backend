#pragma once

#include <Poco/Net/HTTPRequestHandler.h>

#define REGISTER_HANDLER(name) \
    class name: public Poco::Net::HTTPRequestHandler { \
    private: \
    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override; \
}

namespace handlers {

REGISTER_HANDLER(UserMeetingList);

REGISTER_HANDLER(UserMeetingCreate);

REGISTER_HANDLER(UserMeetingGet);

REGISTER_HANDLER(UserMeetingUpdate);

REGISTER_HANDLER(UserMeetingDelete);


}
