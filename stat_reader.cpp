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

void PrintBus(std::ostream& os, std::string& name, transport::Bus *bus){
    if (bus == nullptr){
        os << "Bus " << name << ": not found" << std::endl;
    }
    else {
        if (bus->type == transport::BusType::DirectType){
            os << "Bus " << name << ": " << bus->stops.size() * 2 - 1 << " stops on route, "
               << bus->unique_stops.size() << " unique stops, "
               << std::setprecision(6) << bus->GetDistance() << " route length" <<std::endl;
        }
        else {
            os << "Bus " << name << ": " << bus->stops.size() << " stops on route, "
               << bus->unique_stops.size() << " unique stops, "
               << std::setprecision(6) << bus->GetDistance() << " route length" <<std::endl;
        }
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
