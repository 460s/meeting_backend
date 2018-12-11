#include <meeting_DAO.hpp>
#include <sqlite_session_factory.hpp>
#include <logger_singleton.hpp>

namespace KW = Poco::Data::Keywords;
using domain::MeetingTuple;

void dao::MeetingDAO::Save(Meeting &meeting){

    auto logger = Logger::getInstance()->getLogger();
    auto session = SqliteSessionFactory::getInstance();


    if (meeting.id) {
        logger->information("try to update meeting with id="+std::to_string(meeting.id.value()));
        MeetingTuple mt = Meeting::MeetingStruct2Tuple(meeting);
        session << R"(UPDATE meeting
                    SET name = ?,
                        description = ?,
                        address = ?,
                        signup_description = ?,
                        signup_from_date = ?,
                        signup_to_date = ?,
                        from_date = ?,
                        to_date = ?,
                        published = ?
                    WHERE id = ?)",
                    KW::use(mt),
                    KW::use(meeting.id.value()),
                    KW::now;
        logger->information("Meeting was updated");
    } else {
        logger->information("try to insert new meeting");
        std::vector<MeetingTuple> v;
        v.push_back(Meeting::MeetingStruct2Tuple(meeting));
        session << "INSERT INTO meeting VALUES(NULL, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                    KW::use(v),
                    KW::now;
        int id = -1;
        session << "SELECT last_insert_rowid()", KW::into(id), KW::now;
        if (id != -1){
            meeting.id = id;
            logger->information("Inserted meeting with id="+std::to_string(id));
        } else {
            logger->error("Meeting wasn't inserted");
        }
    }
    session.close();
}

IDAO::MeetingList dao::MeetingDAO::GetList() {

    auto logger = Logger::getInstance()->getLogger();
    logger->information("Try to get all meetings");
    IDAO::MeetingList list;
    list.clear();
    auto session = SqliteSessionFactory::getInstance();
    MeetingTuple mp;
    int id = 0;

    Poco::Data::Statement select(session);
    select << R"(SELECT id,
                        name,
                        description,
                        address,
                        signup_description,
                        signup_from_date,
                        signup_to_date,
                        from_date,
                        to_date,
                        published
                FROM meeting)",
                KW::into(id),
                KW::into(mp),
                KW::range(0, 1);

    while (!select.done() && select.execute()) {
            list.push_back(Meeting::MeetingTuple2Struct(mp, id));
    }
    session.close();
    logger->information("Meetings list was selected");
    return list;
}

std::optional<Meeting> dao::MeetingDAO::Get(int id) {
    auto session = SqliteSessionFactory::getInstance();
    auto logger = Logger::getInstance()->getLogger();
    logger->information("Try to select single meeting");
    if (HasEntity(id)) {
        MeetingTuple mp;
        session << R"(SELECT
                        name,
                        description,
                        address,
                        signup_description,
                        signup_from_date,
                        signup_to_date,
                        from_date,
                        to_date,
                        published
                    FROM meeting
                    WHERE id = ?)",
                    KW::use(id),
                    KW::into(mp),
                    KW::now;
        session.close();
        logger->information("Meeting with id="+ std::to_string(id) +"was selected");
        return Meeting::MeetingTuple2Struct(mp, id);
    }
    session.close();
    return std::nullopt;
}

bool dao::MeetingDAO::Delete(int id) {
    auto logger = Logger::getInstance()->getLogger();
    logger->information("Try to delete single meeting with id="+std::to_string(id));
    auto session = SqliteSessionFactory::getInstance();
    if (HasEntity(id)) {
        session << "DELETE FROM meeting WHERE id = ?", KW::use(id), KW::now;
        session.close();
        logger->information("Meeting was deleted");
        return true;
    }
    logger->warning("This meeting isn't exist");
    session.close();
    return false;
}

bool dao::MeetingDAO::HasEntity(int id) {

    auto session = SqliteSessionFactory::getInstance();
    bool has_meeting = false;
    session << "SELECT COUNT(*) FROM meeting WHERE id = ?", KW::into(has_meeting), KW::use(id), KW::now;
    session.close();
    return has_meeting;
}



