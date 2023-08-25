#include <iostream>
#include <fstream>

#include "transport_catalogue.h"
#include "json.h"
#include "json_reader.h"
using namespace std;

int main()
{
    ifstream inf("/home/esh/code/cpp-transport_catalogue/input.json");
    if (!inf)    {
        exit(1);
    }
    auto json_in = json::Load(inf).GetRoot();
    //auto json_in = json::Load(std::cin).GetRoot();
    transport::TransportCatalogue catalogue;
    reader::FillTheTransportCatalogue(catalogue, json_in.AsMap().at("base_requests").AsArray());
    reader::LoadStatQueries(catalogue, json_in.AsMap().at("stat_requests").AsArray());

}
