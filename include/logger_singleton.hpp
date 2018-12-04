#pragma once
#include "Poco/Logger.h"
#include <iostream>

class Logger{
public:
    static Logger *getInstance();
    Poco::Logger* getLogger();
    Logger(Logger const &) = delete;
    void operator=(Logger const &) = delete;
    ~Logger();

private:
    Logger();
    Poco::Logger* m_logger;
    static Logger *m_instance;
    constexpr static auto K_CONFIG = R"(
                                        logging.loggers.l1.name = infoLogger
                                        logging.loggers.l1.channel = c1
                                        logging.loggers.l1.level = information
                                        logging.channels.c1.class = FileChannel
                                        logging.channels.c1.path = sample.log
                                        logging.channels.c1.rotation = 10 M
                                        logging.channels.c1.formatter = f1
                                        logging.formatters.f1.class = PatternFormatter
		                                logging.formatters.f1.pattern = %d.%m.%Y %H:%M:%S [%p]: %t
		                                logging.formatters.f1.times = UTC
                                        )";
};