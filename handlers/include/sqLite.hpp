#pragma once

#include <meeting.hpp>
#include <Poco/Data/Session.h>

using namespace Poco::Data::Keywords;

class SqLite {
public:
	static SqLite *getInstance();
	std::shared_ptr<Poco::Data::Session> getSession();

	void Save(handlers::Meeting &meeting);
	std::vector<handlers::Meeting> GetList();
	handlers::Meeting Get(int id);
	void Delete(int id);
	bool Contain(int id);

	SqLite(SqLite const &) = delete;
	void operator=(SqLite const &) = delete;

	~SqLite();

private:
	SqLite();
	std::shared_ptr<Poco::Data::Session> m_session;
	static SqLite *m_instance;
};
