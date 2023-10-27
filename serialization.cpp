#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <transport_router.pb.h>
#include <svg.pb.h>
#include <fstream>
#include "serialization.h"

namespace
{

router_serialize::TransportRouter MakeSerializedRouter(const routing::TransportRouter& router) {
    router_serialize::TransportRouter serialize_router;
    auto settings = router.GetSettings();
    serialize_router.mutable_settings()->set_bus_wait_time(settings.bus_wait_time_);
    serialize_router.mutable_settings()->set_bus_velocity(settings.bus_velocity_);

    const auto& graph = router.GetGraph();
    serialize_router.mutable_graph()->set_vertex_count(graph.GetVertexCount());
    for (int i = 0 ; i < graph.GetEdgeCount(); ++i){
        router_serialize::Edge serialize_edge;
        auto edge = graph.GetEdge(i);
        serialize_edge.set_from(edge.from);
        serialize_edge.set_to(edge.to);
        serialize_edge.set_weight(edge.weight);
        *serialize_router.mutable_graph()->add_edge() = std::move(serialize_edge);
    }

    const auto& stops_for_graph = router.GetStopsForGraph();
    for (const auto& [name, stop] : stops_for_graph){
        router_serialize::Vertex serialize_vertex;
        serialize_vertex.set_start_wait(stop.start_wait);
        serialize_vertex.set_end_wait(stop.end_wait);
        serialize_router.mutable_stops_for_graph()->insert({name.data(), serialize_vertex});
    }

    const auto& edge_info = router.GetEdgeInfo();
    for (const auto& [id, edge]: edge_info){
        if (std::holds_alternative<routing::WaitEdge>(edge)) {
            const auto& wait = std::get<routing::WaitEdge>(edge);
            router_serialize::EdgeInfo serialize_edge;
            serialize_edge.mutable_wait()->set_name(std::string(wait.stop_name.data()));
            serialize_edge.mutable_wait()->set_time(wait.time);
            serialize_router.mutable_edge_info()->insert({id, serialize_edge});
        }
        else{
            const auto& bus = std::get<routing::BusEdge>(edge);
            router_serialize::EdgeInfo serialize_edge;
            serialize_edge.mutable_bus()->set_name(bus.bus_name.data());
            serialize_edge.mutable_bus()->set_time(bus.time);
            serialize_edge.mutable_bus()->set_span_count(bus.span_count);
            serialize_router.mutable_edge_info()->insert({id, serialize_edge});
        }
    }

    return serialize_router;
}

transport_catalogue_serialize::TransportCatalogue MakeSerializedCatalogue(const transport::TransportCatalogue& catalogue)
{
    transport_catalogue_serialize::TransportCatalogue serialize_catalogue;

    const transport::StopsInfo& stops = catalogue.GetStops();
    std::map<std::string_view, int> stops_id;
    int stop_id = 0;
    for (const auto& [sv_name, stop] : stops){
        transport_catalogue_serialize::Stop serialize_stop;
        serialize_stop.set_name(stop->name);
        serialize_stop.mutable_coord()->set_lat(stop->coord.lat);
        serialize_stop.mutable_coord()->set_lng(stop->coord.lng);
        stops_id[sv_name] = stop_id;
        ++stop_id;
        serialize_catalogue.mutable_stops()->Add(std::move(serialize_stop));
    }

    const transport::BusesInfo& buses = catalogue.GetBuses();
    for (const auto& [sv_name, bus]: buses){
        transport_catalogue_serialize::Bus serialize_bus;
        serialize_bus.set_name(bus->name);
        serialize_bus.set_bus_type_is_circular(bus->type == transport::BusType::CircularType);
        for (const auto& stop: bus->stops){
            serialize_bus.add_stops_ids(stops_id.at(stop->name));
        }
        serialize_catalogue.mutable_buses()->Add(std::move(serialize_bus));
    }

    const transport::TransportCatalogue::DistancesBetweenStops& distances = catalogue.GetDistances();

    for (const auto& [pair_of_stops, distance] : distances) {
        transport_catalogue_serialize::Distance serialize_distance;
        serialize_distance.set_from(stops_id.at(pair_of_stops.first->name));
        serialize_distance.set_to(stops_id.at(pair_of_stops.second->name));
        serialize_distance.set_distance(distance);
        serialize_catalogue.mutable_distances()->Add(std::move(serialize_distance));
    }

    return serialize_catalogue;
}

map_renderer_serialize::Settings MakeSerializedRendererSettings(const renderer::Settings& settings) {
    map_renderer_serialize::Settings serialize_settings;

    serialize_settings.mutable_image()->set_height(settings.image_.height);
    serialize_settings.mutable_image()->set_width(settings.image_.width);
    serialize_settings.mutable_image()->set_padding(settings.image_.padding);

    serialize_settings.set_line_width(settings.line_width_);
    serialize_settings.set_stop_radius(settings.stop_radius_);

    serialize_settings.mutable_bus_label()->set_font_size(settings.bus_label_.font_size);
    serialize_settings.mutable_bus_label()->mutable_offset()->set_x(settings.bus_label_.offset.x);
    serialize_settings.mutable_bus_label()->mutable_offset()->set_y(settings.bus_label_.offset.y);

    serialize_settings.mutable_stop_label()->set_font_size(settings.stop_label_.font_size);
    serialize_settings.mutable_stop_label()->mutable_offset()->set_x(settings.stop_label_.offset.x);
    serialize_settings.mutable_stop_label()->mutable_offset()->set_y(settings.stop_label_.offset.y);

    auto detect_color = [](const svg::Color& color){
        map_renderer_serialize::Color serialize_color;
        if (std::holds_alternative<std::string>(color)){
            serialize_color.set_name_string(std::get<std::string>(color));
        }
        else if (std::holds_alternative<svg::Rgb>(color)){
            svg::Rgb rgb = std::get<svg::Rgb>(color);
            serialize_color.mutable_name_rgb()->set_is_rgba(false);
            serialize_color.mutable_name_rgb()->set_red(rgb.red);
            serialize_color.mutable_name_rgb()->set_blue(rgb.blue);
            serialize_color.mutable_name_rgb()->set_green(rgb.green);
        }
        else if (std::holds_alternative<svg::Rgba>(color)){
            svg::Rgba rgba = std::get<svg::Rgba>(color);
            serialize_color.mutable_name_rgb()->set_is_rgba(true);
            serialize_color.mutable_name_rgb()->set_red(rgba.red);
            serialize_color.mutable_name_rgb()->set_blue(rgba.blue);
            serialize_color.mutable_name_rgb()->set_green(rgba.green);
            serialize_color.mutable_name_rgb()->set_opacity(rgba.opacity);
        }
        return serialize_color;
    };

    serialize_settings.mutable_underlayer()->set_width(settings.underlayer_.width);
    *serialize_settings.mutable_underlayer()->mutable_color() = detect_color(settings.underlayer_.color);

    for (const auto & color : settings.color_palette_){
        serialize_settings.mutable_color_palette()->Add(detect_color(color));
    }

    return serialize_settings;
}

transport::TransportCatalogue MakeDeserializedTransportCatalogue(const transport_catalogue_serialize::TransportCatalogue& deserialize_catalogue) {
    transport::TransportCatalogue catalogue;

    std::map<int, std::string> stops_id;
    int stop_id = 0;
    for (const auto& deserialize_stop: deserialize_catalogue.stops()){
        if(deserialize_stop.name().empty()) {
            continue;
        }
        transport::Stop stop;
        stops_id[stop_id] = deserialize_stop.name();
        ++stop_id;
        stop.name = deserialize_stop.name();
        stop.coord.lat = deserialize_stop.coord().lat();
        stop.coord.lng = deserialize_stop.coord().lng();
        catalogue.AddStop(std::move(stop));
    }

    for (const auto& deserialize_distance: deserialize_catalogue.distances()){

        auto from = stops_id.at(deserialize_distance.from());
        auto to = stops_id.at(deserialize_distance.to());
        catalogue.AddDistance(
            catalogue.FindStop(from),
            catalogue.FindStop(to),
            deserialize_distance.distance());
    }

    for (const auto& deserialize_bus: deserialize_catalogue.buses()){
        if(deserialize_bus.name().empty()) continue;
        std::vector<std::string> stops_name;
        for (const auto& deserialize_stop: deserialize_bus.stops_ids()){
            auto id = stops_id.at(deserialize_stop);
            stops_name.push_back(id);
        }
        auto bus_type = (deserialize_bus.bus_type_is_circular())?
                            transport::BusType::CircularType: transport::BusType::DirectType;
        catalogue.AddBus(deserialize_bus.name(),
                         bus_type,
                         std::move(stops_name));
    }

    return catalogue;
}

routing::TransportRouter MakeDeserializedTransportRouter(const transport::TransportCatalogue& catalogue,
                                                        const router_serialize::TransportRouter& deserialize_router) {
    routing::Settings settings;
    settings.bus_wait_time_ = deserialize_router.settings().bus_wait_time();
    settings.bus_velocity_ = deserialize_router.settings().bus_velocity();

    const auto& deserialize_graph = deserialize_router.graph();
    graph::DirectedWeightedGraph<double> graph(deserialize_graph.vertex_count());
    for (const auto &dedge: deserialize_graph.edge()){
        graph::Edge<double> edge{};
        edge.from = dedge.from();
        edge.to = dedge.to();
        edge.weight = dedge.weight();
        graph.AddEdge(edge);
    }

    std::unordered_map<std::string, routing::TransportRouter::Vertex> stops_for_graph;
    for (const auto &[name, stop]: deserialize_router.stops_for_graph()){
        routing::TransportRouter::Vertex vertex;
        vertex.start_wait = stop.start_wait();
        vertex.end_wait = stop.end_wait();
        stops_for_graph[name] = vertex;
    }

    std::unordered_map<int, routing::EdgeInfo> edge_info;
    for (const auto& [id, edge]: deserialize_router.edge_info()){
        if (edge.has_wait()){
            routing::WaitEdge wait({edge.wait().name(), edge.wait().time()});
            edge_info.insert({id, wait});
        }
        else{
            const auto count = static_cast<int>(edge.bus().span_count());
            routing::BusEdge bus({edge.bus().name(), edge.bus().time(), count});
            edge_info.insert({id, bus});
        }

    }

    return {catalogue, settings, graph, stops_for_graph, edge_info};
}

renderer::Settings MakeDeserializedRendererSettings(const map_renderer_serialize::Settings& deserialize_settings) {
    renderer::Settings settings;

    settings.image_.height = deserialize_settings.image().height();
    settings.image_.width = deserialize_settings.image().width();
    settings.image_.padding = deserialize_settings.image().padding();

    settings.line_width_ = deserialize_settings.line_width();
    settings.stop_radius_ = deserialize_settings.stop_radius();

    settings.bus_label_.font_size = deserialize_settings.bus_label().font_size();
    settings.bus_label_.offset.x = deserialize_settings.bus_label().offset().x();
    settings.bus_label_.offset.y = deserialize_settings.bus_label().offset().y();

    settings.stop_label_.font_size = deserialize_settings.stop_label().font_size();
    settings.stop_label_.offset.x = deserialize_settings.stop_label().offset().x();
    settings.stop_label_.offset.y = deserialize_settings.stop_label().offset().y();

    auto detect_color = [](const map_renderer_serialize::Color& color ) -> svg::Color{
        if (!color.IsInitialized()) {
            return std::monostate();
        }
        if (!color.name_string().empty()){
            return color.name_string();
        }
        if (!color.name_rgb().is_rgba()){
            svg::Rgb rgb;
            rgb.red = color.name_rgb().red();
            rgb.blue = color.name_rgb().blue();
            rgb.green = color.name_rgb().green();
            return rgb;
        }
        svg::Rgba rgba;
        rgba.red = color.name_rgb().red();
        rgba.blue = color.name_rgb().blue();
        rgba.green = color.name_rgb().green();
        rgba.opacity = color.name_rgb().opacity();
        return rgba;
    };

    settings.underlayer_.width = deserialize_settings.underlayer().width();
    settings.underlayer_.color = detect_color(deserialize_settings.underlayer().color());

    for (const auto &color : deserialize_settings.color_palette()){
        settings.color_palette_.push_back(std::move(detect_color(color)));
    }

    return settings;
}

}

using namespace  std;
namespace serialization {

void Serialize(const transport::TransportCatalogue& catalogue, std::ofstream &output){
    transport_catalogue_serialize::TransportCatalogue serialize_catalogue;

    const transport::StopsInfo& stops = catalogue.GetStops();
    std::map<std::string_view, int> stops_id;
    int stop_id = 0;
    for (const auto& [sv_name, stop] : stops){
        transport_catalogue_serialize::Stop serialize_stop;
        serialize_stop.set_name(stop->name);
        serialize_stop.mutable_coord()->set_lat(stop->coord.lat);
        serialize_stop.mutable_coord()->set_lng(stop->coord.lng);
        stops_id[sv_name] = stop_id;
        ++stop_id;
        serialize_catalogue.mutable_stops()->Add(std::move(serialize_stop));
    }

    const transport::BusesInfo& buses = catalogue.GetBuses();
    for (const auto& [sv_name, bus]: buses){
        transport_catalogue_serialize::Bus serialize_bus;
        serialize_bus.set_name(bus->name);
        serialize_bus.set_bus_type_is_circular(bus->type == transport::BusType::CircularType);
        for (const auto& stop: bus->stops){
            serialize_bus.add_stops_ids(stops_id.at(stop->name));
        }
        serialize_catalogue.mutable_buses()->Add(std::move(serialize_bus));
    }

    const transport::TransportCatalogue::DistancesBetweenStops& distances = catalogue.GetDistances();

    for (const auto& [pair_of_stops, distance] : distances){
        transport_catalogue_serialize::Distance serialize_distance;
        serialize_distance.set_from(stops_id.at(pair_of_stops.first->name));
        serialize_distance.set_to(stops_id.at(pair_of_stops.second->name));
        serialize_distance.set_distance(distance);
        serialize_catalogue.mutable_distances()->Add(std::move(serialize_distance));
    }
    const auto res = serialize_catalogue.SerializeToOstream(&output);
}

void Deserialize(std::ifstream& input, transport::TransportCatalogue& catalogue){
    transport_catalogue_serialize::TransportCatalogue deserialize_catalogue;
    deserialize_catalogue.ParseFromIstream(&input);
    std::map<int, std::string> stops_id;
    int stop_id = 0;
    for (const auto& deserialize_stop: deserialize_catalogue.stops()){
        if(deserialize_stop.name().empty()) {
            continue;
        }
        transport::Stop stop;
        stops_id[stop_id] = deserialize_stop.name();
        ++stop_id;
        stop.name = deserialize_stop.name();
        stop.coord.lat = deserialize_stop.coord().lat();
        stop.coord.lng = deserialize_stop.coord().lng();
        catalogue.AddStop(std::move(stop));
    }

    for (const auto& deserialize_distance: deserialize_catalogue.distances()){

        auto from = stops_id.at(deserialize_distance.from());
        auto to = stops_id.at(deserialize_distance.to());
        catalogue.AddDistance(
            catalogue.FindStop(from),
            catalogue.FindStop(to),
            deserialize_distance.distance());
    }

    for (const auto& deserialize_bus: deserialize_catalogue.buses()){
        if(deserialize_bus.name().empty()) continue;
        std::vector<std::string> stops_name;
        for (const auto& deserialize_stop: deserialize_bus.stops_ids()){
            auto id = stops_id.at(deserialize_stop);
            stops_name.push_back(id);
        }
        auto bus_type = (deserialize_bus.bus_type_is_circular())?
                            transport::BusType::CircularType: transport::BusType::DirectType;
        catalogue.AddBus(deserialize_bus.name(),
                         bus_type,
                         std::move(stops_name));
    }
}


void Serialize(const renderer::Settings &settings,  std::ofstream &output){
    map_renderer_serialize::Settings serialize_settings;

    serialize_settings.mutable_image()->set_height(settings.image_.height);
    serialize_settings.mutable_image()->set_width(settings.image_.width);
    serialize_settings.mutable_image()->set_padding(settings.image_.padding);

    serialize_settings.set_line_width(settings.line_width_);
    serialize_settings.set_stop_radius(settings.stop_radius_);

    serialize_settings.mutable_bus_label()->set_font_size(settings.bus_label_.font_size);
    serialize_settings.mutable_bus_label()->mutable_offset()->set_x(settings.bus_label_.offset.x);
    serialize_settings.mutable_bus_label()->mutable_offset()->set_y(settings.bus_label_.offset.y);

    serialize_settings.mutable_stop_label()->set_font_size(settings.stop_label_.font_size);
    serialize_settings.mutable_stop_label()->mutable_offset()->set_x(settings.stop_label_.offset.x);
    serialize_settings.mutable_stop_label()->mutable_offset()->set_y(settings.stop_label_.offset.y);

    auto detect_color = [](const svg::Color& color){
        map_renderer_serialize::Color serialize_color;
        if (std::holds_alternative<std::string>(color)){
            serialize_color.set_name_string(std::get<std::string>(color));
        }
        else if (std::holds_alternative<svg::Rgb>(color)){
            svg::Rgb rgb = std::get<svg::Rgb>(color);
            serialize_color.mutable_name_rgb()->set_is_rgba(false);
            serialize_color.mutable_name_rgb()->set_red(rgb.red);
            serialize_color.mutable_name_rgb()->set_blue(rgb.blue);
            serialize_color.mutable_name_rgb()->set_green(rgb.green);
        }
        else if (std::holds_alternative<svg::Rgba>(color)){
            svg::Rgba rgba = std::get<svg::Rgba>(color);
            serialize_color.mutable_name_rgb()->set_is_rgba(true);
            serialize_color.mutable_name_rgb()->set_red(rgba.red);
            serialize_color.mutable_name_rgb()->set_blue(rgba.blue);
            serialize_color.mutable_name_rgb()->set_green(rgba.green);
            serialize_color.mutable_name_rgb()->set_opacity(rgba.opacity);
        }
        return serialize_color;
    };

    serialize_settings.mutable_underlayer()->set_width(settings.underlayer_.width);
    *serialize_settings.mutable_underlayer()->mutable_color() = detect_color(settings.underlayer_.color);

    for (const auto & color : settings.color_palette_){
        serialize_settings.mutable_color_palette()->Add(detect_color(color));
    }

    const auto res = serialize_settings.SerializeToOstream(&output);
}

void Deserialize(std::ifstream& input, renderer::Settings &settings){
    map_renderer_serialize::Settings deserialize_settings;
    deserialize_settings.ParseFromIstream(&input);
    settings.image_.height = deserialize_settings.image().height();
    settings.image_.width = deserialize_settings.image().width();
    settings.image_.padding = deserialize_settings.image().padding();

    settings.line_width_ = deserialize_settings.line_width();
    settings.stop_radius_ = deserialize_settings.stop_radius();

    settings.bus_label_.font_size = deserialize_settings.bus_label().font_size();
    settings.bus_label_.offset.x = deserialize_settings.bus_label().offset().x();
    settings.bus_label_.offset.y = deserialize_settings.bus_label().offset().y();

    settings.stop_label_.font_size = deserialize_settings.stop_label().font_size();
    settings.stop_label_.offset.x = deserialize_settings.stop_label().offset().x();
    settings.stop_label_.offset.y = deserialize_settings.stop_label().offset().y();

    auto detect_color = [](const map_renderer_serialize::Color& color ) -> svg::Color{
        if (!color.IsInitialized()) {
            return std::monostate();
        }
        if (!color.name_string().empty()){
            return color.name_string();
        }
        if (!color.name_rgb().is_rgba()){
            svg::Rgb rgb;
            rgb.red = color.name_rgb().red();
            rgb.blue = color.name_rgb().blue();
            rgb.green = color.name_rgb().green();
            return rgb;
        }
        svg::Rgba rgba;
        rgba.red = color.name_rgb().red();
        rgba.blue = color.name_rgb().blue();
        rgba.green = color.name_rgb().green();
        rgba.opacity = color.name_rgb().opacity();
        return rgba;
    };

    settings.underlayer_.width = deserialize_settings.underlayer().width();
    settings.underlayer_.color = detect_color(deserialize_settings.underlayer().color());

    for (const auto &color : deserialize_settings.color_palette()){
        settings.color_palette_.push_back(std::move(detect_color(color)));
    }
}


void Serialize(const routing::TransportRouter &router,  std::ofstream &output){
    router_serialize::TransportRouter serialize_router;
    auto settings = router.GetSettings();
    serialize_router.mutable_settings()->set_bus_wait_time(settings.bus_wait_time_);
    serialize_router.mutable_settings()->set_bus_velocity(settings.bus_velocity_);

    const auto& graph = router.GetGraph();
    serialize_router.mutable_graph()->set_vertex_count(graph.GetVertexCount());
    for (int i = 0 ; i < graph.GetEdgeCount(); ++i){
        router_serialize::Edge serialize_edge;
        auto edge = graph.GetEdge(i);
        serialize_edge.set_from(edge.from);
        serialize_edge.set_to(edge.to);
        serialize_edge.set_weight(edge.weight);
        *serialize_router.mutable_graph()->add_edge() = std::move(serialize_edge);
    }

    const auto& stops_for_graph = router.GetStopsForGraph();
    for (const auto& [name, stop] : stops_for_graph){
        router_serialize::Vertex serialize_vertex;
        serialize_vertex.set_start_wait(stop.start_wait);
        serialize_vertex.set_end_wait(stop.end_wait);
        serialize_router.mutable_stops_for_graph()->insert({name.data(), serialize_vertex});
    }

    const auto& edge_info = router.GetEdgeInfo();
    for (const auto& [id, edge]: edge_info){
        if (std::holds_alternative<routing::WaitEdge>(edge)) {
            const auto& wait = std::get<routing::WaitEdge>(edge);
            router_serialize::EdgeInfo serialize_edge;
            serialize_edge.mutable_wait()->set_name(wait.stop_name.data());
            serialize_edge.mutable_wait()->set_time(wait.time);
            serialize_router.mutable_edge_info()->insert({id, serialize_edge});
        }
        else{
            const auto& bus = std::get<routing::BusEdge>(edge);
            router_serialize::EdgeInfo serialize_edge;
            serialize_edge.mutable_bus()->set_name(bus.bus_name.data());
            serialize_edge.mutable_bus()->set_time(bus.time);
            serialize_edge.mutable_bus()->set_span_count(bus.span_count);
             serialize_router.mutable_edge_info()->insert({id, serialize_edge});
        }
    }
    const auto res = serialize_router.SerializeToOstream(&output);
}

routing::TransportRouter Deserialize(const transport::TransportCatalogue& catalogue, std::ifstream& input){
    auto myLogHandler = [](google::protobuf::LogLevel level, const char* filename, int line,
                           const std::string& message) {
        std::cout << "error: " << message << std::endl;
    };

    google::protobuf::SetLogHandler(myLogHandler);

    router_serialize::TransportRouter deserialize_router;
    const auto res = deserialize_router.ParseFromIstream(&input);

    routing::Settings settings;
    settings.bus_wait_time_ = deserialize_router.settings().bus_wait_time();
    settings.bus_velocity_ = deserialize_router.settings().bus_velocity();

    const auto& deserialize_graph = deserialize_router.graph();
    graph::DirectedWeightedGraph<double> graph(deserialize_graph.vertex_count());
    for (const auto &dedge: deserialize_graph.edge()){
        graph::Edge<double> edge{};
        edge.from = dedge.from();
        edge.to = dedge.to();
        edge.weight = dedge.weight();
        graph.AddEdge(edge);
    }

    std::unordered_map<std::string, routing::TransportRouter::Vertex> stops_for_graph;
    for (const auto &[name, stop]: deserialize_router.stops_for_graph()){
        routing::TransportRouter::Vertex vertex;
        vertex.start_wait = stop.start_wait();
        vertex.end_wait = stop.end_wait();
        stops_for_graph[name] = vertex;
    }

    std::unordered_map<int, routing::EdgeInfo> edge_info;
    for (const auto& [id, edge]: deserialize_router.edge_info()){
        if (edge.has_wait()){
            routing::WaitEdge wait({edge.wait().name(), edge.wait().time()});
            edge_info.insert({id, wait});
        }
        else{
            const auto count = static_cast<int>(edge.bus().span_count());
            routing::BusEdge bus({edge.bus().name(), edge.bus().time(), count});
            edge_info.insert({id, bus});
        }

    }


    return {catalogue, settings, graph, stops_for_graph, edge_info};
}

void Serialize(const SavedState& state, std::ofstream& output) {
    transport_catalogue_serialize::SavedState serializedState;

    auto router = MakeSerializedRouter(state.router);
    auto catalogue = MakeSerializedCatalogue(state.catalogue);
    auto settings = MakeSerializedRendererSettings(state.renderer_settings);

    *serializedState.mutable_router() = router;
    *serializedState.mutable_catalogue() = catalogue;
    *serializedState.mutable_map_renderer_setings() = settings;

    if (!serializedState.SerializeToOstream(&output)) {
        throw std::runtime_error("unable to serialize SavedState");
    }
}

SavedState Deserialize(std::ifstream& strm) {
    transport_catalogue_serialize::SavedState deserialize_state;
    if (!deserialize_state.ParseFromIstream(&strm)) {
        throw std::runtime_error("unable to deserialize TransportCatalogue");
    }

    auto catalogue = MakeDeserializedTransportCatalogue(deserialize_state.catalogue());
    auto router = MakeDeserializedTransportRouter(catalogue, deserialize_state.router());
    auto settings = MakeDeserializedRendererSettings(deserialize_state.map_renderer_setings());

    return { std::move(router), std::move(catalogue), std::move(settings) };
}

} // namespace serialization
