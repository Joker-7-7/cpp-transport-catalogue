#pragma once

/**
 * @file transport_catalogue.h
 * Core transport catalogue: stops, routes, and distances.
 *
 * Stores bus stops and routes in a two-way index for efficient lookup
 * by name. Supports queries for route information (stop count, unique
 * stops, length, curvature) and stop information (which buses serve
 * a given stop).
 */

#include "geo.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <optional>
#include <deque>
#include <set>

/**
 * Catalogue namespace containing the core data store.
 */
namespace catalogue
{
    /**
     * Manages the collection of bus stops, routes, and inter-stop distances.
     *
     * Provides insertion methods (used during JSON parsing) and query
     * methods (used during stat request handling). Stops and routes are
     * stored in std::deque for stable addresses, enabling pointer-based
     * indexing in hash maps.
     */
    class Transport {
    public:
        /**
         * A bus stop with a name and geographic coordinates.
         */
        struct Stop {
            /**
             * Stop name (unique identifier).
             */            std::string name;
            /**
             * Geographic coordinates of the stop.
             */            utils::Coordinates coord;
        };

        /**
         * A bus route with a name, an ordered list of stops, and a type flag.
         */
        struct Route {
            /**
             * Route name (unique identifier).
             */            std::string name;
            /**
             * Ordered names of stops on the route.
             */            std::vector<std::string> stops;
            /**
             * true if the route is circular (round-trip).
             */            bool is_roundtrip_ = false;
        };

        /**
         * Computed information about a route.
         */
        struct RouteInfo {
            /**
             * Total number of stops on the route.
             */            int stops_count = 0;
            /**
             * Number of unique stops on the route.
             */            int unique_stops_count = 0;
            /**
             * Actual road distance of the route in meters.
             */            double length = 0.0;
            /**
             * Curvature (actual distance / geographic distance).
             */            double curvature = 1.0;
        };

        /**
         * Information about buses serving a stop.
         */
        struct StopInfo {
            /**
             * Stop name.
             */            std::string name;
            /**
             * Pointer to the list of bus names serving this stop.
             */            std::vector<std::string>* buses;
        };

        /**
         * Hash functor for pairs of Stop pointers (used in distance map).
         */
        struct PairHash {
            /**
             * Combines hashes of two Stop pointers.
             */
            size_t operator()(const std::pair<const Stop*, const Stop*>& p) const noexcept {
                size_t h1 = std::hash<const Stop*>{}(p.first);
                size_t h2 = std::hash<const Stop*>{}(p.second);
                return h1 * 37 + h2;
            }
        };

        /**
         * Adds a route to the catalogue.
         *
         * @param id           Route name.
         * @param route        Ordered list of stop names.
         * @param is_roundtrip Whether the route is circular.
         */
        void AddRoute(std::string_view id, const std::vector<std::string_view>& route, bool is_roundtrip);

        /**
         * Adds a bus stop to the catalogue.
         *
         * @param name  Stop name.
         * @param coord Geographic coordinates of the stop.
         */
        void AddBusStop(std::string_view name, utils::Coordinates coord);

        /**
         * Sets the road distance between two stops.
         *
         * @param a        Name of the first stop.
         * @param b        Name of the second stop.
         * @param distance Road distance in meters.
         */
        void AddDistance(std::string_view a, std::string_view b, double distance);

        /**
         * Looks up a route by name.
         * @param id Route name.
         * @return Pointer to the route, or nullptr if not found.
         */
        const Route* SearchRoute(std::string_view id) const;

        /**
         * Looks up a bus stop by name.
         * @param name Stop name.
         * @return Pointer to the stop, or nullptr if not found.
         */
        const Stop* SearchBusStop(std::string_view name) const;

        /**
         * Computes route statistics.
         * @param name Route name.
         * @return RouteInfo if the route exists, std::nullopt otherwise.
         */
        std::optional<RouteInfo> GetRouteInfo(const std::string& name) const;

        /**
         * Returns the set of bus names that serve a given stop.
         * @param stop Stop name.
         * @return Sorted set of bus names, or std::nullopt if the stop doesn't exist.
         */
        std::optional<std::set<std::string>> GetBusesInfo(const std::string& stop) const;

        /**
         * Returns the road distance between two stops.
         *
         * Checks both directions (a→b and b→a). Returns 0.0 if no distance
         * has been set.
         *
         * @param from Name of the origin stop.
         * @param to   Name of the destination stop.
         * @return Road distance in meters, or 0.0 if unknown.
         */
        double GetDistance(std::string_view from, std::string_view to) const;

        /**
         * Returns all routes stored in the catalogue.
         */
        const std::deque<Route>& GetAllRoutes() const;

        /**
         * Returns all stops stored in the catalogue.
         */
        const std::deque<Stop>& GetAllStops() const;

    private:
        std::deque<Stop> stops_;
        std::deque<Route> routes_;

        std::unordered_map<std::string_view, const Route*> ref_routes_;
        std::unordered_map<std::string_view, const Stop*> ref_stops_;
        std::unordered_map<std::string_view, std::vector<std::string_view>> stop_to_routes_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, double, PairHash> distances_;
    };
}