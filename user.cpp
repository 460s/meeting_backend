#include <iostream>

namespace ispsystem::hello {

std::string GetUser() {
	auto user = getenv("USERNAME");
	return user == nullptr ? "World" : user;
}

} // namespace ispsystem::hello