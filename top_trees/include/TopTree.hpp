#include <memory>
#include <vector>

#ifndef TOP_TREE_HPP
#define TOP_TREE_HPP

#include "TopTreeInterface.hpp"
#include "TopCluster.hpp"

namespace TopTree {

class TopTree: public ITopTree {
public:
	TopTree();
	TopTree(std::shared_ptr<BaseTree> baseTree); // Construct from underlying tree
	~TopTree();

	// User operations (documented in the ITopTree interface)
	std::shared_ptr<ICluster> Expose(int v, int w);
	std::tuple<std::shared_ptr<ICluster>, std::shared_ptr<ICluster>, std::shared_ptr<EdgeData>> Cut(int v, int w);
	std::shared_ptr<ICluster> Link(int v, int w, std::shared_ptr<EdgeData> edge_data);
	void Restore();

	// Return roots of the top trees
	std::vector<std::shared_ptr<TopCluster>> GetTopTrees();

	// Debug methods
	void PrintGraphviz(const std::shared_ptr<TopCluster> root, const std::string title = "Top Tree") const;
	void PrintRooted(const std::shared_ptr<TopCluster> root) const;
private:
	class Internal;
	std::unique_ptr<Internal> internal;
};

}

#endif // TOP_TREE_HPP
