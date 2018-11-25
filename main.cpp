#include <iostream>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>

using namespace std;

int main() {
	std::string string_uri;
	std::cin >> string_uri;
	Poco::URI uri1(string_uri);
	nlohmann::json result;
	result["schema"]=uri1.getScheme();
	result["host"]=uri1.getHost();
	result["path"]=uri1.getPath();
	std::cout << result << std::endl;
	return 0;
}
