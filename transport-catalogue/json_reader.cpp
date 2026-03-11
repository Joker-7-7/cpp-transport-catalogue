#include "json_reader.h"

using namespace utils;
using namespace reader;
using namespace std::literals;

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
std::vector<Distance> ParseDistances(json::Node node) {
    auto& road_distances = node.AsMap().at("road_distances"s).AsMap();
    std::vector<Distance> res;
    res.reserve(road_distances.size());

    for(auto& [key, value] : road_distances) {
        res.push_back({key, value.AsDouble()});
    }

    return res;
}

Coordinates ParseCoordinates(json::Node node) {
    double lat = node.AsMap().at("latitude"s).AsDouble();
    double lng = node.AsMap().at("longitude"s).AsDouble();
    return {lat, lng};
}

std::vector<std::string_view> ParseRoute(json::Node node) {
    bool is_roundtrip = node.AsMap().at("is_roundtrip"s).AsBool();
    auto& stops = node.AsMap().at("stops"s).AsArray();

    std::vector<std::string_view> result;
    result.reserve(stops.size());
    for(auto& stop : stops) {
        result.push_back(stop.AsString());
    }
        
    if(is_roundtrip) {
 //       result.push_back(stops.begin()->AsString());
        return result;
    }
    for(auto rit = stops.rbegin() + 1; rit != stops.rend(); ++rit) {
        result.push_back((*rit).AsString());
    }

    return result;
}


 void JsonInput::ReadFrom(const json::Document& document, catalogue::Transport& catalogue) {
    auto& root = document.GetRoot();
    auto& base_request = root.AsMap().at("base_requests"s).AsArray();

    for (auto& request : base_request) {
        auto type = request.AsMap().at("type"s).AsString();
        if(type == "Stop") {
            auto name = request.AsMap().at("name"s).AsString();
            catalogue.AddBusStop(name, ParseCoordinates(request));
        }
        else if (type == "Bus"){ 
            auto name = request.AsMap().at("name"s).AsString();
            catalogue.AddRoute( name, ParseRoute(request));
        }
    }

    for (auto& request : base_request) {
        auto type = request.AsMap().at("type"s).AsString();
        if(type == "Stop") {
            auto name = request.AsMap().at("name"s).AsString();
            auto distances = ParseDistances(request);
            for(auto& distance : distances) {
                catalogue.AddDistance(name, distance.stop, distance.value);
            }
        }
    }
}