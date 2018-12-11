#pragma once
#include <optional>
#include <string>
#include <nlohmann/json.hpp>
#include <Poco/Tuple.h>

using nlohmann::json;

namespace domain{


//При расширении структуру нужно будет дописать только указадели подстановки в запросах,
// этот тип и процедуры конвертации структуры в него и обратно
// параметры подстановки добавлять будет уже не нужно
// Без id т.к. не во всех запросах он используется
typedef Poco::Tuple<std::string, //name;
                    std::string, //description;
                    std::string, //address;
                    std::string, //signup_description;
                    int,         //signup_from_date;
                    int,         //signup_to_date;
                    int,         //from_date;
                    int,         //to_date;
                    bool        //published
                    > MeetingTuple;

struct Meeting{
    std::optional<int> id;
    std::string name;
    std::string description;
    std::string address;
    std::string signup_description;
    int signup_from_date;
    int signup_to_date;
    int from_date;
    int to_date;
    bool published;
    static MeetingTuple MeetingStruct2Tuple(const Meeting &m);
    static Meeting MeetingTuple2Struct(const MeetingTuple &mt, int id);
};

void to_json(json &j, const Meeting &m);
void from_json(const json &j, Meeting &m);

}


