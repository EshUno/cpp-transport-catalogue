#include "input_reader.h"
#include <istream>
#include <iostream>


namespace input {
void LoadQueries(std::istream& in, transport::TransportCatalogue &catalogue){
    int queries_count = 0;
    std::string line;

    // заполнение базы данных
    in >> queries_count;
    in.get();
    std::vector<input::Bus> bus_queries;
    std::vector<input::Stop> stop_queries;
    for(auto i = 0; i < queries_count; ++i ){
        getline(in, line);
        if (line[0] == 'S'){
            auto x = input::ParseStop(line);
            stop_queries.push_back(x);
            catalogue.AddStop(x.stop);
        }
        else if (line[0] == 'B'){
            bus_queries.push_back(std::move(input::ParseBus(line)));
        }
    }

    for (auto &x : stop_queries){
        for (auto &one : x.dm){
            catalogue.AddDistance(catalogue.FindStop(x.stop.name), catalogue.FindStop(one.first), one.second);
        }
    }

    for (auto &x : bus_queries){
        catalogue.AddBus(x.name, x.type, x.stops_name);
    }
}

input::Stop ParseStop(std::string& line){
    input::Stop stop;
    line = line.substr(4, line.size());

    auto pos_comma = line.find(':');
    stop.stop.name = line.substr(line.find_first_not_of(' '), pos_comma - 1);
    line = line.substr(pos_comma + 1, line.size());

    pos_comma = line.find(',');
    stop.stop.coord.lat = std::stod(line.substr(line.find_first_not_of(' '), pos_comma - 1));
    line = line.substr(pos_comma + 1, line.size());

    pos_comma = line.find(',');
    if (pos_comma == line.npos){
        stop.stop.coord.lng = std::stod(line.substr(line.find_first_not_of(' '), line.find_last_not_of(' ')));
        line.erase();
    }
    else {
        stop.stop.coord.lng = std::stod(line.substr(line.find_first_not_of(' '), pos_comma - 1));
        line = line.substr(pos_comma + 1, line.size());
    }

    while (!line.empty()){
        auto pos = line.find(',');
        auto pos_m = line.find("m");
        int distance = std::stoi(std::string(line.substr(line.find_first_not_of(' '), pos_m - 1)));
        if (pos == line.npos) {
            line = line.substr(pos_m + 4, line.size());
            stop.dm[line.substr(line.find_first_not_of(' '), pos - 1)] = distance;
            break;
        }
        line = line.substr(pos_m + 4, line.size());
        pos = line.find(',');
        stop.dm[line.substr(line.find_first_not_of(' '), pos - 1)] = distance;
        line = line.substr(pos + 1, line.size());
    }
    return stop;
}

input::Bus ParseBus(std::string& line){
    input::Bus bus;
    if (line.find('-') != line.npos) bus.type = transport::BusType::DirectType;
    else bus.type = transport::BusType::CircularType;
    auto sep = (bus.type == transport::BusType::DirectType)? '-': '>';

    line = line.substr(3, line.size());
    auto pos_name = line.find(':');
    bus.name = line.substr(line.find_first_not_of(' '), pos_name - 1);
    line = line.substr(pos_name + 1, line.size());

    while (!line.empty()){
        auto pos = line.find(sep);
        if (pos == line.npos) {
            bus.stops_name.push_back(line.substr(line.find_first_not_of(' '), line.find_last_not_of(' ')));
            break;
        }
        else bus.stops_name.push_back(line.substr(line.find_first_not_of(' '), line.find_last_not_of(' ', pos - 1)));
        line = line.substr(pos + 1, line.size());
    }

    return bus;
}
} //input

