#include "stat_reader.h"
#include <iomanip>
#include <iostream>
#include <ostream>

namespace output {

void LoadQueries(std::istream& in, transport::TransportCatalogue &catalogue){
    int queries_count = 0;
    std::string line;

    //ввод запросов к базе данных
    in  >> queries_count;
    in.get();
    for(auto i = 0; i < queries_count; ++i ){
        getline(in, line);
        if (line[0] == 'B'){
            auto x = output::ParseBus(line);
            auto bus = catalogue.FindBus(x);
            transport::BusPrintInfo bus_info;
            if (bus != nullptr) bus_info = catalogue.GetBusPrintInfo(bus, 0);
            else {
                bus_info.name = x;
                bus_info.exist = false;
            }
            output::PrintBus(std::cout, &bus_info);
        }
        else if (line[0] == 'S'){
            auto word = output::ParseStop(line);
            output::PrintStop(std::cout, word, catalogue.GetInfoAboutStop(word));
        }

    }
}

std::string ParseBus(std::string& line){
    line = line.substr(3, line.size());
    return line.substr(line.find_first_not_of(' '), line.find_last_not_of(' '));
}
std::string ParseStop(std::string& line){
    line = line.substr(4, line.size());
    return line.substr(line.find_first_not_of(' '), line.find_last_not_of(' '));
}

void PrintBus(std::ostream& os, transport::BusPrintInfo* info){
    // Bus X: R stops on route, U unique stops, L route length, C curvature
    if (info->exist == false){
        os << "Bus " << info->name << ": not found" << std::endl;
    }
    else {
        os << "Bus " << info->name << ": "
           << info->stops_route << " stops on route, "
           << info->unique_stops << " unique stops, "
           << std::setprecision(6) << info->route_length << " route length, "
           << std::setprecision(6) << info->curvature << " curvature" <<std::endl;
    }
}
void PrintStop(std::ostream& os, const std::string& name, std::optional<const std::set<std::string_view>*> st){
    if (st == std::nullopt){
        os << "Stop " << name << ": not found" << std::endl;
    }
    else if (*st == nullptr){
        os << "Stop " << name << ": no buses" << std::endl;
    }
    else {
        os << "Stop " << name << ": buses";
        for (auto &x: **st){
            os << " " << x ;
        }
        os << std::endl;
    }
}
} //output
