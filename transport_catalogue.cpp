#include "transport_catalogue.h"

namespace transport {

double Bus::GetDistance(){
    double res = 0;
    for (auto i = 1; i < stops.size(); ++i){
        res += geo::ComputeDistance(stops[i - 1]->coord, stops[i]->coord);
    }
    if (type == BusType::DirectType) res *= 2;
    return res;
}

size_t TransportCatalogue::GetStopsCount(){
    return storage_stops_.size();
}

void TransportCatalogue::AddStop(Stop stop){
    auto pos = storage_stops_.insert(storage_stops_.begin(), std::move(stop));
    stops_.insert({std::string_view(pos->name), &(*pos)});
}

void TransportCatalogue::AddBus(std::string &name, BusType type, std::vector<std::string>& stops){
    Bus bus;
    bus.name = name;
    bus.type = type;
    for (auto &x : stops){
        auto y = stops_.find(x);
        bus.stops.push_back(y->second);
        info_about_stop[y->first].insert(name);
    }
    for (auto &x : bus.stops){
        bus.unique_stops.insert(x);
    }
    auto pos = storage_buses_.insert(storage_buses_.begin(), std::move(bus));
    buses_.insert({std::string_view(pos->name), &(*pos)});  
}

Stop* TransportCatalogue::FindStop(std::string& name){
    auto it = stops_.find(name);
    if (it != stops_.end()){
        return it->second;
    }
    return nullptr;
}

Bus* TransportCatalogue::FindBus(std::string& name){
    auto it = buses_.find(name);
    if (it != buses_.end()){
        return it->second;
    }
    return nullptr;
}

std::pair<bool,std::set<std::string_view>*> TransportCatalogue::GetInfoAboutStop(std::string &name){
    auto pos = info_about_stop.find(name);
    if (pos == info_about_stop.end()){
        bool exist = (stops_.find(name) != stops_.end())? true: false;
        return {exist, nullptr};
    }
    return {true, &pos->second};
}

} //transport








