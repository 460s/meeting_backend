#include <fstream>
#include <handlers/factory.hpp>
#include <iostream>
#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocketImpl.h>
#include <server.hpp>
#include <sqlite_session_factory.hpp>
#include <logger_singleton.hpp>

using Poco::Data::Keywords::now;

int Server::main(const std::vector<std::string> &args) {

    auto logger = Logger::getInstance()->getLogger();
    Poco::Message m;

    Poco::Data::SQLite::Connector::registerConnector();
    if (std::find(args.begin(), args.end(), "init-db") != args.end()) {
        logger->information("New schema was created");
        Poco::Data::SQLite::Connector::registerConnector();
        auto session = SqliteSessionFactory::getInstance();
        session << R"(DROP TABLE IF EXISTS meeting;
                      CREATE TABLE meeting (
                      id INTEGER PRIMARY KEY AUTOINCREMENT,
                      name TEXT UNIQUE NOT NULL,
                      description TEXT NOT NULL,
                      address TEXT NOT NULL,
                      published INTEGER NOT NULL
                    );)", now;
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
    logger->information("Server started on "+socket_address.toString());
    waitForTerminationRequest();
    server.stopAll();
    logger->information("Server stopped");
    socket.close();
    logger->information("Socket closed");
    return 0;
}