#pragma once

#include <vector>
#include <array>

#include <boost/graph/adjacency_list.hpp>

#include "Topology.h"
#include "vec2.h"

namespace Topologies {
	Topology<boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::property<boost::vertex_index_t, size_t>, boost::property<boost::edge_index_t, size_t>>>
	square_grid(size_t L) {
		boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::property<boost::vertex_index_t, size_t>, boost::property<boost::edge_index_t, size_t>> graph(L * L);

		std::vector<vec2> vertPos;
		vertPos.resize(L * L);

		size_t count = 0;
		for (size_t i = 0; i < L * L; i++) {
			size_t x = i % L;
			size_t y = i / L;

			vertPos[i] = vec2(x * 10, y * 10);

			if (x < L - 1) {
				add_edge(i, i + 1, count++, graph);
			}
			if (y < L - 1) {
				add_edge(i, i + L, count++, graph);
			}
		}

		return Topology<decltype(graph)>(graph, vertPos);
	}
	Topology<boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::property<boost::vertex_index_t, size_t>, boost::property<boost::edge_index_t, size_t>>>
		split_square_grid(size_t L, size_t d) {
		boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::property<boost::vertex_index_t, size_t>, boost::property<boost::edge_index_t, size_t>> graph(L * L + 4 * d);

		enum {
			BOTTOM = 0,
			BL = 1,
			BR = 2,
			LEFT = 3,
			RIGHT = 4,
			UL = 5,
			UR = 6,
			UP = 7,
			NONE
		};
		auto affected_indices = [](size_t L, size_t d) {
			std::array<std::vector<size_t>, 8> indices;
			size_t dir = BOTTOM;
			size_t dc = 1;
			size_t next = L * (((L - 1) / 2) - d) + ((L - 1) / 2);
			indices[dir].push_back(next);
			while (dir != UP) {
				switch (dir) {
				case BOTTOM:
					next += L - 1;
					if (d == 1) {
						dir = LEFT;
					}
					else {
						dir = BL;
					}
					break;
				case BL:

					next += 2 * dc;
					dir = BR;
					break;
				case BR:

					next += L - 1 - 2 * dc++;
					if (dc == d) {
						dir = LEFT;
					}
					else {
						dir = BL;
					}
					break;
				case LEFT:

					next += 2 * dc;
					dir = RIGHT;
					break;
				case RIGHT:

					next += L + 1 - 2 * dc;
					dc = d - 1;
					if (d == 1) {
						dir = UP;
					}
					else {
						dir = UL;
					}
					break;
				case UL:

					next += 2 * dc;
					dir = UR;
					break;
				case UR:

					next += L + 1 - 2 * dc--;
					if (dc == 0) {
						dir = UP;
					}
					else {
						dir = UL;
					}
					break;
				}
				indices[dir].push_back(next);
			}
			return indices;
		};
		
		std::vector<vec2> vertPos;
		vertPos.resize(L * L + 4 * d);

		auto border = affected_indices(L, d);
		auto border_dir = [&border](size_t i) {
			for (size_t dir = 0; dir < border.size(); dir++) {
				for (auto index : border[dir]) {
					if (i == index) {
						return dir;
					}
				}
			}
			return (size_t) (NONE);
		};

		size_t count = 0;
		size_t borderCount = 0;
		for (size_t i = 0; i < L * L; i++) {
			size_t x = i % L;
			size_t y = i / L;

			vertPos[i] = vec2(x * 10, y * 10);

			switch (border_dir(i)) {
			case NONE:
				if (x < L - 1) {
					size_t dir = border_dir(i + 1);
					if (dir == NONE || dir == BOTTOM || dir == BL || dir == LEFT || dir == UL || dir == UP || /*for Bob*/ dir == RIGHT) {
						add_edge(i, i + 1, count++, graph);
					}
					else {
						add_edge(i, L * L + borderCount, count++, graph);
					}
				}
				if (y < L - 1) {
					size_t dir = border_dir(i + L);
					if (dir == NONE || dir == BOTTOM || dir == BL || dir == BR || dir == LEFT || dir == RIGHT) {
						add_edge(i, i + L, count++, graph);
					}
					else if (dir == UL || dir == UP) {
						add_edge(i, L * L + borderCount + 1, count++, graph);
					}
					else if (dir == UR) {
						add_edge(i, L * L + borderCount + 2, count++, graph);
					}
				}
				break;
			case BOTTOM:
				if (x < L - 1) {
					add_edge(i, i + 1, count++, graph);
				}
				if (y < L - 1) {
					add_edge(L * L + borderCount, i + L, count++, graph);
				}
				vertPos[L * L + borderCount] = vec2(x * 10, y * 10);
				borderCount++;
				break;
			case BL:
				if (x < L - 1) {
					add_edge(L * L + borderCount, i + 1, count++, graph);
				}
				if (y < L - 1) {
					add_edge(L * L + borderCount, i + L, count++, graph);
				}
				vertPos[L * L + borderCount] = vec2(x * 10, y * 10);
				borderCount++;
				break;
			case BR:
				if (x < L - 1) {
					add_edge(i, i + 1, count++, graph);
				}
				if (y < L - 1) {
					add_edge(L * L + borderCount, i + L, count++, graph);
				}
				vertPos[L * L + borderCount] = vec2(x * 10, y * 10);
				borderCount++;
				break;
			case LEFT:
				if (x < L - 1) {
					//add_edge(L * L + borderCount, i + 1, count++, graph);
					add_edge(i, i + 1, count++, graph); // for Alice
				}
				if (y < L - 1) {
					add_edge(i, i + L, count++, graph);
				}
				vertPos[L * L + borderCount] = vec2(x * 10, y * 10);
				borderCount++;
				break;
			case RIGHT:
				if (x < L - 1) {
					add_edge(i, i + 1, count++, graph);
				}
				if (y < L - 1) {
					add_edge(i, i + L, count++, graph);
				}
				vertPos[L * L + borderCount] = vec2(x * 10, y * 10);
				borderCount++;
				break;
			case UL:
				if (x < L - 1) {
					add_edge(L * L + borderCount, i + 1, count++, graph);
				}
				if (y < L - 1) {
					add_edge(i, i + L, count++, graph);
				}
				vertPos[L * L + borderCount] = vec2(x * 10, y * 10);
				borderCount++;
				break;
			case UR:
				if (x < L - 1) {
					add_edge(i, i + 1, count++, graph);
				}
				if (y < L - 1) {
					add_edge(i, i + L, count++, graph);
				}
				vertPos[L * L + borderCount] = vec2(x * 10, y * 10);
				borderCount++;
				break;
			case UP:
				if (x < L - 1) {
					add_edge(i, i + 1, count++, graph);
				}
				if (y < L - 1) {
					add_edge(i, i + L, count++, graph);
				}
				vertPos[L * L + borderCount] = vec2(x * 10, y * 10);
				borderCount++;
				break;
			}
		}

		return Topology<decltype(graph)>(graph, vertPos);
	}
}