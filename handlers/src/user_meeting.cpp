#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "Poco/Data/Session.h"
#include <handlers.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <sqlite_session_factory.hpp>
#include <meetingDAO.hpp>

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
    for (auto &meeting : storage.GetList()) {
        result.push_back(meeting);
    }
    response.send() << result;
}

void UserMeetingCreate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    try {
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_CREATED);
        domain::Meeting meeting = nlohmann::json::parse(request.stream());
        auto &storage = GetMeetingDAO();
        storage.Save(meeting);
        response.send() << json(meeting);
    } catch (json::exception &e) {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST, "Ошибки в параметрах встречи");
        response.send();
    }
}

void UserMeetingRead::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    auto &meetings = GetMeetingDAO();
    auto meeting = meetings.Get(m_id);
    if (meeting.has_value()) {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
        response.send() << json(meeting.value());
        return;
    }

    response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
    response.send();
}

void UserMeetingUpdate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
    domain::Meeting meeting;
    try {
        nlohmann::json j = nlohmann::json::parse(request.stream());
        meeting = j;
    } catch (json::exception &e) {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST, "Ошибки в параметрах встречи");
        response.send();
        return;
    }
    auto &meetings = GetMeetingDAO();
    meeting.id = m_id;

    auto session = SqliteSessionFactory::getInstance();
    bool hasMeeting = meetings.HasEntity(m_id, session);
    session.close();
    if (hasMeeting)
        meetings.Save(meeting);
    else {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, "Такая встреча отсутствует");
        response.send();
        return;
    }
    response.send() << json(meeting);
}

void UserMeetingDelete::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    auto &meetings = GetMeetingDAO();
    if (meetings.Delete(m_id)) {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT);
    } else {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
    }
    response.send();
}

} // namespace handlers
