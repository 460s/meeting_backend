#include <meetingDAO.hpp>
#include <sqlite_session_factory.hpp>

namespace KW = Poco::Data::Keywords;
using domain::MeetingTuple;

void dao::MeetingDAO::Save(Meeting &meeting){
    auto session = SqliteSessionFactory::getInstance();

    if (meeting.id) {
        MeetingTuple mt = Meeting::MeetingStruct2Tuple(meeting);
        session << "UPDATE meeting SET name = ?, description = ?, address = ?, published = ? WHERE id = ?",
                    KW::use(mt),
                    KW::use(meeting.id.value()),
                    KW::now;
    } else {
        std::vector<MeetingTuple> v;
        v.push_back(Meeting::MeetingStruct2Tuple(meeting));
        session << "INSERT INTO meeting VALUES(NULL, ?, ?, ?, ?)",
                    KW::use(v),
                    KW::now;
    }
}

IDAO::MeetingList dao::MeetingDAO::GetList() {
    IDAO::MeetingList list;
    auto session = SqliteSessionFactory::getInstance();
    MeetingTuple mp;
    int id;

    Poco::Data::Statement select(session);
    select << "SELECT id, name, description, address, published FROM meeting",
                KW::into(id),
                KW::into(mp),
                KW::range(0, 1);
    while (!select.done()) {
        select.execute();
        list.push_back(Meeting::MeetingTuple2Struct(mp, id));
    }
    session.close();
    return list;
}

std::optional<Meeting> dao::MeetingDAO::Get(int id) {
    auto session = SqliteSessionFactory::getInstance();
    if (HasEntity(id, session)) {
        MeetingTuple mp;
        session << "SELECT name, description, address, published FROM meeting WHERE id = ?",
                    KW::use(id),
                    KW::into(mp),
                    KW::now;
        session.close();

        return Meeting::MeetingTuple2Struct(mp, id);
    }
    session.close();
    return std::nullopt;
}

bool dao::MeetingDAO::Delete(int id) {
    auto session = SqliteSessionFactory::getInstance();
    if (HasEntity(id, session)) {
        session << "DELETE FROM meeting WHERE id = ?", KW::use(id), KW::now;
        session.close();
        return true;
    }
    session.close();
    return false;
}

bool dao::MeetingDAO::HasEntity(int id, Poco::Data::Session &session) {
    bool has_meeting;
    session << "SELECT COUNT(*) FROM meeting WHERE id = ?", KW::into(has_meeting), KW::use(id), KW::now;
    return has_meeting;
}



