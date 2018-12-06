// Задача о спящем парикмахере
// У парикмахера есть одно рабочее место и приемная с несколькими стульями. 
// Когда парикмахер заканчивает подстригать клиента, он отпускает клиента и затем идет в приёмную, 
// чтобы посмотреть, есть ли там ожидающие клиенты. 
// Если они есть, он приглашает одного из них и стрижет его. 
// Если ждущих клиентов нет, он возвращается к своему креслу и спит в нем.
// 
// Каждый приходящий клиент смотрит на то, что делает парикмахер. 
// Если парикмахер спит, то клиент будит его и садится в кресло. 
// Если парикмахер работает, то клиент идет в приёмную. 
// Если в приёмной есть свободный стул, клиент садится и ждёт своей очереди. 
// Если свободного стула нет, то клиент уходит

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include "barber.hpp"
#include "waiting_room.hpp"
#include "customer.hpp"

using namespace std::chrono_literals;

void NewCustomer(const std::string &name, WaitingRoomPtr room, BarberPtr barber) {
	auto c = std::make_shared<Customer>(name);
	c->Run(room, barber);
}

int main(int argc, char **argv) {
	std::cout << std::endl << std::endl << "Open Barber Shop" << std::endl;
	auto room = std::make_shared<WaitingRoom>();

	auto barber = std::make_shared<Barber>();
	std::thread barber_thread([room, barber] {
		barber->Run(room);
	});
	std::vector<std::thread> customer_threads;

	for (int i = 1; i < argc; ++i) {
		customer_threads.emplace_back(NewCustomer, argv[i], room, barber);
	}
	
	for (auto &t : customer_threads) {
		t.join();
	}

	barber->Stop();
	barber_thread.join();
}