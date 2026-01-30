#include "stat_reader.h"

#include <string>

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
