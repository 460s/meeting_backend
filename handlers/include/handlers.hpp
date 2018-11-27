#pragma once

#include <Poco/Net/HTTPRequestHandler.h>

#define REGISTER_HANDLER(name)                                                                                       \
	class name : public Poco::Net::HTTPRequestHandler {                                                              \
	private:                                                                                                         \
		void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override; \
	}

namespace handlers {

REGISTER_HANDLER(UserMeeting);

REGISTER_HANDLER(UserMeetingList);

REGISTER_HANDLER(UserMeetingCreate);

REGISTER_HANDLER(UserMeetingChange);

REGISTER_HANDLER(UserMeetingDelete);

} // namespace handlers
