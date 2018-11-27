#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <handlers.hpp>
#include <iostream>
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
	virtual void Save(Meeting &meeting) = 0;
	virtual MeetingList GetList() = 0;
	virtual bool HasMeeting(int id) = 0;
	virtual Meeting GetMeeting(int id) = 0;
	virtual void Patch(int id, Meeting &meeting) = 0;
	virtual void Delete(int id) = 0;
	virtual ~Storage() {}
};

class MapStorage : public Storage {
public:
	void Save(Meeting &meeting) override {
		int id = m_meetings.size();
		meeting.id = id;
		m_meetings[id] = meeting;
	}
	Storage::MeetingList GetList() override {
		Storage::MeetingList list;
		for (auto [id, meeting] : m_meetings) {
			list.push_back(meeting);
		}
		return list;
	}
	bool HasMeeting(int id) override {
		if (m_meetings.count(id)) {
			return true;
		} else {
			return false;
		}
	}
	Meeting GetMeeting(int id) override {
		return m_meetings[id];
	}
	void Patch(int id, Meeting &meeting) override {
		m_meetings[id] = meeting;
	}
	void Delete(int id) override {
		m_meetings.erase(id);
	}

private:
	using MeetingMap = std::map<int, Meeting>;
	MeetingMap m_meetings;
};

Storage &GetStorage() {
	static MapStorage storage;
	return storage;
}

void UserMeetingList::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "Список доступных встреч получен");

	auto &storage = GetStorage();
	nlohmann::json result = nlohmann::json::array();
	for (auto meeting : storage.GetList()) {
		result.push_back(meeting);
	}
	response.send() << result;
}

void UserMeetingCreate::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "Встреча создана");
	nlohmann::json j = nlohmann::json::parse(request.stream());
	auto &storage = GetStorage();
	Meeting meeting = j;
	//TODO: Неверный json

	storage.Save(meeting);

	response.send() << json(meeting);
}

void UserMeetingGetById::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &storage = GetStorage();
	std::smatch match;
	std::regex_search(request.getURI(), match, std::regex("\\d+"));
	int id = std::stoi(match.str());

	if (storage.HasMeeting(id)) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "Парметры встречи");
		response.send() << json(storage.GetMeeting(id));
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN, "Отсутствует доступ к объекту");
		response.send();
	}
}

void UserMeetingPatch::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &storage = GetStorage();
	std::smatch match;
	std::regex_search(request.getURI(), match, std::regex("\\d+"));
	int id = std::stoi(match.str());

	nlohmann::json j = nlohmann::json::parse(request.stream());
	Meeting meeting = j;
	//TODO: Неверный json

	if (storage.HasMeeting(id)) {
		storage.Patch(id, meeting);
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "Парметры встречи");
		response.send() << json(storage.GetMeeting(id));
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN, "Отсутствует доступ к объекту");
		response.send();
	}
}

void UserMeetingDelete::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &storage = GetStorage();
	std::smatch match;
	std::regex_search(request.getURI(), match, std::regex("\\d+"));
	int id = std::stoi(match.str());

	if (storage.HasMeeting(id)) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT, "Встреча удалена");
		storage.Delete(id);
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN, "Отсутствует доступ к объекту");
	}

	response.send();
}

} // namespace handlers
