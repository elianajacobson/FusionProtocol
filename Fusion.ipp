#ifndef FUSION_IPP
#define FUSION_IPP

#include "Fusion.h"

#include <map>
#include <vector>
#include <random>

namespace RepeaterProtocol {
	template <unsigned N_GHZ, typename Topology>
	template <typename RNG>
	typename Fusion<N_GHZ, Topology>::FusionResult Fusion<N_GHZ, Topology>::operator()(RNG && rand, const Topology & topology, size_t k, double p, double q, TVertex alice, TVertex bob) {
		FusionResult result;
		FusedGraph & fusedGraph = result.graph;
		auto & aliceVertices = result.aliceVertices;
		auto & bobVertices = result.bobVertices;

		std::uniform_real_distribution<double> dist(0.0, 1.0);

		std::vector<FusedGraph::vertex_descriptor> vertexGroups;
		vertexGroups.resize(2 * num_edges(topology.graph()) * k);
		std::vector<size_t> vertexGroupSizes;
		vertexGroupSizes.resize(2 * num_edges(topology.graph()));

		//external links
		{
			typename Topology::GraphType::edge_iterator it, end;
			for (std::tie(it, end) = edges(topology.graph()); it != end; it++) {
				typename Topology::GraphType::vertex_descriptor topoV1 = source(*it, topology.graph());
				typename Topology::GraphType::vertex_descriptor topoV2 = target(*it, topology.graph());

				//keep track of vertex groups
				size_t groupOffset1 = topology.connection_index(topoV1, *it);
				size_t groupOffset2 = topology.connection_index(topoV2, *it);
				size_t vertexOffset1 = groupOffset1 * k;
				size_t vertexOffset2 = groupOffset2 * k;
				size_t & group1Size = vertexGroupSizes[groupOffset1];
				size_t & group2Size = vertexGroupSizes[groupOffset2];

				//attempt a connection along every edge for each timestep
				for (size_t i = 0; i < k; i++) {
					//on success
					if (dist(rand) < p) {
						//add an edge to the graph
						typename FusedGraph::vertex_descriptor v1 = add_vertex(fusedGraph);
						typename FusedGraph::vertex_descriptor v2 = add_vertex(fusedGraph);
						add_edge(v1, v2, fusedGraph);

						//update vertex groups
						vertexGroups[vertexOffset1 + group1Size] = v1;
						vertexGroups[vertexOffset2 + group2Size] = v2;
						group1Size++;
						group2Size++;
					}
				}
			}
		}

		//internal links
		{
			typename Topology::GraphType::vertex_iterator it, end;
			for (std::tie(it, end) = vertices(topology.graph()); it != end; it++) {
				if (*it == alice) {
					typename Topology::GraphType::out_edge_iterator eit, eend;
					std::tie(eit, eend) = out_edges(*it, topology.graph());
					aliceVertices.reserve((eend - eit) * k);

					for (; eit != eend; eit++) {
						size_t groupOffset = topology.connection_index(*it, *eit);
						for (size_t i = k * groupOffset; i < k * groupOffset + vertexGroupSizes[groupOffset]; i++) {
							aliceVertices.push_back(vertexGroups[i]);
						}
					}
					aliceVertices.shrink_to_fit();

					continue;
				}
				else if (*it == bob) {
					typename Topology::GraphType::out_edge_iterator eit, eend;
					std::tie(eit, eend) = out_edges(*it, topology.graph());
					bobVertices.reserve((eend - eit) * k);

					for (; eit != eend; eit++) {
						size_t groupOffset = topology.connection_index(*it, *eit);
						for (size_t i = k * groupOffset; i < k * groupOffset + vertexGroupSizes[groupOffset]; i++) {
							bobVertices.push_back(vertexGroups[i]);
						}
					}
					bobVertices.shrink_to_fit();

					continue;
				}

				//get memory groups
				std::vector<std::pair<size_t, size_t>> memGroups;
				{
					typename Topology::GraphType::out_edge_iterator eit, eend;
					for (std::tie(eit, eend) = out_edges(*it, topology.graph()); eit != eend; eit++) {
						size_t groupOffset = topology.connection_index(*it, *eit);
						if (vertexGroupSizes[groupOffset] > 0) {
							memGroups.emplace_back(groupOffset * k, vertexGroupSizes[groupOffset]);
						}
					}
				}

				{
					std::vector<size_t> memGroupInd;
					std::vector<FusedGraph::vertex_descriptor> v;
					for (size_t nGHZ = N_GHZ; nGHZ > 1; nGHZ--) {
						memGroupInd.resize(nGHZ);
						v.resize(nGHZ);

						while (memGroups.size() >= nGHZ) {
							//pick n random groups by index, order indices in increasing order
							for (size_t i = 0; i < nGHZ; i++) {
								memGroupInd[i] = std::uniform_int_distribution<size_t>(0, memGroups.size() - i - 1)(rand);
								for (size_t j = 0; j < i; j++) {
									if (memGroupInd[j] <= memGroupInd[i]) {
										memGroupInd[i]++;
									}
									else {
										size_t temp = memGroupInd[i];
										memGroupInd[i] = memGroupInd[j];
										memGroupInd[j] = temp;
									}
								}
							}

							//pick vertices
							for (size_t i = 0; i < nGHZ; i++) {
								auto & memGroup = memGroups[memGroupInd[i]];
								size_t m = memGroup.first + std::uniform_int_distribution<size_t>(0, memGroup.second - 1)(rand);

								//remove the vertex from its group
								v[i] = vertexGroups[m];
								vertexGroups[m] = vertexGroups[memGroup.first + memGroup.second - 1]; //copy last mem in group then pop
								memGroup.second--;
							}

							//determine fusion success
							if (dist(rand) < q) {
								for (size_t i = 0; i < nGHZ - 1; i++) {
									add_edge(v[i], v[i + 1], fusedGraph);
								}
							}

							//remove empty groups
							for (auto indPtr = memGroupInd.rbegin(); indPtr != memGroupInd.rend(); indPtr++) {
								if (memGroups[*indPtr].second == 0) {
									memGroups[*indPtr] = memGroups.back();
									memGroups.pop_back();
								}
							}
						}
					}
				}
			}
		}

		return result;
	}
}

#endif