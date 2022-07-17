#include "request_handler.h"

using namespace std;
using namespace constructions;
using namespace request_handler;
using namespace svg;


RequestHandler::RequestHandler(const transport_guide::TransportCatalogue& db,
                               const renderer::MapRenderer& renderer,
                               const transport_router::TransportRouter &transportRouter)
        : db_(db)
        , renderer_(renderer)
        , transportRouter_(transportRouter) {
}

optional<BusStat> RequestHandler::GetBusStat(const string_view& bus_name) const {
    BusStat stat;
    if (!db_.ContainsBus(bus_name)) return nullopt;
    Bus route = *db_.FindRoute(bus_name);
    if (!route.stops.empty() && !route.isRoundTrip) {
        for (int i = static_cast<int>(route.stops.size()) - 2; i != -1; i--) {
            route.stops.push_back(route.stops[i]);
        }
    }
    stat.stopsCount = static_cast<int>(route.stops.size());

    std::set<std::string_view> unique_stops;
    for (const constructions::Stop *stop : route.stops) {
        unique_stops.insert(stop->name);
    }
    stat.uniqueStopsCount = static_cast<int>(unique_stops.size());

    double distance = 0;
    double routeLength = 0;
    for (int i = 0; i < stat.stopsCount - 1; i++) {
        routeLength  += ComputeDistance(route.stops[i]->coordinates,
                                        route.stops[i + 1]->coordinates);

        distance += db_.GetDistanceBtwStops(*route.stops[i],
                                            *route.stops[i + 1]);
    }
    stat.curvature = distance / routeLength ;
    stat.routeLength = static_cast<int>(routeLength * stat.curvature);

    return { stat };
}

std::string_view RequestHandler::GetStopNameFromId(size_t id) const {
    return db_.GetStops()[id].name;
}

const set<string_view>* RequestHandler::GetBusesByStop(const string_view &stop_name) const {
    return db_.GetBusesByStop(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.RenderMap();
}

size_t RequestHandler::BusWaitTime() const {
    return transportRouter_.BusWaitTime();
}

const graph::Edge<double> &RequestHandler::GetEdge(size_t edge_id) const {
    return transportRouter_.GetEdge(edge_id);
}

const transport_router::TransportRouter::EdgeInfo& RequestHandler::GetEdgeInfo(graph::EdgeId edge_id) const {
    return transportRouter_.GetEdgeInfo(edge_id);
}

RequestHandler::OptInfo RequestHandler::BuildRouteBtwStops(std::string_view stop_name_from,
                                                           std::string_view stop_name_to) const {
    return transportRouter_.BuildRoute(stop_name_from, stop_name_to);
}
