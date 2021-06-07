#include <unordered_map>
#include <atomic>
#include <ctime>
#include <omp.h>
#include <fstream>
#include <cinttypes>
#include <set>
#include <mpi.h>
#include <iostream>

#include "Topologies.h"
#include "Fusion.h"
#include "Fusion_Decohere.h"
#include "NetworkSim.h"

static const size_t NGHZ = 4;
static const size_t N = 1500;
static const size_t L = 100;

static const int alice = 3939;
static const int bob = 7979;

struct SimParams {
	SimParams() : p(0.0), q(0.0) {}
	SimParams(double p, double q) : p(p), q(q) {}

	double p, q;
};

std::pair<size_t, size_t> work_range(int rank, int worldSize, size_t nTasks);

static int mpiRank, mpiWorldSize;

int main() {
	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpiWorldSize);

	//prepare simulations
	auto topology = Topologies::square_grid(L);
	NetworkSim<RepeaterProtocol::Fusion_Decohere<NGHZ, decltype(topology)>, decltype(topology)> sim(topology);
	
	std::set<size_t> k;
	std::set<double> p;
	std::set<double> q;
	std::set<double> mu;

	std::ifstream parameters;
	parameters.open("./parameters.txt");
	std::string line;
	auto tokenize = [](const std::string & str) {
		std::vector<std::string> tokens;

		size_t tokB = 0, tokE;
		do {
			tokE = str.find(' ', tokB);
			tokens.push_back(str.substr(tokB, tokE - tokB));
			tokB = tokE + 1;
		} while (tokE != str.npos);

		return tokens;
	};
	for (size_t i = 0; getline(parameters, line); i++) {
		std::vector<std::string> token = tokenize(line);
		if (token[0] == "k") {
			for (size_t i = 1; i < token.size(); i++) {
				k.insert(std::atol(token[i].c_str()));
			}
		}
		else if (token[0] == "p") {
			for (size_t i = 1; i < token.size(); i++) {
				p.insert(std::atof(token[i].c_str()));
			}
		}
		else if (token[0] == "q") {
			for (size_t i = 1; i < token.size(); i++) {
				q.insert(std::atof(token[i].c_str()));
			}
		}
		else if (token[0] == "mu") {
			for (size_t i = 1; i < token.size(); i++) {
				mu.insert(std::atof(token[i].c_str()));
			}
		}
		else if (mpiRank == 0) {
			std::cerr << "No parameter specified in parameter file line " << i << '.' << std::endl;
		}
	}

	std::vector<SimParams> params;
	params.reserve(p.size() * q.size());
	for (auto pit = p.begin(); pit != p.end(); pit++) {
		for (auto qit = q.begin(); qit != q.end(); qit++) {
			params.emplace_back(*pit, *qit);
		}
	}

	//run and time the simulations
	std::vector<std::atomic<uint32_t>> localSuccesses(p.size() * q.size());
	std::vector<std::atomic<uint32_t>> globalSuccesses(p.size() * q.size());

	for (auto mit = mu.begin(); mit != mu.end(); mit++) {
		std::ofstream output;
		if (mpiRank == 0) {
			output.open("./out/results_mu=" + std::to_string(*mit) + "_L=" + std::to_string(L) + ",A=" + std::to_string(alice) + ",B=" + std::to_string(bob) + ".txt");
			output << "k\tp\tq\tr" << std::endl;
		}

		for (auto kit = k.begin(); kit != k.end(); kit++) {
			for (int i = 0; i < localSuccesses.size(); i++) {
				localSuccesses[i] = 0;
			}
			for (int i = 0; i < globalSuccesses.size(); i++) {
				globalSuccesses[i] = 0;
			}

			size_t start, end;
			std::tie(start, end) = work_range(mpiRank, mpiWorldSize, N * localSuccesses.size());

#pragma omp parallel for
			for (int i = start; i < end; i++) {
				static thread_local std::mt19937 rand(labs(((labs(((long)(time(NULL) * 181l) * (((long)(omp_get_thread_num()) - 83l) * 359l)) % 104729l) * 181l) * (((long)(mpiRank)-83l) * 359l)) % 104729l));
				std::exponential_distribution<double> decDist(1.0 / *mit);

				size_t effI = i / N;

				localSuccesses[effI] += sim(rand, *kit, params[effI].p, params[effI].q, alice, bob, decDist);
			}

			MPI_Reduce(localSuccesses.data(), globalSuccesses.data(), localSuccesses.size(), MPI_UINT32_T, MPI_SUM, 0, MPI_COMM_WORLD);

			if (mpiRank == 0) {
				for (size_t i = 0; i < globalSuccesses.size(); i++) {
					double rate = (double)(globalSuccesses[i]) / (double)(N * *kit);
					output << *kit << '\t' << params[i].p << '\t' << params[i].q << '\t' << rate << std::endl;
				}
			}
		}

		if (mpiRank == 0) {
			output.close();
		}
	}

	MPI_Finalize();
	return 0;
}

std::pair<size_t, size_t> work_range(int rank, int worldSize, size_t nTasks) {
	std::pair<size_t, size_t> range;

	size_t localTaskCount = (size_t)(floor((double)(nTasks) / (double)(worldSize)));
	size_t diff = nTasks - worldSize * localTaskCount;

	//find start
	range.first = 0;
	for (int rankI = 0; rankI < rank; rankI++) {
		if (rankI >= worldSize - diff) {
			range.first += localTaskCount + 1;
		}
		else {
			range.first += localTaskCount;
		}
	}

	//get number of tasks
	if (rank >= worldSize - diff) {
		localTaskCount++;
	}

	//set end
	range.second = range.first + localTaskCount;

	return range;
}