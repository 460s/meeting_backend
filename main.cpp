#include <iostream>

#include <Poco/URI.h>
#include <nlohmann/json.hpp>

using namespace std;

int main() {
	
	string inputStr;
	cin >> inputStr;
	Poco::URI uri(inputStr);
	nlohmann::json outputJSON = {
		{"scheme", uri.getScheme()},
		{"host", uri.getHost()},
		{"path", uri.getPath()},
	};

	cout << outputJSON.dump() << endl;
	return 0;
}