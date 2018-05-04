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

	auto dc = new DoubleConnectivity(std::make_shared<TopTree::TopTree>());

	dc->Insert(0,1);
	dc->Insert(1,2);
	dc->Insert(2,3);
	dc->Insert(2,0);
	dc->Insert(1,3);
	auto result = dc->Double_edge_connected(0, 3);
	std::cerr << "Test: " << result << std::endl;

	return 0;
}
