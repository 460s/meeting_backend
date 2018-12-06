#include "customer.hpp"
#include "waiting_room.hpp"

bool WaitingRoom::HasCustomer() const {
	std::lock_guard<std::mutex> lock(m_wr_mutex);
	return !m_customer_list.empty();
}

bool WaitingRoom::IsFull() const {
	std::lock_guard<std::mutex> lock(m_wr_mutex);
	return m_customer_list.size() == m_chair_count;
}

CustomerPtr WaitingRoom::GetCustomer() {
	std::lock_guard<std::mutex> lock(m_wr_mutex);
	auto result = m_customer_list.front();
	m_customer_list.pop_front();
	return result;
}

void WaitingRoom::AddCustomer(CustomerPtr customer) {
	std::lock_guard<std::mutex> lock(m_wr_mutex);
	m_customer_list.push_back(customer);
}
