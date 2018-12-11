#include <thread>
#include <mutex>
#include <iostream>

//RAII - Resource Acquisition Is Initialization
class MutexLocker {
public:
	MutexLocker(std::mutex &mutex) : m_mutex(mutex) {
		m_mutex.lock();
	}
	~MutexLocker() {
		m_mutex.unlock();
	}
private:
	std::mutex &m_mutex;
};

void incrementor_func(int *i, std::mutex &mutex) {
	while(*i < 2'000'000) {
		std::lock_guard<std::mutex> l{mutex}; 
		++(*i);
	}
}


int main() {
	std::mutex mutex;
	int *i = new int{0};
	std::thread incrementor(incrementor_func, i, std::ref(mutex));
	std::thread printer([i, &mutex] {
		while(*i < 2'000'000) {
			std::lock_guard<std::mutex> l{mutex}; 
			if (*i % 2 == 0) {
				std::cout << *i << std::endl;
			}
			if (*i % 20 == 0) {
				return;
			}
		}
	});
	printer.join();
	incrementor.join();
	delete i;
}
