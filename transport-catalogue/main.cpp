#include <iostream>
#include <fstream>
#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

using namespace std;

int main() {
    catalogue::Transport catalogue;
    // fstream in;
    // in.open("/Users/ggevorgyan/Documents/practicum-projects/cpp-transport-catalogue/transport-catalogue/input.json");
    
    // ofstream out;
    // out.open("/Users/ggevorgyan/Documents/practicum-projects/cpp-transport-catalogue/transport-catalogue/out.json");
   
    const auto document = json::Load(std::cin);
    reader::ReadFrom(document, catalogue);

    const auto render_settings = reader::ParseRenderSettings(document);
    renderer::MapRenderer map_renderer(render_settings);

    auto map = map_renderer.Render(catalogue);
    map.Render(std::cout);
}