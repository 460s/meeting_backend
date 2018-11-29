#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <handlers.hpp>
<<<<<<< HEAD
#include <iterator>
#include <nlohmann/json.hpp>
#include <regex>
=======
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
>>>>>>> upstream/develop

namespace handlers {

struct Meeting {
	std::optional<int> id;
	std::string name;
	std::string description;
	std::string address;
<<<<<<< HEAD
	std::string signup_description;
	int signup_from_date;
	int signup_to_date;
	int from_date;
	int to_date;
=======
>>>>>>> upstream/develop
	bool published;
};

using nlohmann::json;

// сериализация (маршалинг)
void to_json(json &j, const Meeting &m) {
<<<<<<< HEAD
	j = json
	{
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
=======
	j = json{
	    {"id", m.id.value()},
	    {"name", m.name},
	    {"description", m.description},
		{"address",     m.address},
	    {"published", m.published}};
>>>>>>> upstream/develop
}

// десериализация (анмаршалинг, распаковка)
void from_json(const json &j, Meeting &m) {
	j.at("name").get_to(m.name);
	j.at("description").get_to(m.description);
	j.at("address").get_to(m.address);
<<<<<<< HEAD
	j.at("signup_description").get_to(m.signup_description);
	j.at("signup_from_date").get_to(m.signup_from_date);
	j.at("signup_to_date").get_to(m.signup_to_date);
	j.at("from_date").get_to(m.from_date);
	j.at("to_date").get_to(m.to_date);
=======
>>>>>>> upstream/develop
	j.at("published").get_to(m.published);
}

class Storage {
public:
	using MeetingList = std::vector<Meeting>;
	virtual void Create(Meeting &meeting) = 0;
	virtual void Change(int id, Meeting &meeting) = 0;
	virtual void Delete(int id) = 0;
	virtual bool Contain(int id) = 0;
	virtual MeetingList GetList() = 0;
	virtual std::optional<Meeting> Get(int id) = 0;
	virtual bool Delete(int id) = 0;
	virtual ~Storage() {}
};

class MapStorage : public Storage {
public:
<<<<<<< HEAD
	void Create(Meeting &meeting) override {
		meeting.id = m_unique_index;
		m_meetings[m_unique_index] = meeting;
		m_unique_index++;
	}

	void Change(int id, Meeting &meeting) override {
		meeting.id = id;
		m_meetings[id] = meeting;
=======
	void Save(Meeting &meeting) override {
		if (meeting.id.has_value()) {
			m_meetings[meeting.id.value()] = meeting;
		} else {
			int id = m_meetings.size();
			meeting.id = id;
			m_meetings[id] = meeting;
		}
>>>>>>> upstream/develop
	}

	void Delete(int id) override {
		m_meetings.erase(id);
	}

	bool Contain(int id) override {
		return m_meetings.find(id) != m_meetings.end();
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
<<<<<<< HEAD
	int m_unique_index;
=======

	bool MeetingInMap(int id) const {
		auto meeting_ptr = m_meetings.find(id);
		return meeting_ptr != m_meetings.end();
	}
>>>>>>> upstream/develop
};

Storage &GetStorage() {
	static MapStorage storage;
	return storage;
}

<<<<<<< HEAD
void UserMeetingList::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
=======
void UserMeetingList::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
>>>>>>> upstream/develop
	auto &storage = GetStorage();
	nlohmann::json result = nlohmann::json::array();
	for (auto meeting : storage.GetList()) {
		result.push_back(meeting);
	}
	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "List of meetings was received");
	response.send() << result;
}

<<<<<<< HEAD
void UserMeeting::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &storage = GetStorage();
	std::smatch match_result;
	std::regex_search(request.getURI(), match_result, std::regex("\\d+"));
	int id = std::stoi(match_result.str());
	std::vector<Meeting> meetings = GetStorage().GetList();
	auto pos_meeting = std::find_if(meetings.begin(), meetings.end(), [&id](const Meeting &meeting) {
		return meeting.id == id;
	});
	if (pos_meeting != meetings.end()) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "Details of meeting");
		response.send() << json(*pos_meeting);
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN, "Denied access");
		response.send();
	}
}

void UserMeetingChange::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &storage = GetStorage();
	std::smatch match_result;
	std::regex_search(request.getURI(), match_result, std::regex("\\d+"));
	int id = std::stoi(match_result.str());
	if (storage.Contain(id)) {
		try {
			Meeting new_meeting = nlohmann::json::parse(request.stream());
			storage.Change(id, new_meeting);
			response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "Parameters of meeting");
			response.send() << json(new_meeting);
		} catch (const std::exception &err) {
			response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST, "Errors in meeting parameters");
			response.send();
		}
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN, "Denied access");
		response.send();
	}
}

void UserMeetingCreate::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
=======
void UserMeetingCreate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	nlohmann::json j = nlohmann::json::parse(request.stream());
>>>>>>> upstream/develop
	auto &storage = GetStorage();
	try {
		Meeting new_meeting = nlohmann::json::parse(request.stream());
		storage.Create(new_meeting);
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_CREATED, "Meeting was created");
		response.send() << json(new_meeting);
	} catch (const std::exception &err) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST, "Errors in meeting parameters");
		response.send();
	}
}

void UserMeetingDelete::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &storage = GetStorage();
	std::smatch match_result;
	std::regex_search(request.getURI(), match_result, std::regex("\\d+"));
	int id = std::stoi(match_result.str());
	if (storage.Contain(id)) {
		storage.Delete(id);
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT, "Meeting was deleted");
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN, "Denied acess");
	}
	response.send();
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
