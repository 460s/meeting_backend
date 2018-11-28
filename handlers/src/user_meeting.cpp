#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <handlers.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <Poco/URI.h>

namespace handlers {

struct Meeting {
	int id;
	std::string name;
	std::string description;
	/*std::string address;
	std::string signup_description;
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
	    {"id", m.id},
		{"name", m.name},
		{"description", m.description},
		/*{"address", m.address},
		{"signup_description", m.signup_description},
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
	/*j.at("address").get_to(m.address);
	j.at("signup_description").get_to(m.signup_description);
	j.at("signup_from_date").get_to(m.signup_from_date);
	j.at("signup_to_date").get_to(m.signup_to_date);
	j.at("from_date").get_to(m.from_date);*/
	j.at("published").get_to(m.published);
}

class Storage {
public:
	using MeetingList = std::vector<Meeting>;
	virtual void Create(Meeting &meeting) = 0;
	virtual void Update(Meeting &meeting, int id) = 0;
	virtual void Delete(int id) = 0;
	virtual Meeting Get(int id) = 0;
	virtual MeetingList GetList() = 0;
	virtual bool Exists(int id) = 0;
	virtual ~Storage() {}
};

class MapStorage : public Storage {
public:
	void Create(Meeting &meeting) override {
		meeting.id = m_uid;
		m_meetings[m_uid] = meeting;
		m_uid++;
	}
	void Update(Meeting &meeting, int id = 0) override {
		meeting.id = id;
		m_meetings[id] = meeting;
	}
	void Delete(int id) override {
		m_meetings.erase(id);
	}
	Meeting Get(int id) override {
		return m_meetings[id];
	}
	Storage::MeetingList GetList() override {
		Storage::MeetingList list;
		for (auto [id, meeting] : m_meetings) {
			list.push_back(meeting);
		}
		return list;
	}
	bool Exists(int id) override {
		auto search_result = m_meetings.find(id);
		return search_result != m_meetings.end();
	}

private:
	using MeetingMap = std::map<int, Meeting>;
	MeetingMap m_meetings;
	int m_uid = 1;
};

Storage &GetStorage() {
	static MapStorage storage;
	return storage;
}
// Вывод сообщений в консоль
void console_log(std::string message){
	std::cout << message << std::endl;
}

void UserMeetingList::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
	auto &storage = GetStorage();
	nlohmann::json result = nlohmann::json::array();
	for (auto meeting : storage.GetList()) {
		result.push_back(meeting);
	}
	response.send() << result;

}

void UserMeetingGet::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

	auto &storage = GetStorage();

	// Получем id
	const auto uri = request.getURI();
	Poco::URI uri_parser(uri);
	std::vector < std::string > segments;
	uri_parser.getPathSegments(segments);
	int id = std::stoi(segments[2]);

	if (storage.Exists(id)) {
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
		response.send() << json(storage.Get(id));
	} else {
		console_log("Trying to get meeting #"+std::to_string(id)+":");
		console_log("Not found.");
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
		response.send();
	}

}

void UserMeetingCreate::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	
	nlohmann::json j = nlohmann::json::parse(request.stream());
	auto &storage = GetStorage();
	Meeting meeting;
	//"Грубо" ловим ошибку, если нам прислали не те данные (нехватает поля, не тот тип поля и т.д.)
	try {
		Meeting meeting = j;
		storage.Create(meeting);
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
		response.send() << json(meeting);
		console_log("Created new meeting:");
		console_log(json(meeting).dump());
	} catch(...) {
		console_log("Trying to create meeting. Validation error");
		console_log(j.dump());
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
		response.send();
	}
}

void UserMeetingUpdate::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
	nlohmann::json j = nlohmann::json::parse(request.stream());
	auto &storage = GetStorage();
	Meeting meeting;

	// Получем id
	const auto uri = request.getURI();
	Poco::URI uri_parser(uri);
	std::vector < std::string > segments;
	uri_parser.getPathSegments(segments);
	int id = std::stoi(segments[2]);

	if (storage.Exists(id)) {
		//"Грубо" ловим ошибку, если нам прислали не те данные (нехватает поля, не тот тип поля и т.д.)
		try {
			Meeting meeting = j;
			storage.Update(meeting, id);
			response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
			response.send() << json(meeting);
			console_log("Updated  meeting #"+std::to_string(id)+":");
			console_log(j.dump());
		} catch(...) {
			console_log("Trying to update meeting. Validation error");
			console_log(j.dump());
			response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
			response.send();
			return;
		}
	} else {
		console_log("Trying to update meeting #"+std::to_string(id)+":");
		console_log("Not found.");
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
		response.send();
	}

}


void UserMeetingDelete::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

	auto &storage = GetStorage();

	// Получем id
	const auto uri = request.getURI();
	Poco::URI uri_parser(uri);
	std::vector < std::string > segments;
	uri_parser.getPathSegments(segments);
	int id = std::stoi(segments[2]);

	if (storage.Exists(id)) {
		storage.Delete(id);
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_NO_CONTENT);
		response.send();
		console_log("Deleted  meeting #"+std::to_string(id));
	} else {
		console_log("Trying to delete meeting #"+std::to_string(id)+":");
		console_log("Not found.");
		response.setStatus(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
		response.send();
	}
}


} // namespace handlers
