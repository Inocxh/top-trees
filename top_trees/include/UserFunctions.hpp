#ifndef USER_FUNCTIONS_HPP
#define USER_FUNCTIONS_HPP

#include "Cluster.hpp"

namespace TopTree {
// USER DEFINED FUNCTIONS:

// Joining and splitting of compress/rake clusters:
extern void Join(std::shared_ptr<ClusterData> leftChild, std::shared_ptr<ClusterData> rightChild, std::shared_ptr<ClusterData> parent);
extern void Split(std::shared_ptr<ClusterData> leftChild, std::shared_ptr<ClusterData> rightChild, std::shared_ptr<ClusterData> parent);

// Creating and destroying Base clusters:
extern void Create(std::shared_ptr<ClusterData> cluster, std::shared_ptr<EdgeData> edge);
extern void Destroy(std::shared_ptr<ClusterData> cluster, std::shared_ptr<EdgeData> edge);

extern std::shared_ptr<ClusterData> InitClusterData();
// END OF USER DEFINED FUNCTIONS
}

#endif // USER_FUNCTIONS_HPP
