#include <regex>
#include <optional>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <handlers.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

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
	virtual MeetingList GetList() = 0;
	virtual std::optional<Meeting> GetById(int id) = 0;
	virtual Meeting Update(int id, const Meeting &meeting) = 0;
	virtual void Delete(int id) = 0;
	virtual ~Storage() {}
};

class MapStorage : public Storage {
public:
	void Create(Meeting &meeting) override {
	    int id = ++this->last_id;
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

	std::optional<Meeting> GetById(int id) override {
        MeetingMap::iterator val_pos;
        val_pos = m_meetings.find(id);
        if(val_pos != m_meetings.end())
            return val_pos->second;
        else
            return std::nullopt;
	}

	Meeting Update(int id, const Meeting &meeting) override {
		m_meetings.erase(id);
		m_meetings.insert(std::pair<int,Meeting>(meeting.id, meeting));
		return m_meetings.at(meeting.id);
	}

	void Delete(int id) override {
	    m_meetings.erase(id);
	}
private:
	using MeetingMap = std::map<int, Meeting>;
	MeetingMap m_meetings;
	int last_id = 0;
};

Storage &GetStorage() {
	static MapStorage storage;
	return storage;
}

void UserMeetingCreate::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    try {
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
		nlohmann::json j = nlohmann::json::parse(request.stream());
		auto &storage = GetStorage();
		Meeting meeting = j;
		storage.Create(meeting);
		response.send() << json(meeting);
	} catch (json::exception &e) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST, "Ошибки в параметрах встречи");
		response.send();
	}
}

void UserMeetingList::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	auto &storage = GetStorage();
	nlohmann::json result = nlohmann::json::array();

	for (auto meeting : storage.GetList()) {
		result.push_back(meeting);
	}

	response.send() << result;
}

void UserMeetingGetById::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    auto &storage = GetStorage();
    std::smatch match_rslt;
    std::regex_search(request.getURI(), match_rslt, std::regex("\\d+"));

    int id = std::stoi(match_rslt.str());
    std::optional<Meeting> opt_meeting = storage.GetById(id);

    if (!opt_meeting){
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, "Такая встреча отсутствует");
		response.send();
    }

    response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "Параметры встречи");
    response.send() << json(opt_meeting.value());
}

void UserMeetingPatch::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

	auto &storage = GetStorage();
	std::smatch match_rslt;
	std::regex_search(request.getURI(), match_rslt, std::regex("\\d+"));

	int id = std::stoi(match_rslt.str());
	std::optional<Meeting> opt_existing_meeting = storage.GetById(id);
    Meeting newMeeting;
    int j_id = -1;
    try {
	    nlohmann::json j = nlohmann::json::parse(request.stream());
	    newMeeting = j;
	    j_id=j.at("id");
    } catch (json::exception &e) {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST, "Ошибки в параметрах встречи");
        response.send();
    }
    std::optional<Meeting> opt_is_exist_new_meeting = storage.GetById(j_id);

    if (opt_is_exist_new_meeting && j_id != id){
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_CONFLICT, "Встреча с таким ID уже есть в системе");
        response.send();
    }

	if (!opt_existing_meeting){
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, "Такая встреча отсутствует");
		response.send();
	}
	newMeeting.id = j_id;
    response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "Параметры встречи");
    response.send() << json(storage.Update(id, newMeeting));
}

void UserMeetingDelete::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    auto &storage = GetStorage();
    std::smatch match_rslt;
    std::regex_search(request.getURI(), match_rslt, std::regex("\\d+"));

    int id = std::stoi(match_rslt.str());
    std::optional<Meeting> opt_meeting = storage.GetById(id);

    if (!opt_meeting){
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, "Такая встреча отсутствует");
        response.send();
    }

    storage.Delete(id);
    response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT, "Встреча удалена");
    response.send();
}

} // namespace handlers
