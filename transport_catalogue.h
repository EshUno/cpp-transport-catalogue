#pragma once

#include <deque>
#include <string_view>
#include <unordered_map>

#include <set>
#include <optional>

#include "domain.h"

namespace transport {

using StopPair = std::pair<const Stop*, const Stop*>;

struct BusPrintInfo{
    // Bus X: R stops on route, U unique stops, L route length, C curvature
    bool exist = false;
    int id = 0;
    std::string_view name;
    int stops_route;
    int unique_stops;
    double route_length;
    double curvature;
};

class TransportCatalogue{
public:
    TransportCatalogue() = default;

    void AddStop(const Stop &stop);
    void AddBus(const std::string &name, BusType type, const std::vector<std::string>& stops);
    void AddDistance(const Stop *from, const Stop *to, int  dm);

    Stop* FindStop(const std::string& name) const;
    Bus* FindBus(const std::string& name) const;

    std::optional<const std::set<std::string_view>*> GetInfoAboutStop(const std::string &name) const;

    //std::pair<bool, const std::set<std::string_view>*> GetInfoAboutStop(const std::string &name) const;

    BusPrintInfo GetBusPrintInfo(const Bus *bus, int id) const;
private:
    size_t GetStopsCount() const;
    double ComputeRouteDistance(const Bus *bus) const;
    double ComputeRouteDistance(std::string_view from, std::string_view to) const;
    struct StopPairHasher {
        size_t operator()(const StopPair& pair) const {
            size_t factorial = 1;
            auto stop_hasher = [this, &factorial](const Stop &stop){
                return s_hasher_(stop.name) * factorial
                       + d_hasher_(stop.coord.lat) * factorial * step
                       + d_hasher_(stop.coord.lng) * factorial * step * step;
            };
            size_t res = stop_hasher(*pair.first);
            factorial = step * step * step;
            res += stop_hasher(*pair.second);
            return res;
        }
    private:
        static const size_t step = 27;
        std::hash<std::string> s_hasher_;
        std::hash<double> d_hasher_;
    };
    // -----------------------------------------------//
    std::deque<Stop> storage_stops_;
    std::unordered_map<std::string_view, Stop*> stops_;

    std::deque<Bus> storage_buses_;
    std::unordered_map<std::string_view, Bus*> buses_;

    // НАЗВАНИЕ ОСТАНОВКИ - сет с маршрутами
    std::unordered_map<std::string_view, std::set<std::string_view>> info_about_stop;
    std::unordered_map<StopPair, int, StopPairHasher> distances_;
};
} //transport


