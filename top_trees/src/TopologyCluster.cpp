#include "UserFunctions.hpp"
#include "TopologyCluster.hpp"

//#define DEBUG

namespace TopTree {

int TopologyCluster::global_index = 0;

TopologyCluster::TopologyCluster() {
	index = global_index++;
}

std::ostream& TopologyCluster::ToString(std::ostream& o) const {
	o << index;
	if (edge != NULL) o << *edge->data;
	else if (vertex != NULL) o << *vertex->data;
	return o;
}
std::ostream& operator<<(std::ostream& o, const TopologyCluster& c) { return c.ToString(o); }

void TopologyCluster::set_first_child(std::shared_ptr<TopologyCluster> child) {
	first = child;
	if (child != NULL) child->parent = shared_from_this();
}
void TopologyCluster::set_second_child(std::shared_ptr<TopologyCluster> child) {
	second = child;
	if (child != NULL) child->parent = shared_from_this();
}

void TopologyCluster::do_join() {
	if (!is_splitted) return;

	if (vertex != NULL) return; // it is the basic cluster at vertex level

	// 1. Ensure that childs are joined
	if (first != NULL) first->do_join();
	if (second != NULL) second->do_join();

	// 2. Calculate outer edges
	calculate_outer_edges();

	// 3. Series of Joins itself
	if (edge != NULL && !edge->subvertice_edge) {
		// Firstly join one of nodes with edge
		if (first != NULL && first->cluster_data != NULL) Join(first->cluster_data, edge_cluster_data, combined_edge_cluster_data);
		else combined_edge_cluster_data = edge_cluster_data;

		if (second != NULL && second->cluster_data != NULL) Join(combined_edge_cluster_data, second->cluster_data, cluster_data);
		else cluster_data = edge_cluster_data;
	} else if (edge != NULL) {
		Join(first->cluster_data, second->cluster_data, cluster_data);
	} else cluster_data = (first != NULL) ? first->cluster_data : second->cluster_data;

	is_splitted = false;
}

void TopologyCluster::remove_all_outer_edges() {
	for (auto o: outer_edges) {
		// Remove outer edge from neighbour
		// bool removed = false;
		for (uint i = 0; i < o.cluster->outer_edges.size(); i++) {
			if (o.cluster->outer_edges[i].edge == o.edge && o.cluster->outer_edges[i].cluster == shared_from_this()) {
				// std::cerr << "Removed edge to " << *o.cluster << " " << *o.cluster->outer_edges[i].cluster << "(" << *o.edge->data << ")" << std::endl;
				o.cluster->outer_edges.erase(o.cluster->outer_edges.begin() + i);
				// removed = true;
				break;
			}
		}
		// if (!removed) std::cerr << "ERROR: Cannot remove edge " << *o.edge->data << " from second vertex" << std::endl;
	}
	outer_edges.clear();
}

void TopologyCluster::calculate_outer_edges(bool check_neighbours) {
	outer_edges.clear();
	if (first == NULL && second == NULL) {
		#ifdef DEBUG
			std::cerr << "Calculating edges for basic cluster of " << *vertex->data << " with " << vertex->neighbours.size() << " neighbours" << std::endl;
		#endif
		// Update outer edges according to underlying vertex
		for (auto n: vertex->neighbours) {
			auto ee = n.edge.lock();
			auto vv = (ee->from == vertex ? ee->to : ee->from);
			#ifdef DEBUG
				std::cerr << "... " << *vv->data << " (superior vertex " << *n.vertex.lock()->data << ") with " << *ee->data << std::endl;
			#endif
			outer_edges.push_back(neighbour{ee, vv->topology_cluster});
			if (vv->topology_cluster == NULL) {
				std::cerr << "ERROR: Cannot get topology cluster for neighbour " << *vv->data << std::endl;
				exit(1);
			}
		}
	} else if (second == NULL) {
		for (auto o: first->outer_edges) outer_edges.push_back(neighbour{o.edge, o.cluster->parent});
	} else {
		// Take only unique edges from both children
		// std::cerr << "First " << *first << " edges:" << std::endl;
		for (auto o: first->outer_edges) {
			// std::cerr << *o.cluster << "(" << *o.edge->data << ")" << std::endl;
			bool unique = true;
			for (auto oo: second->outer_edges) if (o.edge == oo.edge) {
				// edge = o.edge; // not neede because the second for does it
				unique = false;
			}
			if (unique) outer_edges.push_back(neighbour{o.edge, o.cluster->parent});
		}
		// std::cerr << "Second " << *second << " edges:" << std::endl;
		for (auto o: second->outer_edges) {
			// std::cerr << *o.cluster << "(" << *o.edge->data << ")" << std::endl;
			bool unique = true;
			for (auto oo: first->outer_edges) if (o.edge == oo.edge) {
				edge = o.edge;
				unique = false;
			}
			if (unique) outer_edges.push_back(neighbour{o.edge, o.cluster->parent});
		}
	}

	if (check_neighbours) for (auto o: outer_edges) {
		#ifdef DEBUG
			std::cerr << "Checking edge " << *o.edge->data << " to cluster " << *o.cluster << std::endl;
		#endif
		bool found = false;
		for (auto &oo: o.cluster->outer_edges) {
			if (oo.edge == o.edge) {
				if (oo.cluster != shared_from_this()) oo.cluster = shared_from_this();
				// std::cerr << "found " << *oo.cluster << "(" << *oo.edge->data << ")" << std::endl;
				found = true;
				break;
			}
		}
		if (!found) o.cluster->outer_edges.push_back(neighbour{o.edge, shared_from_this()});
	}
}

void TopologyCluster::do_split(std::vector<std::shared_ptr<TopologyCluster>>* splitted_clusters) {
	if (is_splitted) return;

	if (vertex != NULL) return; // it is the basic cluster at vertex level

	// 1. Log that this cluster will be splitted
	if (splitted_clusters != NULL) splitted_clusters->push_back(shared_from_this());

	// 2. Ensure that parent is splitted:
	if (parent != NULL) parent->do_split(splitted_clusters);

	// 3. Series of Splits itself
	if (edge != NULL && !edge->subvertice_edge) {
		if (second != NULL && second->cluster_data != NULL) Split(combined_edge_cluster_data, second->cluster_data, cluster_data);
		else combined_edge_cluster_data = cluster_data;

		if (first != NULL && first->cluster_data != NULL) Split(first->cluster_data, edge_cluster_data, combined_edge_cluster_data);
		else edge_cluster_data = combined_edge_cluster_data;
	} else if (edge != NULL) {
		Split(first->cluster_data, second->cluster_data, cluster_data);
	} else {
		if (first != NULL) first->cluster_data = cluster_data;
		else second->cluster_data = cluster_data;
	}

	is_splitted = true;
}

}
