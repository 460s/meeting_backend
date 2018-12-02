#pragma once
#include <Poco/Data/Session.h>
#include <meeting.hpp>

using domain::Meeting;

class IDAO {
public:
    using MeetingList = std::vector<Meeting>;
    virtual void Save(Meeting &meeting) = 0;
    virtual MeetingList GetList() = 0;
    virtual std::optional<Meeting> Get(int id) = 0;
    virtual bool Delete(int id) = 0;
    virtual bool HasEntity(int id) = 0;
    virtual ~IDAO() = default;
};