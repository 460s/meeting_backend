#include <iostream>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>

using namespace std;

int main() {

        std::string uri_string;
        std::cin >> uri_string;

        Poco:: URI poco_uri(uri_string);

        result["scheme"] = uri1.getScheme();
        result["host"]   = uri1.getHost();
        result["path"]   = uri1.getPath();

        std::cout << result << std::endl;
        return 0;

}
