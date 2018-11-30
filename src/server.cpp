#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocketImpl.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>
#include <handlers/factory.hpp>
#include <iostream>
#include <server.hpp>
#include <config.hpp>

const std::string config::kDBDriver = "SQLite";
const std::string config::kPath2DB = "sample.db";

namespace {

using Poco::Net::Socket;

class ServerSocketImpl : public Poco::Net::ServerSocketImpl {
public:
	using Poco::Net::SocketImpl::init;
};

class ServerSocket : public Socket {
public:
	ServerSocket(const std::string &address, const int port) : Poco::Net::Socket(new ServerSocketImpl()) {
		const Poco::Net::SocketAddress socket_address(address + ":" + std::to_string(port));
		ServerSocketImpl *socket = static_cast<ServerSocketImpl *>(impl());
		socket->init(socket_address.af());
		socket->setReuseAddress(true);
		socket->setReusePort(false);
		socket->bind(socket_address, false);
		socket->listen();
	}
};

} // anonymous namespace

int Server::main(const std::vector<std::string> &args) {

	using Poco::Data::Keywords::now;
	using Poco::Data::Keywords::into;

	auto *parameters = new Poco::Net::HTTPServerParams();
	parameters->setTimeout(10000);
	parameters->setMaxQueued(100);
	parameters->setMaxThreads(1);

	const Poco::Net::ServerSocket socket(ServerSocket("127.0.0.1", 8080));

	Poco::Data::SQLite::Connector::registerConnector();
	Poco::Data::Session session(config::kDBDriver, config::kPath2DB);

	if (std::find(args.begin(), args.end(), "recreate") != args.end()){
		session << "DROP TABLE IF EXISTS meeting;", now;
		session << R"(CREATE TABLE meeting (
					id INTEGER PRIMARY KEY AUTOINCREMENT,
					name TEXT UNIQUE NOT NULL,
					description TEXT NOT NULL,
					address TEXT NOT NULL,
					published INTEGER NOT NULL);)",
					now;
	}

    session.close();

	Poco::Net::HTTPServer server(new handlers::Factory(), socket, parameters);

	server.start();
	waitForTerminationRequest();
	server.stopAll();

	return 0;
}
