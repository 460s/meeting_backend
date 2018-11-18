#include <iostream>
#include <cstdlib>

using namespace std;

namespace ispsystem::hello {
    string GetString () {
        auto user = getenv("USERNAME");
        return user == nullptr ? "World" : user;
    }

}

int main() {
    cout << "Hello " << ispsystem::hello::GetString() << endl;
    return 0;
}