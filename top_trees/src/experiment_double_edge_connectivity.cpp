#include <iostream>
#include <memory>
#include <string>

#include "examples/double_edge_connectivity.hpp"

#include "TopTree.hpp"
#include "TopologyTopTree.hpp"

//#define VERBOSE

#define OPS_COUNT 3

enum opType { INSERT, DELETE, QUERY };
struct operation {
	opType op;
	int vertex_a;
	int vertex_b;
	int param; // used as weight when creating or as index into vector when deleting edges
};

#define INSERT_P 15
#define DELETE_P 15
#define QUERY_P 70

struct operation getRandomOp(int N) {
	int sum = INSERT_P + DELETE_P + QUERY_P;
	int r = rand() % sum;
	if (r < INSERT_P) return operation{INSERT, rand() % N, rand() % N, 0};
	else if (r < INSERT_P + DELETE_P) return operation{DELETE, 0, 0, rand()};
	else return operation{QUERY, rand() % N, rand() % N};
}

std::vector<std::pair<int, int>> initial_edges; // pair(edge to, edge weight)
std::vector<struct operation> operations;


std::tuple<double, double, double> run(DoubleConnectivity *worker, uint N, uint M) {
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
			if (edges.size() > M * 13/10 || op.vertex_a == op.vertex_b) {
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
			if (edges.size() < M * 7/10) {
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

	#ifdef VERBOSE
		std::cerr << "PART 2 - only queries" << std::endl;
	#endif

	begin = clock();
	// Measure only queries
	int query_count = 0;
	for (auto op: operations) {
		if (op.op != QUERY) continue;

		query_count++;
		auto result = worker->Double_edge_connected(op.vertex_a, op.vertex_b);
		#ifdef VERBOSE
			std::cerr << "Query of " << op.vertex_a << " and " << op.vertex_b << ": " << result << std::endl;
		#endif
	}
	end = clock();
	double query_execution_time = double(end - begin) / CLOCKS_PER_SEC;

	// Cleaning
	//delete(worker);

	return std::make_tuple(init_time / M, execution_time / op_count, query_execution_time / query_count);
}



int main(int argc, char const *argv[]) {
	// Init random generator
	auto seed = strtoull(argv[1], NULL, 16);
	srand(seed);
	// Get size of graph (vertices, edges) and number of operations
	int N = atoi(argv[2]);
	int M = atoi(argv[3]);
	int K = atoi(argv[4]);

	// Generate tree and list of operations
	// a) original graph = each vertex is connected to one with lower number
	for (int i = 0; i < M; i++) initial_edges.push_back(std::pair<int,int>(rand() % N, rand() % N));
	// b) operations (type and two vertices)
	for (int i = 0; i < K; i++) operations.push_back(getRandomOp(N));

	// Run both implementations
	auto time_top_tree = std::tuple<double,double,double>(0, 0, 0);
	time_top_tree = run(new DoubleConnectivity(std::make_shared<TopTree::TopTree>()), N, M);

	auto time_topology_top_tree = std::tuple<double,double,double>(0, 0, 0);
	time_topology_top_tree = run(new DoubleConnectivity(std::make_shared<TopTree::TopologyTopTree>()), N, M);

	auto time_topology_top_tree_quick = std::tuple<double,double,double>(0, 0, 0);
	time_topology_top_tree_quick = run(new DoubleConnectivity(std::make_shared<TopTree::TopologyTopTree>(), true), N, M);

	std::cout << std::get<0>(time_top_tree) << " " << std::get<1>(time_top_tree) << " " << std::get<2>(time_top_tree) << " "
		<< std::get<0>(time_topology_top_tree) << " " << std::get<1>(time_topology_top_tree) << " " << std::get<2>(time_topology_top_tree) << " "
		<< std::get<0>(time_topology_top_tree_quick) << " " << std::get<1>(time_topology_top_tree_quick) << " " << std::get<2>(time_topology_top_tree_quick) << std::endl;


	/* Manual testing:

	auto dc = new DoubleConnectivity(std::make_shared<TopTree::TopTree>());
	//auto dc = new DoubleConnectivity(std::make_shared<TopTree::TopologyTopTree>());

	auto a = dc->Insert(0,1);
	auto e = dc->Insert(1,2);
	dc->Insert(2,3);
	dc->Insert(2,0);
	dc->Insert(1,3);
	auto result = dc->Double_edge_connected(0, 3);
	std::cerr << "Test: " << result << std::endl;

	dc->Delete(e);
	result = dc->Double_edge_connected(0, 3);
	std::cerr << "Test: " << result << std::endl;

	dc->Delete(a);
	result = dc->Double_edge_connected(0, 3);
	std::cerr << "Test: " << result << std::endl;

	dc->Insert(0,3);
	result = dc->Double_edge_connected(0, 3);
	std::cerr << "Test: " << result << std::endl;

	result = dc->Double_edge_connected(1, 3);
	std::cerr << "Test: " << result << std::endl;
	*/

	return 0;
}
