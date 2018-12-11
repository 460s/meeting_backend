#include <Poco/AutoPtr.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/Logger.h>
#include <Poco/PatternFormatter.h>
#include <logger.hpp>

namespace meeting {

using Poco::AutoPtr;
using Poco::ConsoleChannel;
using Poco::FormattingChannel;
using Poco::PatternFormatter;

Poco::Logger &InitLogger() {
	AutoPtr<ConsoleChannel> console(new ConsoleChannel);
	AutoPtr<PatternFormatter> formatter(new PatternFormatter);
	formatter->setProperty("pattern", "%Y-%m-%d %H:%M:%S %s: %t");
	AutoPtr<FormattingChannel> channel(new FormattingChannel(formatter, console));
	Poco::Logger::root().setChannel(channel);
	Poco::Logger::root().setLevel("trace");
	return Poco::Logger::get("Logger");
}
Poco::Logger &GetLogger() {
	static Poco::Logger &logger{InitLogger()};
	return logger;
}
} // namespace meeting