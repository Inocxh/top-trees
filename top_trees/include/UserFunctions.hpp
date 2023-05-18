#include <memory>

#ifndef USER_FUNCTIONS_HPP
#define USER_FUNCTIONS_HPP

#include "TopTreeInterface.hpp"

namespace TopTree {

// USER DEFINED FUNCTIONS:

// Joining and splitting of compress/rake clusters:
extern void Join(std::shared_ptr<ICluster> leftChild, std::shared_ptr<ICluster> rightChild, std::shared_ptr<ICluster> parent);
extern void Split(std::shared_ptr<ICluster> leftChild, std::shared_ptr<ICluster> rightChild, std::shared_ptr<ICluster> parent);

// Creating and destroying Base clusters:
extern void Create(std::shared_ptr<ICluster> cluster, std::shared_ptr<EdgeData> edge);
extern void Destroy(std::shared_ptr<ICluster> cluster, std::shared_ptr<EdgeData> edge);

extern void CopyClusterData(std::shared_ptr<ICluster> from, std::shared_ptr<ICluster> to);

extern std::shared_ptr<ClusterData> InitClusterData();
// END OF USER DEFINED FUNCTIONS
}

#endif // USER_FUNCTIONS_HPP
