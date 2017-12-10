#include <vector>
#include <list>
#include <memory>
#include <string>
#include <iostream>

#include "BaseTree.hpp"
#include "BaseTreeInternal.hpp"

namespace TopTree {

std::ostream& operator<<(std::ostream& o, const VertexData& v) { return v.ToString(o); }
std::ostream& operator<<(std::ostream& o, const EdgeData& e) { return e.ToString(o); }

////////////////////////////////////////////////////////////////////////////////

BaseTree::BaseTree() : internal{std::make_unique<Internal>()} {}
BaseTree::~BaseTree() {}

int BaseTree::AddVertex(std::shared_ptr<VertexData> v) {
	if (v == NULL) v = std::make_shared<VertexData>();
	int i = internal->vertices.size();
	internal->vertices.push_back(std::make_shared<Internal::Vertex>(v));

	return i;
}

int BaseTree::AddEdge(int from, int to, std::shared_ptr<EdgeData> e) {
	if (e == NULL) e = std::make_shared<EdgeData>();
	int i = internal->edges.size();
	auto vertex_from = internal->vertices[from];
	auto vertex_to = internal->vertices[to];
	auto edge = std::make_shared<Internal::Edge>(vertex_from, vertex_to, e);

	vertex_from->neighbours.push_back(Internal::neighbour{vertex_to, edge});
	edge->from_iter = std::prev(vertex_from->neighbours.end());

	vertex_to->neighbours.push_back(Internal::neighbour{vertex_from, edge});
	edge->to_iter = std::prev(vertex_to->neighbours.end());

	vertex_from->degree++;
	vertex_to->degree++;

	internal->edges.push_back(edge);
	return i;
}

int BaseTree::AddLeaf(int parent, std::shared_ptr<EdgeData> e, std::shared_ptr<VertexData> v) {
	int vi = AddVertex(v);
	AddEdge(vi, parent, e);
	return vi;
}

void BaseTree::PrintRooted(int root) {
	internal->print_rooted_prefix(internal->vertices[root]);
	//internal->orient_edges_to_root(internal->vertices[root]);
}

void BaseTree::Internal::print_rooted_prefix(const std::shared_ptr<Vertex> root, const std::shared_ptr<Vertex> from, const std::string prefix, bool last_child) const {
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

void BaseTree::Internal::orient_edges_to_root(const std::shared_ptr<Vertex> root, const std::shared_ptr<Vertex> from) {
	for (const auto &v: root->neighbours) {
		if (auto vv = v.vertex.lock()) if (auto ee = v.edge.lock()) {
			if (vv == from) continue;
			if (ee->to != root) {
				std::cout << "Rotating" << std::endl;
			}
			orient_edges_to_root(vv, root);
		}
	}
}

}
