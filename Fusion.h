#pragma once

#include <boost/graph/adjacency_list.hpp>

namespace RepeaterProtocol {
	template <unsigned N_GHZ, typename Topology>
	struct Fusion {
	private:
		typedef typename Topology::GraphType::vertex_descriptor TVertex;

		typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> FusedGraph;
		struct FusionResult {
			FusedGraph graph;
			std::vector<FusedGraph::vertex_descriptor> aliceVertices;
			std::vector<FusedGraph::vertex_descriptor> bobVertices;
		};
	public:
		template <typename RNG>
		FusionResult operator()(RNG && rand, const Topology & topology, size_t k, double p, double q, TVertex alice, TVertex bob);
	};
}

#include "Fusion.ipp"