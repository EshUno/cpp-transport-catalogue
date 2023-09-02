#include <iostream>
#include <fstream>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
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
    reader::FillTheTransportCatalogue(catalogue, json_in.AsDict().at("base_requests").AsArray());
    renderer::Settings settings =  reader::LoadMapRendererSettings(json_in.AsDict().at("render_settings").AsDict());
    renderer::MapRenderer render(settings, catalogue.GetMinCoordinates(), catalogue.GetMaxCoordinates());
    reader::LoadQueries(catalogue, render, json_in.AsDict().at("stat_requests").AsArray(), std::cout);
}
