#pragma once

#include <Poco/Logger.h>

class Loggers {
public:
	Poco::Logger &getSqlLogger() const {
		return m_sql_logger;
	}
	Poco::Logger &getServerLogger() const {
		return m_server_logger;
	}
	Poco::Logger &getHttpResponseLogger() const {
		return m_http_response_logger;
	}
	Poco::Logger &getHttpRequestLogger() const {
		return m_http_request_logger;
	}
	Poco::Logger &getErrorLogger() const {
		return m_error_logger;
	}

private:
	Poco::Logger &m_sql_logger{Poco::Logger::get("SqlLogger")};
	Poco::Logger &m_server_logger{Poco::Logger::get("ServerLogger")};
	Poco::Logger &m_http_response_logger{Poco::Logger::get("HttpResponseLogger")};
	Poco::Logger &m_http_request_logger{Poco::Logger::get("HttpRequestLogger")};
	Poco::Logger &m_error_logger{Poco::Logger::get("ErrorLogger")};
};

inline Loggers &GetLoggers() {
	static Loggers instance;
	return instance;
}
