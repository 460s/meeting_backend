#include "Poco/Data/SQLite/Connector.h"
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

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

class SqliteStorage : public Storage {
public:
	void Save(Meeting &meeting) override {
		if (meeting.id.has_value()) {
			Statement update(GetSession());
			update << "UPDATE meeting SET name = ?, description = ?, address = ?, published = ? WHERE id = ?",
			    use(meeting.name),
			    use(meeting.description),
			    use(meeting.address),
			    use(meeting.published),
			    use(meeting.id.value()),
			    now;
		} else {
			Statement add(GetSession());
			add << "INSERT INTO meeting (name, description, address, published) VALUES (?, ?, ?, ?)",
			    use(meeting.name),
			    use(meeting.description),
			    use(meeting.address),
			    use(meeting.published),
			    now;
		}
	}
	Storage::MeetingList GetList() override {
		MeetingList list;
		Meeting meeting;
		int id;
		Statement select(GetSession());
		select << "SELECT id, name, description, address, published FROM meeting",
		    into(id),
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
		auto &session = GetSession();
		if (HasMeeting(id)) {
			Meeting meeting;
			meeting.id = id;
			Statement get(session);
			get << "SELECT name, description, address, published FROM meeting WHERE id = ?",
			    use(id),
			    into(meeting.name),
			    into(meeting.description),
			    into(meeting.address),
			    into(meeting.published),
			    now;

			return meeting;
		} else {
			return std::nullopt;
		}
	}
	bool Delete(int id) override {
		if (HasMeeting(id)) {
			Statement del(GetSession());
			del << "DELETE FROM meeting WHERE id = ?",
			    use(id),
			    now;
			return true;
		} else {
			return false;
		}
	}

private:
	Session &GetSession() {
		static Session session("SQLite", "sample.db");
		return session;
	}
	bool HasMeeting(int id) {
		Statement hasMeeting(GetSession());
		int meetings_cnt = 0;
		hasMeeting << "SELECT COUNT(*) FROM meeting WHERE id = ?;",
		    use(id),
		    into(meetings_cnt),
		    now;
		return meetings_cnt != 0;
	}
};

Storage &GetStorage() {
	static SqliteStorage storage;
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
