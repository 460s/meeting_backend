#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/NumberParser.h>
#include <Poco/RegularExpression.h>
#include <handlers.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>

namespace handlers {

extern Poco::RegularExpression regexp_user_meeting_id;

using nlohmann::json;

struct Meeting {
	std::optional<int> id;
	std::string name;
	std::string description;
	std::string address;
	/*
	std::string signup_description;
	int signup_from_date;
	int signup_to_date;
	int from_date;
	int to_date;
	*/
	bool published;
};

void to_json(json &j, const Meeting &m) {
	j = json{
	    {"id", m.id.value()},
	    {"name", m.name},
	    {"description", m.description},
	    {"address", m.address},
	    /*
		{"signup_description", m.signup_description},
	    {"signup_from_date", m.signup_from_date},
	    {"signup_to_date", m.signup_to_date},
	    {"from_date", m.from_date},
	    {"to_date", m.to_date},
		*/
	    {"published", m.published}};
}

void from_json(const json &j, Meeting &m) {
	j.at("name").get_to(m.name);
	j.at("description").get_to(m.description);
	j.at("address").get_to(m.address);
	/*
	j.at("signup_description").get_to(m.signup_description);
	j.at("signup_from_date").get_to(m.signup_from_date);
	j.at("signup_to_date").get_to(m.signup_to_date);
	j.at("from_date").get_to(m.from_date);
	j.at("to_date").get_to(m.to_date);
	*/
	j.at("published").get_to(m.published);
}

class Storage {
public:
	using MeetingList = std::vector<Meeting>;
	virtual void Save(Meeting &meeting) = 0;
	virtual MeetingList GetList() = 0;
	virtual std::optional<Meeting> Get(int id) = 0;
	virtual bool Delete(int id) = 0;
	virtual ~Storage() {}
};

class MapStorage : public Storage {
public:
	void Save(Meeting &meeting) override {
		if (meeting.id.has_value()) {
			m_meetings[meeting.id.value()] = meeting;
		} else {
			int id = m_meetings.size();
			meeting.id = id;
			m_meetings[id] = meeting;
		}
	}
	Storage::MeetingList GetList() override {
		Storage::MeetingList list;
		for (auto [id, meeting] : m_meetings) {
			list.push_back(meeting);
		}
		return list;
	}
	std::optional<Meeting> Get(int id) override {
		if (MeetingInMap(id)) {
			return m_meetings[id];
		}
		return std::optional<Meeting>();
	}
	bool Delete(int id) override {
		if (MeetingInMap(id)) {
			m_meetings.erase(id);
			return true;
		}
		return false;
	}

private:
	using MeetingMap = std::map<int, Meeting>;
	MeetingMap m_meetings;

	bool MeetingInMap(int id) const {
		auto meeting_ptr = m_meetings.find(id);
		return meeting_ptr != m_meetings.end();
	}
};

Storage &GetStorage() {
	static MapStorage storage;
	return storage;
}

void UserMeetingList::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &storage = GetStorage();

	nlohmann::json result = nlohmann::json::array();
	for (auto meeting : storage.GetList()) {
		result.push_back(meeting);
	}

	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	response.send() << result;
}

void UserMeetingCreate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	nlohmann::json j = nlohmann::json::parse(request.stream());
	if (j.is_discarded()) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
		response.send() << "Bad meeting JSON";
	}

	auto &storage = GetStorage();

	Meeting meeting;
	try {
		meeting = j;
	} catch (json::exception) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
		response.send() << "Bad meeting parameters";
		return;
	}

	storage.Save(meeting);

	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_CREATED);
	response.send() << json(meeting);
}

void UserMeetingRead::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	//response.setContentType("application/json");
	auto &meetings = GetStorage();
	auto meeting = meetings.Get(m_id);
	if (meeting.has_value()) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
		response.send() << json(meeting.value());
	}

	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
	response.send();
}

void UserMeetingUpdate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
	auto body = nlohmann::json::parse(request.stream());
	auto &meetings = GetStorage();
	Meeting meeting = body;
	meeting.id = m_id;
	meetings.Save(meeting);

	response.send() << json(meeting);
}

void UserMeetingDelete::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &meetings = GetStorage();
	if (meetings.Delete(m_id)) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT);
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
	}
	response.send();
}

} // namespace handlers
