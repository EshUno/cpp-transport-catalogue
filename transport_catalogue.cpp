#include "transport_catalogue.h"
#include <algorithm>
namespace transport {

size_t TransportCatalogue::GetStopsCount() const{
    return storage_stops_.size();
}

void TransportCatalogue::AddStop(const Stop &stop){
    auto pos = storage_stops_.insert(storage_stops_.begin(), stop);
    stops_.insert({std::string_view(pos->name), &(*pos)});
}

void TransportCatalogue::AddBus(const std::string &name, BusType type, const std::vector<std::string>& stops){
    Bus bus;
    bus.name = name;
    bus.type = type;
    auto pos = storage_buses_.insert(storage_buses_.begin(), std::move(bus));
    for (auto &x : stops){
        auto y = stops_.find(x);
        pos->stops.push_back(y->second);
        info_about_stop[y->first].insert(pos->name);
        UpdateMinMaxStopCoordinates(y->second->coord);
    }
    buses_.insert({std::string_view(pos->name), &(*pos)});
}

void TransportCatalogue::AddDistance(const Stop *from, const Stop *to, int  dm){
    distances_.insert({{from, to}, dm});
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

std::optional<const std::set<std::string_view>*> TransportCatalogue::GetInfoAboutStop(const std::string &name) const {
    auto pos = info_about_stop.find(name);
    if (pos == info_about_stop.end()){
        if (stops_.find(name) == stops_.end()) {
            return std::nullopt;
        }

        return {nullptr};
    }
    return {&pos->second};
}

double TransportCatalogue::ComputeRouteDistance(const Bus *bus) const{
    double res = 0;
    for (std::size_t i = 1; i < bus->stops.size(); ++i){
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

BusPrintInfo TransportCatalogue::GetBusPrintInfo(const Bus *bus, int id) const {
    BusPrintInfo info;
    info.id = id;
    info.name = bus->name;
    info.exist = true;
    info.stops_route = (bus->type == transport::BusType::DirectType)?
                           (bus->stops.size() * 2 - 1): (bus->stops.size());

    std::set <Stop *> uniq (bus->stops.begin(), bus->stops.end());
    info.unique_stops = uniq.size();
    info.route_length = ComputeRouteDistance(bus);

    auto get_distance = [](const Bus &bus){
        double res = 0;
        for (size_t i = 1; i < bus.stops.size(); ++i){
            res += geo::ComputeDistance(bus.stops[i - 1]->coord, bus.stops[i]->coord);
        }
        if (bus.type == BusType::DirectType) res *= 2;
        return res;
    };

    info.curvature = static_cast<double>(info.route_length) / get_distance(*bus);

    return (info);
}

geo::Coordinates TransportCatalogue::GetMinCoordinates() const{
    return min_coordinates_;
}

geo::Coordinates TransportCatalogue::GetMaxCoordinates() const{
    return max_coordinates_;
}

const transport::BusesInfo &TransportCatalogue::GetBuses() const{
    return buses_;
}

const transport::StopsInfo &TransportCatalogue::GetStops() const{
    return stops_;
}

const TransportCatalogue::DistancesBetweenStops& TransportCatalogue::GetDistances() const{
    return distances_;
}

StopsInfo TransportCatalogue::GetUsedStops() const{
    std::map<std::string_view, Stop*> res;
    for(auto &bus: buses_){
        for (auto &stop: bus.second->stops){
            if (res.count(stop->name) == 0){
                res[stop->name] = stop;
            }
        }
    }
    return res;
}

void  TransportCatalogue::UpdateMinMaxStopCoordinates(const geo::Coordinates& coordinates){
    min_coordinates_.lat = std::min(min_coordinates_.lat, coordinates.lat);
    min_coordinates_.lng = std::min(min_coordinates_.lng, coordinates.lng);
    max_coordinates_.lat = std::max(max_coordinates_.lat, coordinates.lat);
    max_coordinates_.lng = std::max(max_coordinates_.lng, coordinates.lng);
}
} //transport








