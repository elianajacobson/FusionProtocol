#pragma once

#include <vector>

#include <boost/graph/graph_traits.hpp>

#include "vec2.h"

template <class Graph>
class Topology {
private:
	typedef std::pair<typename boost::graph_traits<Graph>::vertex_descriptor, typename boost::graph_traits<Graph>::edge_descriptor> Connection;
public:
	typedef Graph GraphType;

	Topology() {}
	Topology(const Graph & graph, const std::vector<vec2> & positions) {
		set(graph, positions);
	}

	void set(const Graph & graph, const std::vector<vec2> & positions);

	Connection get_connection(size_t connectionIndex) const;
	size_t connection_index(typename boost::graph_traits<Graph>::vertex_descriptor u, typename boost::graph_traits<Graph>::edge_descriptor e) const;

	const Graph & graph() const { return graph_; }
	const std::vector<vec2> & positions() const { return positions_; }
private:
	Graph graph_;
	std::vector<size_t> connectionIndices_;
	std::vector<Connection> connections_;
	std::vector<vec2> positions_;
	std::vector<std::vector<double>> distance_;
};

#include "Topology.ipp"