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
    std::string_view stop_name;
    double time;
};

struct BusEdge{
    std::string_view bus_name;
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
    std::optional<RoutePrintInfo> BuildRoute(std::string_view from, std::string_view to) const{
        graph::VertexId from_id = stops_for_graph_.at(from).start_wait;
        graph::VertexId to_id = stops_for_graph_.at(to).start_wait;

        auto res = router_->BuildRoute(from_id, to_id);
        if (res == std::nullopt) return std::nullopt;

        RoutePrintInfo route;
        route.total_time = res->weight;


        return route;

    }

private:
    void UpdateStopsForGraph(const transport::StopsInfo& stops);
    void UpdateGraph(const transport::BusesInfo& buses);
    template <typename It>
    void AddBus(std::string_view bus_name, It begin, It end);

    struct EdgeHasher{
        size_t operator()(const graph::Edge<Weight>& edge) const {
            return step * v_hasher(edge.from) +
                   step * step * v_hasher(edge.to) +
                   step * step * step * w_hasher_(edge.weight);
        }
    private:
        static const size_t step = 27;
        std::hash<graph::VertexId> v_hasher;
        std::hash<Weight> w_hasher_;
    };
    struct EdgeEqual {
        bool operator()( const graph::Edge<Weight>& lhs, const graph::Edge<Weight>& rhs ) const
        {
            return EdgeHasher()(lhs) == EdgeHasher()(rhs);
        }
    };

    Graph graph_;
    std::unique_ptr<Router> router_ = nullptr;
    const transport::TransportCatalogue &transport_catalogue_;
    routing::Settings settings_;
    std::unordered_map<std::string_view, Vertex> stops_for_graph_;
    std::unordered_map<graph::Edge<Weight>, EdgeInfo, EdgeHasher, EdgeEqual> edge_info_;

};

template <typename It>
void TransportRouter::AddBus(std::string_view bus_name, It begin, It end){
    double velocity = settings_.bus_velocity_;
    for (auto from = begin; from != end; ++from){
        double wait_time = 0;
        int count_stops = 1;
        std::string_view from_stop = (*from)->name;
        It current = from;
        for (auto to = std::next(from); to != end; ++to, ++count_stops){
            std::string_view left_stop = (*current)->name;
            std::string_view right_stop = (*to)->name;
            wait_time += transport_catalogue_.ComputeRouteDistance(left_stop, right_stop) / velocity;
            graph::Edge<Weight> edge{stops_for_graph_[from_stop].end_wait, stops_for_graph_[right_stop].start_wait, wait_time};
            graph_.AddEdge(edge);
            edge_info_[edge] = BusEdge{bus_name, wait_time, count_stops };
            current = to;
        }
    }
}


} // namespace roiting
