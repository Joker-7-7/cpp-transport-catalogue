#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace router
{
    class TransportRouter {
    public:
        struct RoutingSettings {
            int bus_wait_time = 0;
            double bus_velocity = 0.0;
        };

        enum class RouteItemType {
            Wait,
            Bus
        };

        struct RouteItem {
            RouteItemType type;
            std::string name;   // stop_name для Wait, bus для Bus
            int span_count = 0; // только для Bus
            double time = 0.0;
        };

        struct RouteResult {
            double total_time = 0.0;
            std::vector<RouteItem> items;
        };

        TransportRouter(const catalogue::Transport& db, RoutingSettings settings);

        std::optional<RouteResult> BuildRoute(std::string_view from, std::string_view to) const;

    private:
        struct EdgeInfo {
            RouteItemType type;
            std::string name;
            int span_count = 0;
            double time = 0.0;
        };

        graph::VertexId GetWaitVertex(std::string_view stop) const;
        graph::VertexId GetRideVertex(std::string_view stop) const;

        void BuildGraph();
        void AddWaitEdges();
        void AddBusEdges();

        double ComputeRideTime(double meters) const;

    private:
        const catalogue::Transport& db_;
        RoutingSettings settings_;

        graph::DirectedWeightedGraph<double> graph_;
        std::unique_ptr<graph::Router<double>> router_;

        std::unordered_map<std::string_view, size_t> stop_to_index_;
        std::vector<EdgeInfo> edge_info_;
    };
}