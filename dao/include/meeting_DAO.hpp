#pragma once
#include "IDAO.hpp"

namespace dao{

class MeetingDAO : public IDAO {
public:
    void Save(Meeting &meeting) override;
    IDAO::MeetingList GetList() override;
    std::optional<Meeting> Get(int id) override;
    bool Delete(int id) override;
    bool HasEntity(int id) override;
private:
    std::mutex m_mutex;
};

}