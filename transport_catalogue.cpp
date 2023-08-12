#include "transport_catalogue.h"

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
    }
    buses_.insert({std::string_view(pos->name), &(*pos)});
}

void TransportCatalogue::AddDistance(const Stop *stop1, const Stop *stop2, int  dm){
    distances_.insert({{stop1, stop2}, dm});
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
    // return == std::nullopt -> no such stop
    // return != std::nullopt && *return == nullptr - no buses for stop
    // else has buses

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

BusPrintInfo TransportCatalogue::GetBusPrintInfo(const Bus *bus) const {
    BusPrintInfo info;
    info.name = bus->name;
    info.exist = true;
    info.stops_route = (bus->type == transport::BusType::DirectType)?
                               (bus->stops.size() * 2 - 1): (bus->stops.size());

    std::set <Stop *> uniq (bus->stops.begin(), bus->stops.end());
    info.unique_stops = uniq.size();
    info.route_length = ComputeRouteDistance(bus);

    auto get_distance = [](const Bus &bus){
        double res = 0;
        for (auto i = 1; i < bus.stops.size(); ++i){
            res += geo::ComputeDistance(bus.stops[i - 1]->coord, bus.stops[i]->coord);
        }
        if (bus.type == BusType::DirectType) res *= 2;
        return res;
    };

    info.curvature = static_cast<double>(info.route_length) / get_distance(*bus);

    return (info);
}

} //transport








