#pragma once

/**
 * @file json_reader.h
 * Reads transport catalogue data and settings from a JSON document.
 *
 * Contains functions for populating a catalogue::Transport from JSON base
 * requests, and for parsing render/routing settings from the input document.
 */

#include "geo.h"
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <string>
#include <string_view>
#include <vector>

/**
 * JSON reading and request handling.
 */
namespace reader
{
    /**
     * Parses render settings from a JSON document.
     *
     * Reads the "render_settings" section of the input document and
     * constructs a @ref renderer::RenderSettings object.
     *
     * @param document The full input JSON document.
     * @return Configured render settings.
     */
    renderer::RenderSettings ParseRenderSettings(const json::Document& document);

    /**
     * Parses routing settings from a JSON document.
     *
     * Reads the "routing_settings" section of the input document and
     * constructs a @ref router::TransportRouter::RoutingSettings object.
     *
     * @param document The full input JSON document.
     * @return Configured routing settings.
     */
    router::TransportRouter::RoutingSettings ParseRoutingSettings(const json::Document& document);

    /**
     * Populates the transport catalogue from JSON base requests.
     *
     * Reads the "base_requests" array and fills the catalogue with stops,
     * routes, and inter-stop distances.
     *
     * @param document  The full input JSON document.
     * @param catalogue The transport catalogue to populate (modified in-place).
     */
    void ReadFrom(const json::Document& document, catalogue::Transport& catalogue);
}