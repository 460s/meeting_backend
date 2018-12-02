#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <handlers.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <Poco/URI.h>
#include <optional>
#include <data_session_factory.hpp>

namespace handlers {

using namespace Poco::Data::Keywords;

struct Meeting {
	std::optional<int> id;
	std::string name;
	std::string description;
	std::string address;
	/*std::string signup_description;
	int signup_from_date;
	int signup_to_date;
	int from_date;
	int to_date;*/
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
		/*{"signup_description", m.signup_description},
		{"signup_from_date", m.signup_from_date},
		{"signup_to_date", m.signup_to_date},
		{"from_date", m.from_date},
		{"to_date", m.to_date},*/
		{"published", m.published}
	};
}

// десериализация (анмаршалинг, распаковка)
void from_json(const json &j, Meeting &m) {
	j.at("name").get_to(m.name);
	j.at("description").get_to(m.description);
	j.at("address").get_to(m.address);
	/*j.at("signup_description").get_to(m.signup_description);
	j.at("signup_from_date").get_to(m.signup_from_date);
	j.at("signup_to_date").get_to(m.signup_to_date);
	j.at("from_date").get_to(m.from_date);
	j.at("to_date").get_to(m.to_date);*/
	j.at("published").get_to(m.published);
}

class Storage {
public:
	using MeetingList = std::vector<Meeting>;
	virtual void Save(Meeting &meeting) = 0;
	virtual void Delete(int id) = 0;
	virtual Meeting Get(int id) = 0;
	virtual MeetingList GetList() = 0;
	virtual bool Exists(int id) = 0;
	virtual ~Storage() {}
};

class DBStorage : public Storage {
public:
	void Save(Meeting &meeting) override {
		auto session = DataSessionFactory::getInstance();
		int id;
            if (meeting.id.has_value()) {
				Poco::Data::Statement update(session);
				update << "UPDATE meeting SET name = ?, description = ?, address = ?, published = ? WHERE id = ?",
						use(meeting.name),
						use(meeting.description),
						use(meeting.address),
						use(meeting.published),
						use(meeting.id.value());
				update.execute();
			} else {
				Poco::Data::Statement insert(session);
				insert << "INSERT INTO meeting (name, description, address, published) VALUES(?, ?, ?, ?)",
						use(meeting.name),
						use(meeting.description),
						use(meeting.address),
						use(meeting.published);
				insert.execute();

				Poco::Data::Statement select(session);
				select << "SELECT last_insert_rowid() as id FROM meeting LIMIT 1",
						into(id);
				select.execute();
				meeting.id = id;
			}
		session.close();
	}

	void Delete(int id) override {
		auto session = DataSessionFactory::getInstance();
		Poco::Data::Statement deleteMeeting(session);
		deleteMeeting << "DELETE FROM meeting WHERE id = ?",
				use(id);
		deleteMeeting.execute();
		session.close();
	}

	Meeting Get(int id) override {

		Meeting meeting;
		auto session = DataSessionFactory::getInstance();
		Poco::Data::Statement select(session);
		select << "SELECT name, description, address, published FROM meeting WHERE id = ?",
				into(meeting.name),
				into(meeting.description),
				into(meeting.address),
				into(meeting.published),
				use(id);
		select.execute();
		meeting.id = id;
		session.close();
		return meeting;
            
	}

	Storage::MeetingList GetList() override {
		
		auto session = DataSessionFactory::getInstance();
		Poco::Data::Statement select(session);
		Storage::MeetingList list;
		Meeting meeting;
		int row_id = 0;
		
		select << "SELECT id, name, description, address, published FROM meeting",
				into(row_id),
				into(meeting.name),
				into(meeting.description),
				into(meeting.address),
				into(meeting.published),
				range(0, 1);

		while (!select.done()) {
			select.execute();
			if(row_id) { //чтобы не получать непонятно что, если таблица пустая
				meeting.id = row_id;
				list.push_back(meeting);
			}
			
		}
		session.close();
		
		return list;
	}

	bool Exists(int id) override {
		auto session = DataSessionFactory::getInstance();
		Poco::Data::Statement find(session);
		int meeting_count = 0;
		find << "SELECT COUNT(*) FROM meeting WHERE id = ?",
				into(meeting_count),
				use(id);
		find.execute();
		session.close();
		return meeting_count != 0;
	}

};

Storage &GetStorage() {
	static DBStorage storage;
	return storage;
}
// Вывод сообщений в консоль
void console_log(std::string message){
	std::cout << message << std::endl;
}

void UserMeetingList::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	auto &storage = GetStorage();
	nlohmann::json result = nlohmann::json::array();
	for (auto meeting : storage.GetList()) {
		result.push_back(meeting);
	}
	
	response.send() << result;

}

void UserMeetingGet::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

	auto &storage = GetStorage();

	if (storage.Exists(m_id)) {
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
		response.send() << json(storage.Get(m_id));
	} else {
		console_log("Trying to get meeting #"+std::to_string(m_id)+":");
		console_log("Not found.");
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
		response.send();
	}

}

void UserMeetingCreate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	
	nlohmann::json j = nlohmann::json::parse(request.stream());
	auto &storage = GetStorage();
	Meeting meeting;
	//"Грубо" ловим ошибку, если нам прислали не те данные (нехватает поля, не тот тип поля и т.д.)
	try {
		Meeting meeting = j;
		storage.Save(meeting);
		
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
		response.send() << json(meeting);
		
		console_log("Created new meeting:");
		console_log(json(meeting).dump());
	} catch(json::exception e) {
		console_log("Trying to create meeting. Validation error");
		console_log(j.dump());
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
		response.send();
	}
}

void UserMeetingUpdate::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	nlohmann::json j = nlohmann::json::parse(request.stream());
	auto &storage = GetStorage();
	Meeting meeting;

	if (storage.Exists(m_id)) {
		//"Грубо" ловим ошибку, если нам прислали не те данные (нехватает поля, не тот тип поля и т.д.)
		try {
			Meeting meeting = j;
			meeting.id = m_id;
			storage.Save(meeting);
			response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
			response.send() << json(meeting);
			console_log("Updated  meeting #"+std::to_string(m_id)+":");
			console_log(j.dump());
		} catch(json::exception e) {
			console_log("Trying to update meeting. Validation error");
			console_log(j.dump());
			response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
			response.send();
			return;
		}
	} else {
		console_log("Trying to update meeting #"+std::to_string(m_id)+":");
		console_log("Not found.");
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
		response.send();
	}

}


void UserMeetingDelete::HandleRestRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

	auto &storage = GetStorage();

	if (storage.Exists(m_id)) {
		storage.Delete(m_id);
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT);
		response.send();
		console_log("Deleted  meeting #"+std::to_string(m_id));
	} else {
		console_log("Trying to delete meeting #"+std::to_string(m_id)+":");
		console_log("Not found.");
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
		response.send();
	}
}


} // namespace handlers
