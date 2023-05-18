#include <memory>
#include <vector>

#ifndef ST_TOP_TREE_HPP
#define ST_TOP_TREE_HPP

#include "TopTreeInterface.hpp"

namespace TopTree {

class STTopTree: public ITopTree {
public:
	STTopTree();
	STTopTree(std::shared_ptr<BaseTree> baseTree); // Construct from underlying tree
	~STTopTree();

	void InitFromBaseTree(std::shared_ptr<BaseTree> baseTree);

	// User operations (documented in the ITopTree interface)
	std::shared_ptr<ICluster> Expose(int v, int w);
	std::tuple<std::shared_ptr<ICluster>, std::shared_ptr<ICluster>, std::shared_ptr<EdgeData>> Cut(int v, int w);
	std::shared_ptr<ICluster> Link(int v, int w, std::shared_ptr<EdgeData> edge_data);
	void Restore();
	std::pair<std::shared_ptr<ICluster>, std::shared_ptr<ICluster>> SplitRoot(std::shared_ptr<ICluster> root);
private:
	class Internal;
	std::unique_ptr<Internal> internal;
};

}

#endif // ST_TOP_TREE_HPP
