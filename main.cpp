#include "Poco/URI.h"
#include <nlohmann/json.hpp>

#include <iostream>

using namespace std;
using json = nlohmann::json;


json ifStringEmpty(json s)
{
	if (s == "")
	{
		return nullptr;
	}
	return s;
}

int main()
{
	string text = "";
	cin >> text;
	Poco::URI uri(text);

	json jsonResult =
	{
		{"scheme", ifStringEmpty(uri.getScheme())},
		{"host", ifStringEmpty(uri.getHost())},
		{"path", ifStringEmpty(uri.getPath())}
	};

	cout << jsonResult << endl;

	return 0;
}