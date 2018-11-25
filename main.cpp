#include <iostream>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>

using namespace std;

int main(int argc, char** argv)
{
	Poco::URI uri1("https://github.com/460s/meeting_backend/edit/hello-conan/README.md");
	std::string scheme(uri1.getScheme());
	std::string host(uri1.getHost()); 
	std::string path(uri1.getPath());

	nlohmann::json schemeOut;
	schemeOut["scheme"] = scheme;
	std::cout << schemeOut << std::endl;

	nlohmann::json hostOut;
	hostOut["host"] = host;
	std::cout << hostOut << std::endl;

	nlohmann::json pathOut;
	pathOut["path"] = path;
	std::cout << pathOut << std::endl;

	return 0;

}
