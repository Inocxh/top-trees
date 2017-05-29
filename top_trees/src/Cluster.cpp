#include "Cluster.hpp"

namespace TopTree {

std::ostream& operator<<(std::ostream& o, const Cluster& c) { return c.ToString(o); }

std::shared_ptr<BaseCluster> BaseCluster::construct(std::shared_ptr<BaseTree::Internal::Edge> edge) {
	auto cluster = std::make_shared<BaseCluster>();

	cluster->boundary_a = edge->from;
	cluster->boundary_b = edge->to;

	// Handles:
	// - if leaf: handle is the top most non-rake (base or compress) node having this vertex as one of its endpoints
	if (cluster->boundary_a->degree == 1) cluster->boundary_a->handle = cluster;
	if (cluster->boundary_b->degree == 1) cluster->boundary_b->handle = cluster;

	// Call user defined method
	Create(cluster->data, edge->data);

	return cluster;
}
std::ostream& BaseCluster::ToString(std::ostream& o) const {
	return o << "BaseCluster - endpoints " << *boundary_a->data << ", " << *boundary_b->data;
}


std::shared_ptr<CompressCluster> CompressCluster::construct(std::shared_ptr<Cluster> left, std::shared_ptr<Cluster> right) {
	auto cluster = std::make_shared<CompressCluster>();

	if (left->boundary_a == right->boundary_a || left->boundary_a == right->boundary_b)
		cluster->common_vertex = left->boundary_a;
	else cluster->common_vertex = left->boundary_b;

	cluster->boundary_a = (left->boundary_a == cluster->common_vertex) ? left->boundary_b : left->boundary_a;
	cluster->boundary_b = (right->boundary_a == cluster->common_vertex) ? right->boundary_b : right->boundary_a;

	// Connections:
	cluster->left_child = left;
	cluster->right_child = right;
	left->parent = cluster;
	right->parent = cluster;
	// Foster children (if there are any)
	cluster->left_foster = cluster->common_vertex->rake_tree_left;
	cluster->right_foster = cluster->common_vertex->rake_tree_right;
	if (cluster->left_foster != NULL) cluster->left_foster->parent = cluster;
	if (cluster->right_foster != NULL) cluster->right_foster->parent = cluster;

	// Handles:
	// - if degree at least 2: handle is comprees node around this middle vertex
	cluster->common_vertex->handle = cluster;
	// - if leaf: handle is the top most non-rake (base or compress) node having this vertex as one of its endpoints
	if (cluster->boundary_a->degree == 1) cluster->boundary_a->handle = cluster;
	if (cluster->boundary_b->degree == 1) cluster->boundary_b->handle = cluster;

	// Call user defined method
	Join(left->data, right->data, cluster->data);

	return cluster;
}
std::ostream& CompressCluster::ToString(std::ostream& o) const {
	return o << "CompressCluster - endpoints " << *boundary_a->data << " [" << *common_vertex->data << "] " << *boundary_b->data;
}


std::shared_ptr<RakeCluster> RakeCluster::construct(std::shared_ptr<Cluster> rake_from, std::shared_ptr<Cluster> rake_to) {
	auto cluster = std::make_shared<RakeCluster>();

	cluster->boundary_a = rake_to->boundary_a;
	cluster->boundary_b = rake_to->boundary_b;

	// Connections:
	cluster->left_child = rake_from;
	cluster->right_child = rake_to;
	rake_from->parent = cluster;
	rake_to->parent = cluster;

	// Call user defined method
	Join(rake_from->data, rake_to->data, cluster->data);

	return cluster;
}
std::ostream& RakeCluster::ToString(std::ostream& o) const {
	return o << "RakeCluster - endpoints " << *boundary_a->data << ", " << *boundary_b->data;
}

}
