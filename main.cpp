#include <iostream>

#include <Poco/URI.h>
#include <nlohmann/json.hpp>

using namespace std;

int main() {
	string uri_string;
	cin >> uri_string;
	
	Poco::URI uri(uri_string);
	nlohmann::json result;
	
	result["schema"] = uri.getScheme();
	result["host"] = uri.getHost();
	result["path"] = uri.getPath();

	cout << result << endl;

	return 0;
}

