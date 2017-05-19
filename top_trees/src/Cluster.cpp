#include "Cluster.hpp"

namespace TopTree {

BaseCluster::BaseCluster(std::shared_ptr<BaseTree::Internal::Edge> edge) {
	boundary_a = edge->from;
	boundary_b = edge->to;

	Create(data, edge->data);

	std::cout << "BaseCluster from " << *boundary_a->data << " to " << *boundary_b->data << std::endl;
}

CompressCluster::CompressCluster(std::shared_ptr<Cluster> left, std::shared_ptr<Cluster> right) {
	boundary_a = (left->boundary_a == right->boundary_a || left->boundary_a == right->boundary_b
		? left->boundary_b // because boundary_a is bounded to the second one
		: left->boundary_a // boundary_a is free
	);
	boundary_b = (right->boundary_a == left->boundary_a || right->boundary_a == left->boundary_b
		? right->boundary_b // because boundary_a is bounded to the second one
		: right->boundary_a // boundary_a is free
	);

	Join(left->data, right->data, data);

	std::cout << "Compress from " << *boundary_a->data << " to " << *boundary_b->data << std::endl;
}

RakeCluster::RakeCluster(std::shared_ptr<Cluster> rake_from, std::shared_ptr<Cluster> rake_to) {
	boundary_a = rake_to->boundary_a;
	boundary_b = rake_to->boundary_b;

	Join(rake_from->data, rake_to->data, data);

	std::cout << "Rake from " << *boundary_a->data << " to " << *boundary_b->data << std::endl;
}

}
