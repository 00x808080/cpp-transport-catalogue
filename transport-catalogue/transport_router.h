#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "domain.h"

#include <deque>
#include <vector>
#include <cmath>
#include <memory>

namespace transport_router {

struct RoutingSettings {
    size_t bus_wait_time = 0;
    size_t bus_velocity = 0;
};

class TransportRouter final {
public:
    TransportRouter(RoutingSettings routingSettings, transport_catalogue::TransportCatalogue &db);

public:
    using OptInfo = std::optional<typename graph::Router<double>::RouteInfo>;

    struct EdgeInfo {
        std::string_view bus_name;
        int span_count = 0;
    };

public:
    [[nodiscard]] size_t BusWaitTime() const;

    [[nodiscard]] const graph::Edge<double>& GetEdge(size_t edge_id) const;

    [[nodiscard]] const EdgeInfo& GetEdgeInfo(graph::EdgeId edge_id) const;

    [[nodiscard]] const graph::DirectedWeightedGraph<double>& GetGraph() const;

    [[nodiscard]] OptInfo BuildRoute(std::string_view name_from, std::string_view name_to) const;

private:
    template<typename ItBegin, typename ItEnd>
    void AddEdgesForRoute(ItBegin begin, ItEnd end, const constructions::Bus &bus) {

        for (auto stop_it = begin; stop_it != prev(end); stop_it++) {
            int span_count = 1;
            double distance = 0.0;

            for (auto current_stop_it = stop_it, next_stop_it = next(stop_it);
                 next_stop_it != end;
                 next_stop_it++, current_stop_it++) {

                distance += db_.GetDistanceBtwStops(**current_stop_it, **next_stop_it);
                double time_in_way = (distance / (static_cast<double>(routingSettings_.bus_velocity) / 0.06));
                double time = time_in_way + static_cast<double>(routingSettings_.bus_wait_time);

                graph_.AddEdge({static_cast<size_t>((*stop_it)->id),
                                      static_cast<size_t>((*next_stop_it)->id),
                                      time});

                edge_id_to_info_.push_back(EdgeInfo{bus.name, span_count});
                ++span_count;
            }
        }
    }

    void AddEdges();

    void BuildGraph();

private:
    const RoutingSettings routingSettings_;
    const transport_catalogue::TransportCatalogue &db_;
    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_ptr_;
    std::vector<EdgeInfo> edge_id_to_info_;
};

} // namespace transport_router