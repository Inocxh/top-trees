#include <vector>
#include <memory>

#include "BaseTree.hpp"

namespace TopTree {

// Hide data from .hpp file using PIMP idiom
class BaseTree::BaseTreeData {
public:
	class Vertex;
	class Edge;

	std::vector<std::shared_ptr<Vertex> > vertices;
	// std::vector<std::shared_ptr<Vertex> > leaf_vertices;
	std::vector<std::shared_ptr<Edge> > edges;
};

class BaseTree::BaseTreeData::Vertex {
public:
	int degree;
	std::vector<std::weak_ptr<BaseTree::BaseTreeData::Vertex> > neighbours;
};

class BaseTree::BaseTreeData::Edge {
public:
	Edge(std::shared_ptr<Vertex> a, std::shared_ptr<Vertex> b): vertex_a{a}, vertex_b{b} {}

	std::shared_ptr<Vertex> vertex_a;
	std::shared_ptr<Vertex> vertex_b;
};

////////////////////////////////////////////////////////////////////////////////

BaseTree::BaseTree() : data{std::make_unique<BaseTreeData>()} {}
BaseTree::~BaseTree() {}

int BaseTree::AddVertex() {
	int i = data->vertices.size();
	data->vertices.push_back(std::make_shared<BaseTreeData::Vertex>());
	return i;
}

int BaseTree::AddEdge(int a, int b) {
	int i = data->edges.size();
	auto vertex_a = data->vertices[a];
	auto vertex_b = data->vertices[b];
	vertex_a->neighbours.push_back(vertex_b);
	vertex_b->neighbours.push_back(vertex_a);

	data->edges.push_back(std::make_shared<BaseTreeData::Edge>(vertex_a, vertex_b));
	return i;
}

}
