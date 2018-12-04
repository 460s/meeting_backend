#include <logger_singleton.hpp>
#include "Poco/SimpleFileChannel.h"
#include "Poco/AutoPtr.h"
#include <Poco/PatternFormatter.h>
#include <sstream>
#include "Poco/Util/LoggingConfigurator.h"
#include "Poco/Util/PropertyFileConfiguration.h"

using Poco::SimpleFileChannel;
using Poco::AutoPtr;
using Poco::PatternFormatter;
using Poco::Util::PropertyFileConfiguration;
using Poco::Util::LoggingConfigurator;
Logger *Logger::m_instance = nullptr;

Logger::Logger() {

    std::istringstream istr(this->K_CONFIG);
    AutoPtr<PropertyFileConfiguration> pConfig = new PropertyFileConfiguration(istr);
    LoggingConfigurator configurator;
    configurator.configure(pConfig);

    this->m_logger = &Poco::Logger::get("infoLogger");
}

Logger *Logger::getInstance() {

    if (!m_instance) {
        m_instance = new Logger();
    }
    return m_instance;
}

Poco::Logger* Logger::getLogger() {
    return this->m_logger;
}