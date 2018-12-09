#include <iostream>
#include <handlers.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <sqlite.hpp>
#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <handlers/logger.hpp>

namespace handlers {

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
                {"address", m.address},
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
        virtual ~Storage() {}
    };

    class MapStorage : public Storage {
    public:
        void Save(Meeting &meeting) override {
            if (meeting.id.has_value()) {
                m_meetings[meeting.id.value()] = meeting;
            } else {
                int id = m_meetings.size();
                meeting.id = id;
                m_meetings[id] = meeting;
            }
        }
        Storage::MeetingList GetList() override {
            Storage::MeetingList list;
            for (auto [id, meeting] : m_meetings) {
                list.push_back(meeting);
            }
            return list;
        }
        std::optional<Meeting> Get(int id) override {
            if (MeetingInMap(id)) {
                return m_meetings[id];
            }
            return std::optional<Meeting>();
        }
        bool Delete(int id) override {
            if (MeetingInMap(id)) {
                m_meetings.erase(id);
                return true;
            }
            return false;
        }

    private:
        using MeetingMap = std::map<int, Meeting>;
        MeetingMap m_meetings;

        bool MeetingInMap(int id) const {
            auto meeting_ptr = m_meetings.find(id);
            return meeting_ptr != m_meetings.end();
        }
    };

    using Poco::Data::Keywords::into;
    using Poco::Data::Keywords::now;
    using Poco::Data::Keywords::range;
    using Poco::Data::Keywords::use;
    using Poco::Data::Statement;

    class SqliteStorage : public Storage {
    public:
        std::mutex m_mutex;

        void Save(Meeting &meeting) override {
            std::lock_guard<std::mutex> lock{m_mutex};
            Poco::Logger &logger = Logger::GetLogger();
            if (meeting.id.has_value()) {
                Statement update(m_session);
                auto published = b2i(meeting.published);
                update << "UPDATE meeting SET "
                          "name=?, description=?, address=?, published=? "
                          "WHERE id=?",
                        use(meeting.name),
                        use(meeting.description),
                        use(meeting.address),
                        use(published),
                        use(meeting.id.value()),
                        now;
                std::string message = "Update meeting with id " + std::to_string(meeting.id.value());
                logger.information(message);
            } else {
                Statement insert(m_session);
                int published = b2i(meeting.published);
                insert << "INSERT INTO meeting (name, description, address, published) VALUES(?, ?, ?, ?)",
                        use(meeting.name),
                        use(meeting.description),
                        use(meeting.address),
                        use(published),
                        now;

                Statement select(m_session);
                int id = 0;
                select << "SELECT last_insert_rowid()", into(id), now;
                meeting.id = id;

                std::string message = "Save meeting with id " + std::to_string(meeting.id.value());
                logger.information(message);
            }
        }

        Storage::MeetingList GetList() override {
            std::lock_guard<std::mutex> lock{m_mutex};
            Storage::MeetingList list;
            Meeting meeting;
            Statement select(m_session);
            select << "SELECT id, name, description, address, published FROM meeting",
                    into(meeting.id.emplace()),
                    into(meeting.name),
                    into(meeting.description),
                    into(meeting.address),
                    into(meeting.published),
                    range(0, 1); //  iterate over result set one row at a time

            while (!select.done() && select.execute()) {
                list.push_back(meeting);
            }
            Poco::Logger &logger = Logger::GetLogger();
            logger.information("Get meeting list");
            return list;
        }

        std::optional<Meeting> Get(int id) override {
            std::lock_guard<std::mutex> lock{m_mutex};
            int cnt = 0;
            m_session << "SELECT COUNT(*) FROM meeting WHERE id=?", use(id), into(cnt), now;
            Poco::Logger &logger = Logger::GetLogger();
            std::string message = "Get meeting with id " + std::to_string(id);
            logger.information(message);
            if (cnt > 0) {
                Meeting meeting;
                Statement select(m_session);
                int tmp_id = 0;
                select << "SELECT id, name, description, address, published FROM meeting WHERE id=?",
                        use(id),
                        into(tmp_id),
                        into(meeting.name),
                        into(meeting.description),
                        into(meeting.address),
                        into(meeting.published),
                        now;
                meeting.id = tmp_id;
                return meeting;
            }
            return std::nullopt;
        }

        bool Delete(int id) override {
            std::lock_guard<std::mutex> lock{m_mutex};
            m_session << "DELETE FROM meeting WHERE id=?", use(id), now;
            Poco::Logger &logger = Logger::GetLogger();
            std::string message = "Delete meeting with id " + std::to_string(id);
            logger.information(message);
            return true;
        }

    private:

        Poco::Data::Session m_session{sqlite::TYPE_SESSION, sqlite::DB_PATH};

        int b2i(bool b) {
            return b ? 1 : 0;
        }
    };

    Storage &GetStorage() {
        static SqliteStorage storage;
        return storage;
    }

    void UserMeetingList::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
        auto &storage = GetStorage();
        Poco::Logger &logger = Logger::GetLogger();
        logger.information("Query user meeting list");
        nlohmann::json result = nlohmann::json::array();
        for (auto meeting : storage.GetList()) {
            result.push_back(meeting);
        }
        response.send() << result;
    }

    void UserMeetingCreate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
        nlohmann::json j = nlohmann::json::parse(request.stream());
        auto &storage = GetStorage();
        Poco::Logger &logger = Logger::GetLogger();
        logger.information("Query user meeting create");
        Meeting meeting = j;
        storage.Save(meeting);

        response.send() << json(meeting);
    }

    void UserMeetingRead::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
        auto &meetings = GetStorage();
        auto meeting = meetings.Get(m_id);
        Poco::Logger &logger = Logger::GetLogger();
        logger.information("Query user meeting get");
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
        auto body = nlohmann::json::parse(request.stream());
        auto &meetings = GetStorage();
        Poco::Logger &logger = Logger::GetLogger();
        logger.information("Query user meeting update");
        Meeting meeting = body;
        meeting.id = m_id;
        meetings.Save(meeting);

        response.send() << json(meeting);
    }

    void UserMeetingDelete::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
        auto &meetings = GetStorage();
        Poco::Logger &logger = Logger::GetLogger();
        logger.information("Query user meeting delete");
        if (meetings.Delete(m_id)) {
            response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT);
        } else {
            response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
        }
        response.send();
    }

} // namespace handlers