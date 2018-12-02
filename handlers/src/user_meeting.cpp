#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <handlers.hpp>
#include <server.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <db_handler.hpp>

namespace handlers {
    using namespace Poco::Data::Keywords;

    struct Meeting {
        std::optional<int> id;
        std::string name;
        std::string description;
        std::string address;
        bool published;
    };

    using nlohmann::json;

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
        static Poco::Data::Session &Session();
        using MeetingList = std::vector<Meeting>;
        virtual Poco::Net::HTTPServerResponse::HTTPStatus Save(Meeting &meeting) = 0;
        virtual MeetingList GetList() = 0;
        virtual std::optional<Meeting> Get(int id) = 0;
        virtual bool Delete(int id) = 0;
        virtual ~Storage() {}
    };

    Poco::Data::Session &Storage::Session() {
        static Poco::Data::Session session("SQLite", "sample.db");
        return session;
    }

    class DBStorage : public Storage {
    public:
        Poco::Net::HTTPServerResponse::HTTPStatus Save(Meeting &meeting) override {
            auto &session = Session();
            if (meeting.id.has_value()) {
                if (IsNameQnique(meeting.name)) {
                    Poco::Data::Statement update(session);
                    update << "UPDATE meeting SET name = ?, description = ?, address = ?, published = ? WHERE id = ?",
                            use(meeting.name),
                            use(meeting.description),
                            use(meeting.address),
                            use(meeting.published),
                            use(meeting.id.value());
                    update.execute();
                    return Poco::Net::HTTPServerResponse::HTTP_OK;
                } else {
                    return Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST;
                }

            } else {
                if (IsNameQnique(meeting.name)) {
                    Poco::Data::Statement insert(session);
                    insert << "INSERT INTO meeting (name, description, address, published) VALUES(?, ?, ?, ?)",
                            use(meeting.name),
                            use(meeting.description),
                            use(meeting.address),
                            use(meeting.published);
                    insert.execute();
                    return Poco::Net::HTTPServerResponse::HTTP_OK;
                } else {
                    return Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST;
                }
            }
            return Poco::Net::HTTPServerResponse::HTTP_OK;
        }
        Storage::MeetingList GetList() override {
            Storage::MeetingList list;
            auto &session = Session();
            int meeting_id;
            Poco::Data::Statement select(session);
            Meeting meeting;
            select << "SELECT id, name, description, address, published FROM meeting",
                    into(meeting_id),
                    into(meeting.name),
                    into(meeting.description),
                    into(meeting.address),
                    into(meeting.published),
                    range(0, 1);

            while (!select.done()) {
                select.execute();
                if(meeting.name != "") {
                    meeting.id = meeting_id;
                    list.push_back(meeting);
                }
            }
            return list;
        }
        std::optional<Meeting> Get(int id) override {
            if (MeetingExist(id)) {
                Meeting meeting;
                auto &session = Session();
                Poco::Data::Statement select(session);
                select << "SELECT name, description, address, published FROM meeting WHERE id = ?",
                        into(meeting.name),
                        into(meeting.description),
                        into(meeting.address),
                        into(meeting.published),
                        use(id);
                select.execute();
                meeting.id = id;
                return meeting;
            }
            return std::optional<Meeting>();
        }
        bool Delete(int id) override {
            if (MeetingExist(id)) {
                auto &session = Session();
                Poco::Data::Statement deleteMeeting(session);
                deleteMeeting << "DELETE FROM meeting WHERE id = ?",
                        use(id);
                deleteMeeting.execute();
                return true;
            }
            return false;
        }

    private:
        bool MeetingExist(int id) const {
            auto &session = Session();
            Poco::Data::Statement find(session);
            Poco::Nullable<std::string> meeting_name;
            /*
             * Костыль
             */
            std::string name;
            session << "SELECT name FROM meeting WHERE id = ?",
                    into(meeting_name),
                    use(id),
                    now;
            return !meeting_name.isNull();

        }

        bool IsNameQnique(std::string meeting_name) const {
            auto &session = Session();
            Poco::Data::Statement find(session);
            int meeting_count = 0;
            find << "SELECT COUNT(id) FROM meeting WHERE name = ?",
                    into(meeting_count),
                    use(meeting_name);
            find.execute();
            return meeting_count == 0;
        }
    };

    Storage &GetStorage() {
        static DBStorage storage;
        return storage;
    }


    void UserMeetingList::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
        auto &storage = GetStorage();
        nlohmann::json result = nlohmann::json::array();
        for (auto meeting : storage.GetList()) {
            result.push_back(meeting);
        }
        response.send() << result;
        auto &session = DBHandler::Session();
    }

    void UserMeetingCreate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
        nlohmann::json j = nlohmann::json::parse(request.stream());
        auto &storage = GetStorage();
        Meeting meeting = j;
        Poco::Net::HTTPServerResponse::HTTPStatus status = storage.Save(meeting);
        response.setStatusAndReason(status);
        if (status == Poco::Net::HTTPServerResponse::HTTP_OK) {
            response.send() << json(meeting);
        }
        response.send();

    }

    void UserMeetingRead::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
        //response.setContentType("application/json");
        auto &meetings = GetStorage();
        auto meeting = meetings.Get(m_id);
        if (meeting.has_value()) {
            response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
            response.send() << json(meeting.value());
        }

        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
        response.send();
    }

    void UserMeetingUpdate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
        auto body = nlohmann::json::parse(request.stream());
        auto &meetings = GetStorage();
        Meeting meeting = body;
        meeting.id = m_id;

        Poco::Net::HTTPServerResponse::HTTPStatus status = meetings.Save(meeting);
        response.setStatusAndReason(status);
        if (status == Poco::Net::HTTPServerResponse::HTTP_OK) {
            response.send() << json(meeting);
        }
        response.send();

    }

    void UserMeetingDelete::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
        auto &meetings = GetStorage();
        if (meetings.Delete(m_id)) {
            response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT);
        } else {
            response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
        }
        response.send();
    }

} // namespace handlers