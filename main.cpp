#include <iostream>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>

using namespace std;

int main() {
	string uri_string;
	cin >> uri_string;
	nlohmann::json result;
	Poco::URI uri1(uri_string);
	result["schema"] = uri1.getScheme();
	result["host"] = uri1.getHost();
	result["path"] = uri1.getPath();
	cout << result.dump(1) << endl;
	return 0;
}
