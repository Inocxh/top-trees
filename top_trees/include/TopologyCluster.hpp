#include <memory>
#include <vector>

#ifndef TOPOLOGY_CLUSTER_HPP
#define TOPOLOGY_CLUSTER_HPP

namespace TopTree {
class TopologyCluster;
}

#include "BaseTreeInternal.hpp"
#include "Cluster.hpp"

namespace TopTree {

class TopologyCluster : public std::enable_shared_from_this<TopologyCluster> {
friend class TopologyTopTree;
protected:
	struct neighbour {
		std::shared_ptr<BaseTree::Internal::Edge> edge;
		std::shared_ptr<TopologyCluster> cluster;
	};

	std::shared_ptr<TopologyCluster> parent;
	std::shared_ptr<TopologyCluster> first;
	std::shared_ptr<TopologyCluster> second;

	// Edge between clusters:
	std::shared_ptr<BaseTree::Internal::Edge> edge;
	// Vertex if it is the base topology cluster
	std::shared_ptr<BaseTree::Internal::Vertex> vertex;

	std::vector<neighbour> outer_edges;
	int outer_edges_count;

	// Data of corresponding clusters in the top tree:
	std::shared_ptr<ClusterData> edge_cluster_data;
	std::shared_ptr<ClusterData> combined_edge_cluster_data;
	std::shared_ptr<ClusterData> cluster_data;

	void set_first_child(std::shared_ptr<TopologyCluster> child);
	void set_second_child(std::shared_ptr<TopologyCluster> child);

	int root_vector_index = -1;
	bool is_splitted = true; // Initially clusters are in state that they need do_join method (which is called during construction)
	bool listed_in_delete_list = false;
	bool listed_in_change_list = false;
	bool listed_in_abandon_list = false;
	bool listed_in_recompute_list = false;

	void do_split(std::vector<std::shared_ptr<TopologyCluster>>* splitted_clusters = NULL);
	void do_join();

	void calculate_outer_edges(bool check_neighbours = false);
	void remove_all_outer_edges();
};

}

#endif // TOPOLOGY_CLUSTER_HPP
