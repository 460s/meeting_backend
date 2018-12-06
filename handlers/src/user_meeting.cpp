#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Data/Session.h>
#include <Poco/Logger.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <nlohmann/json.hpp>

#include <iostream>
#include <optional>
#include <mutex>

#include <loggers.hpp>
#include <handlers.hpp>
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
		for (const auto &[id, meeting] : m_meetings) {
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

using Poco::Data::Statement;
using Poco::Data::Keywords::into;
using Poco::Data::Keywords::now;
using Poco::Data::Keywords::range;
using Poco::Data::Keywords::use;

class SqliteStorage : public Storage {
public:
	void Save(Meeting &meeting) override {
		//std::lock_guard<std::mutex> l{m_mutex}; 
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

			std::string msg = "executed query update meeting with id = " + std::to_string(meeting.id.value());
			Poco::Logger &logger = GetLoggers().getSqlLogger();
			logger.information(msg);
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

			std::string msg = "executed query insert meeting with id = " + std::to_string(id);
			Poco::Logger &logger = GetLoggers().getSqlLogger();
			logger.information(msg);
		}
	}

	Storage::MeetingList GetList() override {
		//std::lock_guard<std::mutex> l{m_mutex};
		i++;
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

		Poco::Logger &logger = GetLoggers().getSqlLogger();
		logger.information(std::to_string(i));
		while (!select.done() && select.execute()) {
			list.push_back(meeting);
		}
		return list;
	}

	std::optional<Meeting> Get(int id) override {
		//std::lock_guard<std::mutex> l{m_mutex}; 
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

			std::string msg = "executed query select meeting with id = " + std::to_string(id);
			Poco::Logger &logger = GetLoggers().getSqlLogger();
			logger.information(msg);

			return meeting;
		}
		return std::nullopt;
	}

	bool Delete(int id) override {
		//std::lock_guard<std::mutex> l{m_mutex};
		m_session << "DELETE FROM meeting WHERE id=?", use(id), now;

		Poco::Logger &logger = GetLoggers().getSqlLogger();
		logger.information("executed query delete meeting with id = " + std::to_string(id));

		return true;
	}

private:
	Poco::Data::Session m_session{sqlite::TYPE_SESSION, sqlite::DB_PATH};
	//std::mutex m_mutex;
	int i=0;

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
	for (const auto &meeting : storage.GetList()) {
		result.push_back(meeting);
	}
	response.send() << result;

	Poco::Logger &logger = GetLoggers().getHttpResponseLogger();
	logger.information("sending response(code - HTTP_OK, body - meeting list)");
}

void UserMeetingCreate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	nlohmann::json j = nlohmann::json::parse(request.stream());
	auto &storage = GetStorage();
	Meeting meeting = j;
	storage.Save(meeting);

	response.send() << json(meeting);

	Poco::Logger &logger = GetLoggers().getHttpResponseLogger();
	logger.information("sending response(code - HTTP_OK, body - new meeting)");
}

void UserMeetingRead::HandleRestRequest(Poco::Net::HTTPServerRequest &/*request*/, Poco::Net::HTTPServerResponse &response) {
	auto &meetings = GetStorage();
	auto meeting = meetings.Get(m_id);
	if (meeting.has_value()) {
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
		response.send() << json(meeting.value());

		Poco::Logger &logger = GetLoggers().getHttpResponseLogger();
		logger.information("sending response(code - HTTP_OK, body - meeting with id)");
		return;
	}

	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
	response.send();

	Poco::Logger &logger = GetLoggers().getHttpResponseLogger();
	logger.information("sending response(code - HTTP_NOT_FOUND)");
}

void UserMeetingUpdate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
	auto &meetings = GetStorage();
	auto meeting = meetings.Get(m_id);
	if (meeting.has_value()) {
		auto body = nlohmann::json::parse(request.stream());
		Meeting new_meeting = body;
		new_meeting.id = m_id;
		meetings.Save(new_meeting);
		response.send() << json(new_meeting);

		Poco::Logger &logger = GetLoggers().getHttpResponseLogger();
		logger.information("sending response(code - HTTP_OK, body - updated meeting)");
		return;
	}

	response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
	response.send();

	Poco::Logger &logger = GetLoggers().getHttpResponseLogger();
	logger.information("sending response(code - HTTP_NOT_FOUND)");
}

void UserMeetingDelete::HandleRestRequest(Poco::Net::HTTPServerRequest &/*request*/, Poco::Net::HTTPServerResponse &response) {
	auto &meetings = GetStorage();
	Poco::Logger &logger = GetLoggers().getHttpResponseLogger();
	if (meetings.Delete(m_id)) {
		logger.information("sending response(code - HTTP_NO_CONTENT)");
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT);
	} else {
		logger.information("sending response(code - HTTP_NOT_FOUND)");
		response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
	}
	response.send();
}

} // namespace handlers
