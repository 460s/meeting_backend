#include <fstream>
#include <handlers/factory.hpp>
#include <iostream>
#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocketImpl.h>
#include "Poco/Logger.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/AutoPtr.h"
#include <server.hpp>
#include <sqlite.hpp>
#include <logger.hpp>

using Poco::Data::Keywords::now;
using Poco::Logger;
using Poco::SimpleFileChannel;
using Poco::FormattingChannel;
using Poco::PatternFormatter;
using Poco::AutoPtr;

int Server::main(const std::vector<std::string> &args) {
	AutoPtr<SimpleFileChannel> pChannel(new SimpleFileChannel);
	pChannel->setProperty("path", logger::LOG_PATH);
	pChannel->setProperty("rotation", logger::ROTATION);

	AutoPtr<PatternFormatter> pPF(new PatternFormatter);
	pPF->setProperty("pattern", logger::FORMAT_PATTERN);

	AutoPtr<FormattingChannel> pFC(new FormattingChannel(pPF, pChannel));
	
	Logger::root().setChannel(pFC);
	Logger& logger = Logger::get("SampleLogger");
	logger.information("Starting server");
	
	Poco::Data::SQLite::Connector::registerConnector();
	if (std::find(args.begin(), args.end(), "init-db") != args.end()) {
		Poco::Data::Session session(sqlite::TYPE_SESSION, sqlite::DB_PATH);
		std::ifstream schema_stream(sqlite::SСHEMA_PATH);
		session << schema_stream.rdbuf(), now; // выполнить sql схему
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

	server.start();
	waitForTerminationRequest();
	server.stopAll();
	socket.close();
	logger.information("Closing server");
	return 0;
}
