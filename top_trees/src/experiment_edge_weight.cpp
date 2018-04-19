#include <stdlib.h>
#include <iostream>
#include <ctime>

#include "examples/maximum_edge_weight.hpp"

#include "TopTree.hpp"
#include "TopologyTopTree.hpp"

#define MAX_WEIGHT 10000
#define OPS_COUNT 4

//#define VERBOSE

enum opType { ADD_EDGE, REMOVE_EDGE, GET_WEIGHT, ADD_WEIGHT };
struct operation {
	opType op;
	int vertex_a;
	int vertex_b;
	int weight;
};

std::vector<std::pair<int, int>> vertices; // pair(edge to, edge weight)
std::vector<struct operation> operations;

double run(MaximumEdgeWeight *worker) {
	// Init tree
	std::vector<int> vertex_index;
	vertex_index.push_back(worker->add_vertex(std::to_string(0)));
	for (uint i = 1; i < vertices.size(); i++) {
		vertex_index.push_back(worker->add_vertex(std::to_string(i)));
		worker->add_edge(vertex_index[i], vertex_index[vertices[i].first], vertices[i].second);
	}
	worker->initialize();

	// Start measure time and perform all operations
	// clock_t begin = clock();
	clock_t begin = clock();
	for (auto op: operations) {
		switch (op.op) {
		case ADD_EDGE: {
			#ifdef VERBOSE
				std::cerr << "Adding edge " << op.vertex_a << " and " << op.vertex_b << " with weight " << op.weight << std::endl;
			#endif
			worker->add_edge(op.vertex_a, op.vertex_b, op.weight);
		break;}
		case REMOVE_EDGE: {
			auto result = worker->remove_edge(op.vertex_a, op.vertex_b);
			#ifdef VERBOSE
				std::cerr << "Removing edge " << op.vertex_a << " and " << op.vertex_b << ": " << result << std::endl;
			#endif
		break;}
		case ADD_WEIGHT: {
			#ifdef VERBOSE
				std::cerr << "Adding weight between " << op.vertex_a << " and " << op.vertex_b << ": " << op.weight << std::endl;
			#endif
			worker->add_weight_on_path(op.vertex_a, op.vertex_b, op.weight);
		break;}
		case GET_WEIGHT: {
			auto result = worker->get_max_weight_on_path(op.vertex_a, op.vertex_b);
			#ifdef VERBOSE
				std::cerr << "Getting max weight on path " << op.vertex_a << " and " << op.vertex_b << ": " << result.max_weight << std::endl;
			#endif
		break;}
		}
	}
	clock_t end = clock();
	return double(end - begin) / CLOCKS_PER_SEC;
}

int main(int argc, char *argv[]) {
	// Init random generator
	auto seed = strtoull(argv[1], NULL, 16);
	srand(seed);
	// Get size of tree and number of operations
	int N = atoi(argv[2]);
	int K = atoi(argv[3]);

	// Generate tree and list of operations
	// a) original graph = each vertex is connected to one with lower number
	vertices.push_back(std::pair<int,int>(0,0));
	for (int i = 1; i < N; i++) vertices.push_back(std::pair<int,int>(rand() % i, rand() % MAX_WEIGHT));
	// b) operations (type and two vertices)
	for (int i = 0; i < K; i++) {
		struct operation op{
			static_cast<opType>(rand() % OPS_COUNT),
			rand() % N,
			rand() % N,
			rand() % MAX_WEIGHT
		};
		operations.push_back(op);
	};

	// Run both implementations
	auto time_top_tree = run(new MaximumEdgeWeight(new TopTree::TopTree()));
	//auto time_topology_top_tree = run(new MaximumEdgeWeight(new TopTree::TopologyTopTree()));
	double time_topology_top_tree = 0;

	std::cout << time_top_tree << " " << time_top_tree / K << " " << time_topology_top_tree << " " << time_topology_top_tree / K << std::endl;
}
