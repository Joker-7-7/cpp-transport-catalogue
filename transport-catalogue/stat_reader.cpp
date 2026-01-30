#include "stat_reader.h"

#include <string>
#include <iostream>

void reader::ParseAndPrintStat(const catalogue::Transport& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    // Реализуйте самостоятельно
    size_t space_pos  = request.find(' ');
    
    if(request.substr(0, space_pos) == "Bus") {
        output << transport_catalogue.GetRouteInfo(std::string(request.substr(space_pos + 1))) << std::endl;
    }
    else if(request.substr(0, space_pos) == "Stop") {
        output << transport_catalogue.GetBusesInfo(std::string(request.substr(space_pos + 1))) << std::endl;
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
