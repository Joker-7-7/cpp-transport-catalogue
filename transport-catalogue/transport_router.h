#pragma once

/**
 * @file transport_router.h
 * Route finding between stops using a weighted graph.
 *
 * Builds a graph where each stop is represented by two vertices
 * (wait and ride). Wait edges model the time spent waiting for a bus,
 * and bus edges model the ride time between stops. The shortest path
 * between any two stops is found using the @ref graph::Router.
 */

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

/**
 * Route planning on top of the transport catalogue.
 */
namespace router
{
    /**
     * Builds and queries optimal routes between stops.
     *
     * Constructs an internal graph from the transport catalogue and
     * routing settings, then uses a shortest-path algorithm to answer
     * BuildRoute() queries.
     */
    class TransportRouter {
    public:
        /**
         * Configuration for route computation.
         */
        struct RoutingSettings {
            /**
             * Time spent waiting for a bus at a stop, in minutes.
             */            int bus_wait_time = 0;
            /**
             * Bus speed in km/h.
             */            double bus_velocity = 0.0;
        };

        /**
         * Type of a step in the computed route.
         */
        enum class RouteItemType {
            Wait, ///< Waiting for a bus at a stop.
            Bus   ///< Riding a bus between stops.
        };

        /**
         * A single step in a computed route.
         */
        struct RouteItem {
            /**
             * Whether this step is waiting or riding.
             */            RouteItemType type;
            /**
             * Stop name (for Wait) or bus name (for Bus).
             */            std::string name;
            /**
             * Number of consecutive stops traversed (Bus only).
             */            int span_count = 0;
            /**
             * Time in minutes for this step.
             */            double time = 0.0;
        };

        /**
         * The result of a successful route query.
         */
        struct RouteResult {
            /**
             * Total travel time in minutes.
             */            double total_time = 0.0;
            /**
             * Ordered list of steps.
             */            std::vector<RouteItem> items;
        };

        /**
         * Constructs the router from the catalogue and settings.
         *
         * Builds the internal graph by creating wait-ride vertex pairs for
         * each stop and adding edges for wait times and bus rides.
         *
         * @param db       The transport catalogue (must outlive the router).
         * @param settings Routing parameters (wait time, bus velocity).
         */
        TransportRouter(const catalogue::Transport& db, RoutingSettings settings);

        /**
         * Finds the fastest route between two stops.
         *
         * @param from Name of the origin stop.
         * @param to   Name of the destination stop.
         * @return RouteResult with total time and step-by-step directions,
         *         or std::nullopt if no route exists.
         */
        std::optional<RouteResult> BuildRoute(std::string_view from, std::string_view to) const;

    private:
        /**
         * Metadata stored alongside each graph edge for later route reconstruction.
         */
        struct EdgeInfo {
            RouteItemType type;
            std::string name;
            int span_count = 0;
            double time = 0.0;
        };

        /**
         * Returns the "wait" vertex id for a stop.
         *
         * Each stop gets two vertices: even = wait, odd = ride.
         */
        graph::VertexId GetWaitVertex(std::string_view stop) const;

        /**
         * Returns the "ride" vertex id for a stop.
         */
        graph::VertexId GetRideVertex(std::string_view stop) const;

        /**
         * Builds the full graph (wait edges + bus edges).
         */        void BuildGraph();

        /**
         * Adds edges representing waiting at a stop.
         */        void AddWaitEdges();

        /**
         * Adds edges representing bus rides between consecutive stops.
         */        void AddBusEdges();

        /**
         * Computes ride time in minutes for a given distance.
         * @param meters Distance in meters.
         * @return Ride time in minutes.
         */
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