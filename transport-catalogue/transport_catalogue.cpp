#include "transport_catalogue.h"

#include <unordered_set>
#include <algorithm>

using namespace utils;
using namespace catalogue;

void Transport::AddRoute(std::string_view id, const std::vector<std::string_view>& route) {
    std::vector<std::string> new_route;
    new_route.reserve(route.size());
    for(auto& stop : route) {
        new_route.push_back(std::string(stop));     
    }
    auto it = routes_.emplace(routes_.end(), Route{std::string(id), std::move(new_route)});
    ref_routes_.emplace(it->name, &*it);
    
    for(auto& stop : it->stops) {
        stop_to_routes_[stop].push_back(it->name);
    }
}

void Transport::AddBusStop(std::string_view name, Coordinates coord) {
    auto it = stops_.emplace(stops_.end(), Stop{std::string(name), coord});
    ref_stops_.emplace(it->name, &*it);
    stop_to_routes_[name];
}

 const Transport::Route* Transport::SearchRoute(std::string_view id) const {
    if (auto it = ref_routes_.find(id); it != ref_routes_.end()) {
        return it->second;
    }
    return nullptr;
}

const Transport::Stop* Transport::SearchBusStop(std::string_view name) const {
    if (auto it = ref_stops_.find(name); it != ref_stops_.end()) {
        return it->second;
    }
    return nullptr;
}

std::optional<Transport::RouteInfo> Transport::GetRouteInfo(const std::string& name) const {
    const auto* route = SearchRoute(name);
    if(route == nullptr) {
        return std::nullopt;
    }
    Transport::RouteInfo info;
    info.stops_count = route->stops.size();

    std::unordered_set<std::string_view> unique_elements(route->stops.begin(), route->stops.end());
    info.unique_stops_count = unique_elements.size();
    
    double length = 0.0;
    for(size_t i = 0; i < route->stops.size() - 1; ++i) {
        auto* start = SearchBusStop(route->stops[i]);
        auto* end = SearchBusStop(route->stops[i+1]);
        length += ComputeDistance(start->coord, end->coord);
    }
    info.length = length;
    return info;
}

std::optional<std::set<std::string>> Transport::GetBusesInfo(const std::string& name) const {
    auto stop = stop_to_routes_.find(name);
    if(stop == stop_to_routes_.end()) {
         return std::nullopt;
    }

    std::set<std::string> unique_elements(stop->second.begin(), stop->second.end());
    return unique_elements;
}