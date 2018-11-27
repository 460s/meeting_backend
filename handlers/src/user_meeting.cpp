#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <handlers.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>

namespace handlers {

struct Meeting {
	int id;
	std::string name;
	std::string description;
	bool published;
};

using nlohmann::json;

// сериализация (маршалинг)
void to_json(json &j, const Meeting &m) {
	j = json{
		{"id", m.id},
	    {"name", m.name},
	    {"description", m.description},
	    {"published", m.published}};
}

// десериализация (анмаршалинг, распаковка)
void from_json(const json &j, Meeting &m) {
	j.at("name").get_to(m.name);
	j.at("description").get_to(m.description);
	j.at("published").get_to(m.published);
}

class Storage {
public:
	using MeetingList = std::vector<Meeting>;
	virtual void Save(Meeting &meeting) = 0;
    virtual void Delete(int id) = 0;
    virtual bool Exist(int meeting_id) = 0;
    virtual void Update(int id, Meeting &meeting) = 0;
	virtual MeetingList GetList() = 0;
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
    void Delete(int id) override {
        auto meeting = m_meetings.find(id);
        m_meetings.erase (meeting);
    }
    bool Exist(int meeting_id) override {
        auto meeting = m_meetings.find(meeting_id);
        if ( meeting != m_meetings.end()){
            return true;
        } else {
            return false;
        }
    }
    void Update(int id, Meeting &meeting) override {
        meeting.id = id;
        m_meetings[id] = meeting;
    }

private:
	using MeetingMap = std::map<int, Meeting>;
	MeetingMap m_meetings;
};

Storage &GetStorage() {
	static MapStorage storage;
	return storage;
}

std::vector<std::string> split(const std::string &str_to_split, char delimeter) {
    std::stringstream string_stream(str_to_split);
    std::string item;
    std::vector<std::string> splited_strings;
    while (std::getline(string_stream, item, delimeter)) {
        splited_strings.push_back(item);
    }
    return splited_strings;
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

void UserMeetingCreate::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    auto &storage = GetStorage();
    try {
        nlohmann::json j = nlohmann::json::parse(request.stream());
        Meeting meeting = j;
        storage.Save(meeting);
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_CREATED);
        response.send();
    } catch (std::exception &e) {
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
        response.send();
    }
}

void UserMeetingDelete::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    auto &storage = GetStorage();
    int meeting_id = std::stoi(split(request.getURI(), '/')[3]);
    if (storage.Exist(meeting_id)) {
        storage.Delete(meeting_id);
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT);
    } else {
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
    }
    response.send();
}

void UserMeetingPatch::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    auto &storage = GetStorage();
    int meeting_id = std::stoi(split(request.getURI(), '/')[3]);
    if(storage.Exist(meeting_id)){
        try {
            Meeting meeting = nlohmann::json::parse(request.stream());
            storage.Update(meeting_id, meeting);
            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
        } catch (...) {
            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
        }
    } else {
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
    }
    response.send();
}

    void UserMeeting::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
        auto &storage = GetStorage();
        int meeting_id = std::stoi(split(request.getURI(), '/')[3]);
        //std::vector<Meeting> meetings = GetStorage().GetList();
        Meeting searching_meeting;

        if(storage.Exist(meeting_id)) {
            for (auto meeting : storage.GetList()) {
                if (meeting.id == meeting_id)
                    searching_meeting = meeting;
            }

            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
            response.send() << json(searching_meeting);

        } else {
            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
            response.send();
        }
        response.send();
    }

} // namespace handlers
