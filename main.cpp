#include "Poco/URI.h"
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;

int main(int argc, char **argv)
{
	string uri_string = "";
	cin >> uri_string;
	Poco::URI uri(uri_string);
	string scheme(uri.getScheme());
	string host(uri.getHost());
	string path(uri.getPath());

	nlohmann::json result;
	result["scheme"] = scheme;
	result["host"] = host;
	result["path"] = path;
	cout << result << endl;
	return 0;
}
