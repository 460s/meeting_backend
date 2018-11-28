
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <handlers.hpp>
#include <iterator>
#include <nlohmann/json.hpp>
#include <regex>

namespace handlers {

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
	    {"published", m.published}};
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
	virtual void Create(Meeting &meeting) = 0;
	virtual void Change(int id, Meeting &meeting) = 0;
	virtual void Delete(int id) = 0;
	virtual bool Obtain(int id) = 0;
	virtual MeetingList GetList() = 0;
	virtual ~Storage() {}
};

class MapStorage : public Storage {
public:
	void Create(Meeting &meeting) override {
		meeting.id = m_unique_index;
		m_meetings[m_unique_index] = meeting;
		m_unique_index++;
	}

	bool Obtain(int id) override {
		auto search = m_meetings.find(id);
		if (search != m_meetings.end()) {
			return true;
		} else {
			return false;
		}
	}

	void Change(int id, Meeting &meeting) override {
		meeting.id = id;
		m_meetings[id] = meeting;
	}

	void Delete(int id) override {
		m_meetings.erase(id);
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
	int m_unique_index;
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
	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "List of meetings was received");
	response.send() << result;
}

void UserMeetingGet::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
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
	if (storage.Obtain(id)) {
		try {
			Meeting new_meeting = nlohmann::json::parse(request.stream());
			storage.Change(id, new_meeting);
			response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
			response.send() << json(new_meeting);
		} catch (const std::exception &err) {
			response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
			response.send();
		}
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
		response.send();
	}
}

void UserMeetingCreate::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
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
	if (storage.Obtain(id)) {
		storage.Delete(id);
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT, "Meeting was deleted");
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN, "Denied acess");
	}
	response.send();
}

} // namespace handlers