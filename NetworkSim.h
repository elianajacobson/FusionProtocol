#pragma once

#include <boost/graph/connected_components.hpp>

template <typename Protocol, typename Topology>
class NetworkSim {
public:
	NetworkSim(const Topology & topology, Protocol protocol = Protocol()) : protocol_(protocol), topology_(topology) {}

	template <typename RNG, typename... Args>
	size_t operator()(RNG && rand, Args&&... args) {
		size_t totalConnected = 0;

		//use protocol on topology
		auto connection_results = protocol_(rand, topology_, std::forward<Args>(args)...);

		//get connected components
		std::vector<int> component(num_vertices(connection_results.graph));
		connected_components(connection_results.graph, &component[0]);

		//determine which components have both alice and bob
		std::unordered_map<int, std::pair<bool, bool>> componentToHasAliceBob;
		for (size_t i = 0; i < connection_results.aliceVertices.size(); i++) {
			componentToHasAliceBob[component[connection_results.aliceVertices[i]]].first = true;
		}
		for (size_t i = 0; i < connection_results.bobVertices.size(); i++) {
			componentToHasAliceBob[component[connection_results.bobVertices[i]]].second = true;
		}
		for (auto it = componentToHasAliceBob.begin(); it != componentToHasAliceBob.end(); it++) {
			if ((it->second.first) && (it->second.second)) {
				totalConnected++;
			}
		}

		return totalConnected;
	}
private:
	Protocol protocol_;
	Topology topology_;
};