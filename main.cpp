#include "Poco/URI.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
	std::string string_uri = "https://github.com/460s/meeting_backend/edit/hello-conan/README.md";
	// cin >> string_uri;
	Poco::URI uri(string_uri);
	std::string scheme(uri.getScheme()); // "http"
	std::string host(uri.getHost()); // "www.appinf.com"
	std::string path(uri.getPath()); // "/sample"
	cout << scheme << endl << host << endl << path;
	return 0;	
}
