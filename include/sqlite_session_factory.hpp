#pragma once
#include <Poco/Data/Session.h>

class SqliteSessionFactory {

public:
    static Poco::Data::Session getInstance();
    constexpr static auto K_SСHEMA_PATH = "/etc/schema.sql";
private:
    constexpr static auto K_DB_DRIVER = "SQLite";
    constexpr static auto K_PATH_TO_DB = "sample.db";
};
