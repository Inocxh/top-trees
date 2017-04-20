#include <vector>
#include <list>
#include <memory>
#include <string>
#include <iostream>

#include "BaseTree.hpp"

namespace TopTree {

std::ostream& operator<<(std::ostream& o, const VertexData& v) { return v.ToString(o); }
std::ostream& operator<<(std::ostream& o, const EdgeData& e) { return e.ToString(o); }

////////////////////////////////////////////////////////////////////////////////

// Hide data from .hpp file using PIMP idiom
class BaseTree::BaseTreeData {
public:
	class Vertex;
	class Edge;

	struct neighbour {
		std::weak_ptr<Vertex> vertex;
		std::weak_ptr<Edge> edge;
	};

	std::vector<std::shared_ptr<Vertex> > vertices;
	// std::vector<std::shared_ptr<Vertex> > leaf_vertices;
	std::vector<std::shared_ptr<Edge> > edges;

	void print_rooted_prefix(const std::shared_ptr<Vertex> root, const std::shared_ptr<Vertex> from = NULL, const std::string prefix = "", bool last_child = true) const;
};

class BaseTree::BaseTreeData::Vertex {
public:
	Vertex(std::shared_ptr<VertexData> data):
		data{data}
	{}

	// Vertex parameters
	bool deleted;
	int degree;
	std::shared_ptr<VertexData> data;

	// Linkage to the other objects
	std::list<neighbour> neighbours;
};

class BaseTree::BaseTreeData::Edge {
public:
	Edge(std::shared_ptr<Vertex> a, std::shared_ptr<Vertex> b, std::shared_ptr<EdgeData> data):
		data{data}, vertex_a{a}, vertex_b{b}
	{}

	// Edge parameters
	bool deleted;
	std::shared_ptr<EdgeData> data;

	// Linkage to the other objects
	std::shared_ptr<Vertex> vertex_a;
	std::list<neighbour>::iterator vertex_a_iter;
	std::shared_ptr<Vertex> vertex_b;
	std::list<neighbour>::iterator vertex_b_iter;
};

////////////////////////////////////////////////////////////////////////////////

BaseTree::BaseTree() : data{std::make_unique<BaseTreeData>()} {}
BaseTree::~BaseTree() {}

int BaseTree::AddVertex(std::shared_ptr<VertexData> v) {
	if (v == NULL) v = std::make_shared<VertexData>();
	int i = data->vertices.size();
	data->vertices.push_back(std::make_shared<BaseTreeData::Vertex>(v));

	return i;
}

int BaseTree::AddEdge(int a, int b, std::shared_ptr<EdgeData> e) {
	if (e == NULL) e = std::make_shared<EdgeData>();
	int i = data->edges.size();
	auto vertex_a = data->vertices[a];
	auto vertex_b = data->vertices[b];
	auto edge = std::make_shared<BaseTreeData::Edge>(vertex_a, vertex_b, e);

	vertex_a->neighbours.push_back(BaseTreeData::neighbour{vertex_b, edge});
	edge->vertex_a_iter = std::prev(vertex_a->neighbours.end());

	vertex_b->neighbours.push_back(BaseTreeData::neighbour{vertex_a, edge});
	edge->vertex_b_iter = std::prev(vertex_b->neighbours.end());

	data->edges.push_back(edge);
	return i;
}

int BaseTree::AddLeaf(int parent, std::shared_ptr<EdgeData> e, std::shared_ptr<VertexData> v) {
	int vi = AddVertex(v);
	AddEdge(parent, vi, e);
	return vi;
}

void BaseTree::PrintRooted(int root) {
	data->print_rooted_prefix(data->vertices[root], NULL);
}

void BaseTree::BaseTreeData::print_rooted_prefix(const std::shared_ptr<Vertex> root, const std::shared_ptr<Vertex> from, const std::string prefix, bool last_child) const {
	std::cout << prefix << "|-" << *root->data << std::endl;
	int size = root->neighbours.size();
	if (from == NULL) size++;
	auto prefix_child = prefix + (last_child ? "   " : "|  ");
	if (size) {
		for (const auto &v: root->neighbours) {
			if (auto vv = v.vertex.lock()) if (auto ee = v.edge.lock()) {
				if (vv == from) continue;
				size--;
				std::cout << prefix_child << "|" << *ee->data << std::endl;
				print_rooted_prefix(vv, root, prefix_child, (size <= 1));
			}
		}
	}
}

}
