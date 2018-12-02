#include <meeting_DAO.hpp>
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
    list.clear();
    auto session = SqliteSessionFactory::getInstance();
    MeetingTuple mp;
    int id = 0;

    Poco::Data::Statement select(session);
    select << "SELECT id, name, description, address, published FROM meeting",
                KW::into(id),
                KW::into(mp),
                KW::range(0, 1);

    while (!select.done()) {
        if (select.execute())
            list.push_back(Meeting::MeetingTuple2Struct(mp, id));
    }
    session.close();
    return list;
}

std::optional<Meeting> dao::MeetingDAO::Get(int id) {
    auto session = SqliteSessionFactory::getInstance();
    if (HasEntity(id)) {
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
    if (HasEntity(id)) {
        session << "DELETE FROM meeting WHERE id = ?", KW::use(id), KW::now;
        session.close();
        return true;
    }
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



