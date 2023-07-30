#pragma once
#include <string>
#include <deque>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <set>

#include "geo.h"
namespace transport {
struct Stop{
    std::string name;
    geo::Coordinates coord;
};

enum class BusType{DirectType, CircularType};
struct Bus{
    BusType type;
    std::string name;
    std::vector<Stop*> stops;
    std::set<Stop*> unique_stops;
    double GetDistance();
};

class TransportCatalogue{
public:
    TransportCatalogue() = default;

    void AddStop(Stop stop);
    void AddBus(std::string &name, BusType type, std::vector<std::string>& stops);

    Stop* FindStop(std::string& name);
    Bus* FindBus(std::string& name);

    std::pair<bool,std::set<std::string_view>*> GetInfoAboutStop(std::string &name);
    size_t GetStopsCount();

private:
    std::deque<Stop> storage_stops_;
    std::unordered_map<std::string_view, Stop*> stops_;

    std::deque<Bus> storage_buses_;
    std::unordered_map<std::string_view, Bus*> buses_;


    // НАЗВАНИЕ ОСТАНОВКИ - сет с маршрутами
    std::unordered_map<std::string_view, std::set<std::string_view>> info_about_stop;

};
} //transport


