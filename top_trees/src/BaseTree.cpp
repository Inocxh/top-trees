#include <vector>
#include <list>
#include <memory>
#include <string>
#include <iostream>

#include "BaseTreeInternal.hpp"

namespace TopTree {

std::ostream& operator<<(std::ostream& o, const VertexData& v) { return v.ToString(o); }
std::ostream& operator<<(std::ostream& o, const EdgeData& e) { return e.ToString(o); }

int VertexData::v_counter = 0;
int EdgeData::e_counter = 0;

////////////////////////////////////////////////////////////////////////////////

BaseTree::BaseTree() : internal{std::make_unique<Internal>()} {}
BaseTree::~BaseTree() {}

int BaseTree::AddVertex(std::shared_ptr<VertexData> v) {
	if (v == NULL) v = std::make_shared<VertexData>();
	int i = internal->vertices.size();
	internal->vertices.push_back(std::make_shared<Internal::Vertex>(v));
	internal->vertices[i]->index = i;

	return i;
}

int BaseTree::AddEdge(int from, int to, std::shared_ptr<EdgeData> e) {
	if (e == NULL) e = std::make_shared<EdgeData>();
	int i = internal->edges.size();
	auto vertex_from = internal->vertices[from];
	auto vertex_to = internal->vertices[to];

	auto edge = std::make_shared<Internal::Edge>(vertex_from, vertex_to, e);
	edge->register_at_vertices();

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

void BaseTree::Internal::Edge::register_at_vertices() {
	from->neighbours.push_back(Internal::neighbour{shared_from_this()});
	from_iter = std::prev(from->neighbours.end());

	to->neighbours.push_back(Internal::neighbour{shared_from_this()});
	to_iter = std::prev(to->neighbours.end());

	from->degree++;
	to->degree++;

	// Superior vertices
	if (from->superior_vertex != NULL && !subvertice_edge) {
		from->superior_vertex->neighbours.push_back(Internal::neighbour{shared_from_this()});
		superior_from_iter = std::prev(from->superior_vertex->neighbours.end());
		from->superior_vertex->degree++;
	}
	if (to->superior_vertex != NULL && !subvertice_edge) {
		to->superior_vertex->neighbours.push_back(Internal::neighbour{shared_from_this()});
		superior_to_iter = std::prev(to->superior_vertex->neighbours.end());
		to->superior_vertex->degree++;
	}
}

void BaseTree::Internal::print_rooted_prefix(const std::shared_ptr<Vertex> root, const std::shared_ptr<Vertex> from, const std::string prefix, bool last_child) const {
	std::cout << prefix << "|-" << *root->data << std::endl;
	int size = root->neighbours.size();
	if (from == NULL) size++;
	auto prefix_child = prefix + (last_child ? "   " : "|  ");
	if (size) {
		for (const auto &v: root->neighbours) {
			if (auto ee = v.edge.lock()) {
				auto vv = ee->from;
				if (vv == root) vv = ee->to;
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
		if (auto ee = v.edge.lock()) {
			auto vv = ee->from;
			if (vv == root) vv = ee->to;
			if (vv == from) continue;
			if (ee->to != root) {
				std::cout << "Rotating" << std::endl;
			}
			orient_edges_to_root(vv, root);
		}
	}
}

}
