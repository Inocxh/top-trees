#include <memory>

#include "BaseTreeInternal.hpp"

#ifndef CLUSTER_INTERFACE_HPP
#define CLUSTER_INTERFACE_HPP

namespace TopTree {

struct ClusterData {
	virtual ~ClusterData() {}
};

extern std::shared_ptr<ClusterData> InitClusterData();

/**
 * Generic cluster interface returned from Top Trees methods. Its only public accessible field is shared_pointer to the ClusterData object.
 */
class ICluster {
friend class TopologyCluster;
friend class TopologyTopTree;
public:
	std::shared_ptr<ClusterData> data = InitClusterData();

	int getLeftBoundary() { return (boundary_left->superior_vertex != NULL ? boundary_left->superior_vertex->index : boundary_left->index); }
	int getRightBoundary() { return (boundary_right->superior_vertex != NULL ? boundary_right->superior_vertex->index : boundary_right->index); }

	virtual std::ostream& ToString(std::ostream& o) const = 0;
protected:
	std::shared_ptr<BaseTree::Internal::Vertex> boundary_left;
	std::shared_ptr<BaseTree::Internal::Vertex> boundary_right;
};

// Helper functions:

/**
 * @brief Check if boundary vertices respond to situation when left child is raked on the right one (right one ramains).
 *
 * @return bool
 */
inline bool isLeftRake(std::shared_ptr<ICluster> left, std::shared_ptr<ICluster> right, std::shared_ptr<ICluster> parent) {
	int l = right->getLeftBoundary();
	int r = right->getRightBoundary();
	int pl = parent->getLeftBoundary();
	int pr = parent->getRightBoundary();
	return ((l == pl && r == pr) || (l == pr && r == pl));
}

/**
 * @brief Check if boundary vertices respond to situation when right child is raked on the left one (left one remains).
 *
 * @return bool
 */
inline bool isRightRake(std::shared_ptr<ICluster> left, std::shared_ptr<ICluster> right, std::shared_ptr<ICluster> parent) {
	int l = left->getLeftBoundary();
	int r = left->getRightBoundary();
	int pl = parent->getLeftBoundary();
	int pr = parent->getRightBoundary();
	return ((l == pl && r == pr) || (l == pr && r == pl));
}

/**
 * @brief Check if boundary vertices respond to situation when left and right child were compressed
 *
 * @return bool
 */
inline bool isCompress(std::shared_ptr<ICluster> left, std::shared_ptr<ICluster> right, std::shared_ptr<ICluster> parent) {
	return (!isLeftRake(left, right, parent) && !isRightRake(left, right, parent));
}

}

#endif // CLUSTER_INTERFACE_HPP
