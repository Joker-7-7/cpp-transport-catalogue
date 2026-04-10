#include "transport_router.h"

using namespace router;

TransportRouter::TransportRouter(const catalogue::Transport& db,
                                 RoutingSettings settings)
    : db_(db)
    , settings_(settings) {
    size_t idx = 0;
    for (const auto& stop : db_.GetAllStops()) {
        stop_to_index_[stop.name] = idx++;
    }

    graph_ = graph::DirectedWeightedGraph<double>(stop_to_index_.size() * 2);

    BuildGraph();

    router_ = std::make_unique<graph::Router<double>>(graph_);
}

graph::VertexId TransportRouter::GetWaitVertex(std::string_view stop) const {
    return stop_to_index_.at(stop) * 2;
}

graph::VertexId TransportRouter::GetRideVertex(std::string_view stop) const {
    return stop_to_index_.at(stop) * 2 + 1;
}

double TransportRouter::ComputeRideTime(double meters) const {
    return meters / (settings_.bus_velocity * 1000.0 / 60.0);
}

void TransportRouter::AddWaitEdges() {
    for (const auto& stop : db_.GetAllStops()) {
        const auto from = GetWaitVertex(stop.name);
        const auto to = GetRideVertex(stop.name);
        const double wait_time = static_cast<double>(settings_.bus_wait_time);

        const auto id = graph_.AddEdge({from, to, wait_time});
        if (id >= edge_info_.size()) {
            edge_info_.resize(id + 1);
        }

        edge_info_[id] = {
            RouteItemType::Wait,
            stop.name,
            0,
            wait_time
        };
    }
}

void TransportRouter::AddBusEdges() {
    for (const auto& route : db_.GetAllRoutes()) {
        const auto& stops = route.stops;
        const size_t n = stops.size();

        for (size_t i = 0; i + 1 < n; ++i) {
            double distance_sum = 0.0;

            for (size_t j = i + 1; j < n; ++j) {
                distance_sum += db_.GetDistance(stops[j - 1], stops[j]);

                const int span_count = static_cast<int>(j - i);
                const double ride_time = ComputeRideTime(distance_sum);

                const auto from = GetRideVertex(stops[i]);
                const auto to = GetWaitVertex(stops[j]);

                const auto id = graph_.AddEdge({from, to, ride_time});
                if (id >= edge_info_.size()) {
                    edge_info_.resize(id + 1);
                }

                edge_info_[id] = {
                    RouteItemType::Bus,
                    route.name,
                    span_count,
                    ride_time
                };
            }
        }
    }
}

std::optional<TransportRouter::RouteResult>
TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
    if (!stop_to_index_.count(from) || !stop_to_index_.count(to)) {
        return std::nullopt;
    }

    const auto route = router_->BuildRoute(GetWaitVertex(from), GetWaitVertex(to));
    if (!route) {
        return std::nullopt;
    }

    RouteResult result;
    result.total_time = route->weight;

    for (const auto edge_id : route->edges) {
        const auto& info = edge_info_[edge_id];
        result.items.push_back({
            info.type,
            info.name,
            info.span_count,
            info.time
        });
    }

    return result;
}

void TransportRouter::BuildGraph() {
    AddWaitEdges();
    AddBusEdges();
}