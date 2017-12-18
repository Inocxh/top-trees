#include <memory>
#include <vector>

#ifndef TOPOLOGY_TOP_TREE_HPP
#define TOPOLOGY_TOP_TREE_HPP

#include "TopTreeInterface.hpp"
// #include "Cluster.hpp"

namespace TopTree {

class TopologyTopTree: public ITopTree {
public:
	TopologyTopTree();
	TopologyTopTree(std::shared_ptr<BaseTree> baseTree); // Construct from underlying tree
	~TopologyTopTree();

	// User operations (documented in the ITopTree interface)
	std::shared_ptr<ICluster> Expose(int v, int w);
	std::tuple<std::shared_ptr<ICluster>, std::shared_ptr<ICluster>, std::shared_ptr<EdgeData>> Cut(int v, int w);
	std::shared_ptr<ICluster> Link(int v, int w, std::shared_ptr<EdgeData> edge_data);

	// Return roots of the top trees
	// std::vector<std::shared_ptr<Cluster> > GetTopTrees();

	// Debug methods
	//void PrintGraphviz(const std::shared_ptr<Cluster> root, const std::string title = "Top Tree") const;
	//void PrintRooted(const std::shared_ptr<Cluster> root) const;
private:
	class Internal;
	std::unique_ptr<Internal> internal;
};

}

#endif // TOPOLOGY_TOP_TREE_HPP
