#include <iostream>
#include <memory>
#include <string>

#include "examples/double_edge_connectivity.hpp"

#include "STTopTree.hpp"
#include "TopologyTopTree.hpp"


#define TopTree SplayTopTree
#include "../top-trees/include/two_edge_connected.h"
#undef TopTree

//#define VERBOSE

#define OPS_COUNT 3

enum opType { INSERT, DELETE, QUERY };
struct operation {
	opType op;
	int vertex_a;
	int vertex_b;
	int param; // used as weight when creating or as index into vector when deleting edges
};

struct operation getRandomOp(int N, double R) {
	double query_p = 33;
	double delete_p = (100 - query_p) * (1 - R);
	double insert_p = (100 - query_p) * R;

	int sum = query_p + delete_p + query_p;
	int r = rand() % sum;
	if (r < insert_p) return operation{INSERT, rand() % N, rand() % N, 0};
	else if (r < insert_p + delete_p) return operation{DELETE, 0, 0, rand()};
	else return operation{QUERY, rand() % N, rand() % N};
}

std::vector<std::pair<int, int>> initial_edges; // pair(edge to, edge weight)
std::vector<struct operation> operations;


std::tuple<double, double> run(DoubleConnectivity *worker, uint N, uint M) {
	// Vector for indexing edges
	std::vector<std::shared_ptr<MyEdgeData>> edges;

	// Init graph
	clock_t begin = clock();
	std::vector<int> vertex_index;
	for (auto e: initial_edges) {
		auto edge = worker->Insert(e.first, e.second);
		if (edge != NULL) edges.push_back(edge);
	}
	clock_t end = clock();
	double init_time = double(end - begin) / CLOCKS_PER_SEC;

	#ifdef VERBOSE
		std::cerr << "PART 1 - all operations" << std::endl;
	#endif

	// Start measure time and perform all operations
	begin = clock();
	int op_skipped = 0;
	for (auto op: operations) {
		switch (op.op) {
		case INSERT: {
			if (op.vertex_a == op.vertex_b) {
				op_skipped++;
				continue;
			}
			#ifdef VERBOSE
				std::cerr << "Adding edge " << op.vertex_a << " and " << op.vertex_b << std::endl;
			#endif
			auto edge = worker->Insert(op.vertex_a, op.vertex_b);
			if (edge != NULL) edges.push_back(edge);
		break;}
		case DELETE: {
			// Get edge
			if (edges.size() == 0) {
				op_skipped++;
				continue;
			}
			int index = op.param % edges.size();
			#ifdef VERBOSE
				std::cerr << "Removing edge " << edges[index]->from << " and " << edges[index]->to << std::endl;
			#endif
			worker->Delete(edges[index]);
			// Remove from vector
			edges[index] = edges.back();
			edges.pop_back();
		break;}
		case QUERY: {
			auto result = worker->Double_edge_connected(op.vertex_a, op.vertex_b);
			#ifdef VERBOSE
				std::cerr << "Query of " << op.vertex_a << " and " << op.vertex_b << ": " << result << std::endl;
			#endif
		break;}
		}
	}
	end = clock();
	double execution_time = double(end - begin) / CLOCKS_PER_SEC;
	int op_count = operations.size() - op_skipped;

	// Cleaning
	//delete(worker);

	return std::make_tuple(init_time / M, execution_time / op_count);
}

std::tuple<double, double> run_splay(uint N, uint M) {
	// Vector for indexing edges
	std::vector<std::shared_ptr<EdgeData>> edges;

	// Init graph
	clock_t begin = clock();

	TwoEdgeConnectivity tree = TwoEdgeConnectivity(N);
	std::vector<int> vertex_index;
	for (int i = 0; i < initial_edges.size(); i++) {
		auto e = initial_edges[i];
		if (e.first != e.second) {
			auto edge = tree.insert(e.first, e.second);
			edges.push_back(edge);
		}
	}
	clock_t end = clock();
	double init_time = double(end - begin) / CLOCKS_PER_SEC;

	// Start measure time and perform all operations
	begin = clock();
	int i = 0;
	int op_skipped = 0;
	for (auto op: operations) {
		switch (op.op) {
		case INSERT: {
	
			if (op.vertex_a == op.vertex_b) {
				op_skipped++;
				continue;
			}
			assert(op.vertex_a < N && op.vertex_b < N);
			
			auto edge = tree.insert(op.vertex_a, op.vertex_b);
			
			if (edge != nullptr) edges.push_back(edge);
		break;}
		case DELETE: {
			// Get edge
			if (edges.size() == 0) {
				op_skipped++;
				continue;
			}
			int index = op.param % edges.size();

			tree.remove(edges[index]);
			// Remove from vector
			edges[index] = edges.back();
			edges.pop_back();
		break;}
		case QUERY: {
			auto result = tree.two_edge_connected(op.vertex_a, op.vertex_b);
		break;}
		}
	}
	end = clock();
	double execution_time = double(end - begin) / CLOCKS_PER_SEC;
	int op_count = operations.size() - op_skipped;

	return std::make_tuple(init_time / M, execution_time / op_count);
}


int main(int argc, char const *argv[]) {
	// Init random generator
	auto seed = strtoull(argv[1], NULL, 16);
	srand(seed);
	// Get size of graph (vertices, edges) and number of operations
	int N = atoi(argv[2]);
	int M = atoi(argv[3]);
	int K = atoi(argv[4]);
	int W = atoi(argv[5]);
	double R = atof(argv[6]);

	// Generate tree and list of operations
	// a) original graph = each vertex is connected to one with lower number
	for (int i = 0; i < M; i++) initial_edges.push_back(std::pair<int,int>(rand() % N, rand() % N));
	// b) operations (type and two vertices)
	for (int i = 0; i < K; i++) operations.push_back(getRandomOp(N, R));

	// Run both implementations
	// for (int i = 0; i < W; i++) {
	// 	run(new DoubleConnectivity(std::make_shared<TopTree::STTopTree>()), N, M);
	// }
	auto time_top_tree = std::tuple<double,double>(0, 0);
	//time_top_tree = run(new DoubleConnectivity(std::make_shared<TopTree::STTopTree>()), N, M);

	// for (int i = 0; i < W; i++) {
	// 	run(new DoubleConnectivity(std::make_shared<TopTree::TopologyTopTree>()), N, M);
	// }
	auto time_topology_top_tree = std::tuple<double,double>(0, 0);
	//time_topology_top_tree = run(new DoubleConnectivity(std::make_shared<TopTree::TopologyTopTree>()), N, M);


	for (int i = 0; i < W; i++) {
		run_splay(N,M);
	}
	auto time_splay_top_tree = std::tuple<double,double>(0, 0);
	time_splay_top_tree = run_splay(N,M);

	std::cout << std::get<0>(time_top_tree) << " " << std::get<1>(time_top_tree) << " "
		<< std::get<0>(time_topology_top_tree) << " " << std::get<1>(time_topology_top_tree) << " " 
		<< std::get<0>(time_splay_top_tree) << " " << std::get<1>(time_splay_top_tree) << std::endl;

	return 0;
}
