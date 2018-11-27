#include <iostream>
#include <nlohmann/json.hpp>
#include <Poco/URI.h>

using namespace std;

int main() {
    string s;
    cin >> s;
    Poco::URI uri1(s);
    nlohmann::json j;
    j["schema"] = uri1.getScheme();
    j["host"] = uri1.getHost();
    j["path"] = uri1.getPath();
    cout << j.dump() << endl;
	return 0;
}
