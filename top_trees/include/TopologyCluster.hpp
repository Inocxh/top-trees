#include <memory>
#include <vector>

#ifndef TOPOLOGY_CLUSTER_HPP
#define TOPOLOGY_CLUSTER_HPP

namespace TopTree {
class TopologyCluster;
}

#include "ClusterInterface.hpp"
#include "BaseTreeInternal.hpp"
#include "TopCluster.hpp"

namespace TopTree {
class TopologyCluster : public ICluster, public std::enable_shared_from_this<TopologyCluster> {
friend class TopologyTopTree;
public:
	static int global_index;

	TopologyCluster();

	virtual std::ostream& ToString(std::ostream& o) const;
protected:
	struct neighbour {
		std::shared_ptr<BaseTree::Internal::Edge> edge;
		std::shared_ptr<TopologyCluster> cluster;
	};

	int index;

	bool is_top_cluster = true; // if it contains at least one edge (and is therefore valid top cluster)
	bool is_rake_branch = false; // if had two children and both outer edges are from one child
	bool is_deleted = false;

	std::shared_ptr<TopologyCluster> parent;
	std::shared_ptr<TopologyCluster> first;
	std::shared_ptr<TopologyCluster> second;

	// Edge between clusters:
	std::shared_ptr<BaseTree::Internal::Edge> edge;
	// Vertex if it is the base topology cluster
	std::shared_ptr<BaseTree::Internal::Vertex> vertex;

	std::vector<neighbour> outer_edges;
	int outer_edges_count = 0;

	// Data of corresponding clusters in the top tree:
	std::shared_ptr<ICluster> edge_cluster;
	std::shared_ptr<ICluster> combined_edge_cluster;

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

	bool is_external_boundary_vertex(std::shared_ptr<BaseTree::Internal::Vertex> v);

	void calculate_outer_edges(bool check_neighbours = false);
	void remove_all_outer_edges();

	static std::shared_ptr<BaseTree::Internal::Vertex> get_common_vertex(std::shared_ptr<ICluster> cluster_a, std::shared_ptr<ICluster> cluster_b, bool get_superior = true);
};
std::ostream& operator<<(std::ostream& o, const TopologyCluster& v);

class SimpleCluster: public ICluster, public std::enable_shared_from_this<SimpleCluster> {
friend class TopologyTopTree;
public:
	std::ostream& ToString(std::ostream& o) const { return o; }
protected:
	std::shared_ptr<BaseTree::Internal::Edge> edge = NULL;

	std::shared_ptr<SimpleCluster> parent = NULL;
	std::shared_ptr<ICluster> first = NULL;
	std::shared_ptr<ICluster> second = NULL;

	void do_split();
	bool was_splitted = false;
};

}

#endif // TOPOLOGY_CLUSTER_HPP
