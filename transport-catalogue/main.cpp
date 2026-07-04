#include <iostream>

#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

using namespace std;

int main() {
    catalogue::Transport catalogue;

    const auto document = json::Load(std::cin);
    reader::ReadFrom(document, catalogue);

    const auto render_settings = reader::ParseRenderSettings(document);
    auto routing_settings = reader::ParseRoutingSettings(document);
    
    renderer::MapRenderer map_renderer(render_settings);
    router::TransportRouter router(catalogue, routing_settings);

    reader::RequestHandler handler(catalogue, map_renderer, router);
    handler.ReadJsonRequests(document, std::cout);
}