#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "Poco/Data/Session.h"
#include <handlers.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <sqlite_session_factory.hpp>
#include <meeting_DAO.hpp>

using nlohmann::json;
using domain::MeetingTuple;
namespace KW = Poco::Data::Keywords;

namespace handlers {

IDAO &GetMeetingDAO() {
    static dao::MeetingDAO storage;
    return storage;
}

void UserMeetingList::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
    auto &storage = GetMeetingDAO();
    nlohmann::json result = nlohmann::json::array();
    for (const auto &meeting : storage.GetList()) {
        result.push_back(meeting);
    }
    Logger::getInstance()->getLogger()->information("Response with list of meetings was successfully send");
    response.send() << result;
}

void UserMeetingCreate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    response.setStatus(Poco::Net::HTTPServerResponse::HTTP_CREATED);
    domain::Meeting meeting = nlohmann::json::parse(request.stream());
    auto &storage = GetMeetingDAO();
    storage.Save(meeting);
    Logger::getInstance()->getLogger()->information("Meeting was created, send response with code 200");
    response.send() << json(meeting);
}

void UserMeetingRead::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    auto &meetings = GetMeetingDAO();
    auto meeting = meetings.Get(m_id);
    if (meeting.has_value()) {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
        response.send() << json(meeting.value());
        Logger::getInstance()->getLogger()->information("Get single meeting, send response with code 200");
        return;
    }

    response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
    Logger::getInstance()->getLogger()->warning("Meeting not found, send response with code 404");
    response.send();
}

void UserMeetingUpdate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
    domain::Meeting meeting;

    nlohmann::json j = nlohmann::json::parse(request.stream());
    meeting = j;
    auto &meetings = GetMeetingDAO();
    meeting.id = m_id;

    if (!meetings.HasEntity(m_id)){
        Logger::getInstance()->getLogger()->warning("Updated meeting doesn't exist, send response with code 404");
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, "Такая встреча отсутствует");
        response.send();
        return;
    }
    meetings.Save(meeting);
    Logger::getInstance()->getLogger()->information("Meeting was updated, send response with code 200");
    response.send() << json(meeting);
}

void UserMeetingDelete::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    auto &meetings = GetMeetingDAO();
    if (meetings.Delete(m_id)) {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT);
        Logger::getInstance()->getLogger()->information("Meeting was deleted, send response with code 204");
    } else {
        Logger::getInstance()->getLogger()->warning("Meeting doesn't exist, send response with code 404");
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
    }
    response.send();
}

} // namespace handlers
