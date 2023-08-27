#pragma once
#include <string>
#include <vector>

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

} // namespace name
