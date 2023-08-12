#include "stat_reader.h"
#include <iomanip>
#include <ostream>

namespace output {
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
void PrintStop(std::ostream& os, std::string& name, std::pair<bool,std::set<std::string_view>*> st){
    if (st.first == false){
        os << "Stop " << name << ": not found" << std::endl;
    }
    else if (st.second == nullptr && st.first){
        os << "Stop " << name << ": no buses" << std::endl;
    }
    else {
        os << "Stop " << name << ": buses";
        for (auto &x: *st.second){
            os << " " << x ;
        }
        os << std::endl;
    }
}
} //output
