#include "request_handler.h"
#include "json_builder.h"

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace json;
using namespace reader;
using namespace std::literals;

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */
RequestHandler::RequestHandler(const catalogue::Transport& db,
                               const renderer::MapRenderer& renderer,
                                const router::TransportRouter& router) :
    db_(db),
    renderer_(renderer),
    router_(router) 
{
}

void RequestHandler::ReadJsonRequests(const json::Document& document, std::ostream& output) {
    const auto& root = document.GetRoot();
    const auto& stat_requests = root.AsMap().at("stat_requests"s).AsArray();

    json::Builder builder{};
    auto requests_array = builder.StartArray();

    for (const auto& request : stat_requests) {
        const auto& request_map = request.AsMap();
        const auto& type = request_map.at("type"s).AsString();
        const int id = request_map.at("id"s).AsInt();

        auto response = requests_array.StartDict()
            .Key("request_id"s).Value(id);

        if (type == "Stop"s) {
            const auto& stop = request_map.at("name"s).AsString();
            auto buses = db_.GetBusesInfo(stop);

            if (!buses) {
                response.Key("error_message"s).Value("not found"s);
            } else {
                auto buses_array = response.Key("buses"s).StartArray();
                for (const auto& bus : *buses) {
                    buses_array.Value(bus);
                }
                buses_array.EndArray();
            }
        } else if (type == "Bus"s) {
            const auto& bus = request_map.at("name"s).AsString();
            auto info = db_.GetRouteInfo(bus);

            if (info) {
                response.Key("curvature"s).Value(info->curvature);
                response.Key("route_length"s).Value(info->length);
                response.Key("stop_count"s).Value(info->stops_count);
                response.Key("unique_stop_count"s).Value(info->unique_stops_count);
            } else {
                response.Key("error_message"s).Value("not found"s);
            }
        } else if (type == "Map"s) {
            std::stringstream ss;
            auto map = RenderMap();
            map.Render(ss);

            response.Key("map"s).Value(ss.str());
        }
        else if (type == "Route"s) {
            const auto& from = request_map.at("from"s).AsString();
            const auto& to = request_map.at("to"s).AsString();

            auto route = router_.BuildRoute(from, to);

            if (!route) {
                response.Key("error_message"s).Value("not found"s);
            } else {
                response.Key("total_time"s).Value(route->total_time);

                auto items = response.Key("items"s).StartArray();
                for (const auto& item : route->items) {
                    if (item.type == router::TransportRouter::RouteItemType::Wait) {
                        items.StartDict()
                            .Key("type"s).Value("Wait"s)
                            .Key("stop_name"s).Value(item.name)
                            .Key("time"s).Value(item.time)
                        .EndDict();
                    } else if (item.type == router::TransportRouter::RouteItemType::Bus) {
                        items.StartDict()
                            .Key("type"s).Value("Bus"s)
                            .Key("bus"s).Value(item.name)
                            .Key("span_count"s).Value(item.span_count)
                            .Key("time"s).Value(item.time)
                        .EndDict();
                    }
                }
                items.EndArray();
            }
        }

        response.EndDict();
    }

    json::Document out_document(requests_array.EndArray().Build());
    json::Print(out_document, output);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.Render(db_);
}