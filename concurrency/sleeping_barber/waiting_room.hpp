#pragma once
#include <list>
#include <memory>
#include <mutex>

class Customer;
using CustomerPtr = std::shared_ptr<Customer>;

class WaitingRoom {
public:
	bool HasCustomer() const;
	bool IsFull() const;
	CustomerPtr GetCustomer();
	void AddCustomer(CustomerPtr customer);

private:
	size_t m_chair_count{1};
	std::list<CustomerPtr> m_customer_list;
	mutable std::mutex m_wr_mutex;
};
using WaitingRoomPtr = std::shared_ptr<WaitingRoom>;