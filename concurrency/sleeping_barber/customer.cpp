#include "customer.hpp"
#include "barber.hpp"
#include "waiting_room.hpp"
#include <chrono>
#include <iostream>
#include <thread>
using namespace std::chrono_literals;

Customer::Customer(const std::string &name) : m_name(name) {}

void Customer::Run(WaitingRoomPtr waiting_room, BarberPtr barber) {
	EnterBarberShop(waiting_room, barber);
	while (NeedCut()) {
		Walk();
		EnterBarberShop(waiting_room, barber);
	}
}

void Customer::EnterBarberShop(WaitingRoomPtr waiting_room, BarberPtr barber) {
	if (waiting_room->IsFull()) {
		return;
	}
	if (barber->IsSleeping()) {
		barber->WakeUp(shared_from_this());
	} else {
		waiting_room->AddCustomer(shared_from_this());
	}
	while (m_need_cut) {
		std::this_thread::sleep_for(1ms);
	}
}

void Customer::SetCut() {
	m_need_cut = false;
}

void Customer::Walk() const {
	std::this_thread::sleep_for(1ms);
}

bool Customer::NeedCut() const {
	return m_need_cut;
}

std::string Customer::Name() const {
	return m_name;
}