#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <ctime>

#include "examples/maximum_edge_weight.hpp"

#include "STTopTree.hpp"
#include "TopologyTopTree.hpp"

#define TopTree SplayTopTree
#include "../top-trees/include/add_weight_cluster.hpp"
#undef TopTree

using SplayMaxPath = MaxPathTopTree;
using SplayMaxEdge = Edge<AddWeightCluster,int,None>;

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

std::pair<double, double> run(MaximumEdgeWeight *worker, int N) {
	// Vector for indexing edges
	std::vector<std::pair<int, int>> edges;
	
	// Init tree
	auto begin = std::chrono::system_clock::now();
	//clock_t begin = clock();
	std::vector<int> vertex_index;
	vertex_index.push_back(worker->add_vertex(std::to_string(0)));
	for (uint i = 1; i < vertices.size(); i++) {
		vertex_index.push_back(worker->add_vertex(std::to_string(i)));
	}
	worker->initialize();
	for (uint i = 1; i < vertices.size(); i++) {
		worker->add_edge(vertex_index[i], vertex_index[vertices[i].first], vertices[i].second);
		edges.push_back(std::pair<int,int>(i, vertices[i].first));
	}
	auto end = std::chrono::system_clock::now();
	//clock_t end = clock();
	auto init_time = end - begin;

	// Start measure time and perform all operations
	begin = std::chrono::system_clock::now();
	int op_skipped = 0;
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
			// if (edges.size() < N * 7/10) {
			// 	op_skipped++;
			// 	continue;
			// }
			if (edges.size() == 0) {
				continue;
			}
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
	end = std::chrono::system_clock::now();

	// Cleaning
	delete(worker);

	int op_count = operations.size() - op_skipped;
	auto execution_time = end - begin;
	return std::make_pair(
		((long double) std::chrono::duration_cast<std::chrono::microseconds>(init_time).count()) / N,
		((long double) std::chrono::duration_cast<std::chrono::microseconds>(execution_time).count()) / op_count);
	//return std::make_pair(init_time, execution_time);
}

std::pair<double, double> run_splay(int N) {
	std::vector<std::pair<int,int>> edges;
	std::vector<SplayMaxEdge*> edge_ptrs;
	std::vector<std::vector<std::pair<int,int>>> adjacency_list;

	//Init tree
	auto begin = std::chrono::system_clock::now();
	//clock_t begin = clock();
	adjacency_list.push_back(std::vector<std::pair<int,int>>(1));
	for (uint i = 1; i < vertices.size(); i++) {	
		adjacency_list.push_back(std::vector<std::pair<int,int>>());
		adjacency_list[vertices[i].first].push_back(std::make_pair(i, vertices[i].second));
	}
	
	SplayMaxPath* tree = new SplayMaxPath(N);
	// for (uint i = 1; i < adjacency_list.size(); i++) {
	// 	for (uint j = 0; j < adjacency_list[i].size(); j++) {
	// 		SplayMaxEdge* e = tree->link_ptr(i,adjacency_list[i][j].first,adjacency_list[i][j].second);
	// 		if (e != nullptr) edge_ptrs.push_back(e);
	// 		edges.push_back(std::pair<int,int>(i,vertices[i].first));
	// 	}
	// }
	for (uint i = 1; i < vertices.size(); i++) {
		//std::cerr << "Linking: (" << i << "," << vertices[i].first << ")" << std::endl;
		//std::cerr << "" << i << "--" << vertices[i].first <<std::endl;
		SplayMaxEdge* e = tree->link_ptr(i,vertices[i].first,vertices[i].second);
		if (e != nullptr) edge_ptrs.push_back(e);
		edges.push_back(std::pair<int,int>(i,vertices[i].first));
	}
	auto end = std::chrono::system_clock::now();
	//clock_t end = clock();
	auto init_time =end-begin;
	//Perform operations
	begin = std::chrono::system_clock::now();
	int op_skipped = 0;
	int i = 0;
	for (auto op : operations) {
		switch (op.op) {
		case ADD_EDGE: {
			//std::cerr << "Adding edge: (" << op.vertex_a << "," << op.vertex_b << ")" << std::endl;
			if (op.vertex_a == op.vertex_b) {
				//std::cerr << "skipped!";
				op_skipped++;
				continue;
			}
			int weight = op.param % MAX_WEIGHT;
			SplayMaxEdge* result = tree->link_ptr(op.vertex_a,op.vertex_b,weight);
			if (result != nullptr)  {
				//std::cerr << op.vertex_a << "--" << op.vertex_b;
				//std::cerr << std::endl;
				edge_ptrs.push_back(result);
			}
		break;}
		case REMOVE_EDGE: {
			// if (edge_ptrs.size() < (N * 7) / 10) {
			// 	op_skipped++;
			// 	continue;
			// }
			if (edge_ptrs.size() == 0) {
				continue;
			}
			uint edge_index = op.param % edge_ptrs.size();
			//std::cerr << "removing edge: " << edge_ptrs[edge_index]->get_endpoint(0)->get_id() << ", " << edge_ptrs[edge_index]->get_endpoint(1)->get_id() << std::endl;
			//std::cerr << "top_tree.cut(" << edge_ptrs[edge_index]->get_endpoint(0)->get_id() << ", " << edge_ptrs[edge_index]->get_endpoint(1)->get_id() << ");" << std::endl;
			auto lol = tree->cut_ptr(edge_ptrs[edge_index]);
			edge_ptrs[edge_index] = edge_ptrs.back();
			edge_ptrs.pop_back(); 
		break;}
		case ADD_WEIGHT: {
			//std::cerr << "adding weight " << op.vertex_a << " " <<  op.vertex_b << std::endl;
			// std::cerr << "top_tree.expose(" << op.vertex_a << ");" << std::endl;
			// std::cerr << "top_tree.expose(" << op.vertex_b << ");" << std::endl;
			// std::cerr << "top_tree.deexpose(" << op.vertex_a << ");" << std::endl;
			// std::cerr << "top_tree.deexpose(" << op.vertex_b << ");" << std::endl;
			if (op.vertex_a == op.vertex_b) {
				op_skipped++;
				continue;
			}
			int weight = op.param % MAX_WEIGHT;
			auto c1 = tree->expose(op.vertex_a);
			auto c2 = tree->expose(op.vertex_b);
			if (c1==c2 && c1 && c2) {
				c1->add_weight(weight);
			}
			tree->deexpose(op.vertex_a);
			tree->deexpose(op.vertex_b);
		break;}
		case GET_WEIGHT: {
			//std::cerr << "getting weight " << op.vertex_a << " " <<  op.vertex_b << std::endl;
			// std::cerr << "top_tree.expose(" << op.vertex_a << ");" << std::endl;
			// std::cerr << "top_tree.expose(" << op.vertex_b << ");" << std::endl;
			// std::cerr << "top_tree.deexpose(" << op.vertex_a << ");" << std::endl;
			// std::cerr << "top_tree.deexpose(" << op.vertex_b << ");" << std::endl;
			if (op.vertex_a == op.vertex_b) {
				op_skipped++;
				continue;
			}
			auto c1 = tree->expose(op.vertex_a);
			auto c2 = tree->expose(op.vertex_b);
			tree->deexpose(op.vertex_a);
			tree->deexpose(op.vertex_b);
		break;}
		}
		i++;
	}


	end = std::chrono::system_clock::now();
	int op_count = operations.size() - op_skipped;
	auto execution_time = end-begin;
	delete tree;
	return std::make_pair(
		((long double) std::chrono::duration_cast<std::chrono::microseconds>(init_time).count()) / N,
		((long double) std::chrono::duration_cast<std::chrono::microseconds>(execution_time).count()) / op_count);
	//return std::pair<double,double>(init_time,execution_time );
}

int main(int argc, char *argv[]) {
	// Init random generator
	auto seed = strtoull(argv[1], NULL, 16);
	srand(seed);
	// Get size of tree, number of operations and number of warmups
	int N = atoi(argv[2]);
	int K = atoi(argv[3]);
	int W = atoi(argv[4]);
	double R = atof(argv[5]);

	// Generate tree and list of operations
	// a) original graph = each vertex is connected to one with lower number
	vertices.push_back(std::pair<int,int>(0,0));
	for (int i = 1; i < N; i++) vertices.push_back(std::pair<int,int>(rand() % i, rand() % MAX_WEIGHT));
	// b) operations (type and two vertices)
	for (int i = 0; i < K; i++) {
		opType op;
		if (rand() % 3 == 0) {
			if (rand() % 2 == 0) {
				op = ADD_WEIGHT;
			} else {
				op = GET_WEIGHT;
			}
		} else {
			if ((double) (rand() / (double) RAND_MAX) < R) {
				op = ADD_EDGE;
			} else {
				op = REMOVE_EDGE;
			}			
		}
		struct operation ope{
			op,
			rand() % N,
			rand() % N,
			rand()
		};
		operations.push_back(ope);
	};

	//std::cerr << "Generating of operations ended" << std::endl;

	// Run implementations
	for (int i = 0; i < W; i++) {
		run(new MaximumEdgeWeight(new TopTree::STTopTree()), N);
	}
	auto time_top_tree = run(new MaximumEdgeWeight(new TopTree::STTopTree()), N);
	//auto time_top_tree = std::make_pair(0, 0);

	for (int i = 0; i < W; i++) {
		run(new MaximumEdgeWeight(new TopTree::TopologyTopTree()), N);
	}
	auto time_topology_top_tree = run(new MaximumEdgeWeight(new TopTree::TopologyTopTree()), N);
	//auto time_topology_top_tree = std::make_pair(0, 0);

	for (int i = 0; i < W; i++) {
		run_splay(N);
	}
	auto time_splay_top_tree = run_splay(N);

	std::cout << time_top_tree.first << " " << time_top_tree.second << " " << time_topology_top_tree.first << " " << time_topology_top_tree.second <<  " " << time_splay_top_tree.first << " " <<  time_splay_top_tree.second << std::endl;
}
