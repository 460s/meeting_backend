#include <cstdlib>
#include <iostream>

using namespace std;

namespace ispsystem::hello {
string GetString() {
	auto user = getenv("USERNAME");
	return user == nullptr ? "World" : user;
}

} // namespace ispsystem::hello

int main() {
	cout << "Hello " << ispsystem::hello::GetString() << endl;
	return 0;
}