#pragma once
#include <Poco/Data/Session.h>

class SqliteSessionFactory {

public:
    static Poco::Data::Session getInstance();

private:
    constexpr static auto kDBDriver = "SQLite";
    constexpr static auto kPath2DB = "sample.db";
};
