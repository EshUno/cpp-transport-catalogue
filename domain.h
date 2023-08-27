#pragma once
#include <string>
#include <vector>
#include <map>

#include "geo.h"
namespace transport {

enum class BusType{
    DirectType,
    CircularType
};

struct Stop{
    std::string name;
    geo::Coordinates coord;
};

struct Bus{
    BusType type;
    std::string name;
    std::vector<Stop*> stops;
};

using StopsInfo = std::map<std::string_view, Stop*>;
using BusesInfo = std::map<std::string_view, Bus*>;
} // namespace name
