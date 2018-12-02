#include "Poco/Data/Session.h"
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <handlers.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>

namespace handlers {

struct Meeting {
	std::optional<int> id;
	std::string name;
	std::string description;
	std::string address;
	bool published;
};

// где их объединить с server.cpp ?
auto const SESSION_TYPE = "SQLite";
auto const CONNECTING_STRING = "meetups.db";

using nlohmann::json;
using Poco::Data::Session;
using Poco::Data::Statement;
using namespace Poco::Data::Keywords;

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

class DbStorage : public Storage {
public:
	// DbStorage() {
	// 	std::cout << "DbStorage constructor" << std::endl;
	//  Как инициализировать сессию тут, а в деструкторе закрыть\убить?
	// }
	using MeetingList = std::vector<Meeting>;

	void Save(Meeting &meeting) override {
		Session session(SESSION_TYPE, CONNECTING_STRING);
		if (meeting.id.has_value()) {
			Statement update(session);
			update << R"(
				UPDATE meeting 
				SET name = ?, description = ?, address = ?, published = ?
				WHERE id = ?)",
			    use(meeting.name),
			    use(meeting.description),
			    use(meeting.address),
			    use(meeting.published),
			    use(meeting.id.value());
			update.execute();
		} else {
			std::cout << "save" << meeting.name << std::endl;
			Statement insert(session);
			insert << R"(
				INSERT INTO meeting
					(name, description, address, published)
				  VALUES (?, ?, ?, ?))",
			    use(meeting.name),
			    use(meeting.description),
			    use(meeting.address),
			    use(meeting.published);
			insert.execute();
		}
	};

	MeetingList GetList() override {
		Meeting meeting;
		int id = -1;
		Storage::MeetingList list;
		Session session(SESSION_TYPE, CONNECTING_STRING);
		Statement select(session);

		select << "SELECT id, name, description, address, published FROM meeting",
		    into(id), // как сюда пропихнуть meeting.id ?
		    into(meeting.name),
		    into(meeting.description),
		    into(meeting.address),
		    into(meeting.published),
		    range(0, 1);
		while (!select.done()) {
			select.execute();
			meeting.id = id;
			list.push_back(meeting);
		}
		return list;
	}

	std::optional<Meeting> Get(int id) override {
		return std::optional<Meeting>();
	}

	bool Delete(int id) override {
		Session session(SESSION_TYPE, CONNECTING_STRING);
		Statement deleteMeeting (session);
		deleteMeeting << R"(
				DELETE FROM meeting 
				WHERE id = ?)",
		    use(id);
		deleteMeeting.execute();
		return deleteMeeting.done();
	}

	~DbStorage() {
		std::cout << "DbStorage destructor" << std::endl;
		// m_session.close();
	}

private:
	// Session m_session;
};

Storage &GetStorage() {
	static DbStorage storage;
	return storage;
}

void UserMeetingList::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	auto &storage = GetStorage();
	nlohmann::json result = nlohmann::json::array();
	for (auto meeting : storage.GetList()) {
		result.push_back(meeting);
	}
	response.send() << result;
}

void UserMeetingCreate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	nlohmann::json j = nlohmann::json::parse(request.stream());
	auto &storage = GetStorage();
	Meeting meeting = j;
	storage.Save(meeting);

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
