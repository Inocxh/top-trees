#include <queue>
#include <vector>
#include <sstream>

#include "TopologyTopTree.hpp"
#include "BaseTreeInternal.hpp"
#include "TopologyCluster.hpp"

//#define DEBUG

namespace TopTree {

// Hide data from .hpp file using PIMP idiom
class TopologyTopTree::Internal {
public:
	std::vector<std::shared_ptr<TopologyCluster> > root_clusters;
	std::shared_ptr<BaseTree> base_tree;

	std::shared_ptr<TopologyCluster> construct_basic_clusters(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> parent_edge=NULL);
	std::shared_ptr<TopologyCluster> construct_topology_tree(std::shared_ptr<TopologyCluster> cluster, int level = 0, std::shared_ptr<BaseTree::Internal::Edge> parent_edge = NULL);
	std::shared_ptr<BaseTree::Internal::Vertex> split_vertex(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> parent_edge = NULL);

	//void soft_expose(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w);
	//std::shared_ptr<Cluster> hard_expose(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w);
	//void restore_hard_expose();
	//void guarded_splay(std::shared_ptr<Cluster> node, std::shared_ptr<Cluster> guard = NULL);

	//std::vector<std::shared_ptr<Cluster>> splitted_clusters;

	std::vector<std::shared_ptr<TopologyCluster>> to_calculate_outer_edges;

	// Debug methods:
	//void print_rooted_prefix(const std::shared_ptr<Cluster> cluster, const std::string prefix = "", bool last_child = true) const;
	//void print_graphviz(std::shared_ptr<Cluster> node, const std::string title="") const;
	//void print_graphviz_recursive(std::shared_ptr<Cluster> parent, std::shared_ptr<Cluster> node, const char* edge_label="") const;
	//void print_graphviz_child(std::shared_ptr<Cluster> from, std::shared_ptr<Cluster> to, const char* edge_label="") const;
private:
	//void adjust_parent(std::shared_ptr<Cluster> parent, std::shared_ptr<Cluster> old_child, std::shared_ptr<Cluster> new_child);
	//void rotate_left(std::shared_ptr<Cluster> x);
	//void rotate_right(std::shared_ptr<Cluster> x);

	//void splice(std::shared_ptr<Cluster> node);

	//void soft_expose_handle(std::shared_ptr<Cluster> handle, std::shared_ptr<Cluster> splay_guard = NULL);

	//std::vector<std::shared_ptr<CompressCluster>> hard_expose_transformed_clusters;

};

////////////////////////////////////////////////////////////////////////////////

TopologyTopTree::TopologyTopTree() : internal{std::make_unique<Internal>()} {}

TopologyTopTree::TopologyTopTree(std::shared_ptr<BaseTree> from_base_tree) : TopologyTopTree() {
	internal->base_tree = from_base_tree;

	for (auto v : internal->base_tree->internal->vertices) v->used = false;

	int i = 0;
	for (auto v : internal->base_tree->internal->vertices) {
		if (v->used || v->degree != 1) continue;

		auto root_cluster = internal->construct_basic_clusters(v);
		while (root_cluster->outer_edges.size() > 0) {
			root_cluster = internal->construct_topology_tree(root_cluster);
			for (auto c: internal->to_calculate_outer_edges) c->calculate_outer_edges();
			internal->to_calculate_outer_edges.clear();
		}
		internal->root_clusters.push_back(root_cluster);
		internal->root_clusters[i]->root_vector_index = i;
		i++;
	}
}

//std::vector<std::shared_ptr<Cluster> > TopologyTopTree::GetTopTrees() {
//	return internal->root_clusters;
//}


////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<BaseTree::Internal::Vertex> TopologyTopTree::Internal::split_vertex(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> parent_edge) {
	// Create subvertices
	auto current = std::make_shared<BaseTree::Internal::Vertex>((std::shared_ptr<VertexData>)NULL);
	current->superior_vertex = v;
	v->subvertices.push_back(current);
	auto vertex_to_return = current; // by default we return the first vertex

	auto current_iter = std::prev(v->subvertices.end());
	for (auto& n: v->neighbours) {
		// Copy this edge into subvertice and notice what subvertice it is
		if (current->degree == 2 && &n != &v->neighbours.back()) {
			auto temp = std::make_shared<BaseTree::Internal::Vertex>((std::shared_ptr<VertexData>)NULL);
			temp->superior_vertex = v;
			v->subvertices.push_back(temp);
			// Add edge between them
			auto edge = std::make_shared<BaseTree::Internal::Edge>(current, temp, (std::shared_ptr<EdgeData>)NULL);
			edge->subvertice_edge = true;
			edge->register_at_vertices();

			current = temp;
			current_iter = std::prev(v->subvertices.end());
		}
		if (n.edge.lock() == parent_edge) vertex_to_return = current; // This vertex got edge from parent so we will return this one

		// Add this edge to current subvertice
		current->neighbours.push_back(BaseTree::Internal::neighbour{n.vertex, n.edge});
		current->degree++;
		n.subvertice_iter = current_iter;
	}
	return vertex_to_return;
}


std::shared_ptr<TopologyCluster> TopologyTopTree::Internal::construct_basic_clusters(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> parent_edge) {
	if (v->degree > 3) return construct_basic_clusters(split_vertex(v, parent_edge), parent_edge);

	// Otherwise...
	// 1. Sanity check
	if (parent_edge == NULL and v->degree > 1) {
		std::cerr << "ERROR: Cluster without parent with degree > 1" << std::endl;
		return NULL;
	}

	// 2. Construct basic topology clusters from this vertex and connect with outgoing edges with others
	auto cluster = std::make_shared<TopologyCluster>();
	cluster->vertex = v;
	v->used = true;
	for (auto n : v->neighbours) {
		if (auto vv = n.vertex.lock()) if (auto ee = n.edge.lock()) {
			if (ee == parent_edge) continue;
			if (vv->used) {
				std::cerr << "ERROR: Vertex " << vv << "already used in Topology Tree, underlying tree isn't acyclic!" << std::endl;
				return NULL;
			}
			vv->used = true;
			cluster->outer_edges.push_back(TopologyCluster::neighbour{ee, construct_basic_clusters(vv, ee)});
		}
	}

	// Now there is prepared topology tree with
	return cluster;
}

std::shared_ptr<TopologyCluster> TopologyTopTree::Internal::construct_topology_tree(std::shared_ptr<TopologyCluster> cluster, int level, std::shared_ptr<BaseTree::Internal::Edge> parent_edge) {
	std::shared_ptr<TopologyCluster> first = NULL;
	std::shared_ptr<TopologyCluster> second = NULL;

	// 1. Construct clusters for both children
	for (auto o : cluster->outer_edges) {
		if (o.edge == parent_edge) continue;
		if (first == NULL) first = construct_topology_tree(o.cluster, level, o.edge);
		else second = construct_topology_tree(o.cluster, level, o.edge);
	}

	// 3. Check if this cluster could be added to one of the child clusters:
	if (first != NULL && second != NULL) {
		// Both children, we could add this cluster only to some with only one cluster and without other edges
		if (first->second == NULL && first->outer_edges.size() == 1) {
			std::cerr << "Adding this cluster to the first one child using edge " <<first->outer_edges[0].edge << std::endl;
			first->second = cluster;
			cluster->parent = first;
			// outer edges will be calculated after finishing making all clusters on this level of topology tree
			return first;
		} else if (second->second == NULL && second->outer_edges.size() == 1) {
			std::cerr << "Adding this cluster to the second one child using edge " <<first->outer_edges[0].edge << std::endl;
			second->second = cluster;
			cluster->parent = second;
			// outer edges will be calculated after finishing making all clusters on this level of topology tree
			return second;
		}
	} else if (first != NULL && first->second == NULL && first->outer_edges.size() <= 2) {
		std::cerr << "Adding this cluster to the first one child using edge " << cluster->outer_edges[0].edge << std::endl;
		first->second = cluster;
		cluster->parent = first;
		return first;
	}

	// 4. Cannot add to any cluster, have to create own
	auto new_cluster = std::make_shared<TopologyCluster>();
	new_cluster->first = cluster;
	cluster->parent = first;


	return cluster;
}

TopologyTopTree::~TopologyTopTree() {}
}
