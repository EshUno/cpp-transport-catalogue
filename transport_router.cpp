#include "transport_router.h"
namespace routing {

TransportRouter::TransportRouter(const transport::TransportCatalogue &transport_catalogue, routing::Settings settings):
    transport_catalogue_(transport_catalogue), settings_(settings), graph_(transport_catalogue.GetStops().size() * 2){
    UpdateStopsForGraph(transport_catalogue_.GetStops());
    UpdateGraph(transport_catalogue_.GetBuses());
    router_ = std::make_unique<Router>(graph_);
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
        edge_info_[edge] = WaitEdge{name, wait_time};
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
