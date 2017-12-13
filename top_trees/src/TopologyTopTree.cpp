#include <queue>
#include <vector>
#include <sstream>

#include "TopologyTopTree.hpp"
#include "BaseTreeInternal.hpp"
#include "TopologyCluster.hpp"

//#define DEBUG

namespace TopTree {

// Hide data from .hpp file using PIMP idiom
class TopologyTopTree::Internal {
public:
	std::vector<std::shared_ptr<TopologyCluster> > root_clusters;
	std::shared_ptr<BaseTree> base_tree;

	std::shared_ptr<TopologyCluster> construct_basic_clusters(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> parent_edge=NULL);
	std::shared_ptr<TopologyCluster> construct_topology_tree(std::shared_ptr<TopologyCluster> cluster, int level = 0, std::shared_ptr<BaseTree::Internal::Edge> parent_edge = NULL);
	std::shared_ptr<BaseTree::Internal::Vertex> split_vertex(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> parent_edge = NULL);
	void repair_subvertex_after_cut(std::shared_ptr<BaseTree::Internal::Vertex> v);

	std::tuple<std::shared_ptr<TopologyCluster>, std::shared_ptr<TopologyCluster>, std::shared_ptr<BaseTree::Internal::Edge>> cut(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w);
	std::shared_ptr<TopologyCluster> link(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w, std::shared_ptr<BaseTree::Internal::Edge> edge);

	//void soft_expose(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w);
	//std::shared_ptr<Cluster> hard_expose(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w);
	//void restore_hard_expose();
	//void guarded_splay(std::shared_ptr<Cluster> node, std::shared_ptr<Cluster> guard = NULL);

	std::vector<std::shared_ptr<TopologyCluster>> splitted_clusters;
	std::vector<std::shared_ptr<TopologyCluster>> to_calculate_outer_edges;

	// Debug methods:
	void print_graphviz(std::shared_ptr<TopologyCluster> node, const std::string title="") const;
	void print_graphviz_recursive(std::shared_ptr<TopologyCluster> cluster, std::shared_ptr<BaseTree::Internal::Edge> parent_edge = NULL, std::shared_ptr<TopologyCluster> parent = NULL) const;
private:
	// Used in update_clusters() and helper methods
	// it is declared globally to easier sharing between helper methods
	std::vector<std::shared_ptr<TopologyCluster>> delete_list;
	std::vector<std::shared_ptr<TopologyCluster>> change_list;
	std::vector<std::shared_ptr<TopologyCluster>> abandon_list;

	std::vector<std::shared_ptr<TopologyCluster>> next_delete;
	std::vector<std::shared_ptr<TopologyCluster>> next_change;
	std::vector<std::shared_ptr<TopologyCluster>> next_abandon;

	std::vector<std::shared_ptr<TopologyCluster>> found_roots;

	void update_clusters();
	void update_clusters_join_with_neighbour(std::shared_ptr<TopologyCluster> cluster, std::shared_ptr<TopologyCluster> neighbour);
	void update_clusters_only_child(std::shared_ptr<TopologyCluster> cluster);
};

////////////////////////////////////////////////////////////////////////////////

TopologyTopTree::TopologyTopTree() : internal{std::make_unique<Internal>()} {}

TopologyTopTree::TopologyTopTree(std::shared_ptr<BaseTree> from_base_tree) : TopologyTopTree() {
	internal->base_tree = from_base_tree;

	for (auto v : internal->base_tree->internal->vertices) v->used = false;

	int i = 0;
	for (auto v : internal->base_tree->internal->vertices) {
		if (v->used || v->degree != 1) continue;

		#ifdef DEBUG
			std::cerr << "Constructing basic clusters from vertex " << *v->data << std::endl;
		#endif
		auto root_cluster = internal->construct_basic_clusters(v);
		internal->print_graphviz(root_cluster, "Basic clusters");
		while (root_cluster->outer_edges.size() > 0) {
			#ifdef DEBUG
				//std::cerr << "Actual size of the root cluster: " << root_cluster->outer_edges.size() << std::endl;
			#endif
			root_cluster = internal->construct_topology_tree(root_cluster);
			for (auto c: internal->to_calculate_outer_edges) c->calculate_outer_edges();
			internal->to_calculate_outer_edges.clear();

			internal->print_graphviz(root_cluster, "One level up");
		}
		internal->root_clusters.push_back(root_cluster);
		internal->root_clusters[i]->root_vector_index = i;
		i++;
	}
}

//std::vector<std::shared_ptr<Cluster> > TopologyTopTree::GetTopTrees() {
//	return internal->root_clusters;
//}


////////////////////////////////////////////////////////////////////////////////
// Debug output - Graphviz

void TopologyTopTree::Internal::print_graphviz_recursive(std::shared_ptr<TopologyCluster> cluster, std::shared_ptr<BaseTree::Internal::Edge> parent_edge, std::shared_ptr<TopologyCluster> parent) const {
	if (cluster == NULL) return;

	auto shape = (cluster->first == NULL ? "triangle" : (cluster->second == NULL ? "circle" : "Msquare"));

	std::cout << "\t\"" << cluster << "\" [label=\"";
	if (cluster->edge != NULL) std::cout << *cluster->edge->data;
	else if (cluster->vertex != NULL) std::cout << *cluster->vertex->data;
	std::cout << "\", shape=" << shape << "]" << std::endl;
	if (parent_edge != NULL) {
		std::cout << "\t\"" << parent << "\" -> \"" << cluster << "\" [label=\"" << *parent_edge->data << "\"";
		if (parent_edge->subvertice_edge) std::cout << ", style=dashed";
		std::cout << "]" << std::endl;
	}
	for (auto o: cluster->outer_edges) {
		if (o.edge != parent_edge) print_graphviz_recursive(o.cluster, o.edge, cluster);
	}
}

void TopologyTopTree::Internal::print_graphviz(const std::shared_ptr<TopologyCluster> root, const std::string title) const {
	std::cout << "digraph \"" << root << "\" {" << std::endl;
	std::cout << "\tlabelloc=\"t\"" << std::endl << "\tlabel=\"" << title << "\"" << std::endl;
	print_graphviz_recursive(root);
	std::cout << "}" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Update procedure:

void TopologyTopTree::Internal::update_clusters_join_with_neighbour(std::shared_ptr<TopologyCluster> cluster, std::shared_ptr<TopologyCluster> neighbour) {
	neighbour->listed_in_abandon_list = false;
	if (cluster->parent == NULL && neighbour->parent == NULL) {
		// Add new cluster to above level
		auto parent = std::make_shared<TopologyCluster>();
		parent->set_first_child(cluster);
		parent->set_second_child(neighbour);
		to_calculate_outer_edges.push_back(parent);
		// Add new cluster to next abandon list
		next_abandon.push_back(parent);
		parent->listed_in_abandon_list = true;
	} else {
		if (cluster->parent == NULL) {
			// Add cluster into neighbour's parent
			if (neighbour->parent->second != NULL) std::cerr << "ERROR: Expecting that neighbour's parent would have only one child, but it have both children" << std::endl;
			neighbour->parent->set_second_child(cluster);
		} else if (neighbour->parent == NULL) {
			// Add neighbour into cluster's parent
			if (cluster->parent->second != NULL) std::cerr << "ERROR: Expecting that cluster's parent would have only one child, but it have both children" << std::endl;
			cluster->parent->set_second_child(neighbour);
		} else {
			// Both have parents, use cluster's parent and delete neighbours parent
			neighbour->parent->first = NULL;
			next_delete.push_back(neighbour->parent);
			neighbour->parent->listed_in_delete_list = true;
			neighbour->parent = cluster->parent;
			cluster->parent->second = neighbour;
		}
		to_calculate_outer_edges.push_back(cluster->parent);
		next_change.push_back(cluster->parent);
		cluster->parent->listed_in_change_list = true;
	}
}

void TopologyTopTree::Internal::update_clusters_only_child(std::shared_ptr<TopologyCluster> cluster) {
	// This cluster is the only one child of its parent, ensure that parent exists
	if (cluster->parent == NULL) {
		// Have to create new parent
		auto parent = std::make_shared<TopologyCluster>();
		parent->set_first_child(cluster);
		to_calculate_outer_edges.push_back(parent);
		next_abandon.push_back(parent);
		parent->listed_in_abandon_list = true;
	} else {
		to_calculate_outer_edges.push_back(cluster->parent);
		next_change.push_back(cluster->parent);
		cluster->parent->listed_in_change_list = true;
	}
}

void TopologyTopTree::Internal::update_clusters() {
	// When all lists empty -> end procedure
	if (delete_list.size() == 0 && abandon_list.size() == 0 && change_list.size() == 0) return;

	to_calculate_outer_edges.clear();

	next_delete.clear();
	next_change.clear();
	next_abandon.clear();

	// 1. Run through deleted vertices
	for (auto cluster: delete_list) {
		cluster->do_split(&splitted_clusters);
		if (cluster->parent != NULL) {
			if (cluster->parent->second == NULL) {
				// No second child:
				next_delete.push_back(cluster->parent);
				cluster->parent->listed_in_delete_list = true;
				// Remove child link
				cluster->parent->first = NULL;
			} else {
				// There is another child
				if (cluster == cluster->parent->first) {
					cluster->parent->first = cluster->parent->second;
					cluster->parent->second = NULL;
				} else cluster->parent->second = NULL;

				// Remove edge in the parent cluster
				cluster->parent->edge = NULL;

				// Test another child (which is now the first child of parent)
				if (!cluster->parent->first->listed_in_change_list && cluster->parent->first->listed_in_delete_list) {
					change_list.push_back(cluster->parent->first);
					cluster->parent->first->listed_in_change_list = true;
				}
			}
		}
		// Remove cluster from list and delete it
		cluster->parent = NULL;
		if (cluster->vertex != NULL) cluster->vertex->topology_cluster = NULL;
		cluster->remove_all_outer_edges();

		cluster->listed_in_delete_list = false;
	}

	// 2. Run through changed list that have sibling
	for (auto cluster: change_list) {
		cluster->do_split(&splitted_clusters);
		// Skip clusters removed from list and clusters with no parent or without sibling (parent have only one child)
		if (!cluster->listed_in_change_list || cluster->parent == NULL || cluster->parent->second == NULL) continue;
		auto sibling = (cluster->parent->first == cluster ? cluster->parent->second : cluster->parent->first);

		// If connected with sibling by edge and parent is valid cluster
		// Get common edge:
		std::shared_ptr<BaseTree::Internal::Edge> common_edge = NULL;
		for (auto o: cluster->outer_edges) if (o.cluster == sibling) common_edge = o.edge;
		// Test parent
		if (common_edge != NULL && cluster->parent->outer_edges.size() <= 2) {
			// Everything OK, remove cluster and sibling from change list and add parent into next change list
			cluster->listed_in_change_list = false;
			sibling->listed_in_change_list = false;
			next_change.push_back(cluster->parent);
			cluster->parent->listed_in_change_list = true;
		} else {
			// Parent goes into next delete list
			cluster->parent->first = NULL;
			cluster->parent->second = NULL;
			next_delete.push_back(cluster->parent);
			cluster->parent->listed_in_delete_list = true;

			// This cluster and sibling are now abandon (move to abandon list)
			cluster->parent = NULL;
			cluster->listed_in_change_list = false;
			abandon_list.push_back(cluster);
			cluster->listed_in_abandon_list = true;

			sibling->parent = NULL;
			sibling->listed_in_change_list = false;
			abandon_list.push_back(sibling);
			sibling->listed_in_abandon_list = true;
		}
	}

	// 3. Run through rest of changed list and whole abandon list
	for (auto cluster: change_list) {
		if (cluster->listed_in_change_list && !cluster->listed_in_abandon_list) {
			abandon_list.push_back(cluster);
			cluster->listed_in_abandon_list = true;
		}
		cluster->listed_in_change_list = false;
	}
	for (auto cluster: abandon_list) {
		cluster->do_split(&splitted_clusters);
		if (!cluster->listed_in_abandon_list) continue;
		if (cluster->outer_edges.size() == 3) {
			// Find if there is neighbour with degree 1
			std::shared_ptr<TopologyCluster> neighbour = NULL;
			for (auto o: cluster->outer_edges) if (o.cluster->outer_edges.size() == 1) neighbour = o.cluster;

			if (neighbour != NULL) update_clusters_join_with_neighbour(cluster, neighbour); // Join with neighbour
			else update_clusters_only_child(cluster);
		} else if (cluster->outer_edges.size() >= 1) {
			// Find if there is neighbour with degree <= (4 - #outer_edges)
			std::shared_ptr<TopologyCluster> neighbour = NULL;
			for (auto o: cluster->outer_edges) if (o.cluster->outer_edges.size() <= (4 - cluster->outer_edges.size())) neighbour = o.cluster;

			if (neighbour != NULL && (neighbour->parent == NULL || neighbour->parent->second == NULL))
				update_clusters_join_with_neighbour(cluster, neighbour); // Join with neighbour as in the first case
			else update_clusters_only_child(cluster);
		} else {
			//0 outer edges -> it is root and nothing is needed
			found_roots.push_back(cluster);
		}
		cluster->listed_in_abandon_list = false;
	}

	// 4. Calculate outer edges for parent layer
	for (auto c: to_calculate_outer_edges) c->calculate_outer_edges(true); // with checking neighbours (neighbours may not be on this list and we need to add new edges into them)

	// Continue with above level
	delete_list = next_delete;
	change_list = next_change;
	abandon_list = next_abandon;
	return update_clusters();
}


////////////////////////////////////////////////////////////////////////////////
/// Cut and Link

void TopologyTopTree::Cut(int v_index, int w_index) {
	// Init array for clusters restoration
	internal->splitted_clusters.clear();

	// 0. Get vertices and ensure degree of w is always >= degree of v
	auto v = internal->base_tree->internal->vertices[v_index];
	auto w = internal->base_tree->internal->vertices[w_index];

	// 1. Find edge
	std::shared_ptr<BaseTree::Internal::Edge> edge = NULL;
	for (auto n: v->neighbours) if (n.vertex.lock() == w) { edge = n.edge.lock(); break; }
	if (edge == NULL) {
		std::cerr << "ERROR: Vertices not linked by edge, cannot cut" << std::endl;
		return;
	}

	// 2. Get vertices real connected with the edge (could be vertices itself or its subvertices)
	auto vv = edge->from;
	auto ww = edge->to;
	if (!vv->subvertices.empty()) vv = *((*edge->from_iter).subvertice_iter);
	if (!ww->subvertices.empty()) ww = *((*edge->to_iter).subvertice_iter);

	// 3. Cut itself (save results)
	internal->cut(vv, ww);

	// 4. If were subvertices it may be necessary to update subvertices
	if (vv->superior_vertex != NULL) {
		internal->repair_subvertex_after_cut(vv);
		for (auto n = vv->superior_vertex->neighbours.begin(); n != vv->superior_vertex->neighbours.end(); ++n) {
			if ((*n).edge.lock() == edge) {
				vv->superior_vertex->neighbours.erase(n);
				break;
			}
		}
	}
	if (ww->superior_vertex != NULL) {
		internal->repair_subvertex_after_cut(ww);
		for (auto n = ww->superior_vertex->neighbours.begin(); n != ww->superior_vertex->neighbours.end(); ++n) {
			if ((*n).edge.lock() == edge) {
				ww->superior_vertex->neighbours.erase(n);
				break;
			}
		}
	}

	// 5. Get roots - TODO: do we want to return them, is that necessary?
}

std::tuple<std::shared_ptr<TopologyCluster>, std::shared_ptr<TopologyCluster>, std::shared_ptr<BaseTree::Internal::Edge>> TopologyTopTree::Internal::cut(
	std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w
) {
	auto cluster_v = v->topology_cluster;
	auto cluster_w = w->topology_cluster;

	// 0. Split from both clusters
	cluster_v->do_split(&splitted_clusters);
	cluster_w->do_split(&splitted_clusters);

	// 1. First common edge
	std::shared_ptr<BaseTree::Internal::Edge> edge = NULL;
	for (uint i = 0; i < cluster_v->outer_edges.size(); i++) {
		if (cluster_v->outer_edges[i].cluster == cluster_w) {
			edge = cluster_v->outer_edges[i].edge;
			cluster_v->outer_edges.erase(cluster_v->outer_edges.begin() + i);
			break;
		}
	}
	for (uint i = 0; i < cluster_w->outer_edges.size(); i++) {
		if (cluster_w->outer_edges[i].cluster == cluster_v) {
			cluster_w->outer_edges.erase(cluster_w->outer_edges.begin() + i);
			break;
		}
	}
	// 1.1 Update degrees
	v->degree--;
	w->degree--;

	// 2. Add both clusters into changed list (and empty other lists)
	delete_list.clear();
	abandon_list.clear();
	change_list.clear();
	change_list.push_back(cluster_v);
	cluster_v->listed_in_change_list = true;
	change_list.push_back(cluster_w);
	cluster_w->listed_in_change_list = true;

	// 3. Run update procedure
	found_roots.clear();
	update_clusters();


	// 4. Get results
	if (found_roots.size() != 2) {
		std::cerr << "ERROR: Expecting 2 roots after cut operation, found " << found_roots.size() << " roots!" << std::endl;
	}
	return std::make_tuple(found_roots[0], found_roots[1], edge);
	// expecting that do_join will be called from outside Cut function
}

void TopologyTopTree::Link(int v_index, int w_index, std::shared_ptr<EdgeData> edge_data) {
	// Init array for clusters restoration
	internal->splitted_clusters.clear();

	// 0. Get vertices and ensure degree of w is always >= degree of v
	auto v = internal->base_tree->internal->vertices[v_index];
	auto w = internal->base_tree->internal->vertices[w_index];

	// TODO: Link in way of subvertices
}

std::shared_ptr<TopologyCluster> TopologyTopTree::Internal::link(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w, std::shared_ptr<BaseTree::Internal::Edge> edge) {
	auto cluster_v = v->topology_cluster;
	auto cluster_w = w->topology_cluster;

	// 0. Split from both clusters
	cluster_v->do_split(&splitted_clusters);
	cluster_w->do_split(&splitted_clusters);

	// 1. Add new edge (expecting that outer Link function ensures not adding edge to vertices with degree 3)
	cluster_v->outer_edges.push_back(TopologyCluster::neighbour{edge, cluster_w});
	cluster_w->outer_edges.push_back(TopologyCluster::neighbour{edge, cluster_v});
	// 1.1 Update degrees
	v->degree++;
	w->degree++;

	// 2. Add both clusters into changed list (and empty other lists)
	delete_list.clear();
	abandon_list.clear();
	change_list.clear();
	change_list.push_back(cluster_v);
	cluster_v->listed_in_change_list = true;
	change_list.push_back(cluster_w);
	cluster_w->listed_in_change_list = true;

	// 3. Run update procedure
	found_roots.clear();
	update_clusters();

	// 4. Get results
	if (found_roots.size() != 1) {
		std::cerr << "ERROR: Expecting 1 root after link operation, found " << found_roots.size() << " roots!" << std::endl;
	}
	return found_roots[0];
	// expecting that do_join will be called from outside Cut function
}

////////////////////////////////////////////////////////////////////////////////
/// Functions for construction:

void TopologyTopTree::Internal::repair_subvertex_after_cut(std::shared_ptr<BaseTree::Internal::Vertex> v) {
	std::shared_ptr<BaseTree::Internal::Vertex> first_neighbour = NULL;
	std::shared_ptr<BaseTree::Internal::Vertex> second_neighbour = NULL;

	for (auto n: v->neighbours) {
		if (n.edge.lock()->subvertice_edge) {
			if (first_neighbour == NULL) first_neighbour = n.vertex.lock();
			else second_neighbour = n.vertex.lock();
		}
	}

	if (second_neighbour == NULL) {
		// This vertex is one of end subvertices
		int subvertice_edges_counter = 0;
		for (auto n: first_neighbour->neighbours) if (n.edge.lock()->subvertice_edge) subvertice_edges_counter++;
		if (subvertice_edges_counter == 1) {
			// There is second endpoint (with 2 valid edges) -> we join them back into superior vertex
			for (auto n: v->neighbours) {
				if (auto vv = n.vertex.lock()) if (auto ee = n.edge.lock()) {
					cut(v, vv);
					if (!ee->subvertice_edge) link(v->superior_vertex, vv, ee); // is not subvertice edge and will be reused
					else v->superior_vertex->subvertice_edges.erase(ee->subvertice_edges_iterator); // erase unused subvertice edge
				}
			}
			for (auto n: first_neighbour->neighbours) {
				if (auto vv = n.vertex.lock()) if (auto ee = n.edge.lock()) {
					cut(first_neighbour, vv);
					if (!ee->subvertice_edge) link(v->superior_vertex, vv, ee); // is not subvertice edge and will be reused
					else v->superior_vertex->subvertice_edges.erase(ee->subvertice_edges_iterator); // erase unused subvertice edge
				}
			}
			return;
		} else {
			// We "steal" one edge from the neighbour and then we repair on this vertex
			for (auto n: first_neighbour->neighbours) {
				if (auto vv = n.vertex.lock()) if (auto ee = n.edge.lock()) {
					if (!ee->subvertice_edge) {
						cut(first_neighbour, vv);
						link(v, vv, ee);
						// edge is reused, nothing to do about edges
					}
				}
			}
			return repair_subvertex_after_cut(first_neighbour);
		}
	} else {
		// It is vertex inside "chain" -> remove it and join neighbours
		auto result = cut(first_neighbour, v);
		v->superior_vertex->subvertice_edges.erase(std::get<2>(result)->subvertice_edges_iterator); // erase first subvertice edge
		result = cut(second_neighbour, v);
		link(first_neighbour, second_neighbour, std::get<2>(result)); // reuse second subvertice edge
	}
}

////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<BaseTree::Internal::Vertex> TopologyTopTree::Internal::split_vertex(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> parent_edge) {
	// Create subvertices
	auto current = std::make_shared<BaseTree::Internal::Vertex>(std::make_shared<VertexData>());
	current->superior_vertex = v;
	v->subvertices.push_back(current);
	auto vertex_to_return = current; // by default we return the first vertex

	auto current_iter = std::prev(v->subvertices.end());
	for (auto& n: v->neighbours) {

		// Copy this edge into subvertice and notice what subvertice it is
		if (current->degree == 2 && &n != &v->neighbours.back()) {
			#ifdef DEBUG
				std::cerr << "Creating new subvertex for " << *v->data << std::endl;
			#endif
			auto temp = std::make_shared<BaseTree::Internal::Vertex>(std::make_shared<VertexData>());
			temp->superior_vertex = v;
			v->subvertices.push_back(temp);
			// Add edge between them
			auto edge = std::make_shared<BaseTree::Internal::Edge>(current, temp, std::make_shared<EdgeData>());
			edge->subvertice_edge = true;
			edge->register_at_vertices();
			v->subvertice_edges.push_back(edge);
			edge->subvertice_edges_iterator = std::prev(v->subvertice_edges.end());

			current = temp;

			current_iter = std::prev(v->subvertices.end());
		}
		if (n.edge.lock() == parent_edge) vertex_to_return = current; // This vertex got edge from parent so we will return this one

		// Add this edge to current subvertice
		current->neighbours.push_back(BaseTree::Internal::neighbour{n.vertex, n.edge});
		current->degree++;
		n.subvertice_iter = current_iter;
	}
	return vertex_to_return;
}


std::shared_ptr<TopologyCluster> TopologyTopTree::Internal::construct_basic_clusters(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> parent_edge) {
	if (v->degree > 3) return construct_basic_clusters(split_vertex(v, parent_edge), parent_edge);

	// Otherwise...
	// 1. Sanity check
	if (parent_edge == NULL and v->degree > 1) {
		std::cerr << "ERROR: Cluster without parent with degree > 1" << std::endl;
		return NULL;
	}

	// 2. Construct basic topology clusters from this vertex and connect with outgoing edges with others
	auto cluster = std::make_shared<TopologyCluster>();
	cluster->vertex = v;
	v->topology_cluster = cluster;
	v->used = true;
	for (auto n : v->neighbours) {
		if (auto vv = n.vertex.lock()) if (auto ee = n.edge.lock()) {
			if (ee == parent_edge) continue;
			if (vv->used) {
				std::cerr << "ERROR: Vertex " << *vv->data << " already used in Topology Tree, underlying tree isn't acyclic!" << std::endl;
				return NULL;
			}
			vv->used = true;
			auto child = construct_basic_clusters(vv, ee);
			cluster->outer_edges.push_back(TopologyCluster::neighbour{ee, child});
			cluster->outer_edges_count++;
			child->outer_edges.push_back(TopologyCluster::neighbour{ee, cluster});
			child->outer_edges_count++;
		}
	}

	// Now there is prepared topology tree with
	return cluster;
}

std::shared_ptr<TopologyCluster> TopologyTopTree::Internal::construct_topology_tree(std::shared_ptr<TopologyCluster> cluster, int level, std::shared_ptr<BaseTree::Internal::Edge> parent_edge) {
	std::shared_ptr<TopologyCluster> first = NULL;
	std::shared_ptr<TopologyCluster> second = NULL;

	#ifdef DEBUG
		if (cluster->vertex != NULL) {
			if (cluster->vertex->data == NULL) std::cerr << "At basic cluster [helper]" << std::endl;
			else std::cerr << "At basic cluster " << *cluster->vertex->data << std::endl;
		}
	#endif

	// 1. Construct clusters for both children
	for (auto o : cluster->outer_edges) {
		if (o.edge == parent_edge) continue;
		if (first == NULL) first = construct_topology_tree(o.cluster, level, o.edge);
		else second = construct_topology_tree(o.cluster, level, o.edge);
	}

	// 3. Check if this cluster could be added to one of the child clusters:
	if (first != NULL && second != NULL) {
		// Both children, we could add this cluster only to some with only one cluster and without other edges
		if (first->second == NULL && first->outer_edges_count == 1) {
			#ifdef DEBUG
				std::cerr << "A: Adding this cluster to the first one child" << std::endl;
			#endif
			first->second = cluster;
			cluster->parent = first;
			first->outer_edges_count += cluster->outer_edges_count - 2;
			// outer edges will be calculated after finishing making all clusters on this level of topology tree
			return first;
		} else if (second->second == NULL && second->outer_edges_count == 1) {
			#ifdef DEBUG
				std::cerr << "A: Adding this cluster to the second one child" << std::endl;
			#endif
			second->second = cluster;
			cluster->parent = second;
			second->outer_edges_count += cluster->outer_edges_count - 2;
			// outer edges will be calculated after finishing making all clusters on this level of topology tree
			return second;
		}
	} else if (first != NULL && first->second == NULL && first->outer_edges_count <= 2) {
		#ifdef DEBUG
			std::cerr << "B: Adding this cluster to the first one child" << std::endl;
		#endif
		first->second = cluster;
		cluster->parent = first;
		first->outer_edges_count += cluster->outer_edges_count - 2;
		return first;
	}

	// 4. Cannot add to any cluster, have to create own
	#ifdef DEBUG
		std::cerr << "Creating new cluster " << cluster->outer_edges_count << std::endl;
	#endif
	auto new_cluster = std::make_shared<TopologyCluster>();
	new_cluster->first = cluster;
	new_cluster->vertex = cluster->vertex;
	cluster->parent = new_cluster;
	new_cluster->outer_edges_count = cluster->outer_edges_count;
	to_calculate_outer_edges.push_back(new_cluster);

	return new_cluster;
}

TopologyTopTree::~TopologyTopTree() {}
}
