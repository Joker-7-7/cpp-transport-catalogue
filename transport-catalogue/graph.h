#pragma once

/**
 * @file graph.h
 * Weighted directed graph used for route planning.
 *
 * Implements a directed graph with weighted edges. Each vertex is identified
 * by a size_t index. The graph stores an incidence list for fast lookup of
 * outgoing edges from any vertex.
 */

#include "ranges.h"

#include <cstdlib>
#include <vector>

/**
 * Graph algorithms and data structures.
 */
namespace graph {

/**
 * Integral type for vertex identifiers.
 */using VertexId = size_t;
/**
 * Integral type for edge identifiers.
 */using EdgeId = size_t;

/**
 * A weighted directed edge in the graph.
 *
 * @tparam Weight Numeric type used for edge weights (e.g. double, int).
 */
template <typename Weight>
struct Edge {
    /**
     * Source vertex id.
     */    VertexId from;
    /**
     * Destination vertex id.
     */    VertexId to;
    /**
     * Weight (cost) of traversing this edge.
     */    Weight weight;
};

/**
 * A weighted directed graph with adjacency-list representation.
 *
 * Vertices are numbered from 0 to vertex_count - 1. Edges are added
 * dynamically and stored in a flat list; each vertex maintains a list
 * of incident edge ids.
 *
 * @tparam Weight Numeric type used for edge weights.
 */
template <typename Weight>
class DirectedWeightedGraph {
private:
    using IncidenceList = std::vector<EdgeId>;
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

public:
    /**
     * Constructs an empty graph.
     */    DirectedWeightedGraph() = default;

    /**
     * Constructs a graph with a given number of vertices.
     * @param vertex_count Initial number of vertices (ids 0..vertex_count-1).
     */
    explicit DirectedWeightedGraph(size_t vertex_count);

    /**
     * Adds a directed edge to the graph.
     * @param edge The edge to add.
     * @return The id assigned to the new edge.
     */
    EdgeId AddEdge(const Edge<Weight>& edge);

    /**
     * Returns the total number of vertices in the graph.
     */
    size_t GetVertexCount() const;

    /**
     * Returns the total number of edges in the graph.
     */
    size_t GetEdgeCount() const;

    /**
     * Retrieves an edge by its id.
     * @param edge_id The edge's id.
     * @return Constant reference to the edge.
     */
    const Edge<Weight>& GetEdge(EdgeId edge_id) const;

    /**
     * Returns an iterable range of edge ids outgoing from a vertex.
     * @param vertex The source vertex id.
     * @return A range of incident edge ids.
     */
    IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

private:
    std::vector<Edge<Weight>> edges_;
    std::vector<IncidenceList> incidence_lists_;
};

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidence_lists_(vertex_count) {
}

template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
    edges_.push_back(edge);
    const EdgeId id = edges_.size() - 1;
    incidence_lists_.at(edge.from).push_back(id);
    return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
    return incidence_lists_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
    return edges_.size();
}

template <typename Weight>
const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
    return edges_.at(edge_id);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
    return ranges::AsRange(incidence_lists_.at(vertex));
}
}  // namespace graph