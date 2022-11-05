#include "transport_router.h"

using namespace graph;
using namespace transport_catalogue;
using namespace transport_router;

TransportRouter::TransportRouter(RoutingSettings routingSettings, TransportCatalogue &db)
    : routingSettings_(routingSettings)
    , db_(db)
    {
    BuildGraph();
    router_ptr_ = std::make_unique<graph::Router<double>>(Router<double>{graph_});
}

size_t TransportRouter::BusWaitTime() const {
    return routingSettings_.bus_wait_time;
}

const Edge<double> &TransportRouter::GetEdge(size_t edge_id) const {
    return graph_.GetEdge(edge_id);
}

const TransportRouter::EdgeInfo& transport_router::TransportRouter::GetEdgeInfo(EdgeId edge_id) const {
    return edge_id_to_info_[edge_id];
}

const graph::DirectedWeightedGraph<double> &TransportRouter::GetGraph() const {
    return graph_;
}

TransportRouter::OptInfo TransportRouter::BuildRoute(std::string_view name_from, std::string_view name_to) const {
    size_t from_id = db_.FindStop(name_from)->id;
    size_t to_id = db_.FindStop(name_to)->id;
    return router_ptr_->BuildRoute(from_id, to_id);
}

void TransportRouter::AddEdges() {
    for (const constructions::Bus &bus : db_.GetBusesData()) {
        AddEdgesForRoute(bus.stops.begin(), bus.stops.end(), bus);
        if (!bus.isRoundTrip) {
            AddEdgesForRoute(bus.stops.rbegin(), bus.stops.rend(), bus);
        }
    }
}

void TransportRouter::BuildGraph() {
    graph_ = graph::DirectedWeightedGraph<double>{db_.GetStopsData().size()};
    AddEdges();
}

