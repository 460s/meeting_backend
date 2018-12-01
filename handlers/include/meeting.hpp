#pragma once

#include <optional>
#include <string>

namespace handlers {

struct Meeting {
	std::optional<int> id;
	std::string name;
	std::string description;
	std::string address;
	bool published;
};

} // namespace handlers
