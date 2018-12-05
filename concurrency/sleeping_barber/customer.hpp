#pragma once
#include <string>
#include <memory>

class WaitingRoom;
using WaitingRoomPtr = std::shared_ptr<WaitingRoom>;

class Barber;
using BarberPtr = std::shared_ptr<Barber>;

class Customer : public std::enable_shared_from_this<Customer> {
public:
	Customer(const std::string &name);
	std::string Name() const;
	void Run(WaitingRoomPtr waiting_room, BarberPtr barber);
	void EnterBarberShop(WaitingRoomPtr waiting_room, BarberPtr barber);
	void SetCut();
	void Walk() const;
	bool NeedCut() const;

private:
	std::string m_name;
	bool m_need_cut{true};
};
using CustomerPtr = std::shared_ptr<Customer>;