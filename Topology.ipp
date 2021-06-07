#ifndef TOPOLOGY_IPP
#define TOPOLOGY_IPP

#include "Topology.h"

#include <boost/graph/copy.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>

template <typename Graph>
void Topology<Graph>::set(const Graph & graph, const std::vector<vec2> & positions) {
	boost::copy_graph(graph, graph_);
	positions_ = positions;

	distance_.resize(num_vertices(graph_));
	for (auto it = distance_.begin(); it != distance_.end(); it++) {
		it->resize(num_vertices(graph_));
	}

	connectionIndices_.resize(2 * num_edges(graph_));
	connections_.resize(2 * num_edges(graph_));

	auto vertexIndices = boost::get(boost::vertex_index, graph_);
	auto edgeIndices = boost::get(boost::edge_index, graph_);

	std::vector<double> weights(num_edges(graph_));

	typename Graph::edge_iterator it, end;
	for (std::tie(it, end) = edges(graph_); it != end; it++) {
		size_t id = edgeIndices[*it];
		auto u = source(*it, graph_);
		connectionIndices_[2 * id] = u;
		connections_[2 * id] = std::make_pair(u, *it);
		auto v = target(*it, graph_);
		connectionIndices_[2 * id + 1] = v;
		connections_[2 * id + 1] = std::make_pair(v, *it);

		weights[id] = (positions_[vertexIndices[u]] - positions_[vertexIndices[v]]).modulus();
	}

	boost::iterator_property_map<typename std::vector<double>::iterator, typename boost::property_map<Graph, boost::edge_index_t>::type>
		weightMap(weights.begin(), edgeIndices);

	boost::johnson_all_pairs_shortest_paths(graph_, distance_, vertexIndices, weightMap, 0.0);
}

template <typename Graph>
std::pair<typename boost::graph_traits<Graph>::vertex_descriptor, typename boost::graph_traits<Graph>::edge_descriptor> Topology<Graph>::get_connection(size_t connectionIndex) const {
	return connections_[connectionIndex];
}
template <typename Graph>
size_t Topology<Graph>::connection_index(typename boost::graph_traits<Graph>::vertex_descriptor u, typename boost::graph_traits<Graph>::edge_descriptor e) const {
	auto edgeIDs = boost::get(boost::edge_index, graph_);
	size_t id = edgeIDs[e];
	if (connectionIndices_[2 * id] == u) {
		return 2 * id;
	}
	else if (connectionIndices_[2 * id + 1] == u) {
		return 2 * id + 1;
	}
	else {
		return -1;
	}
}

#endif