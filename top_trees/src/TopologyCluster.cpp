#include "UserFunctions.hpp"
#include "TopologyCluster.hpp"
#include "TopCluster.hpp"

//#define DEBUG

namespace TopTree {

////////////////////////////////////////////////////////////////////////////////
/// SimpleCluster

void SimpleCluster::do_split() {
	if (was_splitted) return;

	if (parent != NULL) parent->do_split();
	if (edge != NULL) Destroy(shared_from_this(), edge->data);
	else Split(first, second, shared_from_this());

	was_splitted = true;
}

////////////////////////////////////////////////////////////////////////////////
/// TopologyCluster

std::shared_ptr<BaseTree::Internal::Vertex> TopologyCluster::get_common_vertex(std::shared_ptr<ICluster> cluster_a, std::shared_ptr<ICluster> cluster_b, bool get_superior) {
	auto common_vertex = cluster_a->boundary_left;

	if (get_superior && common_vertex->superior_vertex != NULL) common_vertex = common_vertex->superior_vertex;

	if (common_vertex != cluster_b->boundary_left && common_vertex != cluster_b->boundary_right
	   && common_vertex != cluster_b->boundary_left->superior_vertex && common_vertex != cluster_b->boundary_right->superior_vertex) common_vertex = cluster_a->boundary_right;

	if (get_superior && common_vertex->superior_vertex != NULL) common_vertex = common_vertex->superior_vertex;

	return common_vertex;
}

int TopologyCluster::global_index = 0;

TopologyCluster::TopologyCluster() {
	index = global_index++;
}

std::ostream& TopologyCluster::ToString(std::ostream& o) const {
	o << index;
	if (edge != NULL) o << *edge->data;
	else if (vertex != NULL) o << *vertex->data;
	if (boundary_left != NULL) o << " (" << *boundary_left->data << "," << *boundary_right->data << ")";
	if (is_rake_branch) o << "R";
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
	if (is_deleted) return;

	#ifdef DEBUG
		std::cerr << "Joining " << *shared_from_this() << std::endl;
	#endif

	is_top_cluster = false; // assume that is not a top cluster (until we notice it bellow)

	if (first == NULL && second == NULL) {
		is_splitted = false;
		#ifdef DEBUG
			std::cerr << "... joined (vertex cluster without edge)" << std::endl;
		#endif
		return; // it is the basic cluster at vertex level
	}

	// 1. Ensure that childs are joined
	if (first != NULL) first->do_join();
	if (second != NULL) second->do_join();

	// 2. Calculate outer edges
	calculate_outer_edges();

	// 3. Join itself:
	if (second == NULL) {
		// Just copy cluster below us, no Join needed
		data = first->data;
		boundary_left = first->boundary_left;
		boundary_right = first->boundary_right;
		is_top_cluster = first->is_top_cluster;
		is_rake_branch = first->is_rake_branch;
		// Ensure that there is no edge cluster (possibly if this cluster lost one of its childs)
		edge = NULL;
		edge_cluster = NULL;
	} else {
		if (edge == NULL) {
			std::cerr << "ERROR: Cluster '" << this << "' with both children but without edge between!" << std::endl;
			exit(1);
		}
		#ifdef DEBUG
			if (edge->subvertice_edge)  std::cerr << "... subvertice edge, joining clusters (without this edge)" << std::endl;
			else std::cerr << "... normal edge, joining around it" << std::endl;
		#endif
		// Normal edge - we Join everything with the edge
		is_top_cluster = !edge->subvertice_edge || first->is_top_cluster || second->is_top_cluster; // if edge or at least one child is top cluster -> this is top cluster too

		// 1. Create base cluster for edge
		edge_cluster = std::make_shared<SimpleCluster>();
		edge_cluster->boundary_left = edge->from;
		edge_cluster->boundary_right = edge->to;
		if (!edge->subvertice_edge) Create(edge_cluster, edge->data);

		// 2. Join with the first (if there is something to Join)
		if (first->is_top_cluster) {
			#ifdef DEBUG
				std::cerr << "... joining " << *first << " with edge with endpoints " << *edge_cluster->boundary_left->data << "," << *edge_cluster->boundary_right->data << std::endl;
			#endif
			combined_edge_cluster = std::make_shared<SimpleCluster>();
			if (first->is_rake_branch) {
				combined_edge_cluster->boundary_left = edge_cluster->boundary_left;
				combined_edge_cluster->boundary_right = edge_cluster->boundary_right;
			} else {
				// It is compress
				// i) get common vertex
				auto common_vertex = first->boundary_left;
				if (common_vertex->superior_vertex != NULL) common_vertex = common_vertex->superior_vertex;
				if (common_vertex != edge_cluster->boundary_left && common_vertex != edge_cluster->boundary_right
				   && common_vertex != edge_cluster->boundary_left->superior_vertex && common_vertex != edge_cluster->boundary_right->superior_vertex
				) common_vertex = first->boundary_right;
				if (common_vertex->superior_vertex != NULL) common_vertex = common_vertex->superior_vertex;

				// ii) Get boundary vertices
				combined_edge_cluster->boundary_left = (common_vertex == first->boundary_left || common_vertex == first->boundary_left->superior_vertex ? first->boundary_right : first->boundary_left);
				combined_edge_cluster->boundary_right = (common_vertex == edge_cluster->boundary_left || common_vertex == edge_cluster->boundary_left->superior_vertex ? edge_cluster->boundary_right : edge_cluster->boundary_left);
			}
			if (!edge->subvertice_edge) Join(first, edge_cluster, combined_edge_cluster);
			else combined_edge_cluster->data = first->data;
			#ifdef DEBUG
				std::cerr << "... combined edge have endpoints " << *combined_edge_cluster->boundary_left->data << "," << *combined_edge_cluster->boundary_right->data << std::endl;
			#endif
		} else combined_edge_cluster = edge_cluster;

		// 3. Join with the second (if there is something to Join)
		if (second->is_top_cluster) {
			#ifdef DEBUG
				std::cerr << "... joining " << *second << " with combined edge" << std::endl;
			#endif
			if (second->is_rake_branch) {
				boundary_left = combined_edge_cluster->boundary_left;
				boundary_right = combined_edge_cluster->boundary_right;
			} else {
				// It is compress
				// i) get common vertex
				auto common_vertex = second->boundary_left;
				//if (common_vertex->superior_vertex != NULL) common_vertex = common_vertex->superior_vertex;
				if (common_vertex != combined_edge_cluster->boundary_left && common_vertex != combined_edge_cluster->boundary_right) common_vertex = second->boundary_right;
				//if (common_vertex->superior_vertex != NULL) common_vertex = common_vertex->superior_vertex;

				// ii) Get boundary vertices
				boundary_left = (common_vertex == second->boundary_left ? second->boundary_right : second->boundary_left);
				boundary_right = (common_vertex == combined_edge_cluster->boundary_left ? combined_edge_cluster->boundary_right : combined_edge_cluster->boundary_left);
			}
			// Join if there is something in the combined edge cluster - either when there is normal edge or there was something in the first cluster
			if (!edge->subvertice_edge || first->is_top_cluster) Join(second, combined_edge_cluster, shared_from_this());
			else data = second->data;
		} else {
			// Copy from combined egde cluster into this cluster
			data = combined_edge_cluster->data;
			boundary_left = combined_edge_cluster->boundary_left;
			boundary_right = combined_edge_cluster->boundary_right;
		}
		//}
	}

	#ifdef DEBUG
		std::cerr << "... joined " << *shared_from_this() << std::endl;
	#endif

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
		boundary_left = first->boundary_left;
		boundary_right = first->boundary_right;
	} else {
		// Take only unique edges from both children
		// std::cerr << "First " << *first << " edges:" << std::endl;
		int first_counter = 0;
		for (auto o: first->outer_edges) {
			// std::cerr << *o.cluster << "(" << *o.edge->data << ")" << std::endl;
			bool unique = true;
			for (auto oo: second->outer_edges) if (o.edge == oo.edge) {
				// edge = o.edge; // not neede because the second for does it
				unique = false;
			}
			if (unique) {
				outer_edges.push_back(neighbour{o.edge, o.cluster->parent});
				first_counter++;
			}
		}
		// std::cerr << "Second " << *second << " edges:" << std::endl;
		int second_counter = 0;
		for (auto o: second->outer_edges) {
			// std::cerr << *o.cluster << "(" << *o.edge->data << ")" << std::endl;
			bool unique = true;
			for (auto oo: first->outer_edges) if (o.edge == oo.edge) {
				edge = o.edge;
				unique = false;
			}
			if (unique) {
				outer_edges.push_back(neighbour{o.edge, o.cluster->parent});
				second_counter++;
			}
		}

		if (first_counter == 2 && second_counter == 0) is_rake_branch = true;
		else if (first_counter == 0 && second_counter == 2) is_rake_branch = true;
		else is_rake_branch = false;
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
	#ifdef DEBUG
		std::cerr << "Splitting " << *shared_from_this() << std::endl;
	#endif

	// 1. Log that this cluster will be splitted
	if (splitted_clusters != NULL) splitted_clusters->push_back(shared_from_this());

	// 2. Ensure that parent is splitted:
	if (parent != NULL) parent->do_split(splitted_clusters);

	if (first == NULL && second == NULL) {
		is_splitted = true;
		return; // it is the basic cluster at vertex level
	}

	// 3. Series of Splits itself
	if (second == NULL) {
		// Just copy data down
		first->data = data;
	} else {
		if (edge == NULL) {
			std::cerr << "ERROR: Cluster '" << this << "' with both children but without edge between!" << std::endl;
			exit(1);
		}
		if (edge->subvertice_edge) {
			// They are joined as rake clusters, first was raked to the second one
			if (first->is_top_cluster && second->is_top_cluster) {
				// Rake Split:
				Split(first, second, shared_from_this());
			} else if (first->is_top_cluster) {
				// Just copy data down
				first->data = data;
			} else if (second->is_top_cluster) {
				// Just copy data down
				second->data = data;
			}
		} else {
			// 1. Split with the second
			if (second->is_top_cluster) {
				Split(second, combined_edge_cluster, shared_from_this());
			} else combined_edge_cluster->data = data;

			// 2. Split with the first
			if (first->is_top_cluster) {
				Split(first, edge_cluster, combined_edge_cluster);
			} else combined_edge_cluster->data = data;

			// 3. Destroy edge cluster
			Destroy(edge_cluster, edge->data);
		}
	}

	#ifdef DEBUG
		std::cerr << "... splitted " << *shared_from_this() << std::endl;
	#endif

	is_splitted = true;
}

bool TopologyCluster::is_external_boundary_vertex(std::shared_ptr<BaseTree::Internal::Vertex> v) {
	if (boundary_left != v && boundary_right != v) return false; // v isn't either boundary vertex, it cannot be external boundary vertex

	for (auto n: outer_edges) {
		if (n.edge->from == v || n.edge->to == v) return true;
	}

	return false;
}

}
