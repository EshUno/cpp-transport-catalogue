#pragma once
#include <variant>
#include <memory>

#include "router.h"
#include "transport_catalogue.h"

namespace routing {

struct Settings{
    double bus_wait_time_ = 0;
    double bus_velocity_ = 0; // m/min
};

struct WaitEdge{
    std::string stop_name;
    double time;
};

struct BusEdge{
    std::string bus_name;
    double time = .0;
    int span_count =  0;
};

using EdgeInfo = std::variant<WaitEdge, BusEdge>;

struct RoutePrintInfo{
    double total_time = 0;
    std::vector<EdgeInfo> edges;
};

class TransportRouter{
public:
    using Weight = double;
    using Graph = graph::DirectedWeightedGraph<Weight>;
    using Router = graph::Router<Weight>;
    struct Vertex {
        graph::VertexId start_wait = 0;
        graph::VertexId end_wait = 0;
    };

    TransportRouter(const transport::TransportCatalogue &transport_catalogue, routing::Settings settings);
    TransportRouter(const transport::TransportCatalogue &transport_catalogue,
                    routing::Settings settings,
                    const Graph& graph,
                    const std::unordered_map<std::string, Vertex>& stops,
                    const std::unordered_map<int, EdgeInfo>& edges);
    std::optional<RoutePrintInfo> BuildRoute(const std::string& from, const std::string& to) const;

    const routing::Settings &GetSettings() const;
    const Graph& GetGraph() const;
    const std::unordered_map<std::string, Vertex> &GetStopsForGraph() const;
    const std::unordered_map<int, EdgeInfo> &GetEdgeInfo() const;
private:
    void UpdateStopsForGraph(const transport::StopsInfo& stops);
    void UpdateGraph(const transport::BusesInfo& buses);
    template <typename It>
    void AddBus(std::string_view bus_name, It begin, It end);

    // Graph graph_;
    std::shared_ptr<Graph> graph_;
    const transport::TransportCatalogue &transport_catalogue_;
    routing::Settings settings_;
    std::unique_ptr<Router> router_ = nullptr;
    std::unordered_map<std::string, Vertex> stops_for_graph_;
    std::unordered_map<int, EdgeInfo> edge_info_;
};

template <typename It>
void TransportRouter::AddBus(std::string_view bus_name, It begin, It end){
    double const velocity = settings_.bus_velocity_;
    for (auto from = begin; from != end; ++from){
        double wait_time = 0;
        int count_stops = 1;
        const auto& from_stop = (*from)->name;
        It current = from;
        for (auto to = std::next(from); to != end; ++to, ++count_stops){
            const auto& left_stop = (*current)->name;
            const auto& right_stop = (*to)->name;
            wait_time += transport_catalogue_.ComputeRouteDistance(left_stop, right_stop) / velocity;
            graph::Edge<Weight> edge{stops_for_graph_[from_stop].end_wait, stops_for_graph_[right_stop].start_wait, wait_time};
            edge_info_[graph_->GetEdgeCount()] = BusEdge{std::string(bus_name), wait_time, count_stops };
            graph_->AddEdge(edge);
            current = to;
        }
    }
}

}  // namespace routing
