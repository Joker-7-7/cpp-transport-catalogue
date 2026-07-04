#pragma once

/**
 * @file request_handler.h
 * Facade that dispatches stat requests to the underlying subsystems.
 *
 * The RequestHandler aggregates the transport catalogue, map renderer, and
 * transport router. It processes the "stat_requests" section of the input
 * JSON and writes the corresponding JSON response array to an output stream.
 */

#include "svg.h"
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <iosfwd>
#include <string_view>
#include <ostream>

/**
 * JSON reading and request handling.
 */
namespace reader
{
    /**
     * Facade that simplifies interaction between the JSON reader and
     *        the other application subsystems.
     *
     * Implements the Facade design pattern. Accepts a JSON document
     * containing stat requests, delegates each request to the appropriate
     * subsystem (catalogue, map renderer, or router), and formats the
     * results as a JSON array.
     */
    class RequestHandler {
    public:
        /**
         * Constructs a handler with references to all subsystems.
         *
         * @param db       The transport catalogue (read-only).
         * @param renderer The map renderer.
         * @param router   The transport router for route-finding queries.
         */
        RequestHandler(const catalogue::Transport& db,
                    const renderer::MapRenderer& renderer,
                    const router::TransportRouter& router);

        /**
         * Processes the "stat_requests" array from the input document.
         *
         * Iterates over stat requests, dispatches each by type ("Stop", "Bus",
         * "Map", "Route"), and writes a JSON array of responses to the output
         * stream.
         *
         * @param document The full input JSON document.
         * @param output   The output stream to write the JSON response to.
         */
        void ReadJsonRequests(const json::Document& document, std::ostream& output);

        /**
         * Renders the full SVG map for the current catalogue.
         * @return An SVG document representing the transport map.
         */
        svg::Document RenderMap() const;

    private:
        const catalogue::Transport& db_;
        const renderer::MapRenderer& renderer_;
        const router::TransportRouter& router_;
    };
}