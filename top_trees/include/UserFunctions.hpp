#include <memory>

#ifndef USER_FUNCTIONS_HPP
#define USER_FUNCTIONS_HPP

#include "BaseTree.hpp"

namespace TopTree {
struct ClusterData {
	virtual ~ClusterData() {}
};

// USER DEFINED FUNCTIONS:

// Joining and splitting of compress/rake clusters:
extern void Join(std::shared_ptr<ClusterData> leftChild, std::shared_ptr<ClusterData> rightChild, std::shared_ptr<ClusterData> parent, bool isRake=false);
extern void Split(std::shared_ptr<ClusterData> leftChild, std::shared_ptr<ClusterData> rightChild, std::shared_ptr<ClusterData> parent, bool isRake=false);

// Creating and destroying Base clusters:
extern void Create(std::shared_ptr<ClusterData> cluster, std::shared_ptr<EdgeData> edge);
extern void Destroy(std::shared_ptr<ClusterData> cluster, std::shared_ptr<EdgeData> edge);

extern std::shared_ptr<ClusterData> InitClusterData();
// END OF USER DEFINED FUNCTIONS
}

#endif // USER_FUNCTIONS_HPP
