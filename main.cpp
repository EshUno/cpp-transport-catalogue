#include <fstream>
#include <iostream>
#include <string_view>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void SaveState(std::ostream& output, const routing::TransportRouter& router,
               const transport::TransportCatalogue& catalogue, const routing::Settings& settings) {

}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }
    const std::string_view mode(argv[1]);
    auto json_in = json::Load(std::cin).GetRoot();

    transport::TransportCatalogue catalogue;
    serialization::Settings const serialization_settings = reader::LoadSerializationSettings(json_in.AsDict().at("serialization_settings").AsDict());
    if (mode == "make_base"sv) {
        reader::FillTheTransportCatalogue(catalogue, json_in.AsDict().at("base_requests").AsArray());
        renderer::Settings renderer_settings =  reader::LoadMapRendererSettings(json_in.AsDict().at("render_settings").AsDict());
        routing::Settings routing_settings = reader::LoadRoutingSettings(json_in.AsDict().at("routing_settings").AsDict());
        routing::TransportRouter router(catalogue, routing_settings);

        const serialization::SavedState saved_state{ std::move(router), std::move(catalogue), std::move(renderer_settings)};

        std::ofstream output(serialization_settings.path, std::ios::binary | std::ios::out | std::ios::trunc);
        serialization::Serialize(saved_state, output);

    } else if (mode == "process_requests"sv) {
        std::ifstream input(serialization_settings.path, std::ios::binary | std::ios::in);
        auto state = serialization::Deserialize(input);

        auto catalogue = std::move(state.catalogue);
        renderer::MapRenderer renderer(state.renderer_settings, catalogue.GetMinCoordinates(), catalogue.GetMaxCoordinates());
        auto router = std::move(state.router);

        reader::LoadQueries(catalogue, renderer, json_in.AsDict().at("stat_requests").AsArray(), std::cout, router);

    } else {
        PrintUsage();
        return 1;
    }
}


//int main(int argc, char* argv[]) {
//    //const std::string_view mode("make_base"sv);
//    // std::ifstream inf("/home/esh/code/cpp-transport_catalogue/test_make.json");
//    const std::string_view mode("process_requests"sv);
//    std::ifstream inf("/home/esh/code/cpp-transport_catalogue/test_process.json");
//    if (!inf){
//        exit(1);
//    }
//    auto json_in = json::Load(inf).GetRoot();

//    transport::TransportCatalogue catalogue;
//    serialization::Settings const serialization_settings = reader::LoadSerializationSettings(json_in.AsDict().at("serialization_settings").AsDict());
//    if (mode == "make_base"sv) {
//        reader::FillTheTransportCatalogue(catalogue, json_in.AsDict().at("base_requests").AsArray());
//        renderer::Settings renderer_settings =  reader::LoadMapRendererSettings(json_in.AsDict().at("render_settings").AsDict());
//        routing::Settings routing_settings = reader::LoadRoutingSettings(json_in.AsDict().at("routing_settings").AsDict());
//        routing::TransportRouter router(catalogue, routing_settings);

//        const serialization::SavedState saved_state{ std::move(router), std::move(catalogue), std::move(renderer_settings) };

//        // TODO: handle exceptions
//        {
//            std::ofstream output(serialization_settings.path, std::ios::binary | std::ios::out | std::ios::trunc);
//            serialization::Serialize(saved_state, output);
//        }


//        /*serialization::Serialize(router, output);
//        serialization::Serialize(catalogue, output);
//        serialization::Serialize(renderer_settings, output);*/

//    } else if (mode == "process_requests"sv) {
//        std::ifstream input(serialization_settings.path, std::ios::binary | std::ios::in);
//        auto state = serialization::Deserialize(input);

//        auto catalogue = std::move(state.catalogue);
//        renderer::MapRenderer renderer(state.renderer_settings, catalogue.GetMinCoordinates(), catalogue.GetMaxCoordinates());
//        auto router = std::move(state.router);

//        reader::LoadQueries(catalogue, renderer, json_in.AsDict().at("stat_requests").AsArray(), std::cout, router);

//        /*serialization::Deserialize(input, catalogue);
//        input.clear();
//        input.seekg(0);

//        renderer::Settings renderer_settings;
//        serialization::Deserialize(input, renderer_settings);
//        renderer::MapRenderer render(renderer_settings, catalogue.GetMinCoordinates(), catalogue.GetMaxCoordinates());
//        input.clear();
//        input.seekg(0);

//        routing::TransportRouter router = serialization::Deserialize(catalogue, input);
//        reader::LoadQueries(catalogue, render, json_in.AsDict().at("stat_requests").AsArray(), std::cout, router);*/
//    } else {
//        PrintUsage();
//        return 1;
//    }
//}
