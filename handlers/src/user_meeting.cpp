#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"
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

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;
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
		Session session("SQLite", "sample.db");
		if (meeting.id.has_value()) {
			Statement update(session);
			update << "UPDATE meeting SET Name = ?, Description = ?, Address = ?, Published = ? WHERE id = ?",
				use(meeting.name),
				use(meeting.description),
				use(meeting.address),
				use(meeting.published),
				use(meeting.id.value());
			update.execute();
		} else {
			Statement insert(session);
			insert << "INSERT INTO meeting (Name, Description, Address, Published) VALUES(?, ?, ?, ?)",
				use(meeting.name),
				use(meeting.description),
				use(meeting.address),
				use(meeting.published);
			insert.execute();
		}
	}
	Storage::MeetingList GetList() override {
		Storage::MeetingList list;
		Meeting meeting;
		int id = 0;
		Session session("SQLite", "sample.db");
		Statement select(session);
		select << "SELECT id, Name, Description, Address, Published FROM meeting",
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
		if (HasMeeting(id)) {
			Meeting meeting;
			Session session("SQLite", "sample.db");
			Statement select(session);
			select << "SELECT Name, Description, Address, Published FROM meeting WHERE id = ?",
				into(meeting.name),
				into(meeting.description),
				into(meeting.address),
				into(meeting.published),
				use(id);
			select.execute();
			meeting.id = id;
			return meeting;
		}
		return std::optional<Meeting>();
	}
	bool Delete(int id) override {
		if (HasMeeting(id)) {
			Session session("SQLite", "sample.db");
			Statement deleteMeeting(session);
			deleteMeeting << "DELETE FROM meeting WHERE id = ?",
				use(id);
			deleteMeeting.execute();
			return true;
		}
		return false;
	}

private:
	bool HasMeeting(int id) const {
		Session session("SQLite", "sample.db");
		Statement find(session);
		int count = 0;
		find << "SELECT Count(id) FROM meeting WHERE id = ? LIMIT 1",
			into(count),
			use(id);
		find.execute();
		return count != 0;
	}
};

Storage &GetStorage() {
	static MapStorage storage;
	return storage;
}

void UserMeetingList::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	auto &storage = GetStorage();
	nlohmann::json result = nlohmann::json::array();
	for (auto meeting : storage.GetList()) {
		result.push_back(meeting);
	}
	response.send() << result << std::endl;
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
		response.send() << json(meeting.value()) << std::endl;
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
