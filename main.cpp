#include <iostream>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>

using namespace std;

int main() {
	string str;
	cout << "Enter uri string: ";
	cin >> str;

	Poco::URI uri(str);

	nlohmann::json json_struct;

	json_struct["scheme"] = uri.getScheme();
	json_struct["host"] = uri.getHost();
	json_struct["path"] = uri.getPath();

	cout << json_struct << endl;
	
	return 0;
}