#include "Cluster.hpp"

namespace TopTree {

std::ostream& operator<<(std::ostream& o, const Cluster& c) { return c.ToString(o); }

BaseCluster::BaseCluster(std::shared_ptr<BaseTree::Internal::Edge> edge) {
	boundary_a = edge->from;
	boundary_b = edge->to;

	Create(data, edge->data);
}
std::ostream& BaseCluster::ToString(std::ostream& o) const {
	return o << "BaseCluster - endpoints " << *boundary_a->data << ", " << *boundary_b->data;
}

CompressCluster::CompressCluster(std::shared_ptr<Cluster> left, std::shared_ptr<Cluster> right) {
	if (left->boundary_a == right->boundary_a || left->boundary_a == right->boundary_b)
		common_vertex = left->boundary_a;
	else common_vertex = left->boundary_b;

	boundary_a = (left->boundary_a == common_vertex) ? left->boundary_b : left->boundary_a;
	boundary_b = (right->boundary_a == common_vertex) ? right->boundary_b : right->boundary_a;

	left_foster = common_vertex->rake_tree_left;
	right_foster = common_vertex->rake_tree_right;

	left_child = left;
	right_child = right;
	Join(left->data, right->data, data);
}
std::ostream& CompressCluster::ToString(std::ostream& o) const {
	return o << "CompressCluster - endpoints " << *boundary_a->data << " [" << *common_vertex->data << "] " << *boundary_b->data;
}

RakeCluster::RakeCluster(std::shared_ptr<Cluster> rake_from, std::shared_ptr<Cluster> rake_to) {
	boundary_a = rake_to->boundary_a;
	boundary_b = rake_to->boundary_b;

	left_child = rake_from;
	right_child = rake_to;
	Join(rake_from->data, rake_to->data, data);
}
std::ostream& RakeCluster::ToString(std::ostream& o) const {
	return o << "RakeCluster - endpoints " << *boundary_a->data << ", " << *boundary_b->data;
}

}
