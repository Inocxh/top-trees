#include <vector>
#include <list>

#include "BaseTree.hpp"
#include "TopCluster.hpp"
#include "TopologyCluster.hpp"

#ifndef BASE_TREE_INTERNAL_HPP
#define BASE_TREE_INTERNAL_HPP

namespace TopTree {

class BaseTree::Internal {
public:
	class Vertex;
	class Edge;

	struct neighbour {
		// std::weak_ptr<Vertex> vertex;
		std::weak_ptr<Edge> edge;
		std::list<std::shared_ptr<Vertex>>::iterator subvertice_iter; // may be set, it is needed to everytime check the edge->subvertice_edge to test if it is subvertice_edge
	};

	std::vector<std::shared_ptr<Vertex> > vertices;
	std::vector<std::shared_ptr<Edge> > edges;

	// Internal functions:
	std::vector<std::shared_ptr<Vertex> > find_leafs();

	void orient_edges_to_root(const std::shared_ptr<Vertex> root, const std::shared_ptr<Vertex> from = NULL);

	void print_rooted_prefix(const std::shared_ptr<Vertex> root, const std::shared_ptr<Vertex> from = NULL, const std::string prefix = "", bool last_child = true) const;
};

// Hide data from .hpp file using PIMP idiom

class BaseTree::Internal::Vertex {
public:
	Vertex(std::shared_ptr<VertexData> data):
		data{data}
	{}

	// Vertex parameters
	bool deleted;
	int degree = 0;
	std::shared_ptr<VertexData> data;
	int index;

	// Linkage to the other objects
	std::list<neighbour> neighbours;

	// Handle
	// - if degree at least 2: handle is comprees node around this middle vertex
	// - if leaf: handle is the top most non-rake (base or compress) node having this vertex as one of its endpoints

	// Points to some BaseCluster that has this vertex as one of its endpoints:
	std::list<std::shared_ptr<TopCluster>> base_handles;
	// Points to the last TopCluster that was found as handle. When this TopCluster is no longer a
	// handle, base_handle is used to recompute it
	std::shared_ptr<TopCluster> last_handle = NULL;

	// Used for building TopTree:
	bool used = false;
	std::shared_ptr<TopCluster> rake_tree_left = NULL;
	std::shared_ptr<TopCluster> rake_tree_right = NULL;

	// Used in TopologyTopTree
	std::shared_ptr<Vertex> superior_vertex = NULL;
	std::list<std::shared_ptr<Vertex>> subvertices;
	std::list<std::shared_ptr<Vertex>>::iterator superior_vertex_subvertices_iter;
	std::list<std::shared_ptr<Edge>> subvertice_edges;
	std::shared_ptr<TopologyCluster> topology_cluster;

	// Used in TopologyTopTree expose procedure
	std::vector<std::shared_ptr<SimpleCluster>> expose_clusters;

	void unlink() {
		neighbours.clear();
		base_handles.clear();
		last_handle = NULL;
		rake_tree_left = NULL;
		rake_tree_right = NULL;
		superior_vertex = NULL;
		subvertices.clear();
		subvertice_edges.clear();
		topology_cluster = NULL;
		expose_clusters.clear();

		deleted = true;
	}

	friend std::ostream& operator<<(std::ostream& o, const Vertex& v) {
		o << *v.data;
		if (v.superior_vertex != NULL) o << "(" << *v.superior_vertex << ")";
		return o;
	}

	static std::shared_ptr<Vertex> get_superior(std::shared_ptr<Vertex> v) {
		if (v->superior_vertex != NULL) return v->superior_vertex;
		else return v;
	}
};

class BaseTree::Internal::Edge : public std::enable_shared_from_this<Edge> {
public:
	Edge(std::shared_ptr<Vertex> from, std::shared_ptr<Vertex> to, std::shared_ptr<EdgeData> data):
		data{data}, from{from}, to{to}
	{}

	void register_at_vertices();

	// Edge parameters
	bool deleted;
	std::shared_ptr<EdgeData> data;

	// Linkage to the other objects
	std::shared_ptr<Vertex> from;
	std::list<neighbour>::iterator from_iter;
	std::list<neighbour>::iterator superior_from_iter;
	std::shared_ptr<Vertex> to;
	std::list<neighbour>::iterator to_iter;
	std::list<neighbour>::iterator superior_to_iter;

	// Used in TopologyTopTree
	bool subvertice_edge = false;
	std::list<std::shared_ptr<Edge>>::iterator subvertice_edges_iterator;

	void unlink() {
		from = NULL;
		to = NULL;

		deleted = true;
	}
};

// Edges in TopologyTree
// a) normal edge
//     - from / to = real vertex to which this edge points
//     - from_iter / to_iter = iter in the neighbours list
//     - superior_from_iter / superior_to_iter = iterator to the superior vertex's neighbours list
//     - subvertice_edge = false
//     - subvertice_edges_iterator = NULL
// b) subvertice edge
//     - from / to = real vertex to which this edge points
//     - from_iter / to_iter = iter in the neighbours list
//     - superior_from_iter / superior_to_iter = NULL
//     - subvertice_edge = true
//     - subvertice_edges_iterator = iter in the superior vertex's subvertice_edges list

}

#endif // BASE_TREE_INTERNAL_HPP
