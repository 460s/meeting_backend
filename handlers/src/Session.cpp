#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Data/Session.h>

#include <Meeting.hpp>
#include <Session.hpp>

using namespace Poco::Data::Keywords;

struct UniqueSession {
	std::unique_ptr<Poco::Data::Session> unique_session;
	UniqueSession() : unique_session(new Poco::Data::Session("SQLite", "sample.db")) {
		Poco::Data::Session session = *unique_session.get();
		session << R"(CREATE TABLE IF NOT EXISTS meeting(
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			name TEXT UNIQUE NOT NULL,
			description TEXT NOT NULL,
			address TEXT NOT NULL,
			published INTEGER NOT NULL))",
			now;
	}
};

Session *Session::m_instance = NULL;

Session::Session() {
	Poco::Data::SQLite::Connector::registerConnector();
	std::shared_ptr<UniqueSession> session(std::make_shared<UniqueSession>());
	std::shared_ptr<Poco::Data::Session> poco_session(session, session->unique_session.get());
	m_session.swap(poco_session);
}

Session *Session::getInstance() {
	if (!m_instance) {
		m_instance = new Session();
	}
	return m_instance;
}

std::shared_ptr<Poco::Data::Session> Session::getSession() {
	return m_session;
}

bool Session::Contain(int id) {
	Poco::Data::Session session = *m_instance->getSession();
	Poco::Nullable<std::string> name;
	session << "SELECT name FROM meeting WHERE id = ?",
		into(name),
		use(id),
		now;
	return !name.isNull();
}

void Session::Save(handlers::Meeting &meeting) {
	if (meeting.name == "" || meeting.description == "" || meeting.address == "") {
		throw std::exception();
	}
	Poco::Data::Session session = *m_instance->getSession();
	if (meeting.id.has_value()) {
		session << "UPDATE meeting SET name = ?, description = ?, address = ?, published = ? --\n\
				WHERE id = ?",
			use(meeting.name),
			use(meeting.description),
			use(meeting.address),
			use(meeting.published),
			use(meeting.id.value()),
			now;
	} else {
		session << "INSERT INTO meeting (name, description, address, published) --\n\
				VALUES (?, ?, ?, ?)",
			use(meeting.name),
			use(meeting.description),
			use(meeting.address),
			use(meeting.published),
			now;
		int temporary_id = 0;
		session << "SELECT id FROM meeting WHERE name = ?",
			into(temporary_id),
			use(meeting.name),
			now;
		meeting.id = temporary_id;
	}
}

handlers::Meeting Session::Get(int id) {
	Poco::Data::Session session = *m_instance->getSession();
	handlers::Meeting meeting;
	int temporary_id = 0;
	session << "SELECT id, name, description, address, published --\n\
		FROM meeting WHERE id = ?",
		into(temporary_id),
		into(meeting.name),
		into(meeting.description),
		into(meeting.address),
		into(meeting.published),
		use(id),
		now;
	meeting.id = temporary_id;
	return meeting;
}

std::vector<handlers::Meeting> Session::GetList() {
	Poco::Data::Session session = *m_instance->getSession();
	std::vector<handlers::Meeting> list;
	handlers::Meeting meeting;
	int temporary_id = 0;
	Poco::Data::Statement select(session);
	select << "SELECT id, name, description, address, published FROM meeting",
		into(temporary_id),
		into(meeting.name),
		into(meeting.description),
		into(meeting.address),
		into(meeting.published),
		range(0, 1);
	while (!select.done()) {
		select.execute();
		if (meeting.name != "") {
			meeting.id = temporary_id;
			list.push_back(meeting);
		}
	}
	return list;
}

void Session::Delete(int id) {
	Poco::Data::Session session = *m_instance->getSession();
	session << "DELETE FROM meeting WHERE id = ?", use(id), now;
}
