#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "Poco/Data/Session.h"
#include <handlers.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <config.hpp>

namespace handlers {

struct Meeting {
	std::optional<int> id;
	std::string name;
	std::string description;
	std::string address;
	bool published;
};
//При расширении структуру нужно будет дописать только указадели подстановки в запросах,
// этот тип и процедуры конвертации структуры в него и обратно
// параметры подстановки добавлять будет уже не нужно
// Без id т.к. не во всех запросах он используется
typedef Poco::Tuple<std::string, std::string, std::string, bool> MeetingTuple;

using nlohmann::json;
namespace KW = Poco::Data::Keywords;

// сериализация (маршалинг)
void to_json(json &j, const Meeting &m) {
	j = json{
	    {"id", m.id.value()},
	    {"name", m.name},
	    {"description", m.description},
		{"address",     m.address},
	    {"published", m.published}};
}

// десериализация (анмаршалинг, распаковка)
void from_json(const json &j, Meeting &m) {
	j.at("name").get_to(m.name);
	j.at("description").get_to(m.description);
	j.at("address").get_to(m.address);
	j.at("published").get_to(m.published);
}

class Storage {
public:
	using MeetingList = std::vector<Meeting>;
	virtual void Save(Meeting &meeting) = 0;
	virtual MeetingList GetList() = 0;
	virtual std::optional<Meeting> Get(int id) = 0;
	virtual bool Delete(int id) = 0;
	virtual bool HasMeeting(int id, Poco::Data::Session &session) = 0;
	virtual ~Storage() {}

	MeetingTuple MeetingStruct2Tuple(const Meeting &m){
		MeetingTuple tuple;
		tuple.set<0>(m.name);
		tuple.set<1>(m.description);
		tuple.set<2>(m.address);
		tuple.set<3>(m.published);
		return tuple;
	}

	Meeting MeetingTuple2Struct(const MeetingTuple &mt, int id){
		Meeting meeting;
		meeting.id = id;
		meeting.name = mt.get<0>();
		meeting.description = mt.get<1>();
		meeting.address = mt.get<2>();
		meeting.published = mt.get<3>();
		return meeting;
	}
};

class MeetingDAO : public Storage {
public:
	void Save(Meeting &meeting) override {
        Poco::Data::Session session(config::kDBDriver, config::kPath2DB);

	    if (meeting.id) {
			MeetingTuple mt = MeetingStruct2Tuple(meeting);
			session << "UPDATE meeting SET name = ?, description = ?, address = ?, published = ? WHERE id = ?",
			            KW::use(mt),
                        KW::use(meeting.id.value()),
                        KW::now;
		} else {
            std::vector<MeetingTuple> v;
            v.push_back(MeetingStruct2Tuple(meeting));
			session << "INSERT INTO meeting VALUES(NULL, ?, ?, ?, ?)",
			            KW::use(v),
			            KW::now;
		}
	}

	Storage::MeetingList GetList() override {

	    Storage::MeetingList list;
        Poco::Data::Session session(config::kDBDriver, config::kPath2DB);

        MeetingTuple mp;
        int id;
        Poco::Data::Statement select(session);

        select << "SELECT id, name, description, address, published FROM meeting",
                    KW::into(id),
                    KW::into(mp),
                    KW::range(0, 1);

        while (!select.done()) {
            select.execute();
            list.push_back(MeetingTuple2Struct(mp, id));
        }

        session.close();
		return list;
	}

	std::optional<Meeting> Get(int id) override {

		Poco::Data::Session session(config::kDBDriver, config::kPath2DB);

		if (HasMeeting(id, session)) {
			MeetingTuple mp;
			session << R"(SELECT name, description, address, published FROM meeting WHERE id = ?)",
			                KW::use(id),
			                KW::into(mp),
                            KW::now;
            session.close();

			return MeetingTuple2Struct(mp, id);
		}
		session.close();
		return std::nullopt;
	}

	bool Delete(int id) override {

        Poco::Data::Session session(config::kDBDriver, config::kPath2DB);

		if (HasMeeting(id, session)) {
		    session << "DELETE FROM meeting WHERE id = ?", KW::use(id), KW::now;
			session.close();
			return true;
		}
		session.close();
		return false;
	}

    bool HasMeeting(int id, Poco::Data::Session &session) override {
        bool has_meeting;
        session << "SELECT COUNT(*) FROM meeting WHERE id = ?", KW::into(has_meeting), KW::use(id), KW::now;
        return has_meeting;
    }
};

Storage &GetMeetingDAO() {
	static MeetingDAO storage;
	return storage;
}

void UserMeetingList::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	auto &storage = GetMeetingDAO();
	nlohmann::json result = nlohmann::json::array();
	for (auto meeting : storage.GetList()) {
		result.push_back(meeting);
	}
	response.send() << result;
}

void UserMeetingCreate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    try {
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_CREATED);
        nlohmann::json j = nlohmann::json::parse(request.stream());
        auto &storage = GetMeetingDAO();
        Meeting meeting = j;
        storage.Save(meeting);
        response.send() << json(meeting);
    } catch (json::exception &e) {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST, "Ошибки в параметрах встречи");
        response.send();
    }
}

void UserMeetingRead::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	//response.setContentType("application/json");
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
    Meeting meeting;
	try {
        nlohmann::json j = nlohmann::json::parse(request.stream());
        meeting = j;
        std::cout << "er222er" << std::endl;
    } catch (json::exception &e) {
	    std::cout << "erer" << std::endl;
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST, "Ошибки в параметрах встречи");
        response.send();
        return;
    }
	auto &meetings = GetMeetingDAO();
	meeting.id = m_id;

    Poco::Data::Session session(config::kDBDriver, config::kPath2DB);
    bool hasMeeting = meetings.HasMeeting(m_id, session);
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
