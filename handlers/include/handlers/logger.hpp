#pragma once

#include <Poco/Logger.h>
class Logger{
public:
    static Poco::Logger &GetLogger() {
        static Poco::Logger &logger = Poco::Logger::get("Logger");
        return logger;
    }
};

