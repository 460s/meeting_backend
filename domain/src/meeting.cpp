#include <meeting.hpp>

domain::MeetingTuple domain::Meeting::MeetingStruct2Tuple(const Meeting &m) {
    MeetingTuple tuple;
    tuple.set<0>(m.name);
    tuple.set<1>(m.description);
    tuple.set<2>(m.address);
    tuple.set<3>(m.published);
    return tuple;
}

domain::Meeting domain::Meeting::MeetingTuple2Struct(const MeetingTuple &mt, int id) {
    Meeting meeting;
    meeting.id = id;
    meeting.name = mt.get<0>();
    meeting.description = mt.get<1>();
    meeting.address = mt.get<2>();
    meeting.published = mt.get<3>();
    return meeting;
}

// сериализация (маршалинг)
void domain::to_json(json &j, const Meeting &m) {
    j = json{
            {"id",          m.id.value()},
            {"name",        m.name},
            {"description", m.description},
            {"address",     m.address},
            {"published",   m.published}};
}

// десериализация (анмаршалинг, распаковка)
void domain::from_json(const json &j, Meeting &m) {
    j.at("name").get_to(m.name);
    j.at("description").get_to(m.description);
    j.at("address").get_to(m.address);
    j.at("published").get_to(m.published);
}
