#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include <nlohmann/json.hpp>

#include <sqLite.hpp>
#include <meeting.hpp>
#include <handlers.hpp>

namespace handlers {

void to_json(nlohmann::json &j, const Meeting &m) {
	j = nlohmann::json{
	    {"id", m.id.value()},
	    {"name", m.name},
	    {"description", m.description},
	    {"address", m.address},
	    {"published", m.published}};
}

void from_json(const nlohmann::json &j, Meeting &m) {
	j.at("name").get_to(m.name);
	j.at("description").get_to(m.description);
	j.at("address").get_to(m.address);
	j.at("published").get_to(m.published);
}

void UserMeetingList::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	SqLite *data_base = SqLite::getInstance();
	nlohmann::json result = nlohmann::json::array();
	for (auto meeting : data_base->GetList()) {
		result.push_back(meeting);
	}
	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "List of meetings was received");
	response.send() << result;
}

void UserMeetingCreate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	SqLite *data_base = SqLite::getInstance();
	try {
		Meeting new_meeting = nlohmann::json::parse(request.stream());
		data_base->Save(new_meeting);
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_CREATED, "Meeting was created");
		response.send() << nlohmann::json(new_meeting);
	} catch (const std::exception &errc) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST, "Errors in meeting parameters");
		response.send();
	}
}

void UserMeetingRead::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	SqLite *data_base = SqLite::getInstance();
	if (data_base->Contain(m_id)) {
		Meeting meeting = data_base->Get(m_id);
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "Details of meeting");
		response.send() << nlohmann::json(meeting);
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, "Meeting not found");
		response.send();
	}
}

void UserMeetingUpdate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	SqLite *data_base = SqLite::getInstance();
	if (data_base->Contain(m_id)) {
		try {
			Meeting new_meeting = nlohmann::json::parse(request.stream());
			new_meeting.id = m_id;
			data_base->Save(new_meeting);
			response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "Details of meeting");
			response.send() << nlohmann::json(new_meeting);
		} catch (const std::exception &err) {
			response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST, "Errors in meeting parameters");
			response.send();
		}
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, "Meeting not found");
		response.send();
	}
}

void UserMeetingDelete::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	SqLite *data_base = SqLite::getInstance();
	if (data_base->Contain(m_id)) {
		data_base->Delete(m_id);
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT, "Meeting delete");
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, "Meeting not found");
	}
	response.send();
}

} // namespace handlers
