#include "Cluster.hpp"

namespace TopTree {

std::ostream& operator<<(std::ostream& o, const Cluster& c) { return c.ToString(o); }

std::shared_ptr<BaseCluster> BaseCluster::construct(std::shared_ptr<BaseTree::Internal::Edge> edge) {
	auto cluster = std::make_shared<BaseCluster>();

	cluster->boundary_left = edge->from;
	cluster->boundary_right = edge->to;

	// Handles:
	// - if leaf: handle is the top most non-rake (base or compress) node having this vertex as one of its endpoints
	if (cluster->boundary_left->degree == 1) cluster->boundary_left->handle = cluster;
	if (cluster->boundary_right->degree == 1) cluster->boundary_right->handle = cluster;

	// Call user defined method
	Create(cluster->data, edge->data);

	return cluster;
}
void BaseCluster::flip() {
	auto temp = boundary_left;
	boundary_left = boundary_right;
	boundary_right = temp;
}
void BaseCluster::normalize() {
	// Recursive call in top-down direction
	if (parent != NULL) parent->normalize();
}
std::ostream& BaseCluster::ToString(std::ostream& o) const {
	return o << "BaseCluster - endpoints " << *boundary_left->data << ", " << *boundary_right->data;
}


std::shared_ptr<CompressCluster> CompressCluster::construct(std::shared_ptr<Cluster> left, std::shared_ptr<Cluster> right) {
	auto cluster = std::make_shared<CompressCluster>();

	if (left->boundary_left == right->boundary_left || left->boundary_left == right->boundary_right)
		cluster->common_vertex = left->boundary_left;
	else cluster->common_vertex = left->boundary_right;

	cluster->boundary_left = (left->boundary_left == cluster->common_vertex) ? left->boundary_right : left->boundary_left;
	cluster->boundary_right = (right->boundary_left == cluster->common_vertex) ? right->boundary_right : right->boundary_left;

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
	if (cluster->boundary_left->degree == 1) cluster->boundary_left->handle = cluster;
	if (cluster->boundary_right->degree == 1) cluster->boundary_right->handle = cluster;

	// Call user defined method
	Join(left->data, right->data, cluster->data);

	return cluster;
}
void CompressCluster::flip() {
	auto temp = boundary_left;
	boundary_left = boundary_right;
	boundary_right = temp;

	auto temp_child = left_child;
	left_child = right_child;
	right_child = temp_child;

	temp_child = left_foster;
	left_foster = right_foster;
	right_foster = temp_child;
}
void CompressCluster::normalize() {
	// Recursive call in top-down direction
	if (parent != NULL) parent->normalize();

	// Check endpoints
	if (left_child->boundary_left != boundary_left) left_child->flip();
	if (right_child->boundary_right != boundary_right) right_child->flip();

	// Check fosters connection
	if (left_foster != NULL && left_foster->boundary_right != common_vertex) left_foster->flip();
	if (right_foster != NULL && left_foster->boundary_right != common_vertex) left_foster->flip();
}
std::ostream& CompressCluster::ToString(std::ostream& o) const {
	return o << "CompressCluster - endpoints " << *boundary_left->data << " [" << *common_vertex->data << "] " << *boundary_right->data;
}


std::shared_ptr<RakeCluster> RakeCluster::construct(std::shared_ptr<Cluster> rake_from, std::shared_ptr<Cluster> rake_to) {
	auto cluster = std::make_shared<RakeCluster>();

	cluster->boundary_left = rake_to->boundary_left;
	cluster->boundary_right = rake_to->boundary_right;

	// Connections:
	cluster->left_child = rake_from;
	cluster->right_child = rake_to;
	rake_from->parent = cluster;
	rake_to->parent = cluster;

	// Call user defined method
	Join(rake_from->data, rake_to->data, cluster->data);

	return cluster;
}
void RakeCluster::flip() {} // Nothing to do
void RakeCluster::normalize() {
	// Recursive call in top-down direction
	if (parent != NULL) parent->normalize();

	// Joined by the right boundary
	if (left_child->boundary_right != boundary_right) left_child->flip();
	if (right_child->boundary_right != boundary_right) right_child->flip();
}
std::ostream& RakeCluster::ToString(std::ostream& o) const {
	return o << "RakeCluster - endpoints " << *boundary_left->data << ", " << *boundary_right->data;
}

}
