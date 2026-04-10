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
        return result;
    }
    for(auto rit = stops.rbegin() + 1; rit != stops.rend(); ++rit) {
        result.push_back((*rit).AsString());
    }

    return result;
}

void reader::ReadFrom(const json::Document& document, catalogue::Transport& catalogue) {
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
            auto is_roundtrip = request.AsMap().at("is_roundtrip"s).AsBool();
            catalogue.AddRoute( name, ParseRoute(request), is_roundtrip);
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

static svg::Color ParseColor(const json::Node& node) {
    if (node.IsString()) {
        return node.AsString();
    }

    const auto& arr = node.AsArray();
    if (arr.size() == 3) {
        return svg::Rgb{
            static_cast<uint8_t>(arr[0].AsInt()),
            static_cast<uint8_t>(arr[1].AsInt()),
            static_cast<uint8_t>(arr[2].AsInt())
        };
    }

    return svg::Rgba{
        static_cast<uint8_t>(arr[0].AsInt()),
        static_cast<uint8_t>(arr[1].AsInt()),
        static_cast<uint8_t>(arr[2].AsInt()),
        arr[3].AsDouble()
    };
}

renderer::RenderSettings reader::ParseRenderSettings(const json::Document& document) {
    const auto& root = document.GetRoot().AsMap();
    const auto& settings_map = root.at("render_settings").AsMap();

    renderer::RenderSettings settings;
    settings.width = settings_map.at("width").AsDouble();
    settings.height = settings_map.at("height").AsDouble();
    settings.padding = settings_map.at("padding").AsDouble();

    settings.line_width = settings_map.at("line_width").AsDouble();
    settings.stop_radius = settings_map.at("stop_radius").AsDouble();

    settings.bus_label_font_size = settings_map.at("bus_label_font_size").AsInt();
    {
        const auto& arr = settings_map.at("bus_label_offset").AsArray();
        settings.bus_label_offset = {arr[0].AsDouble(), arr[1].AsDouble()};
    }

    settings.stop_label_font_size = settings_map.at("stop_label_font_size").AsInt();
    {
        const auto& arr = settings_map.at("stop_label_offset").AsArray();
        settings.stop_label_offset = {arr[0].AsDouble(), arr[1].AsDouble()};
    }

    settings.underlayer_color = ParseColor(settings_map.at("underlayer_color"));
    settings.underlayer_width = settings_map.at("underlayer_width").AsDouble();

    for (const auto& color_node : settings_map.at("color_palette").AsArray()) {
        settings.color_palette.push_back(ParseColor(color_node));
    }

    return settings;
}

router::TransportRouter::RoutingSettings reader::ParseRoutingSettings(const json::Document& document) {
    const auto& root = document.GetRoot().AsMap();
    const auto& settings = root.at("routing_settings").AsMap();

    return {
        settings.at("bus_wait_time").AsInt(),
        settings.at("bus_velocity").AsDouble()
    };
}
