#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
namespace reader {
struct Bus{
    transport::BusType type;
    std::string name;
    std::vector<std::string> stops_name;
};

struct Stop{
    transport::Stop stop;
    std::unordered_map<std::string, int> dm;
};

void FillTheTransportCatalogue(transport::TransportCatalogue &tc, const json::Array &requests);
reader::Stop CreateStop(const json::Dict &request);
reader::Bus CreateBus(const json::Dict &request);

void LoadQueries(const transport::TransportCatalogue &tc, renderer::MapRenderer &mr, const json::Array &requests, std::ostream& os);
json::Dict PrintBus(const transport::BusPrintInfo* info);
json::Dict PrintStop(int id, std::optional<const std::set<std::string_view>*> st);
json::Dict PrintMap(int id, const svg::Document& doc);

renderer::Settings LoadMapRendererSettings(const json::Dict &settings);
svg::Color DetectColor(const json::Node& node);

} // reader

