#include "stat_reader.h"

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

void reader::ParseAndPrintStat(const catalogue::Transport& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    size_t space_pos  = request.find(' ');
    
    if(request.substr(0, space_pos) == "Bus") {
        std::string bus = std::string(request.substr(space_pos + 1));
        auto info = transport_catalogue.GetRouteInfo(bus);
        output << std::fixed <<  std::setprecision(6) << "Bus " << bus << ": ";
        if(info.has_value()) {
            output << 
                    (*info).stops_count << " stops on route, " <<
                    (*info).unique_stops_count << " unique stops, " <<
                    (*info).length << " route length, " << 
                    (*info).curvature << " curvature" << std::endl;
       }
       else { 
            output << "not found" << std::endl;
       }
    }
    else if(request.substr(0, space_pos) == "Stop") {
        std::string stop = std::string(request.substr(space_pos + 1));
        auto buses = transport_catalogue.GetBusesInfo(stop);
         output << "Stop " << stop << ": ";
        if(buses == std::nullopt) {
            output << "not found" << std::endl;
        }
        else if(buses->size() == 0) {
            output << "no buses" << std::endl;
        }
        else {
             output << "buses ";
            for(auto& bus : *buses) {
                output << bus << " ";
            }
            output << std::endl;
        }
    }
}

void reader::ReadRequests(std::istream& input, const catalogue::Transport& transport_catalogue, std::ostream& output) {
    int stat_request_count;
    input >> stat_request_count >> std::ws;
    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        getline(input, line);
        reader::ParseAndPrintStat(transport_catalogue, line, output);
    }
}