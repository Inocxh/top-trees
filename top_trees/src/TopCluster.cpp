#include "TopCluster.hpp"

namespace TopTree {
std::ostream& operator<<(std::ostream& o, const TopCluster& c) { return c.ToString(o); }

void TopCluster::set_left_child(std::shared_ptr<TopCluster> child) {
	left_child = child;
	if (child != NULL) child->parent = shared_from_this();
}
void TopCluster::set_right_child(std::shared_ptr<TopCluster> child) {
	right_child = child;
	if (child != NULL) child->parent = shared_from_this();
}
void TopCluster::set_left_foster(std::shared_ptr<TopCluster> child) {
	left_foster = child;
	if (child != NULL) child->parent = shared_from_this();
}
void TopCluster::set_right_foster(std::shared_ptr<TopCluster> child) {
	right_foster = child;
	if (child != NULL) child->parent = shared_from_this();
}

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
void BaseCluster::do_split(std::vector<std::shared_ptr<TopCluster>>* splitted_clusters) {
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
void BaseCluster::normalize_for_splay() {
	// Recursive call in top-down direction
	if (parent != NULL) parent->normalize_for_splay();
}
std::ostream& BaseCluster::ToString(std::ostream& o) const {
	return o << "BaseCluster - endpoints " << *boundary_left->data << ", " << *boundary_right->data;
}
std::ostream& BaseCluster::_short_name(std::ostream& o) const {
	return o << *boundary_left->data << "," << *boundary_right->data;
}

std::shared_ptr<CompressCluster> CompressCluster::construct(std::shared_ptr<TopCluster> left, std::shared_ptr<TopCluster> right) {
	auto cluster = std::make_shared<CompressCluster>();

	// Basic connections (needed by do_join):
	cluster->left_child = left;
	cluster->right_child = right;
	left->parent = cluster;
	right->parent = cluster;

	cluster->correct_endpoints();

	// Foster children (if there are any)
	cluster->left_foster = cluster->common_vertex->rake_tree_left;
	cluster->right_foster = cluster->common_vertex->rake_tree_right;
	if (cluster->left_foster != NULL) cluster->left_foster->parent = cluster;
	if (cluster->right_foster != NULL) cluster->right_foster->parent = cluster;

	cluster->do_join();

	return cluster;
}
void CompressCluster::do_join() {
	if (!is_splitted) return;

	// 1. ensure that childs are fine (joined):
	left_child->do_join();
	right_child->do_join();
	if (left_foster != NULL) left_foster->do_join();
	if (right_foster != NULL) right_foster->do_join();

	// 2. Compute boundary and common vertices and check handles
	correct_endpoints();

	// 3. Call user defined method:
	// 3.1 If there are foster children first do Join into virtual rake nodes
	auto left_data = left_child->data;
	if (left_foster != NULL) {
		left_data = InitClusterData();
		Join(left_foster->data, left_child->data, left_data, true);
	}
	auto right_data = right_child->data;
	if (right_foster != NULL) {
		right_data = InitClusterData();
		Join(right_foster->data, right_child->data, right_data, true);
	}
	// 3.2 Normal Join
	Join(left_data, right_data, data, rakerized); // if rakerized act as Rake cluster during Joining

	is_splitted = false;
}
void CompressCluster::do_split(std::vector<std::shared_ptr<TopCluster>>* splitted_clusters) {
	if (is_splitted) return;

	// 1. Log that this cluster will be splitted
	if (splitted_clusters != NULL) splitted_clusters->push_back(shared_from_this());

	// 2. Ensure that parent is splitted:
	if (parent != NULL) parent->do_split(splitted_clusters);

	// 3. Call user defined method:
	// 3.1 If there are foster children first prepare virtual rake nodes
	auto left_data = left_child->data;
	if (left_foster != NULL) left_data = InitClusterData();
	auto right_data = right_child->data;
	if (right_foster != NULL) right_data = InitClusterData();
	// 3.2 Normal Split
	Split(left_data, right_data, data, rakerized); // if rakerized act as Rake cluster during Splitting
	// 3.3 If there are foster children Split virtual rake nodes
	if (left_foster != NULL) Split(left_foster->data, left_child->data, left_data, true);
	if (right_foster != NULL) Split(right_foster->data, right_child->data, right_data, true);

	is_splitted = true;
}
void CompressCluster::correct_endpoints() {
	// 1. Correct endpoints
	if (left_child->boundary_left == right_child->boundary_left || left_child->boundary_left == right_child->boundary_right)
		common_vertex = left_child->boundary_left;
	else common_vertex = left_child->boundary_right;

	boundary_left = (left_child->boundary_left == common_vertex) ? left_child->boundary_right : left_child->boundary_left;
	boundary_right = (right_child->boundary_left == common_vertex) ? right_child->boundary_right : right_child->boundary_left;

	// 2. Check handles:
	// - if degree at least 2: handle is comprees node around this middle vertex
	common_vertex->handle = shared_from_this();
	// - if leaf: handle is the top most non-rake (base or compress) node having this vertex as one of its endpoints
	if (boundary_left->degree == 1) boundary_left->handle = shared_from_this();
	if (boundary_right->degree == 1) boundary_right->handle = shared_from_this();
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
void CompressCluster::normalize_for_splay() {
	// Recursive call in top-down direction
	correct_endpoints();
	if (parent != NULL) parent->normalize_for_splay();

	// Check endpoints
	if (left_child->boundary_left != boundary_left) left_child->flip();
	if (right_child->boundary_right != boundary_right) right_child->flip();

	// Check fosters connection (by their right boundary)
	if (left_foster != NULL && left_foster->boundary_right != common_vertex) left_foster->flip();
	if (right_foster != NULL && right_foster->boundary_right != common_vertex) right_foster->flip();

}
std::ostream& CompressCluster::ToString(std::ostream& o) const {
	return o << "CompressCluster - endpoints " << *boundary_left->data << " [" << *common_vertex->data << "] " << *boundary_right->data;
}
std::ostream& CompressCluster::_short_name(std::ostream& o) const {
	return o << "N" << *common_vertex->data << "\\n" << *boundary_left->data << "," << *boundary_right->data;;
}


std::shared_ptr<RakeCluster> RakeCluster::construct(std::shared_ptr<TopCluster> rake_from, std::shared_ptr<TopCluster> rake_to) {
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
	correct_endpoints();

	// 3. Call user defined method:
	Join(rake_from->data, rake_to->data, data, true);

	is_splitted = false;
}
void RakeCluster::do_split(std::vector<std::shared_ptr<TopCluster>>* splitted_clusters) {
	if (is_splitted) return;

	// 1. Log that this cluster will be splitted
	if (splitted_clusters != NULL) splitted_clusters->push_back(shared_from_this());

	// 2. Ensure that parent is splitted:
	if (parent != NULL) parent->do_split(splitted_clusters);

	// 3. Call user defined method:
	Split(left_child->data, right_child->data, data, true);

	is_splitted = true;
}
void RakeCluster::correct_endpoints() {
	// nicknames
	auto rake_from = left_child;
	auto rake_to = right_child;

	boundary_left = rake_to->boundary_left;
	boundary_right = rake_to->boundary_right;
}

void RakeCluster::flip() {
	auto temp = boundary_left;
	boundary_left = boundary_right;
	boundary_right = temp;

	auto temp_child = left_child;
	left_child = right_child;
	right_child = temp_child;
}
void RakeCluster::normalize_for_splay() {
	// Recursive call in top-down direction
	correct_endpoints();
	if (parent != NULL) parent->normalize_for_splay();

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
