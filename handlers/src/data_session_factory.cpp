#include <data_session_factory.hpp>


Poco::Data::Session DataSessionFactory::getInstance() {
    Poco::Data::Session session("SQLite", "meetings.db");
    return session;
}