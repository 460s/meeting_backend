#include <Poco/Data/Session.h>

#include <Meeting.hpp>
#include <SqLite.hpp>

using namespace Poco::Data::Keywords;

struct SingleSession {
	std::unique_ptr<Poco::Data::Session> single_session;
	SingleSession() : single_session(new Poco::Data::Session("SQLite", "sample.db")) {
	}
};

SqLite *SqLite::m_instance = NULL;

SqLite::SqLite() {
	std::shared_ptr<SingleSession> unique_session(std::make_shared<SingleSession>());
	std::shared_ptr<Poco::Data::Session> session(unique_session, unique_session->single_session.get());
	m_session.swap(session);
}

SqLite *SqLite::getInstance() {
	if (!m_instance) {
		m_instance = new SqLite();
		Poco::Data::Session session = *m_instance->getSession();
		session << R"(CREATE TABLE IF NOT EXISTS meeting(
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			name TEXT UNIQUE NOT NULL,
			description TEXT NOT NULL,
			address TEXT NOT NULL,
			published INTEGER NOT NULL))",
			now;
	}
	return m_instance;
}

std::shared_ptr<Poco::Data::Session> SqLite::getSession() {
	return m_session;
}

bool SqLite::Contain(int id) {
	Poco::Data::Session session = *m_instance->getSession();
	Poco::Nullable<std::string> name;
	session << "SELECT name FROM meeting WHERE id = ?",
		into(name),
		use(id),
		now;
	return !name.isNull();
}

void SqLite::Save(handlers::Meeting &meeting) {
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

handlers::Meeting SqLite::Get(int id) {
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

std::vector<handlers::Meeting> SqLite::GetList() {
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

void SqLite::Delete(int id) {
	Poco::Data::Session session = *m_instance->getSession();
	session << "DELETE FROM meeting WHERE id = ?", use(id), now;
}
