#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/Session.h"
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <bitset>
#include <handlers.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

namespace handlers {

using namespace Poco::Data::Keywords;

class SQLConnection {
public:
	static SQLConnection *getInstance();
	Poco::Data::Session *getSession();

private:
	SQLConnection();
	~SQLConnection();
	Poco::Data::Session *m_ptr_session;
	static SQLConnection *m_ptr_instance;
};

SQLConnection::SQLConnection() {
	Poco::Data::Session session("SQLite", "sample.db");
	m_ptr_session = &session;
}

SQLConnection *SQLConnection::getInstance() {
	if (!m_ptr_instance) {
		m_ptr_instance = new SQLConnection();
	}
	return m_ptr_instance;
}

Poco::Data::Session *SQLConnection::getSession() {
	return m_ptr_session;
}

SQLConnection *SQLConnection::m_ptr_instance = NULL;

struct Meeting {
	std::optional<int> id;
	std::string name;
	std::string description;
	std::string address;
	bool published;
};

using nlohmann::json;

// сериализация (маршалинг)
void to_json(json &j, const Meeting &m) {
	j = json{
	    {"id", m.id.value()},
	    {"name", m.name},
	    {"description", m.description},
	    {"address", m.address},
	    {"published", m.published}};
}

// десериализация (анмаршалинг, распаковка)
void from_json(const json &j, Meeting &m) {
	j.at("name").get_to(m.name);
	j.at("description").get_to(m.description);
	j.at("address").get_to(m.address);
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

class StorageStorage : public Storage {
public:
	void Save(Meeting &meeting) override {
		SQLConnection *connection = SQLConnection::getInstance();
		Poco::Data::Session session(*connection->getSession());
		Poco::Data::Statement select(session); //Nullptr err

		//session << "DROP TABLE IF EXISTS meeting", now;
		/*session << R"(CREATE TABLE IF NOT EXISTS meeting(
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		name TEXT UNIQUE NOT NULL,
		description TEXT NOT NULL,
		address TEXT NOT NULL,
		published INTEGER NOT NULL))",
	    now;
*/
		if (meeting.id.has_value()) {
			m_meetings[meeting.id.value()] = meeting;
		} else {
			int id = m_unique_index++;
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
	int m_unique_index;

	bool MeetingInMap(int id) const {
		return m_meetings.find(id) != m_meetings.end();
	}
};

Storage &GetStorage() {
	static StorageStorage storage;
	return storage;
}

void UserMeetingList::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &storage = GetStorage();
	nlohmann::json result = nlohmann::json::array();
	for (auto meeting : storage.GetList()) {
		result.push_back(meeting);
	}
	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "List of meetings was received");
	response.send() << result;
}

void UserMeetingCreate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &storage = GetStorage();
	try {
		Meeting new_meeting = nlohmann::json::parse(request.stream());
		storage.Save(new_meeting);
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_CREATED, "Meeting was created");
		response.send() << json(new_meeting);
	} catch (const std::exception &e) {
		std::cerr << "Caught " << e.what() << std::endl;
		std::cerr << "Type " << typeid(e).name() << std::endl;
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST, "Errors in meeting parameters");
		response.send();
	}
}

void UserMeetingRead::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	auto &meetings = GetStorage();
	auto meeting = meetings.Get(m_id);
	if (meeting.has_value()) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK, "Details of meeting");
		response.send() << json(meeting.value());
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, "Meeting not found");
		response.send();
	}
}

void UserMeetingUpdate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
	auto &meetings = GetStorage();
	auto meeting = meetings.Get(m_id);
	if (meeting.has_value()) {
		try {
			Meeting new_meeting = nlohmann::json::parse(request.stream());
			new_meeting.id = m_id;
			meetings.Save(new_meeting);
			response.send() << json(new_meeting);
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
	auto &meetings = GetStorage();
	if (meetings.Delete(m_id)) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT, "Meeting delete");
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, "Meeting not found");
	}
	response.send();
}

} // namespace handlers
