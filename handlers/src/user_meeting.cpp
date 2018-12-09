#include "handlers.hpp"
#include <iostream>
#include <logger.hpp>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <sqlite.hpp>

namespace handlers {

struct Meeting {
	std::optional<int> id;
	std::string name;
	std::string description;
	std::string address;
	bool published{false};
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
	virtual ~Storage() = default;
};

using Poco::Data::Keywords::into;
using Poco::Data::Keywords::now;
using Poco::Data::Keywords::range;
using Poco::Data::Keywords::use;
using Poco::Data::Statement;

class SqliteStorage : public Storage {
public:
	void Save(Meeting &meeting) override {
		std::lock_guard<std::mutex> l{m_mutex};
		if (meeting.id.has_value()) {
			Statement update(m_session);
			auto published = b2i(meeting.published);
			update << "UPDATE meeting SET "
			          "name=?, description=?, address=?, published=? "
			          "WHERE id=?",
			    use(meeting.name),
			    use(meeting.description),
			    use(meeting.address),
			    use(published),
			    use(meeting.id.value()),
			    now;
		} else {
			Statement insert(m_session);
			int published = b2i(meeting.published);
			insert << "INSERT INTO meeting (name, description, address, published) VALUES(?, ?, ?, ?)",
			    use(meeting.name),
			    use(meeting.description),
			    use(meeting.address),
			    use(published),
			    now;

			Statement select(m_session);
			int id = 0;
			select << "SELECT last_insert_rowid()", into(id), now;
			meeting.id = id;
		}
	}

	Storage::MeetingList GetList() override {
		std::lock_guard<std::mutex> l{m_mutex};
		Storage::MeetingList list;
		Meeting meeting;
		Statement select(m_session);
		select << "SELECT id, name, description, address, published FROM meeting",
		    into(meeting.id.emplace()),
		    into(meeting.name),
		    into(meeting.description),
		    into(meeting.address),
		    into(meeting.published),
		    range(0, 1); //  iterate over result set one row at a time

		while (!select.done() && select.execute() > 0) {
			list.push_back(meeting);
		}
		return list;
	}

	std::optional<Meeting> Get(int id) override {
		std::lock_guard<std::mutex> l{m_mutex};
		int cnt = 0;
		m_session << "SELECT COUNT(*) FROM meeting WHERE id=?", use(id), into(cnt), now;
		if (cnt > 0) {
			Meeting meeting;
			Statement select(m_session);
			int tmp_id = 0;
			select << "SELECT id, name, description, address, published FROM meeting WHERE id=?",
			    use(id),
			    into(tmp_id),
			    into(meeting.name),
			    into(meeting.description),
			    into(meeting.address),
			    into(meeting.published),
			    now;
			meeting.id = tmp_id;
			return meeting;
		}
		return std::nullopt;
	}

	bool Delete(int id) override {
		std::lock_guard<std::mutex> l{m_mutex};
		m_session << "DELETE FROM meeting WHERE id=?", use(id), now;
		return true;
	}

private:
	std::mutex m_mutex;

	Poco::Data::Session m_session{sqlite::TYPE_SESSION, sqlite::DB_PATH};

	int b2i(bool b) {
		return b ? 1 : 0;
	}
};

Storage &GetStorage() {
	static SqliteStorage storage;
	return storage;
}

void UserMeetingList::HandleRestRequest(Poco::Net::HTTPServerRequest &/*request*/, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);

	auto &storage = GetStorage();
	nlohmann::json result = nlohmann::json::array();
	for (auto meeting : storage.GetList()) {
		result.push_back(meeting);
	}
	meeting::GetLogger().information("UserMeetingList: " + result.dump());
	response.send() << result;
}

void UserMeetingCreate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	nlohmann::json j = nlohmann::json::parse(request.stream());
	auto &storage = GetStorage();
	Meeting meeting = j;
	storage.Save(meeting);

	meeting::GetLogger().information("UserMeetingCreate: " + std::to_string(meeting.id.value()));
	response.send() << json(meeting);
}

void UserMeetingRead::HandleRestRequest(Poco::Net::HTTPServerRequest &/*request*/, Poco::Net::HTTPServerResponse &response) {
	auto &meetings = GetStorage();
	auto meeting = meetings.Get(m_id);
	if (meeting.has_value()) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
		meeting::GetLogger().information("UserMeetingRead: " + std::to_string(m_id));
		response.send() << json(meeting.value());
		return;
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
	
	meeting::GetLogger().information("UserMeetingUpdate: " + std::to_string(meeting.id.value()));
	response.send() << json(meeting);
}

void UserMeetingDelete::HandleRestRequest(Poco::Net::HTTPServerRequest &/*request*/, Poco::Net::HTTPServerResponse &response) {
	auto &meetings = GetStorage();
	if (meetings.Delete(m_id)) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT);
	} else {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
	}
	meeting::GetLogger().information("UserMeetingDelete: " + std::to_string(m_id));
	response.send();
}

} // namespace handlers
