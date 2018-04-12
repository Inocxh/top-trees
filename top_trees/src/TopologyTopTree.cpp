#include <queue>
#include <vector>
#include <sstream>

#include "TopologyTopTree.hpp"
#include "BaseTreeInternal.hpp"
#include "TopologyCluster.hpp"

//#define DEBUG
//#define DEBUG_GRAPHVIZ
//#define DEBUG_GRAPHVIZ_VERBOSE

namespace TopTree {

// Hide data from .hpp file using PIMP idiom
class TopologyTopTree::Internal {
public:
	std::vector<std::shared_ptr<TopologyCluster> > root_clusters;
	std::shared_ptr<BaseTree> base_tree;

	std::shared_ptr<TopologyCluster> construct_basic_clusters(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> parent_edge=NULL);
	std::shared_ptr<TopologyCluster> construct_topology_tree(std::shared_ptr<TopologyCluster> cluster, int level = 0, std::shared_ptr<BaseTree::Internal::Edge> parent_edge = NULL);
	std::shared_ptr<BaseTree::Internal::Vertex> split_vertex(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> parent_edge = NULL);
	std::shared_ptr<BaseTree::Internal::Vertex> repair_subvertex_after_cut(std::shared_ptr<BaseTree::Internal::Vertex> v);
	std::shared_ptr<BaseTree::Internal::Vertex> get_vertex_to_link(std::shared_ptr<BaseTree::Internal::Vertex> v);

	std::tuple<std::shared_ptr<TopologyCluster>, std::shared_ptr<TopologyCluster>> cut(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w, std::shared_ptr<BaseTree::Internal::Edge> edge);
	std::shared_ptr<TopologyCluster> link(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w, std::shared_ptr<BaseTree::Internal::Edge> edge);

	std::list<std::shared_ptr<ICluster>> expose_get_clusters(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> second_v, bool continue_above_common);

	//void soft_expose(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w);
	//std::shared_ptr<Cluster> hard_expose(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w);
	//void restore_hard_expose();
	//void guarded_splay(std::shared_ptr<Cluster> node, std::shared_ptr<Cluster> guard = NULL);

	std::vector<std::shared_ptr<TopologyCluster>> splitted_clusters;
	std::vector<std::shared_ptr<TopologyCluster>> to_calculate_outer_edges;
	std::vector<std::shared_ptr<SimpleCluster>> expose_simple_clusters;

	#ifdef DEBUG_GRAPHVIZ
		void print_graphviz(std::shared_ptr<TopologyCluster> node, const std::string title="", bool full = false) const;
		void print_graphviz_recursive(std::shared_ptr<TopologyCluster> cluster, std::shared_ptr<BaseTree::Internal::Edge> parent_edge = NULL, std::shared_ptr<TopologyCluster> parent = NULL, bool edges_to_childs = false, bool gray = false) const;
	#endif
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

TopologyTopTree::TopologyTopTree(std::shared_ptr<BaseTree> baseTree) : TopologyTopTree() {
	InitFromBaseTree(baseTree);
}

void TopologyTopTree::InitFromBaseTree(std::shared_ptr<BaseTree> baseTree) {
	internal->base_tree = baseTree;

	for (auto v : internal->base_tree->internal->vertices) v->used = false;

	int i = 0;
	for (auto v : internal->base_tree->internal->vertices) {
		if (v->used || v->degree != 1) continue;

		#ifdef DEBUG
			std::cerr << "Constructing basic clusters from vertex " << *v->data << std::endl;
		#endif
		auto root_cluster = internal->construct_basic_clusters(v);
		#ifdef DEBUG_GRAPHVIZ
			internal->print_graphviz(root_cluster, "Basic clusters");
		#endif
		while (root_cluster->outer_edges.size() > 0) {
			root_cluster = internal->construct_topology_tree(root_cluster);
			for (auto c: internal->to_calculate_outer_edges) c->calculate_outer_edges();
			internal->to_calculate_outer_edges.clear();
		}
		internal->root_clusters.push_back(root_cluster);
		internal->root_clusters[i]->root_vector_index = i;
		i++;
	}

	for (auto c: internal->splitted_clusters) c->do_join();
	internal->splitted_clusters.clear();

	#ifdef DEBUG_GRAPHVIZ
		for (auto root_cluster: internal->root_clusters) internal->print_graphviz(root_cluster, "Full", true);
	#endif
}

//std::vector<std::shared_ptr<Cluster> > TopologyTopTree::GetTopTrees() {
//	return internal->root_clusters;
//}


#ifdef DEBUG_GRAPHVIZ
////////////////////////////////////////////////////////////////////////////////
// Debug output - Graphviz

void TopologyTopTree::Internal::print_graphviz_recursive(std::shared_ptr<TopologyCluster> cluster, std::shared_ptr<BaseTree::Internal::Edge> parent_edge, std::shared_ptr<TopologyCluster> parent, bool edges_to_childs, bool gray) const {
	if (cluster == NULL) return;

	auto shape = (cluster->first == NULL ? "triangle" : (cluster->second == NULL ? "circle" : "Msquare"));
	auto fillcolor = gray ? "gray" : "white";
	if (cluster->outer_edges.size() > 3) fillcolor = "red";
	else if (cluster->outer_edges.size() == 3 && cluster->second != NULL) fillcolor = "orange";

	std::cout << "\t\"" << cluster << "\" [label=\"" << *cluster << "\", shape=" << shape << ", style=filled, fillcolor=\"" << fillcolor << "\"]" << std::endl;
	if (parent_edge != NULL) {
		std::cout << "\t\"" << parent << "\" -> \"" << cluster << "\" [label=\"" << *parent_edge->data << "\"";
		if (parent_edge->subvertice_edge) std::cout << ", style=dashed";
		std::cout << "]" << std::endl;
	}
	if (edges_to_childs) {
		if (cluster->first != NULL) std::cout << "\t\"" << cluster << "\" -> \"" << cluster->first << "\" [color=orange, weight=0.5]" << std::endl;
		if (cluster->second != NULL) std::cout << "\t\"" << cluster << "\" -> \"" << cluster->second << "\" [color=orange, weight=0.5]" << std::endl;
	}
	for (auto o: cluster->outer_edges) {
		if (o.edge != parent_edge) print_graphviz_recursive(o.cluster, o.edge, cluster, edges_to_childs, gray);
	}

	// Parent test
	if (cluster->parent != NULL && cluster->parent->first != cluster && cluster->parent->second != cluster)
		std::cout << "\t\"" << cluster << "\" -> \"" << cluster->parent << "\" [style=bold, title=ERROR]" << std::endl;
}

void TopologyTopTree::Internal::print_graphviz(const std::shared_ptr<TopologyCluster> root, const std::string title, bool full) const {
	std::cout << "digraph \"" << root << "\" {" << "rankdir=\"LR\";" << std::endl;
	std::cout << "\tlabelloc=\"t\"" << std::endl << "\tlabel=\"" << title << "\"" << std::endl;
	if (full) {
		int level = 0;
		auto cluster = root;
		bool gray = false;
		while (cluster != NULL) {
			std::cout << "subgraph cluster_level_" << level << " {" << std::endl << "\tlabel=\"Level " << level << "\"" << std::endl << "\tgraph [style=dotted]" << std::endl;
			print_graphviz_recursive(cluster, NULL, NULL, true, gray);
			std::cout << "}" << std::endl;
			level++;
			cluster = cluster->first;
			gray = !gray;
		}
	} else print_graphviz_recursive(root);
	std::cout << "}" << std::endl;
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// Update procedure:

void TopologyTopTree::Internal::update_clusters_join_with_neighbour(std::shared_ptr<TopologyCluster> cluster, std::shared_ptr<TopologyCluster> neighbour) {
	#ifdef DEBUG
		std::cerr << "... joining " << *cluster << " with neighbour " << *neighbour << std::endl;
	#endif

	// Ensure that they are already splitted:
	cluster->do_split(&splitted_clusters);
	neighbour->do_split(&splitted_clusters);

	neighbour->listed_in_abandon_list = false;
	if (cluster->parent == NULL && neighbour->parent == NULL) {
		// Add new cluster to above level
		auto parent = std::make_shared<TopologyCluster>();
		splitted_clusters.push_back(parent);
		parent->set_first_child(cluster);
		parent->set_second_child(neighbour);
		to_calculate_outer_edges.push_back(parent);
		// Add new cluster to next abandon list
		next_abandon.push_back(parent);
		parent->listed_in_abandon_list = true;
		#ifdef DEBUG
			std::cerr << "... constructing new parent " << *parent << std::endl;
		#endif
	} else {
		if (cluster->parent == NULL) {
			// Add cluster into neighbour's parent
			if (neighbour->parent->second != NULL) std::cerr << "ERROR: Expecting that neighbour's parent would have only one child, but it have both children" << std::endl;
			neighbour->parent->set_second_child(cluster);
			#ifdef DEBUG
				std::cerr << "..." << *neighbour->parent << " set childs " << *neighbour->parent->first << " and " << *neighbour->parent->second << std::endl;
			#endif
		} else if (neighbour->parent == NULL) {
			// Add neighbour into cluster's parent
			if (cluster->parent->second != NULL) std::cerr << "ERROR: Expecting that cluster's parent would have only one child, but it have both children" << std::endl;
			cluster->parent->set_second_child(neighbour);
			#ifdef DEBUG
				std::cerr << "..." << *cluster->parent << " set childs " << *cluster->parent->first << " and " << *cluster->parent->second << std::endl;
			#endif
		} else {
			// Both have parents, use cluster's parent and delete neighbours parent
			neighbour->parent->first = NULL;
			next_delete.push_back(neighbour->parent);
			neighbour->parent->listed_in_delete_list = true;
			neighbour->parent = cluster->parent;
			cluster->parent->second = neighbour;
			#ifdef DEBUG
				std::cerr << "... joined under one parent " << *cluster->parent << " with childs " << *cluster->parent->first << " and " << *cluster->parent->second << std::endl;
			#endif
		}
		to_calculate_outer_edges.push_back(cluster->parent);
		next_change.push_back(cluster->parent);
		cluster->parent->listed_in_change_list = true;
	}
}

void TopologyTopTree::Internal::update_clusters_only_child(std::shared_ptr<TopologyCluster> cluster) {
	// Ensure that it is already splitted:
	cluster->do_split(&splitted_clusters);

	// This cluster is the only one child of its parent, ensure that parent exists
	if (cluster->parent == NULL) {
		// Have to create new parent
		auto parent = std::make_shared<TopologyCluster>();
		splitted_clusters.push_back(parent);
		parent->set_first_child(cluster);
		parent->vertex = cluster->vertex;
		to_calculate_outer_edges.push_back(parent);
		next_abandon.push_back(parent);
		parent->listed_in_abandon_list = true;

		#ifdef DEBUG
			std::cerr << "... creating new parent for cluster " << *cluster << ": " << *parent << std::endl;
		#endif
	} else {
		#ifdef DEBUG
			std::cerr << "... updating parent of cluster " << *cluster << std::endl;
		#endif
		to_calculate_outer_edges.push_back(cluster->parent);
		next_change.push_back(cluster->parent);
		cluster->parent->listed_in_change_list = true;
	}
}

void TopologyTopTree::Internal::update_clusters() {
	// When all lists empty -> end procedure
	if (delete_list.size() == 0 && abandon_list.size() == 0 && change_list.size() == 0) return;

	#ifdef DEBUG
		std::cerr << std::endl << "Delete list(" << delete_list.size() << "): ";
		for (auto cluster: delete_list) std::cerr << *cluster << ", ";
		std::cerr << std::endl;

		std::cerr << "Change list(" << change_list.size() << "): ";
		for (auto cluster: change_list) std::cerr << *cluster << ", ";
		std::cerr << std::endl;

		std::cerr << "Abandon list(" << abandon_list.size() << "): ";
		for (auto cluster: abandon_list) std::cerr << *cluster << ", ";
		std::cerr << std::endl;
	#endif

	to_calculate_outer_edges.clear();

	next_delete.clear();
	next_change.clear();
	next_abandon.clear();

	// 1. Run through deleted vertices
	for (auto cluster: delete_list) {
		#ifdef DEBUG
			std::cerr << "Deleting cluster " << *cluster << std::endl;
		#endif
		cluster->do_split(&splitted_clusters);
		if (cluster->parent != NULL) {
			if (cluster->parent->second == NULL) {
				#ifdef DEBUG
					std::cerr << "... no second child -> delete parent" << std::endl;
				#endif
				// No second child:
				next_delete.push_back(cluster->parent);
				cluster->parent->listed_in_delete_list = true;
				// Remove child link
				cluster->parent->first = NULL;
			} else {
				// There is another child
				if (cluster == cluster->parent->first) cluster->parent->first = cluster->parent->second;
				cluster->parent->second = NULL;
				cluster->parent->first->do_split(&splitted_clusters); // split it because we need to recompute it after all operations

				#ifdef DEBUG
					std::cerr << "... another child exists: " << *cluster->parent->first << std::endl;
				#endif

				// Remove edge in the parent cluster
				cluster->parent->edge = NULL;

				// Test another child (which is now the first child of parent)
				if (!cluster->parent->first->listed_in_change_list && !cluster->parent->first->listed_in_delete_list) {
					change_list.push_back(cluster->parent->first);
					cluster->parent->first->listed_in_change_list = true;
				}
			}
		}
		// Remove cluster from list and delete it
		cluster->parent = NULL;
		cluster->is_deleted = true; // there may be link from splitted_vertices list, we do not want do join this cluster again
		// Remove only inner clusters (basic at vertex level should remain)
		if (cluster->vertex == NULL || cluster->vertex->topology_cluster != cluster) {
			// if (cluster->vertex != NULL) cluster->vertex->topology_cluster = NULL;
			cluster->remove_all_outer_edges();
		}
		cluster->listed_in_delete_list = false;
	}

	// 2. Run through changed list that have sibling
	for (auto cluster: change_list) {
		// Skip clusters removed from list and clusters with no parent or without sibling (parent have only one child)
		if (!cluster->listed_in_change_list || cluster->parent == NULL || cluster->parent->second == NULL) continue;

		cluster->do_split(&splitted_clusters);
		auto sibling = (cluster->parent->first == cluster ? cluster->parent->second : cluster->parent->first);
		sibling->do_split(&splitted_clusters);

		#ifdef DEBUG
			std::cerr << "Checking cluster " << *cluster << std::endl;
		#endif

		// If connected with sibling by edge and parent is valid cluster
		// Get common edge:
		std::shared_ptr<BaseTree::Internal::Edge> common_edge = NULL;
		for (auto o: cluster->outer_edges) if (o.cluster == sibling) common_edge = o.edge;
		// Test parent
		if (common_edge != NULL && (cluster->outer_edges.size() + sibling->outer_edges.size()) <= 4) {
			#ifdef DEBUG
				std::cerr << "... connected with sibling " << *sibling << " by edge " << *common_edge->data << std::endl;
			#endif
			// Everything OK, remove cluster and sibling from change list and add parent into next change list
			cluster->listed_in_change_list = false;
			sibling->listed_in_change_list = false;
			next_change.push_back(cluster->parent);
			cluster->parent->listed_in_change_list = true;
			to_calculate_outer_edges.push_back(cluster->parent);
		} else {
			#ifdef DEBUG
				if (common_edge == NULL) std::cerr << "... not connected with sibling " << *sibling << " by edge, parent " << *cluster->parent << " will be deleted" << std::endl;
				else std::cerr << "... with sibling " << *sibling << " it is no longer valid cluster, parent (too many outer edges) " << *cluster->parent << " will be deleted" << std::endl;
			#endif
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

		#ifdef DEBUG
			std::cerr << "Checking abandon cluster  " << *cluster << " with outer edges size " << cluster->outer_edges.size() << std::endl;
			// for (auto x: cluster->outer_edges) std::cerr << "..." << *x.edge->data << "---" << *x.cluster << std::endl;
		#endif

		if (cluster->outer_edges.size() == 3) {
			// Find if there is neighbour with degree 1
			std::shared_ptr<TopologyCluster> neighbour = NULL;
			for (auto o: cluster->outer_edges) if (o.cluster->outer_edges.size() == 1) neighbour = o.cluster;

			if (neighbour != NULL) update_clusters_join_with_neighbour(cluster, neighbour); // Join with neighbour
			else update_clusters_only_child(cluster);
		} else if (cluster->outer_edges.size() >= 1) {
			// Find if there is neighbour with degree <= (4 - #outer_edges)
			std::shared_ptr<TopologyCluster> neighbour = NULL;
			for (auto o: cluster->outer_edges) {
				// Test if neighbour have low degree and if it have no sibling - if yes choose it
				if (o.cluster->outer_edges.size() <= (4 - cluster->outer_edges.size()) && (o.cluster->parent == NULL || o.cluster->parent->second == NULL)) neighbour = o.cluster;
			}

			if (neighbour != NULL) update_clusters_join_with_neighbour(cluster, neighbour); // Join with neighbour as in the first case
			else update_clusters_only_child(cluster);
		} else {
			//0 outer edges -> it is root and nothing is needed
			if (cluster->parent != NULL && cluster->parent->second == NULL && !cluster->parent->listed_in_delete_list) {
				// only this one child of parent
				next_delete.push_back(cluster->parent);
				cluster->parent->listed_in_delete_list = true;
			}
			cluster->parent = NULL;
			found_roots.push_back(cluster);
			#ifdef DEBUG
				std::cerr << "Found root " << *cluster << std::endl;
			#endif
		}
		cluster->listed_in_abandon_list = false;
	}

	// 4. Calculate outer edges for parent layer
	for (auto c: to_calculate_outer_edges) {
		#ifdef DEBUG
			std::cerr << "Computing outer edges for " << *c << std::endl;
		#endif
		c->calculate_outer_edges(true); // with checking neighbours (neighbours may not be on this list and we need to add new edges into them)
		#ifdef DEBUG
			std::cerr << "Outer edges for " << *c << " computed" << std::endl;
		#endif
	}

	// Continue with above level
	delete_list = next_delete;
	change_list = next_change;
	abandon_list = next_abandon;
	return update_clusters();
}


////////////////////////////////////////////////////////////////////////////////
/// Cut and Link

std::tuple<std::shared_ptr<ICluster>, std::shared_ptr<ICluster>, std::shared_ptr<EdgeData>> TopologyTopTree::Cut(int v_index, int w_index) {
	// Restore previous expose (if needed)
	Restore();

	// 0. Get vertices
	auto v = internal->base_tree->internal->vertices[v_index];
	auto w = internal->base_tree->internal->vertices[w_index];

	#ifdef DEBUG
		std::cerr << "Starting Cut operation between " << *v->data << " and " << *w->data << std::endl;
	#endif

	// 1. Find edge
	std::shared_ptr<BaseTree::Internal::Edge> edge = NULL;
	for (auto n: v->neighbours) if (n.vertex.lock() == w) { edge = n.edge.lock(); break; }
	if (edge == NULL) {
		std::cerr << "ERROR: Vertices not linked by edge, cannot cut" << std::endl;
		return std::make_tuple((std::shared_ptr<ICluster>)NULL, (std::shared_ptr<ICluster>)NULL, (std::shared_ptr<EdgeData>)NULL);
	}

	#ifdef DEBUG
		std::cerr << "Got edge to cut: " << *edge->data << std::endl;
	#endif

	// 2. Get vertices real connected with the edge (could be vertices itself or its subvertices)
	auto vv = edge->from;
	auto ww = edge->to;

	// 3. Cut itself (save results)
	auto result = internal->cut(vv, ww, edge);

	// 4. If were subvertices it is necessary to update subvertices and to delete edge even from superior vertices
	// 4.1 Delete edges from superior vertices
	if (vv->superior_vertex != NULL) vv->superior_vertex->neighbours.erase(edge->superior_from_iter);
	if (ww->superior_vertex != NULL) ww->superior_vertex->neighbours.erase(edge->superior_to_iter);
	// 4.2 Repair connections
	if (vv->superior_vertex != NULL) vv = internal->repair_subvertex_after_cut(vv);
	if (ww->superior_vertex != NULL) ww = internal->repair_subvertex_after_cut(ww);

	// 5. Get roots - TODO: do we want to return them, is that necessary?
	if (vv == w || vv->superior_vertex == w) std::swap(vv, ww);
	auto root_v = vv->topology_cluster;
	while (root_v->parent != NULL) root_v = root_v->parent;
	auto root_w = ww->topology_cluster;
	while (root_w->parent != NULL) root_w = root_w->parent;

	// 6. Restore all splitted clusters
	for (auto c: internal->splitted_clusters) c->do_join();
	internal->splitted_clusters.clear();

	// Remove edge from underlying Base tree
	edge->from->neighbours.erase(edge->from_iter);
	edge->to->neighbours.erase(edge->to_iter);
	v->degree--;
	w->degree--;
	// Node will be deleted by garbage collector

	#ifdef DEBUG_GRAPHVIZ
		internal->print_graphviz(root_v, "Cut - Result A", true);
		internal->print_graphviz(root_w, "Cut - Result B", true);
	#endif

	return std::make_tuple(root_v, root_w, edge->data);
}

std::tuple<std::shared_ptr<TopologyCluster>, std::shared_ptr<TopologyCluster>> TopologyTopTree::Internal::cut(
	std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w, std::shared_ptr<BaseTree::Internal::Edge> edge
) {
	// This function is not aware of splitted vertices (not needs it)

	// 0. Firstly test edge
	if (edge->from == w && edge->to == v) std::swap(v, w);
	if (edge->from != v || edge->to != w) {
		std::cerr << "Wrong edge between vertices " << *v->data << " and " << *w->data << ", got edge " << *edge->data << " between " << *edge->from->data << " and " << *edge->to->data << std::endl;
		exit(1);
	}

	// 1. Get clusters and split from them
	auto cluster_v = v->topology_cluster;
	auto cluster_w = w->topology_cluster;

	#ifdef DEBUG
		std::cerr << "===" << std::endl << "Starting internal cut operation between " << *cluster_v << " and " << *cluster_w << std::endl;
	#endif

	cluster_v->do_split(&splitted_clusters);
	cluster_w->do_split(&splitted_clusters);

	// 2. Remove edge from neighbours list
	edge->from->neighbours.erase(edge->from_iter);
	edge->to->neighbours.erase(edge->to_iter);
	// 2.1 Update degrees
	v->degree--;
	w->degree--;
	// 2.2 Remove edge from clusters
	cluster_v->calculate_outer_edges();
	cluster_w->calculate_outer_edges();

	// 3. Add both clusters into changed list (and empty other lists)
	delete_list.clear();
	abandon_list.clear();
	change_list.clear();
	change_list.push_back(cluster_v);
	cluster_v->listed_in_change_list = true;
	change_list.push_back(cluster_w);
	cluster_w->listed_in_change_list = true;

	// 4. Run update procedure
	found_roots.clear();
	update_clusters();

	// 5. Get results
	if (found_roots.size() != 2) {
		std::cerr << "ERROR: Expecting 2 roots after cut operation, found " << found_roots.size() << " roots!" << std::endl;
	}
	return std::make_tuple(found_roots[0], found_roots[1]);
	// expecting that do_join will be called from outside Cut function
}

std::shared_ptr<ICluster> TopologyTopTree::Link(int v_index, int w_index, std::shared_ptr<EdgeData> edge_data) {
	// Restore previous expose (if needed)
	Restore();

	// 0. Get vertices
	auto v = internal->base_tree->internal->vertices[v_index];
	auto w = internal->base_tree->internal->vertices[w_index];

	#ifdef DEBUG
		std::cerr << "Starting Link operation between " << *v->data << " and " << *w->data << std::endl;
	#endif

	// 1. Split vertices into subvertices if needed
	auto vv = internal->get_vertex_to_link(v);
	auto ww = internal->get_vertex_to_link(w);

	// 2. Create edge
	auto edge = std::make_shared<BaseTree::Internal::Edge>(vv, ww, edge_data);

	// 3. Link vertices
	auto result = internal->link(vv, ww, edge);

	// 4. Restore all splitted clusters
	for (auto c: internal->splitted_clusters) c->do_join();
	internal->splitted_clusters.clear();

	#ifdef DEBUG_GRAPHVIZ
		internal->print_graphviz(result, "Link result", true);
	#endif

	return result;
}

std::shared_ptr<BaseTree::Internal::Vertex> TopologyTopTree::Internal::get_vertex_to_link(std::shared_ptr<BaseTree::Internal::Vertex> v) {
	#ifdef DEBUG
		std::cerr << "Getting vertex for link for vertex " << *v->data << std::endl;
	#endif

	if (!v->subvertices.empty()) {
		#ifdef DEBUG
			std::cerr << "Will add subvertex to other subvertices of " << *v->data << std::endl;
		#endif

		// Have to add new subvertice, we add at as second subvertice in the chain
		// 1. Prepare new subvertice
		auto subvertex = std::make_shared<BaseTree::Internal::Vertex>(std::make_shared<VertexData>());
		subvertex->index = v->index; // index of the subvertex is the same as index of the superior vertex (from the Join point of view it is the same vertex)
		subvertex->superior_vertex = v;
		subvertex->topology_cluster = std::make_shared<TopologyCluster>();
		splitted_clusters.push_back(subvertex->topology_cluster);
		subvertex->topology_cluster->vertex = subvertex;

		// 2. Cut between first and second subvertex
		auto first = v->subvertices.begin();
		auto second = std::next(first);
		std::shared_ptr<BaseTree::Internal::Edge> edge;
		//std::cerr << "First subvertex is " << *(*first)->topology_cluster << " and second " << *(*second)->topology_cluster << std::endl;
		for (auto n: (*first)->neighbours) if (n.vertex.lock() == *second) edge = n.edge.lock();
		//std::cerr << "Edge: " << edge << std::endl;
		cut(*first, *second, edge);

		// 3. Link first-new-second
		// 3.1 Insert new subvertex a
		subvertex->superior_vertex_subvertices_iter = v->subvertices.insert(second, subvertex);
		// 3.2 Link itself
		link(*first, subvertex, edge); // reuse original edge
		auto edge2 = std::make_shared<BaseTree::Internal::Edge>(subvertex, *second, std::make_shared<EdgeData>());
		edge2->subvertice_edge = true;
		v->subvertice_edges.push_back(edge2);
		edge2->subvertice_edges_iterator = std::prev(v->subvertice_edges.end());
		link(subvertex, *second, edge2);

		return subvertex;
	} else if (v->degree == 3) {
		// Have to split vertex
		#ifdef DEBUG
			std::cerr << "Will split vertex " << *v->data << std::endl;
		#endif

		// 1. Prepare two subvertices
		auto subvertexA = std::make_shared<BaseTree::Internal::Vertex>(std::make_shared<VertexData>());
		subvertexA->index = v->index; // index of the subvertex is the same as index of the superior vertex (from the Join point of view it is the same vertex)
		auto subvertexB = std::make_shared<BaseTree::Internal::Vertex>(std::make_shared<VertexData>());
		subvertexB->index = v->index; // index of the subvertex is the same as index of the superior vertex (from the Join point of view it is the same vertex)
		subvertexA->superior_vertex = v;
		subvertexB->superior_vertex = v;
		subvertexA->topology_cluster = std::make_shared<TopologyCluster>();
		splitted_clusters.push_back(subvertexA->topology_cluster);
		subvertexA->topology_cluster->vertex = subvertexA;
		subvertexB->topology_cluster = std::make_shared<TopologyCluster>();
		splitted_clusters.push_back(subvertexB->topology_cluster);
		subvertexB->topology_cluster->vertex = subvertexB;

		// 2. Add subvertices to superior vertex's list of subvertices
		v->subvertices.push_back(subvertexA);
		subvertexA->superior_vertex_subvertices_iter = std::prev(v->subvertices.end());
		v->subvertices.push_back(subvertexB);
		subvertexB->superior_vertex_subvertices_iter = std::prev(v->subvertices.end());

		// 2. Reconnect first two edges to subvertexA and third edge to subvertexB
		auto n = v->neighbours.begin();
		while (n != v->neighbours.end()) {
			// (because cut operation deletes from this std::list we need to get next pointer before calling cut)
			auto next_n = std::next(n);
			if (auto vv = (*n).vertex.lock()) if (auto ee = (*n).edge.lock()) {
				#ifdef DEBUG
					std::cerr << "Working on edge " << *ee->data << " to the " << *vv->data << std::endl;
				#endif

				auto subvertex = subvertexA;
				(*n).subvertice_iter = v->subvertices.begin();
				if (subvertexA->degree == 2) {
					subvertex = subvertexB;
					(*n).subvertice_iter = std::next((*n).subvertice_iter);
				}

				if (ee->from == v) {
					ee->superior_from_iter = ee->from_iter;
					cut(v, ee->to, ee);
					link(subvertex, ee->to, ee);
				} else {
					ee->superior_to_iter = ee->to_iter;
					cut(ee->from, v, ee);
					link(ee->from, subvertex, ee);
				}
			}
			n = next_n;
		}

		// 3. Connect subvertices one to the other
		auto edge = std::make_shared<BaseTree::Internal::Edge>(subvertexA, subvertexB, std::make_shared<EdgeData>());
		edge->subvertice_edge = true;
		v->subvertice_edges.push_back(edge);
		edge->subvertice_edges_iterator = std::prev(v->subvertice_edges.end());
		link(subvertexA, subvertexB, edge);

		return subvertexB; // subvertexB has one free slot for the new edge
	} else return v; // else we can use the vertex itself
}

std::shared_ptr<TopologyCluster> TopologyTopTree::Internal::link(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w, std::shared_ptr<BaseTree::Internal::Edge> edge) {
	// This function is not aware of splitted vertices (not needs it)

	auto cluster_v = v->topology_cluster;
	auto cluster_w = w->topology_cluster;

	#ifdef DEBUG
		std::cerr << "==========" << std::endl;
		std::cerr << "Linking " << *v->data << " and " << *w->data << std::endl;
	#endif

	// 0. Split from both clusters
	cluster_v->do_split(&splitted_clusters);
	cluster_w->do_split(&splitted_clusters);

	// 1. Add new edge (expecting that outer Link function ensures not adding edge to vertices with degree 3)
	cluster_v->outer_edges.push_back(TopologyCluster::neighbour{edge, cluster_w});
	cluster_w->outer_edges.push_back(TopologyCluster::neighbour{edge, cluster_v});
	// 1.1 Update edge
	edge->from = v;
	edge->to = w;
	edge->register_at_vertices();
	// 1.2 Add edge to clusters
	cluster_v->calculate_outer_edges();
	cluster_w->calculate_outer_edges();

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

std::shared_ptr<BaseTree::Internal::Vertex> TopologyTopTree::Internal::repair_subvertex_after_cut(std::shared_ptr<BaseTree::Internal::Vertex> v) {
	std::shared_ptr<BaseTree::Internal::Vertex> first_neighbour = NULL;
	std::shared_ptr<BaseTree::Internal::Edge> first_neighbour_edge = NULL;
	std::shared_ptr<BaseTree::Internal::Vertex> second_neighbour = NULL;
	std::shared_ptr<BaseTree::Internal::Edge> second_neighbour_edge = NULL;

	for (auto n: v->neighbours) {
		if (n.edge.lock()->subvertice_edge) {
			if (first_neighbour == NULL) {
				first_neighbour = n.vertex.lock();
				first_neighbour_edge = n.edge.lock();
			} else {
				second_neighbour = n.vertex.lock();
				second_neighbour_edge = n.edge.lock();
			}
		}
	}

	if (second_neighbour == NULL) {
		// This vertex is one of end subvertices
		int subvertice_edges_counter = 0;
		for (auto n: first_neighbour->neighbours) if (n.edge.lock()->subvertice_edge) subvertice_edges_counter++;
		if (subvertice_edges_counter == 1) {
			// There is second endpoint (with 2 valid edges) -> we join them back into superior vertex
			// 1. Superior vertex may not have its TopologyCluster, create it
			if (v->superior_vertex->topology_cluster == NULL) {
				v->superior_vertex->topology_cluster = std::make_shared<TopologyCluster>();
				splitted_clusters.push_back(v->superior_vertex->topology_cluster);
				v->superior_vertex->topology_cluster->vertex = v->superior_vertex;
				#ifdef DEBUG
					std::cerr << "Created cluster for superior vertex " << *v->superior_vertex->data << " with " << v->superior_vertex->neighbours.size() << " neighbours" << std::endl;
				#endif
			}

			// 2. Run through neighbours and cut them, saving into list
			std::vector<std::pair<std::shared_ptr<BaseTree::Internal::Vertex>, std::shared_ptr<BaseTree::Internal::Edge>>> neighbours_list;
			// 2.A - neighbours from the first endpoint
			auto n = v->neighbours.begin();
			while (n != v->neighbours.end()) {
				// (because cut operation deletes from this std::list we need to get next pointer before calling cut)
				auto next_n = std::next(n);
				if (auto vv = (*n).vertex.lock()) if (auto ee = (*n).edge.lock()) {
					auto result = cut(v, vv, ee);
					#ifdef DEBUG_GRAPHVIZ_VERBOSE
						print_graphviz(std::get<0>(result), "Consolidating subvertices - After A cut 1/2", true);
						print_graphviz(std::get<1>(result), "Consolidating subvertices - After A cut 2/2", true);
					#endif
					if (!ee->subvertice_edge) {
						// Remove from superior's vertex neighbour list (otherwise the link process add an duplicity edge)
						v->superior_vertex->neighbours.erase(ee->from == v ? ee->superior_from_iter : ee->superior_to_iter);
						neighbours_list.push_back(std::make_pair(vv, ee));
					}
					else v->superior_vertex->subvertice_edges.erase(ee->subvertice_edges_iterator); // erase unused subvertice edge
				}
				n = next_n;
			}
			// 2.B - neighbours from the second endpoint
			n = first_neighbour->neighbours.begin();
			while (n != first_neighbour->neighbours.end()) {
				// (because cut operation deletes from this std::list we need to get next pointer before calling cut)
				auto next_n = std::next(n);
				if (auto vv = (*n).vertex.lock()) if (auto ee = (*n).edge.lock()) {
					auto result = cut(first_neighbour, vv, ee);
					#ifdef DEBUG_GRAPHVIZ_VERBOSE
						print_graphviz(std::get<0>(result), "Consolidating subvertices - After B cut 1/2", true);
						print_graphviz(std::get<1>(result), "Consolidating subvertices - After B cut 2/2", true);
					#endif
					if (!ee->subvertice_edge) {
						// Remove from superior's vertex neighbour list (otherwise the link process add an duplicity edge)
						v->superior_vertex->neighbours.erase(ee->from == first_neighbour ? ee->superior_from_iter : ee->superior_to_iter);
						neighbours_list.push_back(std::make_pair(vv, ee));
					}
					else v->superior_vertex->subvertice_edges.erase(ee->subvertice_edges_iterator); // erase unused subvertice edge
				}
				n = next_n;
			}
			v->superior_vertex->subvertices.clear();

			// 3. Connect all to the superior vertex
			for (auto n: neighbours_list) {
				link(v->superior_vertex, n.first, n.second);
			}

			return v->superior_vertex;
		} else {
			// We "steal" one edge from the neighbour and then we repair on this vertex
			for (auto n: first_neighbour->neighbours) {
				if (auto vv = n.vertex.lock()) if (auto ee = n.edge.lock()) {
					if (!ee->subvertice_edge) {
						auto result = cut(first_neighbour, vv, ee);
						#ifdef DEBUG_GRAPHVIZ_VERBOSE
							print_graphviz(std::get<0>(result), "Stealing from neighbour - After cut 1/2", true);
							print_graphviz(std::get<1>(result), "Stealing from neighbour - After cut 2/2", true);
						#endif
						// Edit superior vertex neighbours list and set subvertice_iter of ee edge to this new vertex
						std::shared_ptr<TopologyCluster> result2;
						if (ee->from == first_neighbour) {
							result2 = link(v, vv, ee);
							auto old_iter = (*ee->superior_from_iter).subvertice_iter;
							if (*std::prev(old_iter) == v) (*ee->superior_from_iter).subvertice_iter = std::prev(old_iter);
							else if (*std::next(old_iter) == v) (*ee->superior_from_iter).subvertice_iter = std::next(old_iter);
							else std::cerr << "ERROR: Cannot find updated subvertice iter for edge after steal operation" << std::endl;
						} else {
							result2 = link(v, vv, ee);
							auto old_iter = (*ee->superior_to_iter).subvertice_iter;
							if (*std::prev(old_iter) == v) (*ee->superior_to_iter).subvertice_iter = std::prev(old_iter);
							else if (*std::next(old_iter) == v) (*ee->superior_to_iter).subvertice_iter = std::next(old_iter);
							else std::cerr << "ERROR: Cannot find updated subvertice iter for edge after steal operation" << std::endl;
						}
						#ifdef DEBUG_GRAPHVIZ_VERBOSE
							print_graphviz(result2, "Stealing from neighbour - After link", true);
						#endif
						// edge is reused, nothing to do about edges
						break;
					}
				}
			}
			return repair_subvertex_after_cut(first_neighbour);
		}
	} else {
		// It is vertex inside "chain" -> remove it and join neighbours
		auto result = cut(first_neighbour, v, first_neighbour_edge);
		#ifdef DEBUG_GRAPHVIZ_VERBOSE
			print_graphviz(std::get<0>(result), "After first chain cut 1/2", true);
			print_graphviz(std::get<1>(result), "After first chain cut 2/2", true);
		#endif
		v->superior_vertex->subvertice_edges.erase(first_neighbour_edge->subvertice_edges_iterator); // erase first subvertice edge
		result = cut(second_neighbour, v, second_neighbour_edge);
		#ifdef DEBUG_GRAPHVIZ_VERBOSE
			print_graphviz(std::get<0>(result), "After second chain cut 1/2", true);
			print_graphviz(std::get<1>(result), "After second chain cut 2/2", true);
		#endif
		auto result2 = link(first_neighbour, second_neighbour, second_neighbour_edge); // reuse second subvertice edge
		#ifdef DEBUG_GRAPHVIZ_VERBOSE
			print_graphviz(result2, "After chain link", true);
		#endif
		// Delete vertex from supervertice's subvertices list
		v->superior_vertex->subvertices.erase(v->superior_vertex_subvertices_iter);
		return first_neighbour;
	}
}

////////////////////////////////////////////////////////////////////////////////
/// Expose

std::list<std::shared_ptr<ICluster>> TopologyTopTree::Internal::expose_get_clusters(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> second_v, bool continue_above_common) {
	std::list<std::shared_ptr<ICluster>> list;

	auto last_cluster = v->topology_cluster;
	auto cluster = last_cluster->parent; // we starts one level above base cluster
	bool was_added = false;
	while (cluster != NULL) {
		#ifdef DEBUG
			std::cerr << "Testing cluster " << *cluster << " - vertex " << *v->data << " external: " << cluster->is_external_boundary_vertex(v) << std::endl;
		#endif
		if (was_added || !cluster->is_external_boundary_vertex(v) || (cluster->edge != NULL && (cluster->edge->from == second_v || cluster->edge->to == second_v))) {
			if (cluster->edge != NULL) {
				if (!was_added && last_cluster->is_top_cluster) {
					#ifdef DEBUG
						std::cerr << "    Adding first cluster " << *last_cluster << std::endl;
					#endif
					auto new_simple_cluster = std::make_shared<SimpleCluster>();
					new_simple_cluster->boundary_left = last_cluster->boundary_left;
					new_simple_cluster->boundary_right = last_cluster->boundary_right;
					new_simple_cluster->first = last_cluster->first;
					new_simple_cluster->second = last_cluster->second;
					new_simple_cluster->data = last_cluster->data;
					expose_simple_clusters.push_back(new_simple_cluster); // to allow splitting it in Restore operation
					list.push_back(new_simple_cluster);
				}
				was_added = true;

				// If we are in the same area as the other run exit
				if (!continue_above_common && (
					(last_cluster == cluster->first && cluster->second->is_splitted) || (last_cluster == cluster->second && cluster->first->is_splitted)
				)) return list;

				std::shared_ptr<SimpleCluster> edge_cluster = NULL;
				if (!cluster->edge->subvertice_edge) {
					edge_cluster = std::make_shared<SimpleCluster>();
					edge_cluster->boundary_left = cluster->edge->from;
					edge_cluster->boundary_right = cluster->edge->to;
					edge_cluster->edge = cluster->edge;
					expose_simple_clusters.push_back(edge_cluster); // to allow splitting it in Restore operation
					Create(edge_cluster, cluster->edge->data);
				}

				std::shared_ptr<TopologyCluster> sibling_cluster = NULL;
				if (last_cluster == cluster->first && cluster->second->is_top_cluster && !cluster->second->is_splitted) sibling_cluster = cluster->second;
				else if (last_cluster == cluster->second && cluster->first->is_top_cluster && !cluster->first->is_splitted) sibling_cluster = cluster->first;

				std::shared_ptr<ICluster> new_cluster = NULL;
				if (edge_cluster != NULL && sibling_cluster != NULL) {
					#ifdef DEBUG
						std::cerr << "    Joining edge with endpoints " << *edge_cluster->boundary_left->data << "-" << *edge_cluster->boundary_right->data
							  << " with cluster " << *sibling_cluster << " with endpoints " << *sibling_cluster->boundary_left->data << "-" << *sibling_cluster->boundary_right->data << std::endl;
					#endif
					// Combine them into one newly created SimpleCluster
					auto new_simple_cluster = std::make_shared<SimpleCluster>();
					if (sibling_cluster->is_rake_branch) {
						new_simple_cluster->boundary_left = edge_cluster->boundary_left;
						new_simple_cluster->boundary_right = edge_cluster->boundary_right;
					} else {
						// Find common vertex and construct compress cluster around it
						auto common_vertex = TopologyCluster::get_common_vertex(sibling_cluster, edge_cluster, !cluster->edge->subvertice_edge);

						new_simple_cluster->boundary_left = (common_vertex == edge_cluster->boundary_left || common_vertex == edge_cluster->boundary_left->superior_vertex ? edge_cluster->boundary_right : edge_cluster->boundary_left);
						new_simple_cluster->boundary_right = (common_vertex == sibling_cluster->boundary_left || common_vertex == sibling_cluster->boundary_left->superior_vertex ? sibling_cluster->boundary_right : sibling_cluster->boundary_left);
					}
					new_simple_cluster->first = edge_cluster;
					edge_cluster->parent = new_simple_cluster;
					new_simple_cluster->second = sibling_cluster;
					Join(edge_cluster, sibling_cluster, new_simple_cluster);
					expose_simple_clusters.push_back(new_simple_cluster); // to allow splitting it in Restore operation
					new_cluster = new_simple_cluster;
				} else if (edge_cluster != NULL) new_cluster = edge_cluster;
				else if (sibling_cluster != NULL) new_cluster = sibling_cluster;

				if (new_cluster != NULL) {
					#ifdef DEBUG
						std::cerr << "    Adding new cluster with endpoints " << *new_cluster->boundary_left->data << "-" << *new_cluster->boundary_right->data << std::endl;
					#endif
					list.push_back(new_cluster);
				}
			}
		}
		last_cluster = cluster;
		cluster = cluster->parent;
	}
	return list;
}

std::shared_ptr<ICluster> TopologyTopTree::Expose(int v_index, int w_index) {
	// Restore previous expose (if needed)
	Restore();

	// 0. Get vertices and their clusters
	auto v = internal->base_tree->internal->vertices[v_index];
	auto w = internal->base_tree->internal->vertices[w_index];

	#ifdef DEBUG
		std::cerr << "Starting Expose of path between " << *v->data << " and " << *w->data << std::endl;
	#endif

	// If vertex is splitted into subvertices choose some
	if (!v->subvertices.empty()) v = v->subvertices.front();
	if (!w->subvertices.empty()) w = w->subvertices.front();
	// Get clusters
	auto cluster_v = v->topology_cluster;
	auto cluster_w = w->topology_cluster;

	// 1. Split from both base clusters
	cluster_v->do_split(&internal->splitted_clusters);
	cluster_w->do_split(&internal->splitted_clusters);

	// 2. Get all clusters that contains v/w as non-boundary vertex and save them into two lists
	auto clusters_list = internal->expose_get_clusters(v, w, true);
	auto second_list = internal->expose_get_clusters(w, v, false);
	// 2.1 Join lists
	for (auto it = second_list.rbegin(); it != second_list.rend(); ++it) clusters_list.push_back(*it);

	#ifdef DEBUG
		std::cerr << "Clusters before inner raking: ";
		for (auto c: clusters_list) std::cerr << *c->boundary_left->data << "-" << *c->boundary_right->data << ", ";
		std::cerr << std::endl;
	#endif

	// 3. Join all clusters that must be raked
	auto vertex = v;
	auto it = clusters_list.begin();
	while (it != clusters_list.end()) {
		auto next_it = std::next(it); // because we may erase this iterator when joining clusters
		if (vertex->superior_vertex != NULL) vertex = vertex->superior_vertex;
		auto cluster = *it;
		// If cluster can be connected leave this cluster there and only update next join vertex
		if (vertex == cluster->boundary_left || vertex == cluster->boundary_left->superior_vertex) vertex = cluster->boundary_right;
		else if (vertex == cluster->boundary_right || vertex == cluster->boundary_right->superior_vertex) vertex = cluster->boundary_left;
		else {
			// Cannot join with previous cluster, previous cluster must be raked onto this cluster
			// or this cluster must be raked on the following cluster

			// 0. Choose variant, get other cluster and common vertex
			bool prev_join = false;
			bool next_join = false;
			std::shared_ptr<ICluster> other_cluster;
			std::shared_ptr<BaseTree::Internal::Vertex> common_vertex;
			// a) Try previous cluster
			if (it != clusters_list.begin()) {
				other_cluster = *std::prev(it);
				common_vertex = TopologyCluster::get_common_vertex(other_cluster, cluster);
				if (common_vertex != NULL) {
					#ifdef DEBUG
						std::cerr << " - Raking PREVIOUS cluster with endpoint " << *other_cluster->boundary_left->data << "," << *other_cluster->boundary_right->data
						<< " onto cluster " << *cluster->boundary_left->data << "," << *cluster->boundary_right->data << std::endl;
					#endif
					prev_join = true;
				}
			}
			// b) otherwise join with next cluster
			if (!prev_join && it != std::prev(clusters_list.end())) {
				other_cluster = *std::next(it);
				common_vertex = TopologyCluster::get_common_vertex(other_cluster, cluster);
				if (common_vertex != NULL) {
					#ifdef DEBUG
						std::cerr << " - Raking cluster with endpoint " << *cluster->boundary_left->data << "," << *cluster->boundary_right->data
						<< " onto NEXT cluster " << *other_cluster->boundary_left->data << "," << *other_cluster->boundary_right->data << std::endl;
					#endif
					next_join = true;
				}
			}

			if (!prev_join && !next_join) {
				std::cerr << "ERROR: cannot join this cluster with previous or next cluster in the exposed list" << std::endl;
				exit(1);
			}

			// When raking previous onto this one the new join vertex have to be found
			// (when raking this one onto the next this cycle would run once one and no changes to the join vertex are needed)
			if (prev_join) {
				// Get other side vertex than common vertex from this cluster
				if (cluster->boundary_left == common_vertex || cluster->boundary_left->superior_vertex == common_vertex) vertex = cluster->boundary_right;
				else vertex = cluster->boundary_left;
			}

			// 1. Prepare new cluster
			auto new_cluster = std::make_shared<SimpleCluster>();
			// 1.1 Set parents (if they are simple clusters)
			new_cluster->first = other_cluster;
			auto other_cluster_simple = std::dynamic_pointer_cast<SimpleCluster>(other_cluster);
			if (other_cluster_simple != NULL) other_cluster_simple->parent = new_cluster;
			new_cluster->second = cluster;
			auto cluster_simple = std::dynamic_pointer_cast<SimpleCluster>(cluster);
			if (cluster_simple != NULL) cluster_simple->parent = new_cluster;
			// 1.2 Set boundaries (raked onto current cluster) and Join
			if (prev_join) {
				// a) Rake previous onto this one
				new_cluster->boundary_left = cluster->boundary_left;
				new_cluster->boundary_right = cluster->boundary_right;
			} else {
				// b) Rake this one onto the next one
				new_cluster->boundary_left = other_cluster->boundary_left;
				new_cluster->boundary_right = other_cluster->boundary_right;
			}

			Join(other_cluster, cluster, new_cluster);
			#ifdef DEBUG
				std::cerr << " -> Joined into " << *new_cluster->boundary_left->data << "," << *new_cluster->boundary_right->data << std::endl;
			#endif

			// 2. Remove old clusters and add new one to the list
			if (prev_join) {
				// 2.1 Remove previous cluster from list
				clusters_list.erase(std::prev(it));
				// 2.2 Insert new cluster previously to the current cluster
				clusters_list.insert(it, new_cluster);
				// 2.3 Remove current cluster
				clusters_list.erase(it);
			} else {
				// 2.1 Remove next cluster from list
				clusters_list.erase(std::next(it));
				// 2.2 Insert new cluster previously to the current cluster
				clusters_list.insert(it, new_cluster);
				// 2.3 Next iterator will be newly inserted
				next_it = std::prev(it);
				// 2.4 Remove current cluster
				clusters_list.erase(it);
			}
		}
		it = next_it;
	}

	#ifdef DEBUG
		std::cerr << "Clusters before rake joining of end: ";
		for (auto c: clusters_list) std::cerr << *c->boundary_left->data << "-" << *c->boundary_right->data << ", ";
		std::cerr << std::endl;
	#endif

	// 4. Ensure that it ends with w (otherwise rake last cluster)
	while (vertex != w && vertex != w->superior_vertex) {
		// 1. Get clusters
		auto cluster_last = std::dynamic_pointer_cast<SimpleCluster>(clusters_list.back());
		clusters_list.pop_back();
		auto cluster_prev = std::dynamic_pointer_cast<SimpleCluster>(clusters_list.back());
		clusters_list.pop_back();

		// 2. Construct cluster
		auto cluster = std::make_shared<SimpleCluster>();
		cluster->first = cluster_prev;
		cluster_prev->parent = cluster;
		cluster->second = cluster_last;
		cluster_last->parent = cluster;
		// 2.1 Set boundaries (rake onto prev cluster) and Join
		cluster->boundary_left = cluster_prev->boundary_left;
		cluster->boundary_right = cluster_prev->boundary_right;
		Join(cluster_prev, cluster_last, cluster);

		// 3. calculate new end vertex - opposite vertex than common vertex with previous cluster
		auto common_vertex = v;
		if (!clusters_list.empty()) common_vertex = TopologyCluster::get_common_vertex(clusters_list.back(), cluster);
		if (common_vertex->superior_vertex != NULL) common_vertex = common_vertex->superior_vertex;

		if (cluster->boundary_left == common_vertex || cluster->boundary_left->superior_vertex == common_vertex) vertex = cluster->boundary_right;
		else vertex = cluster->boundary_left;
		if (vertex->superior_vertex != NULL) vertex = vertex->superior_vertex;

		// 4. Push new cluster into list
		clusters_list.push_back(cluster);
	}

	#ifdef DEBUG
		std::cerr << "Clusters before compressing: ";
		for (auto c: clusters_list) std::cerr << *c->boundary_left->data << "-" << *c->boundary_right->data << ", ";
		std::cerr << std::endl;
	#endif

	// 5. Compress join all remaining clusters
	while (clusters_list.size() > 1) {
		// 1. Get clusters
		auto cluster_a = std::dynamic_pointer_cast<SimpleCluster>(clusters_list.back());
		clusters_list.pop_back();
		auto cluster_b = std::dynamic_pointer_cast<SimpleCluster>(clusters_list.back());
		clusters_list.pop_back();

		// 2. Construct cluster
		auto cluster = std::make_shared<SimpleCluster>();
		cluster->first = cluster_a;
		cluster_a->parent = cluster;
		cluster->second = cluster_b;
		cluster_b->parent = cluster;

		// 3. Find common vertex
		auto common_vertex = TopologyCluster::get_common_vertex(cluster_a, cluster_b);

		// 4. Set boundaries
		cluster->boundary_left = (cluster_a->boundary_left == common_vertex || cluster_a->boundary_left->superior_vertex == common_vertex ? cluster_a->boundary_right : cluster_a->boundary_left);
		cluster->boundary_right = (cluster_b->boundary_left == common_vertex || cluster_b->boundary_left->superior_vertex == common_vertex ? cluster_b->boundary_right : cluster_b->boundary_left);

		// 5. Join and push new cluster into list
		Join(cluster_a, cluster_b, cluster);
		clusters_list.push_back(cluster);
	}

	// 6. Return resulting cluster
	return clusters_list.back();
}

void TopologyTopTree::Restore() {
	if (internal->expose_simple_clusters.empty()) return; // no need to restore anything

	// Split temporary clusters
	for (auto c: internal->expose_simple_clusters) c->do_split();
	internal->expose_simple_clusters.clear();

	// Join original clusters
	for (auto c: internal->splitted_clusters) {
		while (c != NULL && c->is_splitted) {
			c->do_join();
			c = c->parent;
		}
	}
	internal->splitted_clusters.clear();
}

////////////////////////////////////////////////////////////////////////////////
/// Functions for construction:

std::shared_ptr<BaseTree::Internal::Vertex> TopologyTopTree::Internal::split_vertex(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> parent_edge) {
	// Create subvertices
	auto current = std::make_shared<BaseTree::Internal::Vertex>(std::make_shared<VertexData>());
	current->index = v->index; // index of the subvertex is the same as index of the superior vertex (from the Join point of view it is the same vertex)
	current->superior_vertex = v;
	v->subvertices.push_back(current);
	auto vertex_to_return = current; // by default we return the first vertex

	auto current_iter = std::prev(v->subvertices.end());
	for (auto n = v->neighbours.begin(); n != v->neighbours.end(); ++n) {
		// Copy this edge into subvertice and notice what subvertice it is
		auto edge = (*n).edge.lock();
		auto target_vertex = (*n).vertex.lock();

		// 1. If this subvertice is full create a new one
		if (current->degree == 2 && n != std::prev(v->neighbours.end())) {
			#ifdef DEBUG
				std::cerr << "Creating new subvertex for " << *v->data << std::endl;
			#endif
			auto temp = std::make_shared<BaseTree::Internal::Vertex>(std::make_shared<VertexData>());
			temp->index = v->index; // index of the subvertex is the same as index of the superior vertex (from the Join point of view it is the same vertex)
			temp->superior_vertex = v;
			v->subvertices.push_back(temp);
			temp->superior_vertex_subvertices_iter = std::prev(v->subvertices.end());

			// Add edge between them (subvertice edge)
			auto inner_edge = std::make_shared<BaseTree::Internal::Edge>(current, temp, std::make_shared<EdgeData>());
			inner_edge->subvertice_edge = true;
			inner_edge->register_at_vertices();

			v->subvertice_edges.push_back(inner_edge);
			inner_edge->subvertice_edges_iterator = std::prev(v->subvertice_edges.end());

			current = temp;

			current_iter = std::prev(v->subvertices.end());
		}

		// 2. Test if this edge is the parent edge (so it will be connected with this subvertex) and if so remember it so we will return this one
		if (edge == parent_edge) vertex_to_return = current;

		// 3. Add this edge to current subvertice
		current->neighbours.push_back(BaseTree::Internal::neighbour{target_vertex, edge});
		current->degree++;
		(*n).subvertice_iter = current_iter;

		// 4. Update edge itself - its from/to will be updated to this vertex and iter will be added
		if (edge->from == v) {
			edge->from = current;
			edge->superior_from_iter = edge->from_iter;
			edge->from_iter = std::prev(current->neighbours.end());
		} else {
			edge->to = current;
			edge->superior_to_iter = edge->to_iter;
			edge->to_iter = std::prev(current->neighbours.end());
		}
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
	splitted_clusters.push_back(cluster);
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
	splitted_clusters.push_back(new_cluster);
	new_cluster->first = cluster;
	new_cluster->vertex = cluster->vertex;
	cluster->parent = new_cluster;
	new_cluster->outer_edges_count = cluster->outer_edges_count;
	to_calculate_outer_edges.push_back(new_cluster);

	return new_cluster;
}

TopologyTopTree::~TopologyTopTree() {}
}
