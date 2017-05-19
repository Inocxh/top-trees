#include <memory>

#include "BaseTree.hpp"

#ifndef TOP_TREE_HPP
#define TOP_TREE_HPP

namespace TopTree {

struct ClusterData {
	virtual ~ClusterData() {}
};

// USER DEFINED FUNCTIONS:
// (abstract functions to be defined in custom Top Trees inherited from this class)

// Joining and splitting of compress/rake clusters:
extern void Join(std::shared_ptr<ClusterData> leftChild, std::shared_ptr<ClusterData> rightChild, std::shared_ptr<ClusterData> parent);
extern void Split(std::shared_ptr<ClusterData> leftChild, std::shared_ptr<ClusterData> rightChild, std::shared_ptr<ClusterData> parent);

// Creating and destroying Base clusters:
extern void Create(std::shared_ptr<ClusterData> cluster, std::shared_ptr<EdgeData> edge);
extern void Destroy(std::shared_ptr<ClusterData> cluster, std::shared_ptr<EdgeData> edge);

extern std::shared_ptr<ClusterData> InitClusterData();
// END OF USER DEFINED FUNCTIONS

class TopTree {
public:
	TopTree();
	TopTree(std::shared_ptr<BaseTree> baseTree); // Construct from underlying tree
	~TopTree();

	// Functions that could be called on Top Tree:
private:
	class Internal;
	std::unique_ptr<Internal> internal;
};

}

#endif // TOP_TREE_HPP
