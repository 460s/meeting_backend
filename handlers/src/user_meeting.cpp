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
	virtual MeetingList GetList() = 0;
	virtual int Patch(int id, Meeting &meeting) = 0;
	virtual int Delete(int id) = 0;
	virtual ~Storage() {}
	virtual Meeting Get(int ident) = 0;
	virtual int size() = 0;
};

class MapStorage : public Storage {
public:
	void Save(Meeting &meeting) override {
		int id = m_meetings.size();
		//std::cout << id;
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
	int Patch(int id, Meeting &meeting) override {
		//std::cout << id;
		if (id < m_meetings.size()) {
			m_meetings[id] = meeting;
			return 0;
		} else
			return 1;
	}
	int Delete(int id) override {
		if (id < m_meetings.size()) {
			m_meetings.erase(id);
			return 0;
		} else
			return 1;
	}
	Meeting Get(int ident) override {
		for (auto [id, meeting] : m_meetings) {
			if (id == ident) return meeting;
		}
	}
	int size() override {
		return m_meetings.size();
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
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	auto &storage = GetStorage();
	nlohmann::json result = nlohmann::json::array();
	for (auto meeting : storage.GetList()) {
		result.push_back(meeting);
	}
	response.send() << result;
}

void UserMeetingCreate::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	nlohmann::json j = nlohmann::json::parse(request.stream());
	//std::cout << "1" << std::endl;
	auto &storage = GetStorage();
	Meeting meeting = j;
	storage.Save(meeting);

	response.send() << json(meeting);
}

void UserMeetingPatch::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);

	nlohmann::json j = nlohmann::json::parse(request.stream());

	auto &storage = GetStorage();
	Meeting meeting = j;
	int id = std::stoi(request.getURI().substr(14));
	int resp = storage.Patch(id, meeting);
	if (resp == 0)
		response.send() << json(meeting);
	else
		response.send() << "Hasn't meeting with id = " << id;
}

void UserMeetingDelete::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	auto &storage = GetStorage();
	int id = std::stoi(request.getURI().substr(14));
	int resp = storage.Delete(id);
	if (resp == 0) {
		response.send() << "Success";
	} else
		response.send() << "Hasn't meeting with id = " << id;
}

void UserMeetingGet::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	auto &storage = GetStorage();
	int id = std::stoi(request.getURI().substr(14));
	if (id < storage.size()) {
		Meeting resp = storage.Get(id);
		response.send() << json(resp);
	} else
		response.send() << "Hasn't meeting with id = " << id;
}

} // namespace handlers
