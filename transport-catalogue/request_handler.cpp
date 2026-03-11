#include "request_handler.h"


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
                                const renderer::MapRenderer& renderer) :
                                db_(db), renderer_(renderer)
{

}
  void RequestHandler::ReadJsonRequests(const json::Document& document, std::ostream& output)  {
    auto& root = document.GetRoot();
    auto& stat_requests = root.AsMap().at("stat_requests"s).AsArray();
    Array arr_requests;

    for (auto& request : stat_requests) {
        auto type = request.AsMap().at("type"s).AsString();
        
        if(type == "Stop") {
            auto stop = request.AsMap().at("name"s).AsString();
            auto id = request.AsMap().at("id"s).AsInt();
            Dict stop_dic;
            stop_dic["request_id"] = id;

            auto buses = db_.GetBusesInfo(stop);
            if(buses == std::nullopt) {
                stop_dic["error_message"] = "not found"s;
            }
            else {
                Array arr;
                for(auto& bus : *buses) {
                    arr.push_back(bus);
                }
                stop_dic["buses"] = arr;
            }
            arr_requests.push_back({stop_dic});
        }
        else if (type == "Bus") {
            auto& bus = request.AsMap().at("name"s).AsString();
            auto id = request.AsMap().at("id"s).AsInt();
            auto info = db_.GetRouteInfo(bus);
            Dict bus_dic;
            bus_dic["request_id"] = id;

            if(info.has_value()) {
                bus_dic["curvature"] = (*info).curvature;
                bus_dic["route_length"] = (*info).length;
                bus_dic["stop_count"] = (*info).stops_count;
                bus_dic["unique_stop_count"] = (*info).unique_stops_count;
            }
            else { 
                bus_dic["error_message"] = "not found"s;
            }
            arr_requests.push_back({bus_dic});
        }
        else if(type == "Map") {
            auto id = request.AsMap().at("id"s).AsInt();
            Dict bus_dic;
            bus_dic["request_id"] = id;
            std::stringstream ss;
            auto map = RenderMap();
            map.Render(ss);
            bus_dic["map"] = ss.str();
            arr_requests.push_back({bus_dic});
        }
    }

    Document out_document(arr_requests);
    json::Print(out_document, output);
  }

   svg::Document RequestHandler::RenderMap() const{
        return renderer_.Render(db_);
   }