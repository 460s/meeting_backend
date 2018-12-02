#pragma once
#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"

class DataSessionFactory {

public:
    static Poco::Data::Session getInstance();
};