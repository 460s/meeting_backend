#pragma once
#include <Poco/Data/Session.h>

class SqliteSessionFactory {

public:
    static Poco::Data::Session getInstance();

private:
    static std::string kDBDriver;
    static std::string kPath2DB;
};
