#pragma once

#include <nlohmann/json.hpp>
#include <optional>

namespace handlers {

struct Meeting {
	std::optional<int> id;
	std::string name;
	std::string description;
	std::string address;
	std::string signup_description;
	int signup_from_date;
	int signup_to_date;
	int from_date;
	int to_date;
	bool published{false};

	bool operator == (const Meeting & MeetingB) const {
	return this->name == MeetingB.name && this->description == MeetingB.description &&
			this->address == MeetingB.address && this->signup_description == MeetingB.signup_description && 
			this->signup_from_date == MeetingB.signup_from_date && this->signup_to_date == MeetingB.signup_to_date &&
			this->from_date == MeetingB.from_date && this->to_date == MeetingB.to_date &&
			this->published == MeetingB.published;
	}

};

class Storage {
public:
	using MeetingList = std::vector<Meeting>;
	virtual void Save(Meeting &meeting) = 0;
	virtual MeetingList GetList() = 0;
	virtual std::optional<Meeting> Get(int id) = 0;
	virtual bool Delete(int id) = 0;
	virtual ~Storage() = default;
};

// сериализация (маршалинг)
inline void to_json(nlohmann::json &j, const Meeting &m) {
	j = nlohmann::json{
		{"id", m.id.value()},
		{"name", m.name},
		{"description", m.description},
		{"address", m.address},
		{"signup_description", m.signup_description},
		{"signup_from_date", m.signup_from_date},
		{"signup_to_date", m.signup_to_date},
		{"from_date", m.from_date},
		{"to_date", m.to_date},
		{"published", m.published}};
}

// десериализация (анмаршалинг, распаковка)
inline void from_json(const nlohmann::json &j, Meeting &m) {
	j.at("name").get_to(m.name);
	j.at("description").get_to(m.description);
	j.at("address").get_to(m.address);
	j.at("signup_description").get_to(m.signup_description);
	j.at("signup_from_date").get_to(m.signup_from_date);
	j.at("signup_to_date").get_to(m.signup_to_date);
	j.at("from_date").get_to(m.from_date);
	j.at("to_date").get_to(m.to_date);
	j.at("published").get_to(m.published);
}

} // namespace handlers
