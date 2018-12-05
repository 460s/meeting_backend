#include <fstream>
#include <handlers/factory.hpp>
#include <iostream>
#include <logger.hpp>
#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Data/SQLite/Utility.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocketImpl.h>
#include <server.hpp>
#include <sqlite.hpp>

using Poco::Data::Keywords::now;

int Server::main(const std::vector<std::string> &args) {
	meeting::GetLogger().information("Start Server");

	Poco::Data::SQLite::Connector::registerConnector();
	Poco::Data::SQLite::Utility::setThreadMode(Poco::Data::SQLite::Utility::THREAD_MODE_SINGLE);
	if (std::find(args.begin(), args.end(), "init-db") != args.end()) {
		Poco::Data::Session session(sqlite::TYPE_SESSION, sqlite::DB_PATH);
		std::ifstream schema_stream(sqlite::SСHEMA_PATH);
		session << schema_stream.rdbuf(), now; // выполнить sql схему
		return 0;
	}

	auto *parameters = new Poco::Net::HTTPServerParams();
	parameters->setTimeout(10000);
	parameters->setMaxQueued(100);
	parameters->setMaxThreads(2);

	Poco::Net::SocketAddress socket_address("127.0.0.1:8080");
	Poco::Net::ServerSocket socket;
	socket.bind(socket_address, true, false);
	socket.listen(100);

	Poco::Net::HTTPServer server(new handlers::Factory(), socket, parameters);

	server.start();
	waitForTerminationRequest();
	server.stopAll();
	socket.close();
	return 0;
}
