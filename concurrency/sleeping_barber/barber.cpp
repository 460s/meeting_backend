#include "barber.hpp"
#include "customer.hpp"
#include "waiting_room.hpp"
#include <iostream>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

void Barber::Run(WaitingRoomPtr waiting_room) {
	while (true) {
		std::cout << "Has customer?" << std::endl;
		if (waiting_room->HasCustomer()) {
			std::cout << "yes" << std::endl;
			m_active_customer = waiting_room->GetCustomer();
			Cut();
		} else {
			std::cout << "no" << std::endl;
			Sleep();
			if (m_should_stop) {
				return;
			}
			Cut();
		}
	}
}

void Barber::Cut() {
	std::cout << "Start cut " << m_active_customer->Name() << std::endl;
	std::this_thread::sleep_for(100ms);
	m_active_customer->SetCut();
	std::cout << "Finish cut " << m_active_customer->Name() << std::endl;
	m_active_customer.reset();
}

void Barber::Sleep() {
	std::cout << "Sleep" << std::endl;
	m_is_sleeping = true;
	while (m_active_customer == nullptr) {
		std::this_thread::sleep_for(1ms);
		if (m_should_stop) {
			return;
		}
	}
}

void Barber::WakeUp(CustomerPtr customer) {
	m_is_sleeping = false;
	std::cout << "Wake up" << std::endl;
	m_active_customer = customer;
}

bool Barber::IsSleeping() const {
	return m_is_sleeping;
}

void Barber::Stop() {
	m_should_stop = true;
}