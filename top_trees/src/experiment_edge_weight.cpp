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
	int param; // used as weight when creating or as index into vector when deleting edges
};

std::vector<std::pair<int, int>> vertices; // pair(edge to, edge weight)
std::vector<struct operation> operations;

double run(MaximumEdgeWeight *worker) {
	// Vector for indexing edges
	std::vector<std::pair<int, int>> edges;

	// Init tree
	std::vector<int> vertex_index;
	vertex_index.push_back(worker->add_vertex(std::to_string(0)));
	for (uint i = 1; i < vertices.size(); i++) {
		vertex_index.push_back(worker->add_vertex(std::to_string(i)));
		worker->add_edge(vertex_index[i], vertex_index[vertices[i].first], vertices[i].second);
		edges.push_back(std::pair<int,int>(i, vertices[i].first));
	}
	worker->initialize();

	// Start measure time and perform all operations
	// clock_t begin = clock();
	clock_t begin = clock();
	for (auto op: operations) {
		switch (op.op) {
		case ADD_EDGE: {
			int weight = op.param % MAX_WEIGHT;
			#ifdef VERBOSE
				std::cerr << "Adding edge " << vertex_index[op.vertex_a] << " and " << vertex_index[op.vertex_b] << " with weight " << weight << std::endl;
			#endif
			int result = worker->add_edge(vertex_index[op.vertex_a], vertex_index[op.vertex_b], weight);
			if (result >= 0) edges.push_back(std::pair<int,int>(op.vertex_a,op.vertex_b));
		break;}
		case REMOVE_EDGE: {
			// Get edge
			int index = op.param % edges.size();
			bool result = worker->remove_edge(vertex_index[edges[index].first], vertex_index[edges[index].second]);
			#ifdef VERBOSE
				std::cerr << "Removing edge " << vertex_index[edges[index].first] << " and " << vertex_index[edges[index].second] << ": " << result << std::endl;
			#endif
			if (result) {
				// Remove from vector
				edges[index] = edges.back();
				edges.pop_back();
			} else {
				std::cerr << "ERROR: Problem during removing edge " << vertex_index[edges[index].first] << "-" << vertex_index[edges[index].second] << std::endl;
			}
		break;}
		case ADD_WEIGHT: {
			int weight = op.param % MAX_WEIGHT;
			#ifdef VERBOSE
				std::cerr << "Adding weight between " << vertex_index[op.vertex_a] << " and " << vertex_index[op.vertex_b] << ": " << weight << std::endl;
			#endif
			worker->add_weight_on_path(vertex_index[op.vertex_a], vertex_index[op.vertex_b], weight);
		break;}
		case GET_WEIGHT: {
			auto result = worker->get_max_weight_on_path(vertex_index[op.vertex_a], vertex_index[op.vertex_b]);
			#ifdef VERBOSE
				std::cerr << "Getting max weight on path " << vertex_index[op.vertex_a] << " and " << vertex_index[op.vertex_b] << ": [" << result.exists << "] " << result.max_weight << std::endl;
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
			rand()
		};
		operations.push_back(op);
	};

	// Run both implementations
	auto time_top_tree = run(new MaximumEdgeWeight(new TopTree::TopTree()));
	//double time_top_tree = 0;
	auto time_topology_top_tree = run(new MaximumEdgeWeight(new TopTree::TopologyTopTree()));
	//double time_topology_top_tree = 0;

	std::cout << time_top_tree << " " << time_top_tree / K << " " << time_topology_top_tree << " " << time_topology_top_tree / K << std::endl;
}
