#include <iostream>
#include <fstream>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
using namespace std;

int main()
{
    ifstream inf("/home/esh/code/cpp-transport_catalogue/example1.json");
    if (!inf)    {
        exit(1);
    }
    auto json_in = json::Load(inf).GetRoot();

    //auto json_in = json::Load(std::cin).GetRoot();
    transport::TransportCatalogue catalogue;
    reader::FillTheTransportCatalogue(catalogue, json_in.AsDict().at("base_requests").AsArray());
    renderer::Settings renderer_settings =  reader::LoadMapRendererSettings(json_in.AsDict().at("render_settings").AsDict());
    renderer::MapRenderer render(renderer_settings, catalogue.GetMinCoordinates(), catalogue.GetMaxCoordinates());
    routing::Settings routing_settings = reader::LoadRoutingSettings(json_in.AsDict().at("routing_settings").AsDict());
    routing::TransportRouter router(catalogue, routing_settings);

    reader::LoadQueries(catalogue, render, json_in.AsDict().at("stat_requests").AsArray(), std::cout, router);
}
