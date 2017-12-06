#include <memory>
#include <vector>

#ifndef TOP_TREE_HPP
#define TOP_TREE_HPP

#include "BaseTree.hpp"
#include "Cluster.hpp"

namespace TopTree {

class TopTree {
public:
	TopTree();
	TopTree(std::shared_ptr<BaseTree> baseTree); // Construct from underlying tree
	~TopTree();

	// Operations
	std::shared_ptr<Cluster> Expose(int v, int w); // Expose path v-w in the root cluster (vertices are refered by its indexes) and return this root cluster
	std::tuple<std::shared_ptr<Cluster>, std::shared_ptr<Cluster>, std::shared_ptr<EdgeData>> Cut(int v, int w); // Cut between vertices u and v and return two new roots
	std::shared_ptr<Cluster> Link(int v, int w, std::shared_ptr<EdgeData> edge_data = NULL); // Add edge u-w and return new root cluster

	// Return roots of the top trees
	std::vector<std::shared_ptr<Cluster> > GetTopTrees();

	// Debug methods
	void PrintGraphviz(const std::shared_ptr<Cluster> root, const std::string title = "Top Tree") const;
	void PrintRooted(const std::shared_ptr<Cluster> root) const;
private:
	class Internal;
	std::unique_ptr<Internal> internal;
};

}

#endif // TOP_TREE_HPP
