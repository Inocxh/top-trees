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

void TopCluster::unlink() {
	parent = NULL;
	left_foster = NULL;
	right_foster = NULL;
	left_child = NULL;
	right_child = NULL;
}

std::shared_ptr<BaseCluster> BaseCluster::construct(std::shared_ptr<BaseTree::Internal::Edge> edge) {
	auto cluster = std::make_shared<BaseCluster>();

	cluster->edge = edge;
	cluster->do_join();

	return cluster;
}
void BaseCluster::do_join() {
	if (!is_splitted || is_deleted) return;
	// No child, no need to join them

	// 1. Set boundaries:
	boundary_left = edge->from;
	boundary_right = edge->to;

	// 2. Set base handles:
	// Handle is the top most non-rake (base or compress) node having this vertex as one of its endpoints (for leafs) or as its midpoint (when degree of vertex >=2)
	// Base handle is used for recomputing handles when some change occurs (during rotating, splaying, splicing) and last_handle is no longer handle.
	//boundary_left->base_handle = shared_from_this();
	if (!handles_registered) { // to not register them more than once
		boundary_left->base_handles.push_back(shared_from_this());
		boundary_left_handles_iterator = std::prev(boundary_left->base_handles.end());

		//boundary_right->base_handle = shared_from_this();
		boundary_right->base_handles.push_back(shared_from_this());
		boundary_right_handles_iterator = std::prev(boundary_right->base_handles.end());

		handles_registered = true;
	}

	// 3. Call user defined method:
	Create(shared_from_this(), edge->data);

	is_splitted = false;
}
void BaseCluster::do_split(std::vector<std::shared_ptr<TopCluster>>* splitted_clusters) {
	if (is_splitted) return;

	// 1. Log that this cluster will be splitted
	if (splitted_clusters != NULL) splitted_clusters->push_back(shared_from_this());

	// 2. Ensure that parent is splitted:
	if (parent != NULL) parent->do_split(splitted_clusters);

	// 3. Call user defined method:
	Destroy(shared_from_this(), edge->data);

	is_splitted = true;
}
void BaseCluster::unregister() {
	if (boundary_left->last_handle == shared_from_this()) boundary_left->last_handle = NULL;
	if (boundary_right->last_handle == shared_from_this()) boundary_right->last_handle = NULL;

	if (handles_registered) {
		boundary_left->base_handles.erase(boundary_left_handles_iterator);
		boundary_right->base_handles.erase(boundary_right_handles_iterator);
	}
	edge->from->neighbours.erase(edge->from_iter);
	edge->to->neighbours.erase(edge->to_iter);

	boundary_left->degree--;
	boundary_right->degree--;

	is_deleted = true;
}

void BaseCluster::unlink() {
	if (!is_deleted) unregister();

	TopCluster::unlink();
	edge = NULL;
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
bool BaseCluster::is_handle_for(std::shared_ptr<BaseTree::Internal::Vertex> v) {
	return (boundary_left == v || boundary_right == v);
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
	cluster->set_left_child(left);
	cluster->set_right_child(right);

	cluster->correct_endpoints();

	// Foster children (if there are any)
	cluster->set_left_foster(cluster->common_vertex->rake_tree_left);
	cluster->common_vertex->rake_tree_left = NULL;
	cluster->set_right_foster(cluster->common_vertex->rake_tree_right);
	cluster->common_vertex->rake_tree_right = NULL;

	cluster->do_join();

	return cluster;
}
void CompressCluster::do_join() {
	if (!is_splitted || is_deleted) return;

	// 1. ensure that childs are fine (joined):
	left_child->do_join();
	right_child->do_join();
	if (left_foster != NULL) left_foster->do_join();
	if (right_foster != NULL) right_foster->do_join();

	// 2. Compute boundary and common vertices
	correct_endpoints();

	// 3. Call user defined method:
	// 3.1 If there are foster children first do Join into virtual rake nodes
	auto left = left_child;
	if (left_foster != NULL) {
		left_foster_rake = RakeCluster::construct(left_foster, left_child, true);
		left = left_foster_rake;
	}
	auto right = right_child;
	if (right_foster != NULL) {
		right_foster_rake = RakeCluster::construct(right_foster, right_child, true);
		right = right_foster_rake;
	}
	// 3.2 Normal Join
	Join(left, right, shared_from_this());

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
	auto left = left_child;
	if (left_foster != NULL) left = left_foster_rake;
	auto right = right_child;
	if (right_foster != NULL) right = right_foster_rake;
	// 3.2 Normal Split
	Split(left, right, shared_from_this());
	// 3.3 If there are foster children Split virtual rake nodes
	if (left_foster != NULL) Split(left_foster, left_child, left);
	if (right_foster != NULL) Split(right_foster, right_child, right);

	is_splitted = true;
}
void CompressCluster::correct_endpoints() {
	// 1. Find common vertex
	if (left_child->boundary_left == right_child->boundary_left || left_child->boundary_left == right_child->boundary_right)
		common_vertex = left_child->boundary_left;
	else if (left_child->boundary_right == right_child->boundary_left || left_child->boundary_right == right_child->boundary_right)
		common_vertex = left_child->boundary_right;
	else {
		std::cerr << "ERROR: " << *shared_from_this() << " Cannot find common vertex of clusters " << *left_child << " and " << *right_child << std::endl;
		// exit(1);
		*(int*)0 = 0; // HACK: produce segfault to see "better" stacktrace output in valgrind
	}

	// 2. Correct endpoints
	boundary_left = (left_child->boundary_left == common_vertex) ? left_child->boundary_right : left_child->boundary_left;
	boundary_right = (right_child->boundary_left == common_vertex) ? right_child->boundary_right : right_child->boundary_left;
}
void CompressCluster::unregister() {
	if (boundary_left->last_handle == shared_from_this()) boundary_left->last_handle = NULL;
	if (boundary_right->last_handle == shared_from_this()) boundary_right->last_handle = NULL;
	if (common_vertex->last_handle == shared_from_this()) common_vertex->last_handle = NULL;

	is_deleted = true;
}

void CompressCluster::unlink() {
	if (!is_deleted) unregister();

	TopCluster::unlink();
	left_foster_rake = NULL;
	right_foster_rake = NULL;
}

void CompressCluster::flip() {
	auto temp = boundary_left;
	boundary_left = boundary_right;
	boundary_right = temp;

	auto temp_child = left_child;
	left_child = right_child;
	right_child = temp_child;

	temp_child = left_foster;
	auto temp_foster_rake = left_foster_rake;
	left_foster = right_foster;
	left_foster_rake = right_foster_rake;
	right_foster = temp_child;
	right_foster_rake = temp_foster_rake;
}
void CompressCluster::normalize_for_splay() {
	// Recursive call in top-down direction
	correct_endpoints();

	if (parent != NULL) parent->normalize_for_splay();

	// Check endpoints
	if (left_child->boundary_left != boundary_left) left_child->flip();
	if (right_child->boundary_right != boundary_right) right_child->flip();

	// Check fosters connection (by their right boundary)
	if (left_foster != NULL && left_foster->boundary_right != common_vertex) {
		if (left_foster->boundary_left == common_vertex) left_foster->flip();
		else {
			std::cerr << "Left foster " << *left_foster << " of " << *shared_from_this() << " does not have common vertex" << std::endl;
			exit(1);
		}
	}

	if (right_foster != NULL && right_foster->boundary_right != common_vertex) {
		if (right_foster->boundary_left == common_vertex) right_foster->flip();
		else {
			std::cerr << "Right foster " << *right_foster << " of " << *shared_from_this() << " does not have common vertex" << std::endl;
			exit(1);
		}
	}
}
bool CompressCluster::is_handle_for(std::shared_ptr<BaseTree::Internal::Vertex> v) {
	return (boundary_left == v || boundary_right == v || common_vertex == v);
}
std::ostream& CompressCluster::ToString(std::ostream& o) const {
	return o << "CompressCluster - endpoints " << *boundary_left->data << " [" << *common_vertex->data << "] " << *boundary_right->data;
}
std::ostream& CompressCluster::_short_name(std::ostream& o) const {
	return o << "N" << *common_vertex->data << "\\n" << *boundary_left->data << "," << *boundary_right->data;;
}


std::shared_ptr<RakeCluster> RakeCluster::construct(std::shared_ptr<TopCluster> rake_from, std::shared_ptr<TopCluster> rake_to, bool virtual_cluster) {
	auto cluster = std::make_shared<RakeCluster>();

	// Basic connections (needed by do_join):
	if (virtual_cluster) {
		// If virtual rake node (in extended clusters model) only set oneside links
		cluster->left_child = rake_from;
		cluster->right_child = rake_to;
	} else {
		cluster->set_left_child(rake_from);
		cluster->set_right_child(rake_to);
	}

	cluster->do_join();

	return cluster;
}
void RakeCluster::do_join() {
	if (!is_splitted || is_deleted) return;

	// nicknames
	auto rake_from = left_child;
	auto rake_to = right_child;

	// 1. ensure that childs are fine (joined):
	rake_from->do_join();
	rake_to->do_join();

	// 2. Set boundary vertices:
	correct_endpoints();

	// 3. Call user defined method:
	Join(rake_from, rake_to, shared_from_this());

	is_splitted = false;
}
void RakeCluster::do_split(std::vector<std::shared_ptr<TopCluster>>* splitted_clusters) {
	if (is_splitted) return;

	// 1. Log that this cluster will be splitted
	if (splitted_clusters != NULL) splitted_clusters->push_back(shared_from_this());

	// 2. Ensure that parent is splitted:
	if (parent != NULL) parent->do_split(splitted_clusters);

	// 3. Call user defined method:
	Split(left_child, right_child, shared_from_this());

	is_splitted = true;
}
void RakeCluster::correct_endpoints() {
	// nicknames
	auto rake_from = left_child;
	auto rake_to = right_child;

	boundary_left = rake_to->boundary_left;
	boundary_right = rake_to->boundary_right;
}
void RakeCluster::unregister() {
	is_deleted = true;
}

void RakeCluster::unlink() {
	if (!is_deleted) unregister();

	TopCluster::unlink();
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
bool RakeCluster::is_handle_for(std::shared_ptr<BaseTree::Internal::Vertex> v) {
	return false;
}
std::ostream& RakeCluster::ToString(std::ostream& o) const {
	return o << "RakeCluster - endpoints " << *boundary_left->data << ", " << *boundary_right->data;
}
std::ostream& RakeCluster::_short_name(std::ostream& o) const {
	return o << *boundary_left->data << "," << *boundary_right->data;
}

}
