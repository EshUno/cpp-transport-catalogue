#include "transport_router.h"
namespace routing {

TransportRouter::TransportRouter(const transport::TransportCatalogue &transport_catalogue, routing::Settings settings):
    graph_(transport_catalogue.GetStops().size() * 2), transport_catalogue_(transport_catalogue), settings_(settings) {
    UpdateStopsForGraph(transport_catalogue_.GetStops());
    UpdateGraph(transport_catalogue_.GetBuses());
    router_ = std::make_unique<Router>(graph_);
}

std::optional<RoutePrintInfo> TransportRouter::BuildRoute(std::string_view from, std::string_view to) const{
    graph::VertexId from_id = stops_for_graph_.at(from).start_wait;
    graph::VertexId to_id = stops_for_graph_.at(to).start_wait;

    auto res = router_->BuildRoute(from_id, to_id);
    if (res == std::nullopt) return std::nullopt;

    RoutePrintInfo route;
    route.total_time = res->weight;
    for (auto id: res->edges){
        route.edges.push_back(edge_info_.at(id));
    }

    return route;
}

void  TransportRouter::UpdateStopsForGraph(const transport::StopsInfo& stops){
    graph::VertexId ind = 0;
    for (auto [name, _] : stops){
        stops_for_graph_[name] = {ind, ind + 1};
        ind += 2;
    }
}

void TransportRouter::UpdateGraph(const transport::BusesInfo& buses){
    double  wait_time = settings_.bus_wait_time_;
    for(auto [name, stop]: stops_for_graph_){
        graph::Edge<Weight> edge {stop.start_wait, stop.end_wait, wait_time};
        edge_info_[graph_.GetEdgeCount()] = WaitEdge{name, wait_time};
        graph_.AddEdge(edge);
    }

    for (auto [name, bus] : buses){
        if (bus->type == transport::BusType::CircularType){

            AddBus(name, bus->stops.begin(), bus->stops.end());
        }
        else {
            AddBus(name, bus->stops.begin(), bus->stops.end());
            AddBus(name, bus->stops.rbegin(), bus->stops.rend());

        }
    }
}

}
