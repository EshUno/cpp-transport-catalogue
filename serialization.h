#pragma once
#include <filesystem>
#include <unordered_map>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"


namespace serialization {
using Path = std::filesystem::path;

struct Settings{
    Path path;
};

struct SavedState final
{
    routing::TransportRouter router;

    transport::TransportCatalogue catalogue;

    renderer::Settings renderer_settings;
};

void Serialize(const transport::TransportCatalogue& catalogue,  std::ofstream &output);
void Deserialize(std::ifstream& input, transport::TransportCatalogue& catalogue);

void Serialize(const renderer::Settings &settings,  std::ofstream &output);
void Deserialize(std::ifstream& input, renderer::Settings &settings);

void Serialize(const routing::TransportRouter &router,  std::ofstream &output);
routing::TransportRouter Deserialize(const transport::TransportCatalogue& catalogue, std::ifstream& input);

void Serialize(const SavedState& state, std::ofstream& output);
SavedState Deserialize(std::ifstream& input);

} // namespace serialization
