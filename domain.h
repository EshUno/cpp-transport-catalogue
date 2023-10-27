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
    geo::Coordinates coord;
    std::string name;
};

struct Bus{
    std::vector<Stop*> stops;
    BusType type;
    std::string name;

};

using StopsInfo = std::map<std::string_view, Stop*>;
using BusesInfo = std::map<std::string_view, Bus*>;
}  // namespace transport
