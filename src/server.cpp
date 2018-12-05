#include <fstream>
#include <handlers/factory.hpp>
#include <iostream>
#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocketImpl.h>
#include <server.hpp>
#include <sqlite.hpp>
#include <Poco/AutoPtr.h>
#include <Poco/FileChannel.h>
#include <handlers/logger.hpp>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>

using Poco::Data::Keywords::now;

int Server::main(const std::vector<std::string> &args) {
    Poco::Data::SQLite::Connector::registerConnector();
    Poco::AutoPtr<Poco::FileChannel> pChannel(new Poco::FileChannel);
    pChannel->setProperty("path", "sample.log");
    pChannel->setProperty("rotation", "1024 K");
    Poco::AutoPtr<Poco::PatternFormatter> pPF(new Poco::PatternFormatter);
    pPF->setProperty("pattern", "%Y-%m-%d %H:%M:%S %s %p: %t");
    Poco::AutoPtr<Poco::FormattingChannel> pFC(new Poco::FormattingChannel(pPF, pChannel));
    Poco::Logger::root().setChannel(pFC);
    Poco::Logger &logger = Logger::GetLogger();
    logger.information("Initialize server");

    if (std::find(args.begin(), args.end(), "init-db") != args.end()) {
        Poco::Data::Session session(sqlite::TYPE_SESSION, sqlite::DB_PATH);
        std::ifstream schema_stream(sqlite::SСHEMA_PATH);
        session << schema_stream.rdbuf(), now; // выполнить sql схему
        logger.information("Initialize database");
        return 0;
    }

    auto *parameters = new Poco::Net::HTTPServerParams();
    parameters->setTimeout(10000);
    parameters->setMaxQueued(100);
    parameters->setMaxThreads(1);

    Poco::Net::SocketAddress socket_address("127.0.0.1:8080");
    logger.information("Start server at 127.0.0.1:8080");
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