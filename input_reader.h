#pragma once
#include <deque>
#include <string>
#include "transport_catalogue.h"

void LoadQueries(std::istream& in);
namespace input {
struct Bus{
    transport::BusType type;
    std::string name;
    std::vector<std::string> stops_name;
};
transport::Stop ParseStop(std::string& line);
input::Bus ParseBus(std::string& line);

} //input
