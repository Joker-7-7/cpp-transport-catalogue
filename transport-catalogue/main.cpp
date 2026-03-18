#include <iostream>
#include <fstream>

#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

using namespace std;

int main() {
    catalogue::Transport catalogue;

     ifstream input;
    input.open("/Users/ggevorgyan/Documents/practicum-projects/cpp-transport-catalogue/transport-catalogue/input.json");

    const auto document = json::Load(input);
    reader::ReadFrom(document, catalogue);

    const auto render_settings = reader::ParseRenderSettings(document);
    renderer::MapRenderer map_renderer(render_settings);

    reader::RequestHandler handler(catalogue, map_renderer);
    handler.ReadJsonRequests(document, std::cout);
}