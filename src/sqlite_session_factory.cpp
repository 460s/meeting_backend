#include <sqlite_session_factory.hpp>
#include <Poco/Data/Session.h>

std::string SqliteSessionFactory::kDBDriver  = "SQLite";
std::string SqliteSessionFactory::kPath2DB = "sample.db";

Poco::Data::Session SqliteSessionFactory::getInstance() {
    Poco::Data::Session session(SqliteSessionFactory::kDBDriver, SqliteSessionFactory::kPath2DB);
    return session;
}

