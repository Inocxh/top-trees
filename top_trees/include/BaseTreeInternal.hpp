#include <vector>
#include <list>

#ifndef BASE_TREE_INTERNAL_HPP
#define BASE_TREE_INTERNAL_HPP

#include "BaseTree.hpp"

namespace TopTree {

class BaseTree::Internal {
public:
	class Vertex;
	class Edge;

	struct neighbour {
		std::weak_ptr<Vertex> vertex;
		std::weak_ptr<Edge> edge;
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

	// Linkage to the other objects
	std::list<neighbour> neighbours;

	// Used for building TopTree:
	bool used;
};

class BaseTree::Internal::Edge {
public:
	Edge(std::shared_ptr<Vertex> from, std::shared_ptr<Vertex> to, std::shared_ptr<EdgeData> data):
		data{data}, from{from}, to{to}
	{}

	// Edge parameters
	bool deleted;
	std::shared_ptr<EdgeData> data;

	// Linkage to the other objects
	std::shared_ptr<Vertex> from;
	//std::list<neighbour>::iterator from_iter;
	std::shared_ptr<Vertex> to;
	//std::list<neighbour>::iterator to_iter;
};

}

#endif // BASE_TREE_INTERNAL_HPP
