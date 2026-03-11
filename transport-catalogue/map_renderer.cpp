#include "map_renderer.h"

#include <set>
#include <utility>

using namespace renderer;
using namespace utils;

MapRenderer::MapRenderer(RenderSettings settings)
    : settings_(std::move(settings)) {
}

std::vector<const catalogue::Transport::Route*> MapRenderer::GetSortedRoutes(
    const catalogue::Transport& catalogue) const {

    std::vector<const catalogue::Transport::Route*> routes;
    routes.reserve(catalogue.GetAllRoutes().size());

    for (const auto& route : catalogue.GetAllRoutes()) {
        routes.push_back(&route);
    }

    std::sort(routes.begin(), routes.end(),
              [](const auto* lhs, const auto* rhs) {
                  return lhs->name < rhs->name;
              });

    return routes;
}

std::vector<Coordinates> MapRenderer::CollectRouteCoordinates(
    const catalogue::Transport& catalogue) const {

    std::vector<Coordinates> result;

    for (const auto& route : catalogue.GetAllRoutes()) {
        if (route.stops.empty()) {
            continue;
        }

        for (const std::string& stop_name : route.stops) {
            if (const auto* stop = catalogue.SearchBusStop(stop_name)) {
                result.push_back(stop->coord);
            }
        }
    }

    return result;
}

std::vector<const catalogue::Transport::Stop*> MapRenderer::GetSortedUsedStops(
    const catalogue::Transport& catalogue) const {

    std::set<std::string> used_stop_names;

    for (const auto& route : catalogue.GetAllRoutes()) {
        for (const std::string& stop_name : route.stops) {
            used_stop_names.insert(stop_name);
        }
    }

    std::vector<const catalogue::Transport::Stop*> result;
    result.reserve(used_stop_names.size());

    for (const std::string& stop_name : used_stop_names) {
        if (const auto* stop = catalogue.SearchBusStop(stop_name)) {
            result.push_back(stop);
        }
    }

    return result;
}

std::vector<std::string> MapRenderer::GetRouteEndpoints(
    const catalogue::Transport::Route& route) const {

    std::vector<std::string> result;

    if (route.stops.empty()) {
        return result;
    }

    result.push_back(route.stops.front());

    if (!route.is_roundtrip_) {
        const std::string& second_endpoint = route.stops[route.stops.size() / 2];
        if (second_endpoint != route.stops.front()) {
            result.push_back(second_endpoint);
        }
    }

    return result;
}

void MapRenderer::RenderRouteLines(
    svg::Document& doc,
    const catalogue::Transport& catalogue,
    const SphereProjector& projector,
    const std::vector<const catalogue::Transport::Route*>& routes) const {

    if (settings_.color_palette.empty()) {
        return;
    }

    size_t color_index = 0;

    for (const auto* route : routes) {
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
            if (const auto* stop = catalogue.SearchBusStop(stop_name)) {
                polyline.AddPoint(projector(stop->coord));
            }
        }

        doc.Add(polyline);
        ++color_index;
    }
}

void MapRenderer::RenderRouteLabels(
    svg::Document& doc,
    const catalogue::Transport& catalogue,
    const SphereProjector& projector,
    const std::vector<const catalogue::Transport::Route*>& routes) const {

    if (settings_.color_palette.empty()) {
        return;
    }

    size_t color_index = 0;

    for (const auto* route : routes) {
        if (route->stops.empty()) {
            continue;
        }

        const svg::Color route_color =
            settings_.color_palette[color_index % settings_.color_palette.size()];

        auto add_bus_label = [&](const std::string& stop_name) {
            const auto* stop = catalogue.SearchBusStop(stop_name);
            if (!stop) {
                return;
            }

            const svg::Point pos = projector(stop->coord);

            svg::Text underlayer;
            underlayer.SetPosition(pos)
                      .SetOffset(settings_.bus_label_offset)
                      .SetFontSize(settings_.bus_label_font_size)
                      .SetFontFamily("Verdana")
                      .SetFontWeight("bold")
                      .SetData(route->name)
                      .SetFillColor(settings_.underlayer_color)
                      .SetStrokeColor(settings_.underlayer_color)
                      .SetStrokeWidth(settings_.underlayer_width)
                      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            svg::Text label;
            label.SetPosition(pos)
                 .SetOffset(settings_.bus_label_offset)
                 .SetFontSize(settings_.bus_label_font_size)
                 .SetFontFamily("Verdana")
                 .SetFontWeight("bold")
                 .SetData(route->name)
                 .SetFillColor(route_color);

            doc.Add(underlayer);
            doc.Add(label);
        };

        for (const std::string& endpoint : GetRouteEndpoints(*route)) {
            add_bus_label(endpoint);
        }

        ++color_index;
    }
}

void MapRenderer::RenderStopPoints(
    svg::Document& doc,
    const SphereProjector& projector,
    const std::vector<const catalogue::Transport::Stop*>& stops) const {

    for (const auto* stop : stops) {
        svg::Circle circle;
        circle.SetCenter(projector(stop->coord))
              .SetRadius(settings_.stop_radius)
              .SetFillColor("white");

        doc.Add(circle);
    }
}

void MapRenderer::RenderStopLabels(
    svg::Document& doc,
    const SphereProjector& projector,
    const std::vector<const catalogue::Transport::Stop*>& stops) const {

    for (const auto* stop : stops) {
        const svg::Point pos = projector(stop->coord);

        svg::Text underlayer;
        underlayer.SetPosition(pos)
                  .SetOffset(settings_.stop_label_offset)
                  .SetFontSize(settings_.stop_label_font_size)
                  .SetFontFamily("Verdana")
                  .SetData(stop->name)
                  .SetFillColor(settings_.underlayer_color)
                  .SetStrokeColor(settings_.underlayer_color)
                  .SetStrokeWidth(settings_.underlayer_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text label;
        label.SetPosition(pos)
             .SetOffset(settings_.stop_label_offset)
             .SetFontSize(settings_.stop_label_font_size)
             .SetFontFamily("Verdana")
             .SetData(stop->name)
             .SetFillColor("black");

        doc.Add(underlayer);
        doc.Add(label);
    }
}

svg::Document MapRenderer::Render(const catalogue::Transport& catalogue) const {
    svg::Document doc;

    const auto all_coords = CollectRouteCoordinates(catalogue);
    SphereProjector projector(all_coords.begin(), all_coords.end(),
                              settings_.width, settings_.height, settings_.padding);

    const auto sorted_routes = GetSortedRoutes(catalogue);
    const auto used_stops = GetSortedUsedStops(catalogue);

    RenderRouteLines(doc, catalogue, projector, sorted_routes);
    RenderRouteLabels(doc, catalogue, projector, sorted_routes);
    RenderStopPoints(doc, projector, used_stops);
    RenderStopLabels(doc, projector, used_stops);

    return doc;
}