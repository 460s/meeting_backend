#include <sqlite_session_factory.hpp>
#include <Poco/Data/Session.h>

Poco::Data::Session SqliteSessionFactory::getInstance() {
    Poco::Data::Session session(SqliteSessionFactory::kDBDriver, SqliteSessionFactory::kPath2DB);
    return session;
}

