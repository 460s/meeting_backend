#pragma once
#include <memory>

class WaitingRoom;
using WaitingRoomPtr = std::shared_ptr<WaitingRoom>;

class Customer;
using CustomerPtr = std::shared_ptr<Customer>;

class Barber {
public:

	void Run(WaitingRoomPtr waiting_room);
	void Cut();
	void Sleep();
	void WakeUp(CustomerPtr customer);
	bool IsSleeping() const;
	void Stop();

private:
	CustomerPtr m_active_customer;
	bool m_is_sleeping{false};
	bool m_should_stop{false};
};
using BarberPtr = std::shared_ptr<Barber>;