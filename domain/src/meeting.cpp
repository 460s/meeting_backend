#include <meeting.hpp>

domain::MeetingTuple domain::Meeting::MeetingStruct2Tuple(const Meeting &m) {
    MeetingTuple tuple;
    tuple.set<0>(m.name);
    tuple.set<1>(m.description);
    tuple.set<2>(m.address);
    tuple.set<3>(m.signup_description);
    tuple.set<4>(m.signup_from_date);
    tuple.set<5>(m.signup_to_date);
    tuple.set<6>(m.from_date);
    tuple.set<7>(m.to_date);
    tuple.set<8>(m.published);
    return tuple;
}

domain::Meeting domain::Meeting::MeetingTuple2Struct(const MeetingTuple &mt, int id) {
    Meeting meeting;
    meeting.id = id;
    meeting.name = mt.get<0>();
    meeting.description = mt.get<1>();
    meeting.address = mt.get<2>();
    meeting.signup_description = mt.get<3>();
    meeting.signup_from_date = mt.get<4>();
    meeting.signup_to_date = mt.get<5>();
    meeting.from_date = mt.get<6>();
    meeting.to_date = mt.get<7>();
    meeting.published = mt.get<8>();
    return meeting;
}

// сериализация (маршалинг)
void domain::to_json(json &j, const Meeting &m) {
    j = json{
            {"id",          m.id.value()},
            {"name",        m.name},
            {"description", m.description},
            {"address",     m.address},
            {"signup_description", m.signup_description},
            {"signup_from_date", m.signup_from_date},
            {"signup_to_date", m.signup_to_date},
            {"from_date", m.from_date},
            {"to_date", m.to_date},
            {"published",   m.published}};
}

// десериализация (анмаршалинг, распаковка)
void domain::from_json(const json &j, Meeting &m) {
    j.at("name").get_to(m.name);
    j.at("description").get_to(m.description);
    j.at("address").get_to(m.address);
    j.at("published").get_to(m.published);
    j.at("signup_description").get_to(m.signup_description);
    j.at("signup_from_date").get_to(m.signup_from_date);
    j.at("signup_to_date").get_to(m.signup_to_date);
    j.at("from_date").get_to(m.from_date);
    j.at("to_date").get_to(m.to_date);
}
