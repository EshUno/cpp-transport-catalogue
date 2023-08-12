#include <iostream>
#include <fstream>

#include "stat_reader.h"
#include "transport_catalogue.h"
#include "input_reader.h"
using namespace std;

namespace tests {
#include <cassert>
using namespace transport;
using namespace input;

void TestAddStop(){
    transport::TransportCatalogue trs;
    std::vector<std::string> data = {{"Stop Tolstopaltsevo: 55.611087, 37.208290"},
        {"Stop Marushkino: 55.595884, 37.209755"},
        {"Stop Biryulyovo Zapadnoye: 55.574371, 37.651700"}

    };
//    for (auto &x: data){
//        trs.AddStop(input::ParseStop(x));
//    }

//    assert(trs.GetStopsCount() == 3);

}
void TestBusInput(){
    std::vector<std::string> data = {{"Bus 256: Biryulyovo Zapadnoye > Biryusinka > Biryulyovo Zapadnoye"},
        {"Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"}
    };

    std::vector<input::Bus> res;
    for (auto &x: data){
        res.push_back(ParseBus(x));
    }

    assert(res[0].name == "256");
    assert(res[1].name == "750");

    assert(res[0].stops_name.size() == 3);
    assert(res[1].stops_name.size() == 3);
}
void TestStopInput(){
    std::vector<std::string> data = {{"Stop Tolstopaltsevo: 55.611087, 37.208290"},
        {"Stop Marushkino: 55.595884, 37.209755"},
        {"Stop Biryulyovo Zapadnoye: 55.574371, 37.651700"}

    };

    std::vector<input::Stop> res;
    for (auto &x: data){
        res.push_back(ParseStop(x));
    }

//    assert(res[0].name == "Tolstopaltsevo");
//    assert(res[1].name == "Marushkino");
//    assert(res[2].name == "Biryulyovo Zapadnoye");

//    assert(res[0].coord.lat == 55.611087);
//    assert(res[0].coord.lng == 37.208290);
}
void TestInput(){
    TestStopInput();
    TestBusInput();
    TestAddStop();
}
}

int main()
{
    //tests::TestInput();
    ifstream inf("/home/esh/code/transport_catalogue/tsC_case1_input.txt");
    //ifstream inf("/home/esh/code/transport_catalogue/test3.txt");
    if (!inf)
    {
        exit(1);
    }
    transport::TransportCatalogue catalogue;
    input::LoadQueries(inf, catalogue);
    output::LoadQueries(inf, catalogue);
    return 0;
}
