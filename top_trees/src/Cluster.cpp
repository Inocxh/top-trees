#include "Cluster.hpp"

namespace TopTree {

std::ostream& operator<<(std::ostream& o, const Cluster& c) { return c.ToString(o); }

std::shared_ptr<BaseCluster> BaseCluster::construct(std::shared_ptr<BaseTree::Internal::Edge> edge) {
	auto cluster = std::make_shared<BaseCluster>();

	cluster->edge = edge;
	cluster->do_join();

	return cluster;
}
void BaseCluster::do_join() {
	if (!is_splitted) return;
	// No child, no need to join them

	// 1. Set boundaries:
	boundary_left = edge->from;
	boundary_right = edge->to;

	// 2. Compute handles:
	// - if leaf: handle is the top most non-rake (base or compress) node having this vertex as one of its endpoints
	if (boundary_left->degree == 1) boundary_left->handle = shared_from_this();
	if (boundary_right->degree == 1) boundary_right->handle = shared_from_this();

	// 3. Call user defined method:
	Create(data, edge->data);

	is_splitted = false;
}
void BaseCluster::do_split(std::vector<std::shared_ptr<Cluster>>* splitted_clusters) {
	if (is_splitted) return;

	// 1. Log that this cluster will be splitted
	if (splitted_clusters != NULL) splitted_clusters->push_back(shared_from_this());

	// 2. Ensure that parent is splitted:
	if (parent != NULL) parent->do_split(splitted_clusters);

	// 3. Call user defined method:
	Destroy(data, edge->data);

	is_splitted = true;
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
std::ostream& BaseCluster::_short_name(std::ostream& o) const {
	return o << *boundary_left->data << "," << *boundary_right->data;
}

std::shared_ptr<CompressCluster> CompressCluster::construct(std::shared_ptr<Cluster> left, std::shared_ptr<Cluster> right) {
	auto cluster = std::make_shared<CompressCluster>();

	// Basic connections (needed by do_join):
	cluster->left_child = left;
	cluster->right_child = right;
	left->parent = cluster;
	right->parent = cluster;

	cluster->do_join();

	// Foster children (if there are any)
	cluster->left_foster = cluster->common_vertex->rake_tree_left;
	cluster->right_foster = cluster->common_vertex->rake_tree_right;
	if (cluster->left_foster != NULL) cluster->left_foster->parent = cluster;
	if (cluster->right_foster != NULL) cluster->right_foster->parent = cluster;

	return cluster;
}
void CompressCluster::do_join() {
	if (!is_splitted) return;

	// 1. ensure that childs are fine (joined):
	left_child->do_join();
	right_child->do_join();

	// 2. Compute boundary and common vertices:
	if (left_child->boundary_left == right_child->boundary_left || left_child->boundary_left == right_child->boundary_right)
		common_vertex = left_child->boundary_left;
	else common_vertex = left_child->boundary_right;

	boundary_left = (left_child->boundary_left == common_vertex) ? left_child->boundary_right : left_child->boundary_left;
	boundary_right = (right_child->boundary_left == common_vertex) ? right_child->boundary_right : right_child->boundary_left;

	// 3. Compute handles:
	// - if degree at least 2: handle is comprees node around this middle vertex
	common_vertex->handle = shared_from_this();
	// - if leaf: handle is the top most non-rake (base or compress) node having this vertex as one of its endpoints
	if (boundary_left->degree == 1) boundary_left->handle = shared_from_this();
	if (boundary_right->degree == 1) boundary_right->handle = shared_from_this();

	// 4. Call user defined method:
	Join(left_child->data, right_child->data, data);

	is_splitted = false;
}
void CompressCluster::do_split(std::vector<std::shared_ptr<Cluster>>* splitted_clusters) {
	if (is_splitted) return;

	// 1. Log that this cluster will be splitted
	if (splitted_clusters != NULL) splitted_clusters->push_back(shared_from_this());

	// 2. Ensure that parent is splitted:
	if (parent != NULL) parent->do_split(splitted_clusters);

	// 3. Call user defined method:
	Split(left_child->data, right_child->data, data);

	is_splitted = true;
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
std::ostream& CompressCluster::_short_name(std::ostream& o) const {
	return o << "N" << *common_vertex->data << "\\n" << *boundary_left->data << "," << *boundary_right->data;;
}


std::shared_ptr<RakeCluster> RakeCluster::construct(std::shared_ptr<Cluster> rake_from, std::shared_ptr<Cluster> rake_to) {
	auto cluster = std::make_shared<RakeCluster>();

	// Basic connections (needed by do_join):
	cluster->left_child = rake_from;
	cluster->right_child = rake_to;
	rake_from->parent = cluster;
	rake_to->parent = cluster;

	cluster->do_join();

	return cluster;
}
void RakeCluster::do_join() {
	if (!is_splitted) return;

	// nicknames
	auto rake_from = left_child;
	auto rake_to = right_child;

	// 1. ensure that childs are fine (joined):
	rake_from->do_join();
	rake_to->do_join();

	// 2. Set boundary vertices:
	boundary_left = rake_to->boundary_left;
	boundary_right = rake_to->boundary_right;

	// 3. Call user defined method:
	Join(rake_from->data, rake_to->data, data);

	is_splitted = false;
}
void RakeCluster::do_split(std::vector<std::shared_ptr<Cluster>>* splitted_clusters) {
	if (is_splitted) return;

	// 1. Log that this cluster will be splitted
	if (splitted_clusters != NULL) splitted_clusters->push_back(shared_from_this());

	// 2. Ensure that parent is splitted:
	if (parent != NULL) parent->do_split(splitted_clusters);

	// 3. Call user defined method:
	Split(left_child->data, right_child->data, data);

	is_splitted = true;
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
std::ostream& RakeCluster::_short_name(std::ostream& o) const {
	return o << *boundary_left->data << "," << *boundary_right->data;
}

}
