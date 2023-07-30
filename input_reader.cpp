#include "input_reader.h"
#include "stat_reader.h"
#include <istream>
#include <iostream>

void LoadQueries(std::istream& in){
    transport::TransportCatalogue catalogue;
    int queries_count = 0;
    std::string line;

    // заполнение базы данных
    in >> queries_count;
    in.get();
    std::vector<input::Bus> bus_queries;
    for(auto i = 0; i < queries_count; ++i ){
        getline(in, line);
        if (line[0] == 'S'){
            catalogue.AddStop(input::ParseStop(line));
        }
        else if (line[0] == 'B'){
            bus_queries.push_back(std::move(input::ParseBus(line)));
        }
    }
    for (auto &x : bus_queries){
        catalogue.AddBus(x.name, x.type, x.stops_name);
    }

   //ввоз запросов к базе данных
    in  >> queries_count;
    in.get();
    for(auto i = 0; i < queries_count; ++i ){
        getline(in, line);
        if (line[0] == 'B'){
            auto x = output::ParseBus(line);
            output::PrintBus(std::cout, x, catalogue.FindBus(x));
        }
        else if (line[0] == 'S'){
            auto x = output::ParseStop(line);
            output::PrintStop(std::cout, x, catalogue.GetInfoAboutStop(x));
        }

    }
}

namespace input {
transport::Stop ParseStop(std::string& line){
    transport::Stop stop;
    line = line.substr(4, line.size());

    auto pos_name = line.find(':');
    stop.name = line.substr(line.find_first_not_of(' '), pos_name - 1);
    line = line.substr(pos_name + 1, line.size());

    auto pos_comma = line.find(',');
    stop.coord.lat = std::stod(line.substr(line.find_first_not_of(' '), pos_comma - 1));
    line = line.substr(pos_comma + 1, line.size());

    stop.coord.lng = std::stod(line.substr(line.find_first_not_of(' '), line.find_last_not_of(' ')));

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

