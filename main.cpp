#include <iostream>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>

using namespace std;

int main(int argc, char** argv)
{
	std::string input;
	std::cin >> input;
	Poco::URI uri1(input);
	std::string scheme(uri1.getScheme());
	std::string host(uri1.getHost()); 
	std::string path(uri1.getPath());

	nlohmann::json result;
	result["scheme"] = scheme;
	result["host"] = host;
	result["path"] = path;
	std::cout << result << std::endl;

	return 0;

}
