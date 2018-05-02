#include <iostream>
#include <memory>
#include <string>

#include "examples/double_edge_connectivity.hpp"

#include "TopTree.hpp"
#include "TopologyTopTree.hpp"


int main(int argc, char const *argv[]) {
	// Init random generator
	auto seed = strtoull(argv[1], NULL, 16);
	srand(seed);
	// Get size of graph and number of operations
	int N = atoi(argv[2]);
	int K = atoi(argv[3]);

	// TODO

	return 0;
}
