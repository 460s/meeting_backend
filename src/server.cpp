#include <Poco/AutoPtr.h>
#include <Poco/FileChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/Logger.h>
#include <Poco/PatternFormatter.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Data/Session.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocketImpl.h>

#include <fstream>
#include <iostream>

#include <handlers/factory.hpp>
#include <loggers.hpp>
#include <server.hpp>
#include <sqlite.hpp>

using Poco::Data::Keywords::now;

int Server::main(const std::vector<std::string> &args) {
	Poco::Data::SQLite::Connector::registerConnector();

	Poco::AutoPtr<Poco::FileChannel> pChannel(new Poco::FileChannel);
	pChannel->setProperty("path", "sample.log");
	pChannel->setProperty("rotation", "never");
	pChannel->setProperty("archive", "timestamp");

	Poco::AutoPtr<Poco::PatternFormatter> pPF(new Poco::PatternFormatter);
	pPF->setProperty("pattern", "%Y-%m-%d %H:%M:%S %s: %t");
	Poco::AutoPtr<Poco::FormattingChannel> pFC(new Poco::FormattingChannel(pPF, pChannel));

	Poco::Logger::root().setChannel(pFC);

	if (std::find(args.begin(), args.end(), "init-db") != args.end()) {
		Poco::Data::Session session(sqlite::TYPE_SESSION, sqlite::DB_PATH);
		std::ifstream schema_stream(sqlite::SСHEMA_PATH);
		session << schema_stream.rdbuf(), now; // выполнить sql схему

		Poco::Logger &sql_logger = GetLoggers().getSqlLogger();
		sql_logger.information("initialize new database(SQLite, sample.db)");
		return 0;
	}

	auto *parameters = new Poco::Net::HTTPServerParams();
	parameters->setTimeout(10000);
	parameters->setMaxQueued(100);
	parameters->setMaxThreads(1);

	Poco::Net::SocketAddress socket_address("127.0.0.1:8080");
	Poco::Net::ServerSocket socket;
	socket.bind(socket_address, true, false);
	socket.listen(100);

	Poco::Net::HTTPServer server(new handlers::Factory(), socket, parameters);

	Poco::Logger &server_logger = GetLoggers().getServerLogger();

	server.start();
	server_logger.information("server started with socket address(127.0.0.1:8080)");
	waitForTerminationRequest();
	server.stopAll();
	server_logger.information("server closed");
	socket.close();
	server_logger.information("socket closed 127.0.0.1:8080");
	return 0;
}
