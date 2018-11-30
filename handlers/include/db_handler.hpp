#pragma once

#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"

class DBHandler {
public:
    static Poco::Data::Session &Session();
};