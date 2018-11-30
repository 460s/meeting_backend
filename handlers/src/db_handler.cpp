#include <db_handler.hpp>

Poco::Data::Session &DBHandler::Session() {
    static Poco::Data::Session session("SQLite", "sample.db");
    return session;

}
