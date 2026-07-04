# cpp-transport-catalogue

Transport Catalogue — a console application for storing and querying public transport data.

## Overview

The application reads a JSON input file containing a database of bus stops and routes, processes user queries, and writes a JSON response to standard output. Supported features:

- **Stop queries** — list all buses serving a given stop.
- **Bus queries** — route statistics: stop count, unique stops, length, curvature.
- **Map rendering** — SVG map generation with route polylines, stop markers, and labels.
- **Route planning** — optimal path between two stops (time-based, with wait and ride segments).

## Requirements

- **C++20** compiler (Clang or GCC)
- **CMake** >= 3.19
- **Make** (or any CMake-compatible build system)

## Build

```sh
cd cpp-transport-catalogue
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

The executable is `build/cpp-transport-catalogue`.

## Usage

The application reads JSON from `stdin` and writes the result to `stdout`:

```sh
./cpp-transport-catalogue < input.json > output.json
```

## Input Format

The input JSON has three sections:

### `base_requests` — Transport data

Stops and routes that form the catalogue:

```json
{
  "base_requests": [
    {
      "type": "Stop",
      "name": "Riviersky Way",
      "latitude": 43.587,
      "longitude": 39.724,
      "road_distances": {
        "Morskaya Street": 850
      }
    },
    {
      "type": "Bus",
      "name": "14",
      "stops": ["Riviersky Way", "Morskaya Street", "Park", "Riviersky Way"],
      "is_roundtrip": true
    }
  ]
}
```

- `is_roundtrip: true` — circular route; stops are used as-is.
- `is_roundtrip: false` — linear route; the return path is automatically appended (stops in reverse, excluding the first).

### `stat_requests` — Queries

```json
{
  "stat_requests": [
    { "id": 1, "type": "Stop", "name": "Riviersky Way" },
    { "id": 2, "type": "Bus", "name": "14" },
    { "id": 3, "type": "Map" },
    { "id": 4, "type": "Route", "from": "Riviersky Way", "to": "Park" }
  ]
}
```

### `render_settings` — Map visualization

```json
{
  "render_settings": {
    "width": 1200,
    "height": 800,
    "padding": 50,
    "line_width": 14,
    "stop_radius": 5,
    "bus_label_font_size": 20,
    "bus_label_offset": [7, 15],
    "stop_label_font_size": 18,
    "stop_label_offset": [7, -3],
    "underlayer_color": [255, 255, 255, 0.85],
    "underlayer_width": 3,
    "color_palette": ["green", [255, 160, 0], "red"]
  }
}
```

Colors can be specified as:
- **String**: `"red"`, `"green"`, `"blue"`, etc.
- **RGB array**: `[255, 160, 0]`
- **RGBA array**: `[255, 255, 255, 0.85]`

### `routing_settings` — Route planner

```json
{
  "routing_settings": {
    "bus_wait_time": 6,
    "bus_velocity": 40
  }
}
```

- `bus_wait_time` — minutes spent waiting for a bus at a stop.
- `bus_velocity` — bus speed in km/h.

## Output Format

```json
[
  {
    "buses": ["14", "22"],
    "request_id": 1
  },
  {
    "curvature": 1.23,
    "request_id": 2,
    "route_length": 5430,
    "stop_count": 8,
    "unique_stop_count": 5
  },
  {
    "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg...",
    "request_id": 3
  },
  {
    "items": [
      { "stop_name": "Riviersky Way", "time": 6, "type": "Wait" },
      { "bus": "14", "span_count": 2, "time": 8.5, "type": "Bus" }
    ],
    "request_id": 4,
    "total_time": 14.5
  }
]
```

## Project Structure

```
transport-catalogue/
├── CMakeLists.txt             # Build configuration (C++20)
├── README.md
├── input.json                 # Sample input file
└── transport-catalogue/
    ├── main.cpp               # Entry point
    ├── domain.h               # Domain entity placeholder
    ├── geo.h                  # Coordinates & haversine distance
    ├── graph.h                # Weighted directed graph
    ├── router.h               # Floyd-Warshall shortest-path router
    ├── json.h                 # JSON parser & printer
    ├── json.cpp
    ├── json_builder.h         # Fluent JSON builder with context validation
    ├── json_builder.cpp
    ├── json_reader.h          # Reads catalogue data & settings from JSON
    ├── json_reader.cpp
    ├── svg.h                  # SVG document model (Circle, Polyline, Text, etc.)
    ├── svg.cpp
    ├── transport_catalogue.h  # Core catalogue: stops, routes, distances
    ├── transport_catalogue.cpp
    ├── map_renderer.h         # SVG map rendering with spherical projection
    ├── map_renderer.cpp
    ├── transport_router.h     # Route planner: graph-based path finding
    ├── transport_router.cpp
    ├── request_handler.h      # Facade dispatching stat requests
    ├── request_handler.cpp
    └── ranges.h               # Iterator range wrapper
```

## Architecture

```
┌──────────────┐
│   main.cpp   │
└──────┬───────┘
       │ JSON input
       ▼
┌──────────────┐     ┌──────────────────┐
│ json_reader  │────▶│ TransportCatalogue│
│ ParseSettings│     │ (stops, routes,  │
│ ReadFrom     │     │  distances)       │
└──────────────┘     └────────┬─────────┘
                              │
         ┌────────────────────┼────────────────────┐
         ▼                    ▼                    ▼
┌─────────────────┐  ┌───────────────┐  ┌──────────────────┐
│   MapRenderer   │  │TransportRouter│  │  RequestHandler  │
│ (SVG map)       │  │ (path finding)│  │  (Facade)        │
└────────┬────────┘  └───────┬───────┘  └────────┬─────────┘
         │                   │                    │
         │    ┌──────────────┘                    │
         │    │  graph::DirectedWeightedGraph     │
         │    │  graph::Router (Floyd-Warshall)   │
         │    │                                   │
         └────┼───────────────────────────────────┘
              ▼
      JSON output (std::cout)
```

### Key design decisions

- **Stable addresses**: Stops and routes are stored in `std::deque` to ensure pointers remain valid after insertions. Hash maps index by `string_view` → `const T*` for O(1) lookup without string copies.

- **Two-vertex-per-stop graph**: For route planning, each stop is represented by two vertices — a *wait* vertex (where passengers board) and a *ride* vertex (where passengers travel). Wait edges model the time spent waiting; bus edges model the ride time between consecutive stops.

- **Spherical projection**: Map rendering uses a uniform zoom coefficient computed from the bounding box of all coordinates, ensuring the entire route network fits the canvas with equal scaling in both axes.

- **Fluent JSON builder**: `json::Builder` uses context classes (`KeyItemContext`, `DictItemContext`, `ArrayItemContext`) returned by each method to enforce correct JSON structure at compile time, preventing malformed output like values without keys or misplaced `EndDict`/`EndArray` calls.

- **Floyd-Warshall algorithm**: `graph::Router` precomputes all-pairs shortest paths once at construction time. After that, `BuildRoute()` queries are O(1) for the weight lookup + O(E) for path reconstruction.
