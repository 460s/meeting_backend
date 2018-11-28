#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/NumberParser.h>
#include <Poco/RegularExpression.h>
#include <handlers.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

namespace handlers {

extern Poco::RegularExpression expr_user_meeting_id;

struct Meeting {
	int id;
	std::string name;
	std::string description;
	std::string address;
	std::string signup_description;
	int signup_from_date;
	int signup_to_date;
	int from_date;
	int to_date;
	bool published;
};

using nlohmann::json;

// сериализация (маршалинг)
void to_json(json &j, const Meeting &m) {
	j = json{
	    {"id", m.id},
	    {"name", m.name},
	    {"description", m.description},
	    {"address", m.address},
	    {"signup_description", m.signup_description},
	    {"signup_from_date", m.signup_from_date},
	    {"signup_to_date", m.signup_to_date},
	    {"from_date", m.from_date},
	    {"to_date", m.to_date},
	    {"published", m.published}
		};
}

// десериализация (анмаршалинг, распаковка)
void from_json(const json &j, Meeting &m) {
	j.at("name").get_to(m.name);
	j.at("description").get_to(m.description);
	j.at("address").get_to(m.address);
	j.at("signup_description").get_to(m.signup_description);
	j.at("signup_from_date").get_to(m.signup_from_date);
	j.at("signup_to_date").get_to(m.signup_to_date);
	j.at("from_date").get_to(m.from_date);
	j.at("to_date").get_to(m.to_date);
	j.at("published").get_to(m.published);
}

class Storage {
public:
	using MeetingList = std::vector<Meeting>;
	virtual void Save(Meeting &meeting) = 0;
	virtual void Save(Meeting &meeting, const int &id) = 0;
	virtual bool Get(const int &id, Meeting &meeting) = 0;
	virtual bool Delete (const int &id) = 0;
	virtual MeetingList GetList() = 0;
	virtual ~Storage() {}
};

class MapStorage : public Storage {
public:
	void Save(Meeting &meeting) override {
		int id = m_meetings_size++;
		meeting.id = id;
		m_meetings[id] = meeting;
	}
	void Save(Meeting &meeting, const int &id) override {
		m_meetings[id] = meeting;
	}
	bool Get(const int &id, Meeting &meeting) override {
		try {
			meeting = m_meetings.at(id);
			return true;
		} catch (std::out_of_range) {
			return false;
		}
	}
	bool Delete(const int &id) override {
		return m_meetings.erase(id);
	}
	Storage::MeetingList GetList() override {
		Storage::MeetingList list;
		for (auto [id, meeting] : m_meetings) {
			list.push_back(meeting);
		}
		return list;
	}

private:
	using MeetingMap = std::map<int, Meeting>;
	MeetingMap m_meetings;
	int m_meetings_size = 0;
};

Storage &GetStorage() {
	static MapStorage storage;
	return storage;
}

void UserMeetingList::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &storage = GetStorage();
	nlohmann::json result = nlohmann::json::array();
	for (auto meeting : storage.GetList()) {
		result.push_back(meeting);
	}
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	response.send() << result;
}

void UserMeetingCreate::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	nlohmann::json j = nlohmann::json::parse(request.stream());
	if (j.is_discarded()) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
		response.send();
		return;
	}
	auto &storage = GetStorage();
	Meeting meeting;
	try {
		meeting = j;
	} catch (json::exception) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
		response.send();
		return;
	}	
	storage.Save(meeting);
	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_CREATED);
	response.send() << json(meeting);
}

int ExtractMeetingId(const std::string &uri) {
	std::vector<std::string> number;
	expr_user_meeting_id.split(uri, number);
	return Poco::NumberParser::parse(number[1]);
}

void UserMeetingDetails::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &storage = GetStorage();
	int id = ExtractMeetingId(request.getURI());
	Meeting meeting;
	if (storage.Get(id, meeting)) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
		response.send();
		return;
	}
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	response.send() << json(meeting);
}

void UserMeetingEditDetails::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	nlohmann::json j = nlohmann::json::parse(request.stream());
	if (j.is_discarded()) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
		response.send();
		return;
	}
	auto &storage = GetStorage();
	int id = ExtractMeetingId(request.getURI());
	Meeting meeting;
	try {
		int j_id = j.at("id");
		if (j_id != id) {
			response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
			response.send();
			return;
		}
		meeting = j;
	} catch (json::exception) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
		response.send();
		return;
	}
	storage.Save(meeting, id);
	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
	response.send() << json(meeting);
}

void UserMeetingDelete::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &storage = GetStorage();
	int id = ExtractMeetingId(request.getURI());
	if (storage.Delete(id)) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT);
		response.send();
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
		response.send();
	}
}

} // namespace handlers
