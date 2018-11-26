
#include <iostream>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>

int main() {
	std::string uri_str;
	std::cin >> uri_str;
	Poco::URI uri(uri_str);
	
	nlohmann::json result;
	result["schema"] = uri.getScheme();
	result["host"] = uri.getHost();
	result["path"] = uri.getPath();
	std::cout << result << std::endl;
	
	return 0;
}
