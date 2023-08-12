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

size_t TransportCatalogue::GetStopsCount() const{
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

void TransportCatalogue::AddDistance(Stop *stop, std::unordered_map<std::string, int>& dm){
    for (auto &x : dm){
        distances_.insert({{stop, FindStop(x.first)}, x.second});
    }
}

Stop* TransportCatalogue::FindStop(const std::string& name) const{
    auto it = stops_.find(name);
    if (it != stops_.end()){
        return it->second;
    }
    return nullptr;
}

Bus* TransportCatalogue::FindBus(const std::string& name) const{
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

double TransportCatalogue::ComputeRouteDistance(Bus *bus) const{
    double res = 0;
    for (auto i = 1; i < bus->stops.size(); ++i){
        res += ComputeRouteDistance(bus->stops[i-1]->name, bus->stops[i]->name);
    }
    if (bus->type == transport::BusType::DirectType){
        for (auto i = bus->stops.size() - 1; i > 0; --i){
            res += ComputeRouteDistance(bus->stops[i]->name, bus->stops[i - 1]->name);
        }
    }
    return res;
}

double TransportCatalogue::ComputeRouteDistance(std::string_view from, std::string_view to) const{
    auto key = std::make_pair(stops_.at(from), stops_.at(to));
    return (distances_.count(key) > 0) ? distances_.at(key)
            : distances_.at({stops_.at(to), stops_.at(from)});
}

BusPrintInfo TransportCatalogue::GetBusPrintInfo(Bus *bus) const {
    BusPrintInfo info;
    info.name = bus->name;
    info.exist = true;
    info.stops_route = (bus->type == transport::BusType::DirectType)?
                               (bus->stops.size() * 2 - 1): (bus->stops.size());
    info.unique_stops = bus->unique_stops.size();
    info.route_length = ComputeRouteDistance(bus);
    info.curvature = static_cast<double>(info.route_length) / bus->GetDistance();

    return (info);
}

} //transport








