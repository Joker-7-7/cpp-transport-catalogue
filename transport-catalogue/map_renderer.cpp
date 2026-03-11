#include "map_renderer.h"

#include <algorithm>

using namespace renderer;
using namespace utils;

MapRenderer::MapRenderer(RenderSettings settings)
    : settings_(std::move(settings)) {
}

std::vector<const catalogue::Transport::Route*> MapRenderer::GetSortedRoutes(const catalogue::Transport& catalogue) const {
    std::vector<const catalogue::Transport::Route*> routes;
    for (const auto& route : catalogue.GetAllRoutes()) {
        routes.push_back(&route);
    }

    std::sort(routes.begin(), routes.end(), [](const auto* lhs, const auto* rhs) {
        return lhs->name < rhs->name;
    });

    return routes;
}

std::vector<Coordinates> MapRenderer::CollectRouteCoordinates(const catalogue::Transport& catalogue) const {
    std::vector<Coordinates> result;

    for (const auto& route : catalogue.GetAllRoutes()) {
        if (route.stops.empty()) {
            continue;
        }

        for (const std::string& stop_name : route.stops) {
            const auto* stop = catalogue.SearchBusStop(stop_name);
            if (stop) {
                result.push_back(stop->coord);
            }
        }
    }

    return result;
}

svg::Document MapRenderer::Render(const catalogue::Transport& catalogue) const {
    svg::Document doc;

    const auto all_coords = CollectRouteCoordinates(catalogue);
    SphereProjector projector(all_coords.begin(), all_coords.end(),
                              settings_.width, settings_.height, settings_.padding);

    const auto sorted_routes = GetSortedRoutes(catalogue);

    size_t color_index = 0;
    for (const auto* route : sorted_routes) {
        if (route->stops.empty()) {
            continue;
        }

        svg::Polyline polyline;
        polyline.SetFillColor(svg::NoneColor)
                .SetStrokeColor(settings_.color_palette[color_index % settings_.color_palette.size()])
                .SetStrokeWidth(settings_.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (const std::string& stop_name : route->stops) {
            const auto* stop = catalogue.SearchBusStop(stop_name);
            if (stop) {
                polyline.AddPoint(projector(stop->coord));
            }
        }

        doc.Add(polyline);
        ++color_index;
    }

    return doc;
}
