#pragma once

#include <Meeting.hpp>
#include <Poco/Data/Session.h>

using namespace Poco::Data::Keywords;

class Session {
public:
	static Session *getInstance();
	std::shared_ptr<Poco::Data::Session> getSession();

	void Save(handlers::Meeting &meeting);
	std::vector<handlers::Meeting> GetList();
	handlers::Meeting Get(int id);
	void Delete(int id);
	bool Contain(int id);

	Session(Session const &) = delete;
	void operator=(Session const &) = delete;
	
	~Session();

private:
	Session();
	std::shared_ptr<Poco::Data::Session> m_session;
	static Session *m_instance;
};