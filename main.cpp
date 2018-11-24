#include <iostream>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>

using namespace std;

int main() {
	string inputUri;
	cin >> inputUri;
	Poco::URI uri(inputUri);
	nlohmann::json outputJson = 
	{
	  {"scheme", uri.getScheme()},
	  {"host", uri.getHost()},
	  {"path", uri.getPath()}
	};
	cout << outputJson.dump(4) << endl;
	return 0;
}
