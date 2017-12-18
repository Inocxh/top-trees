#include <memory>
#include <vector>

#ifndef TOP_TREE_INTERFACE_HPP
#define TOP_TREE_INTERFACE_HPP

#include "BaseTree.hpp"
#include "UserFunctions.hpp"

namespace TopTree {

/**
 * Generic cluster interface returned from Top Trees methods. Its only public accessible field is shared_pointer to the ClusterData object.
 */
class ICluster {
public:
	std::shared_ptr<ClusterData> data = InitClusterData();
	virtual std::ostream& ToString(std::ostream& o) const = 0;
};

/**
 * Generic interface for all Top Trees implementations. It provides basic operations (Cut, Join and Expose).
 */
class ITopTree {
public:
	virtual ~ITopTree() {}

	// User operations

	/**
	 * @brief Exposes given path at the root cluster and returns pointer to that root Cluster.
	 *
	 * @details Returned pointer is valid until other Top Trees operation are executed, after that it is considered obsolete and invalid.
	 *
	 * @param v Index of the first endpoint of wanted path. Indexes are these returned by creating vertices in the BaseTree.
	 * @param w Index of the second endpoint of wanted path. Indexes are these returned by creating vertices in the BaseTree.
	 *
	 * @return shared_ptr to the exposed root Cluster or NULL when this path cannot be exposed.
	 */
	virtual std::shared_ptr<ICluster> Expose(int v, int w) = 0;

	/**
	 * @brief Cuts the edge between given vertices and returns pointers to new root Clusters.
	 *
	 * @details Returned pointer is valid until other Top Trees operation are executed, after that it is considered obsolete and invalid.
	 * Also is expected that v-w is one edge. If it is path or they are in different subtrees the returned EdgeData pointer
	 * would be NULL to inform about that situation.
	 *
	 * @param v Index of the first endpoint of wanted path. Indexes are these returned by creating vertices in the BaseTree.
	 * @param w Index of the second endpoint of wanted path. Indexes are these returned by creating vertices in the BaseTree.
	 *
	 * @return [description]
	 */
	virtual std::tuple<std::shared_ptr<ICluster>, std::shared_ptr<ICluster>, std::shared_ptr<EdgeData>> Cut(int v, int w) = 0;

	/**
	 * @brief Link given vertices by and edge with given EdgeData and returns pointer to the new root cluster's data.
	 *
	 * @details Returned pointer is valid until other Top Trees operation are executed, after that it is considered obsolete and invalid.
	 *
	 * @param v Index of the first endpoint of wanted path. Indexes are these returned by creating vertices in the BaseTree.
	 * @param w Index of the second endpoint of wanted path. Indexes are these returned by creating vertices in the BaseTree.
	 * @param edge_data Shared pointer to EdgeData for the newly created edge.
	 *
	 * @return shared_ptr to the newly created root's ClusterData or NULL when link cannot be done (if both vertices were in the same top tree)
	 */
	virtual std::shared_ptr<ICluster> Link(int v, int w, std::shared_ptr<EdgeData> edge_data) = 0;
};

}

#endif // TOP_TREE_INTERFACE_HPP
