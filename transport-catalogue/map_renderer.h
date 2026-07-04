#pragma once

/**
 * @file map_renderer.h
 * Renders the transport catalogue map as an SVG document.
 *
 * Uses a spherical projection to map geographic coordinates onto a 2D plane,
 * then draws route polylines, route labels, stop markers, and stop labels
 * with customizable rendering settings.
 */

#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <optional>
#include <cmath>

#include "svg.h"
#include "transport_catalogue.h"

/**
 * Map rendering and coordinate projection.
 */
namespace renderer {

/**
 * Configuration for map rendering.
 *
 * Holds dimensions, colors, font sizes, offsets, and other visual parameters
 * used by @ref MapRenderer to produce the SVG output.
 */
struct RenderSettings {
    /**
     * Canvas width in pixels.
     */    double width = 0.0;
    /**
     * Canvas height in pixels.
     */    double height = 0.0;
    /**
     * Padding from canvas edges in pixels.
     */    double padding = 0.0;

    /**
     * Stroke width of route polylines.
     */    double line_width = 0.0;
    /**
     * Radius of stop circle markers.
     */    double stop_radius = 0.0;

    /**
     * Font size for bus (route) name labels.
     */    int bus_label_font_size = 0;
    /**
     * Offset of bus label text from its anchor point.
     */    svg::Point bus_label_offset;

    /**
     * Font size for stop name labels.
     */    int stop_label_font_size = 0;
    /**
     * Offset of stop label text from its anchor point.
     */    svg::Point stop_label_offset;

    /**
     * Color of the underlayer (halo) drawn behind text.
     */    svg::Color underlayer_color;
    /**
     * Stroke width of the underlayer.
     */    double underlayer_width = 0.0;

    /**
     * Palette of colors cycled through for route lines and labels.
     */    std::vector<svg::Color> color_palette;
};

/**
 * Tolerance for floating-point comparisons.
 */inline const double EPSILON = 1e-6;

/**
 * Checks whether a value is effectively zero.
 * @param value The value to test.
 * @return true if |value| < EPSILON.
 */
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

/**
 * Projects geographic coordinates onto a 2D plane using a spherical Mercator-like projection.
 *
 * Computes a uniform zoom coefficient so that all input coordinates fit
 * within the specified canvas dimensions (minus padding). Latitude decreases
 * downward (y-axis), longitude increases rightward (x-axis).
 */
class SphereProjector {
public:
    /**
     * Constructs a projector from a range of geographic coordinates.
     *
     * @tparam PointInputIt Iterator over @ref utils::Coordinates values.
     * @param points_begin Start of the coordinate range.
     * @param points_end   End of the coordinate range.
     * @param max_width    Available canvas width in pixels.
     * @param max_height   Available canvas height in pixels.
     * @param padding      Padding from canvas edges in pixels.
     */
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) {
                return lhs.lng < rhs.lng;
            });

        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) {
                return lhs.lat < rhs.lat;
            });

        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    /**
     * Projects geographic coordinates to a screen point.
     * @param coords Geographic coordinates (lat, lng).
     * @return Corresponding @ref svg::Point on the canvas.
     */
    svg::Point operator()(utils::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_ = 0.0;
    double min_lon_ = 0.0;
    double max_lat_ = 0.0;
    double zoom_coeff_ = 0.0;
};

/**
 * Renders a transport catalogue as an SVG map.
 *
 * Orchestrates the rendering pipeline: projection setup, route line drawing,
 * route label placement, stop marker drawing, and stop label placement.
 */
class MapRenderer {
public:
    /**
     * Constructs a renderer with the given visual settings.
     * @param settings Rendering configuration (colors, sizes, offsets, etc.).
     */
    explicit MapRenderer(RenderSettings settings);

    /**
     * Produces the full SVG map document.
     * @param catalogue The transport catalogue to render.
     * @return An @ref svg::Document containing all map elements.
     */
    svg::Document Render(const catalogue::Transport& catalogue) const;

private:
    /**
     * Collects all routes sorted alphabetically by name.
     */
    std::vector<const catalogue::Transport::Route*> GetSortedRoutes(
        const catalogue::Transport& catalogue) const;

    /**
     * Gathers all coordinates from every stop in every route.
     */
    std::vector<utils::Coordinates> CollectRouteCoordinates(
        const catalogue::Transport& catalogue) const;

    /**
     * Returns stops that appear in at least one route, sorted by name.
     */
    std::vector<const catalogue::Transport::Stop*> GetSortedUsedStops(
        const catalogue::Transport& catalogue) const;

    /**
     * Returns the endpoint stop names where route labels should be placed.
     *
     * For round-trip routes the single origin is returned. For linear routes
     * both the first and the midpoint stop are returned (if different).
     */
    std::vector<std::string> GetRouteEndpoints(const catalogue::Transport::Route& route) const;

    /**
     * Renders polylines for each route.
     */
    void RenderRouteLines(svg::Document& doc,
                          const catalogue::Transport& catalogue,
                          const SphereProjector& projector,
                          const std::vector<const catalogue::Transport::Route*>& routes) const;

    /**
     * Renders route name labels (with underlayer) at route endpoints.
     */
    void RenderRouteLabels(svg::Document& doc,
                           const catalogue::Transport& catalogue,
                           const SphereProjector& projector,
                           const std::vector<const catalogue::Transport::Route*>& routes) const;

    /**
     * Renders circle markers for stops.
     */
    void RenderStopPoints(svg::Document& doc,
                          const SphereProjector& projector,
                          const std::vector<const catalogue::Transport::Stop*>& stops) const;

    /**
     * Renders stop name labels (with underlayer).
     */
    void RenderStopLabels(svg::Document& doc,
                          const SphereProjector& projector,
                          const std::vector<const catalogue::Transport::Stop*>& stops) const;

private:
    RenderSettings settings_;
};

}  // namespace renderer