#include <sqlite_session_factory.hpp>
#include <Poco/Data/Session.h>

Poco::Data::Session SqliteSessionFactory::getInstance() {
    Poco::Data::Session session(SqliteSessionFactory::K_DB_DRIVER, SqliteSessionFactory::K_PATH_TO_DB);
    return session;
}

