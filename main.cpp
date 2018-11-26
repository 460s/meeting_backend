#include "Poco/URI.h"
#include <nlohmann/json.hpp>

#include <iostream>

using namespace std;
using json = nlohmann::json;


string ifStringEmpty(string s)
{
	if (s == "")
	{
		return "none";
	}
	return s;
}

int main()
{
	string text = "";
	cin >> text;
	Poco::URI uri(text);

	string scheme(ifStringEmpty(uri.getScheme()));
	string host(ifStringEmpty(uri.getHost()));
	string path(ifStringEmpty(uri.getPath()));

	json jsonResult =
	{
		{"scheme", scheme},
		{"host", host},
		{"path", path}
	};

	cout << jsonResult << endl;

	return 0;
}