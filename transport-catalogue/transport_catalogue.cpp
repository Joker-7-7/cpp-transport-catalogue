#include "transport_catalogue.h"

#include <unordered_set>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace utils;
using namespace catalogue;

void Transport::AddRoute(const std::string& id, const std::vector<std::string_view>& route)
{
    std::vector<std::string> res;
    res.reserve(route.size());
    for(auto& el : route)
        res.push_back(std::string(el));
    routes_[id] = res;
}

void Transport::AddBusStop(const std::string& name, Coordinates coord)
{
    stops_[name] = coord;
}

 std::vector<std::string> Transport::SearchRoute(const std::string& id) const
{
    auto it = routes_.find(id);
    if(it == routes_.end()) {
        return {};
    }
    return it->second;
}

std::optional<Coordinates> Transport::SearchBusStop(const std::string& name) const
{
    auto it = stops_.find(name);
    if(it == stops_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::string Transport::GetRouteInfo(const std::string& name) const
{
    std::stringstream res;
    res << "Bus " + name + ": ";
    auto route = SearchRoute(name);
    if(route.empty()) {
        return res.str() + "not found";
    }
    
    res << std::to_string(route.size()) + " stops on route, ";

    std::unordered_set<std::string_view> unique_elements(route.begin(), route.end());
    res << std::to_string(unique_elements.size()) + " unique stops, ";
    
    double length = 0.0;
    for(size_t i = 0; i < route.size() - 1; ++i) {
        auto start = SearchBusStop(std::string(route[i]));
        auto end = SearchBusStop(std::string(route[i+1]));
        length += ComputeDistance(*start, *end);
    }
    res << std::fixed <<  std::setprecision(6) << length << " route length";
    return res.str();
}

std::string Transport::GetBusesInfo(const std::string& name) const
{
    std::stringstream res;
    res << "Stop " + name + ": ";

    if(auto bus = SearchBusStop(name); bus == std::nullopt) {
        res << "not found";
        return res.str();
    }

    std::vector<std::string_view> resBuses;
    for(auto& [stop, route] : routes_)
    {
        if(std::find(route.begin(), route.end(), name) != route.end())
        {
            resBuses.push_back(stop);
        }
    }

    if(resBuses.empty()) {
        res << "no buses";
        return res.str();
    }

    res << "buses ";
    std::sort(resBuses.begin(), resBuses.end());
    for(auto& bus : resBuses){
        res << bus << " ";
    }
    return res.str();
}
