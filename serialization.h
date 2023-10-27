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

void Serialize(const SavedState& state, std::ofstream& output);
SavedState Deserialize(std::ifstream& input);

} // namespace serialization
