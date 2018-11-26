
#include <iostream>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>

int main() {

	std::string uriString;
	std::cin >> uriString;
	Poco::URI uri(uriString);
	nlohmann::json result;

	result["schema"] = uri.getScheme();
	result["host"] = uri.getHost();
	result["path"] = uri.getPath();

	std::cout << result << std::endl; 
	return 0;
}
