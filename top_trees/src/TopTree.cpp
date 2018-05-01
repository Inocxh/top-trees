#include <queue>
#include <vector>
#include <sstream>

#include "ClusterInterface.hpp"
#include "TopTree.hpp"
#include "BaseTreeInternal.hpp"

// #define DEBUG
// #define DEBUG_GRAPHVIZ

//#define WARNINGS

namespace TopTree {

// Hide data from .hpp file using PIMP idiom
class TopTree::Internal {
public:
	std::list<std::shared_ptr<TopCluster>> root_clusters;
	std::shared_ptr<BaseTree> base_tree;

	std::shared_ptr<TopCluster> construct_cluster(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> e=NULL);

	void soft_expose(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w);
	std::shared_ptr<TopCluster> hard_expose(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w);
	void guarded_splay(std::shared_ptr<TopCluster> node, std::shared_ptr<TopCluster> guard = NULL);

	std::vector<std::shared_ptr<TopCluster>> splitted_clusters;
	std::vector<std::shared_ptr<CompressCluster>> hard_expose_transformed_clusters;

	std::shared_ptr<TopCluster> get_handle(std::shared_ptr<BaseTree::Internal::Vertex> v);

	// Debug methods:
	#ifdef DEBUG
		void print_rooted_prefix(const std::shared_ptr<TopCluster> cluster, const std::string prefix = "", bool last_child = true) const;
	#endif
	#ifdef DEBUG_GRAPHVIZ
		void print_graphviz(std::shared_ptr<TopCluster> node, const std::string title="");
		void print_graphviz_recursive(std::shared_ptr<TopCluster> parent, std::shared_ptr<TopCluster> node, const char* edge_label="") const;
		void print_graphviz_child(std::shared_ptr<TopCluster> from, std::shared_ptr<TopCluster> to, const char* edge_label="") const;
	#endif
private:
	int graphviz_counter = 0;
	void adjust_parent(std::shared_ptr<TopCluster> parent, std::shared_ptr<TopCluster> old_child, std::shared_ptr<TopCluster> new_child);
	void rotate_left(std::shared_ptr<TopCluster> x);
	void rotate_right(std::shared_ptr<TopCluster> x);

	void splice(std::shared_ptr<TopCluster> node);

	void soft_expose_handle(std::shared_ptr<TopCluster> handle, std::shared_ptr<TopCluster> splay_guard = NULL);
};

////////////////////////////////////////////////////////////////////////////////

TopTree::TopTree() : internal{std::make_unique<Internal>()} {}

TopTree::TopTree(std::shared_ptr<BaseTree> baseTree) : TopTree() {
	InitFromBaseTree(baseTree);
}

void TopTree::InitFromBaseTree(std::shared_ptr<BaseTree> baseTree) {
	internal->base_tree = baseTree;

	for (auto v : internal->base_tree->internal->vertices) v->used = false;

	for (auto v : internal->base_tree->internal->vertices) {
		if (v->used || v->degree != 1) continue;
		auto cluster = internal->construct_cluster(v);
		internal->root_clusters.push_back(cluster);
		cluster->root_clusters_iterator = std::prev(internal->root_clusters.end());
	}

	#ifdef DEBUG_GRAPHVIZ
		for (auto root_cluster: internal->root_clusters) internal->print_graphviz(root_cluster, "Full");
	#endif
}

#ifdef DEBUG
////////////////////////////////////////////////////////////////////////////////
// Debug output - console

void TopTree::Internal::print_rooted_prefix(const std::shared_ptr<TopCluster> cluster, const std::string prefix, bool last_child) const {
	std::cerr << *cluster << std::endl;
	auto prefix_child = prefix + (last_child ? "   " : "|  ");

	if (cluster->left_foster != NULL) {
		std::cerr << prefix_child << "|-Foster left: ";
		print_rooted_prefix(cluster->left_foster, prefix_child, false);
	}
	if (cluster->left_child != NULL) {
		std::cerr << prefix_child << "|-Left: ";
		print_rooted_prefix(cluster->left_child, prefix_child, false);
	}
	if (cluster->right_child != NULL) {
		std::cerr << prefix_child << "|-Right: ";
		print_rooted_prefix(cluster->right_child, prefix_child, (cluster->right_foster == NULL));
	}
	if (cluster->right_foster != NULL) {
		std::cerr << prefix_child << "|-Foster right: ";
		print_rooted_prefix(cluster->right_foster, prefix_child, true);
	}
}

/*
void TopTree::PrintRooted(const std::shared_ptr<TopCluster> root) const {
	internal->print_rooted_prefix(root);

	for (auto v: internal->base_tree->internal->vertices) {
		if (v->handle != NULL)
			std::cerr << "HANDLE " << *v->data << ": " << *v->handle << std::endl;
		else std::cerr << "HANDLE " << *v->data << ": NONE" << std::endl;
	}
}
*/
#endif

#ifdef DEBUG_GRAPHVIZ
////////////////////////////////////////////////////////////////////////////////
// Debug outpu - Graphviz

void TopTree::Internal::print_graphviz_child(std::shared_ptr<TopCluster> from, std::shared_ptr<TopCluster> to, const char* edge_label) const {
	to->_short_name(std::cout << "\t\"" << to << "\" [label=\"") << "\",shape=";
	if (to->isCompress()) std::cout << "box";
	else if (to->isRake()) std::cout << "diamond";
	else std::cout << "circle";
	std::cout << "]" << std::endl;

	// Debug (show broken parent links):
	if (to->parent != from && to->parent->parent != from) std::cout << "\t\"" << to << "\" -> \"PARENT: " << to->parent << "\" [style=bold]" << std::endl;

	// Edge
	if (from == NULL) return;
	std::cout << "\t\"" << from << "\" -> \"" << to << "\" [label=\"" << edge_label << "\"";
	if (to == from->left_foster || to == from->right_foster) std::cout << ", style=dashed";
	std::cout << "]" << std::endl;
}

void TopTree::Internal::print_graphviz_recursive(std::shared_ptr<TopCluster> parent, std::shared_ptr<TopCluster> node, const char* edge_label) const {
	if (node == NULL) return;
	print_graphviz_child(parent, node, edge_label);

	print_graphviz_recursive(node, node->left_foster, "LF");
	print_graphviz_recursive(node, node->left_child, "L");
	print_graphviz_recursive(node, node->right_foster, "RF");
	print_graphviz_recursive(node, node->right_child, "R");
}

void TopTree::Internal::print_graphviz(const std::shared_ptr<TopCluster> root, const std::string title) {
	std::ostringstream ss;
	ss << "[" << graphviz_counter << "] " << title;

	std::cerr << "GRAPHVIZ: " << ss.str() << std::endl;
	std::cout << "digraph \"" << root << "\" {" << std::endl;
	std::cout << "\tlabelloc=\"t\"" << std::endl << "\tlabel=\"" << ss.str() << "\"" << std::endl;
	print_graphviz_recursive(NULL, root);
	std::cout << "}" << std::endl;
	graphviz_counter++;
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Soft and hard expose related functions

std::shared_ptr<ICluster> TopTree::Expose(int v, int w) {
	Restore();

	if (v == w) {
		#ifdef WARNINGS
			std::cerr << "WARNING: Cannot expose one vertex " << *internal->base_tree->internal->vertices[v] << std::endl;
		#endif
		return NULL;
	}

	auto vertexV = internal->base_tree->internal->vertices[v];
	auto vertexW = internal->base_tree->internal->vertices[w];

	#ifdef DEBUG
		std::cerr << "[Exposing " << *vertexV->data << "," << *vertexW->data << "]" << std::endl;
	#endif

	internal->soft_expose(vertexV, vertexW);
	return internal->hard_expose(vertexV, vertexW);
}

std::shared_ptr<TopCluster> TopTree::Internal::get_handle(std::shared_ptr<BaseTree::Internal::Vertex> v) {
	if (v->base_handles.size() == 0) return NULL;

	if (v->last_handle == NULL || !v->last_handle->is_handle_for(v)) v->last_handle = v->base_handles.front();

	// Try to go up until it stops to be handle for v
	while (true) {
		auto parent = v->last_handle->parent;
		while (parent != NULL && parent->isRake()) parent = parent->parent;
		if (parent != NULL && parent->is_handle_for(v)) v->last_handle = parent;
		else break;
	}
	return v->last_handle;
}

// A. Splaying
void TopTree::Internal::adjust_parent(std::shared_ptr<TopCluster> parent, std::shared_ptr<TopCluster> old_child, std::shared_ptr<TopCluster> new_child) {
	// Ensure that both children are splitted before any action
	new_child->do_split(&splitted_clusters);
	old_child->do_split(&splitted_clusters);

	if (parent != NULL) {
		if (parent->left_child == old_child) parent->set_left_child(new_child);
		else if (parent->right_child == old_child) parent->set_right_child(new_child);
		else if (parent->left_foster == old_child) parent->set_left_foster(new_child);
		else if (parent->right_foster == old_child) parent->set_right_foster(new_child);
		else {
			std::cerr << "ERROR: " << *old_child << " is not any child of " << *parent << std::endl;
			exit(1);
		}
	} else {
		// x was one of the roots, replace it in place in the vector
		root_clusters.erase(old_child->root_clusters_iterator);
		root_clusters.push_back(new_child);
		new_child->root_clusters_iterator = std::prev(root_clusters.end());
	}
}

//     x               y
//   A   y    ->    x    C
//      B C        A B
void TopTree::Internal::rotate_left(std::shared_ptr<TopCluster> x) {
	auto parent = x->parent;
	auto y = x->right_child;

	#ifdef DEBUG
		std::cerr << "Rotating left around " << *x << std::endl;
	#endif

	adjust_parent(parent, x, y);

	// Connect:
	x->set_right_child(y->left_child);
	y->set_left_child(x);
	y->parent = parent;

	// Adjust endpoints:
	x->correct_endpoints();
	y->correct_endpoints();
}

//     x               y
//   y   C    ->    A    x
//  A B                 B C
void TopTree::Internal::rotate_right(std::shared_ptr<TopCluster> x) {
	auto parent = x->parent;
	auto y = x->left_child;

	#ifdef DEBUG
		std::cerr << "Rotating right around " << *x << std::endl;
	#endif

	adjust_parent(parent, x, y);

	// Connect:
	x->set_left_child(y->right_child);
	y->set_right_child(x);
	y->parent = parent;

	// Adjust endpoints:
	x->correct_endpoints();
	y->correct_endpoints();
}

void TopTree::Internal::guarded_splay(std::shared_ptr<TopCluster> node, std::shared_ptr<TopCluster> guard) {
	#ifdef DEBUG
		std::cerr << "Splaying " << *node;
		if (guard != NULL) std::cerr << " with guard " << *guard;
		std::cerr << std::endl;
	#endif
	if (node->isBase()) {
		// Not splay base clusters
		if (node->parent != NULL && node->parent != guard && node->parent->isCompress()) guarded_splay(node->parent, guard);
		return;
	}
	while (true) {
		if (node->parent == guard || node->parent == NULL) return;
		if (node->parent->parent == guard || node->parent->parent == NULL) {
			// Zig rotate (last under guard)
			if (node == node->parent->left_child) rotate_right(node->parent);
			else rotate_left(node->parent);
			return;
		}
		// Zig-Zag rotate or Zig-Zig rotate
		// 1. step
		if (node == node->parent->left_child) rotate_right(node->parent);
		else rotate_left(node->parent);
		// 2. step
		if (node == node->parent->left_child) rotate_right(node->parent);
		else rotate_left(node->parent);
	}
}

// B. Splicing
// Splicing occur only after splaying -> at most two rake nodes to the root of some compress tree
void TopTree::Internal::splice(std::shared_ptr<TopCluster> node) {
	auto left_nodes = std::vector<std::shared_ptr<TopCluster>>();
	auto right_nodes = std::vector<std::shared_ptr<TopCluster>>();

	#ifdef DEBUG
		std::cerr << "Splicing " << *node << std::endl;
	#endif

	// 1. Go up to the root of a compress tree and split other nodes to left
	// and right siblings
	auto root = node->parent;
	if (root->isCompress() && (root->left_child == node || root->right_child == node)) return;
	auto current = node;
	while (root->isRake()) {
		// The most generic variant is when there are two rake nodes to the nearest
		// root of a compress tree
		if (current == root->left_child) right_nodes.push_back(root->right_child);
		else left_nodes.push_back(root->left_child);

		// Inner clusters will be deleted by garbage collection after, we only
		// need to not joining them so mark them as joined
		root->is_splitted = false;

		current = root;
		root = root->parent;
	}
	if (!root->is_splitted) root->do_split(&splitted_clusters);

	#ifdef DEBUG
		std::cerr << "Splice root is " << *root << std::endl;
	#endif

	if (root->parent != NULL) {
		// When there is cluster above we must ensure that the left child
		// is not connector with this upper compress cluster

		// When above cluster is compress: left child must not have boundary that is used as connector
		if (root->parent->isCompress() && (root->left_child->boundary_left == root->parent->common_vertex || root->left_child->boundary_right == root->parent->common_vertex)) root->flip();

		// When above cluster is rake: left child must not have boundary that is used as rake node
		if (root->parent->isRake() && (root->left_child->boundary_left == root->parent->boundary_right || root->left_child->boundary_right == root->parent->boundary_right)) root->flip();
	}

	// 2. Now root is root of some compress tree -> add (foster)childs
	// in preparation that given node will be new left child of the given root
	if (current == root->left_foster) {
		right_nodes.push_back(root->left_child);
		if (root->right_foster != NULL) right_nodes.push_back(root->right_foster);
	} else {
		left_nodes.push_back(root->left_child);
		if (root->left_foster != NULL) left_nodes.push_back(root->left_foster);
	}

	// 3. construct new left and right foster trees
	std::shared_ptr<TopCluster> new_left_foster = NULL;
	if (!left_nodes.empty()) {
		new_left_foster = left_nodes.back();
		left_nodes.pop_back();
		new_left_foster->correct_endpoints();

		#ifdef DEBUG
			std::cerr << "L- First left rake is " << *new_left_foster << std::endl;
		#endif

		while (!left_nodes.empty()) {
			auto right = left_nodes.back();
			left_nodes.pop_back();
			right->correct_endpoints();

			#ifdef DEBUG
				std::cerr << " - next left rake is " << *right << std::endl;
			#endif

			auto temp = std::make_shared<RakeCluster>();

			temp->set_left_child(new_left_foster);
			temp->set_right_child(right);

			// This cluster will need joining:
			temp->correct_endpoints();
			temp->is_splitted = true;
			splitted_clusters.push_back(temp);

			new_left_foster = temp;
		}
	}

	// The same for right nodes
	std::shared_ptr<TopCluster> new_right_foster = NULL;
	if (!right_nodes.empty()) {
		new_right_foster = right_nodes.back();
		right_nodes.pop_back();
		new_right_foster->correct_endpoints();

		#ifdef DEBUG
			std::cerr << "R- First right rake is " << *new_right_foster << std::endl;
		#endif

		while (!right_nodes.empty()) {
			auto left = right_nodes.back();
			right_nodes.pop_back();
			left->correct_endpoints();

			#ifdef DEBUG
				std::cerr << " - next right rake is " << *left << std::endl;
			#endif

			auto temp = std::make_shared<RakeCluster>();

			temp->set_right_child(new_right_foster);
			temp->set_left_child(left);

			// This cluster will need joining:
			temp->correct_endpoints();
			temp->is_splitted = true;
			splitted_clusters.push_back(temp);

			new_right_foster = temp;
		}
	}

	// 4. Connect everything in place
	root->set_left_child(node);
	root->set_left_foster(new_left_foster);
	root->set_right_foster(new_right_foster);
	root->correct_endpoints();
}

// C. Soft expose itself
void TopTree::Internal::soft_expose_handle(std::shared_ptr<TopCluster> N, std::shared_ptr<TopCluster> extern_splay_guard) {
	if (N == NULL) return;

	#ifdef DEBUG
		std::cerr << "Soft expose handle: " << *N << std::endl;
	#endif

	// 0. Normalize from given node
	N->normalize_for_splay();
	#ifdef DEBUG
		for (auto root : root_clusters) {
			std::ostringstream ss;
			ss << "after normalization from " << *N;
			#ifdef DEBUG_GRAPHVIZ
				print_graphviz(root, ss.str());
			#endif
		}
	#endif

	// 1. Splay within each compress and rake subtree in the path from N_w to the root
	auto node = N;
	while (node != NULL) {
		// 1.1 Make node the root of its compress tree (and leaf of a rake tree)
		// Get guard
		auto guard = node->parent;
		auto last = node;
		while (guard != extern_splay_guard && guard != NULL
			&& !guard->isRake()
			&& last != guard->left_foster && last != guard->right_foster
		) {
			last = guard;
			guard = guard->parent;
		}
		// Splay within this compress tree
		guarded_splay(node, guard);

		if (node->parent == NULL) break;
		// 1.2 Splay on node's parent (which is a rake cluster) within rake tree
		auto orig_parent = node->parent;
		if (orig_parent->isRake()) {
			guard = orig_parent->parent;
			while (guard != extern_splay_guard && guard != NULL && guard->isRake()) guard = guard->parent;
			guarded_splay(orig_parent, guard);
		}

		// 1.3 If N have different parent than orig_parent:
		if (node->parent != orig_parent && node->parent != extern_splay_guard) guarded_splay(node->parent, orig_parent);

		// For next run - node in above compress tree under which N is
		if (orig_parent->isRake()) node = orig_parent->parent;
		else node = orig_parent;
	}

	#ifdef DEBUG
		for (auto root : root_clusters) {
			std::ostringstream ss;
			ss << "after splaying from " << *N;
			#ifdef DEBUG_GRAPHVIZ
				print_graphviz(root, ss.str());
			#endif
		}
	#endif

	// 2. Perform a series of splices from N to the root, making N part of the topmost compress subtree
	node = N;
	//while (node->parent != NULL && node->parent != extern_splay_guard) {
	while (node->parent != NULL) {
		splice(node);
		node = node->parent;
	}

	#ifdef DEBUG
		for (auto root : root_clusters) {
			std::ostringstream ss;
			ss << "after splicing from " << *N;
			#ifdef DEBUG_GRAPHVIZ
				print_graphviz(root, ss.str());
			#endif
		}
	#endif

	// 3. Splay N and making it the root of the entire tree
	N->normalize_for_splay();
	guarded_splay(N, extern_splay_guard);

	// 4. Restore all clusters
	for (auto c: splitted_clusters) c->do_join();
}

void TopTree::Internal::soft_expose(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w) {
	// Init array for clusters restoration
	splitted_clusters.clear();

	// A. Making handle of w root node of its top tree
	auto Nw = get_handle(w);
	auto Nv = get_handle(v);
	#ifdef DEBUG
		std::cerr << "Soft expose of " << *v->data << ", " << *w->data << std::endl;
		if (Nv != NULL && Nw != NULL) std::cerr << "with handles: " << *Nv << ", " << *Nw << std::endl;
	#endif
	soft_expose_handle(Nw);

	if (Nw != NULL && w->degree == 1) {
		if (Nw->isBase() && Nw->parent != NULL && Nw == Nw->parent->left_child) Nw->parent->flip();
		if (Nw->isCompress() && (Nw->left_child->boundary_left == w || Nw->left_child->boundary_right == w)) Nw->flip();
	}

	// B. Moving handle of v near the handle of w
	splitted_clusters.clear();
	Nv = get_handle(v); // update to be sure
	if (Nv == Nw || Nv == NULL || (Nw != NULL && v == Nw->boundary_left) || (Nw != NULL && v == Nw->boundary_right)) {
		#ifdef DEBUG
			std::cerr << "Skipping second expose" << std::endl;
		#endif
		// We are done is such situation
	} else {
		if (w->degree == 1) soft_expose_handle(Nv);
		else if (w->degree >= 2) soft_expose_handle(Nv, Nw); // Nw as guard
	}

	// C. Flipping children
	if (Nw != NULL && Nw->isCompress()) {
		if (Nv == Nw->left_child) Nw->flip();
		if ((Nw->left_child->boundary_left == v && Nw->left_child->boundary_right == w)
		|| (Nw->left_child->boundary_left == w && Nw->left_child->boundary_right == v)) Nw->flip();

		// Edge case for foster children
		// 1. If left -> move to right foster
		if (Nw->left_foster != NULL && ((Nw->left_foster->boundary_left == v && Nw->left_foster->boundary_right)
		|| (Nw->left_foster->boundary_left == w && Nw->left_foster->boundary_right == v))) Nw->flip();
		// 2. Swap foster and normal children
		if (Nw->right_foster != NULL && ((Nw->right_foster->boundary_left == v && Nw->right_foster->boundary_right)
		|| (Nw->right_foster->boundary_left == w && Nw->right_foster->boundary_right == v))) {
			Nw->do_split(&splitted_clusters);
			auto temp = Nw->right_foster;
			Nw->right_foster = Nw->right_child;
			Nw->right_child = temp;
			Nw->do_join();
		}
	}
	if (Nv != NULL && Nv->isCompress()) {
		if ((Nv->left_child->boundary_left == v && Nv->left_child->boundary_right == w)
		 || (Nv->left_child->boundary_left == w && Nv->left_child->boundary_right == v)) Nv->flip();
	}

	#ifdef DEBUG
		for (auto root : root_clusters) {
			std::ostringstream ss;
			ss << "after soft expose of " << *v->data << ", " << *w->data;
			#ifdef DEBUG_GRAPHVIZ
				print_graphviz(root, ss.str());
			#endif
		}
	#endif
}

// D. Hard expose
std::shared_ptr<TopCluster> TopTree::Internal::hard_expose(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w) {
	#ifdef DEBUG
		std::cerr << "Hard expose of path " << *v->data << "," << *w->data << std::endl;
	#endif

	auto Nw = get_handle(w);
	auto Nv = get_handle(v);
	if (Nw == NULL || Nv == NULL) return NULL;
	if (Nw != Nv && Nw->parent == NULL && Nv->parent == NULL) return NULL; // They are in different top trees

	// Wanted node is node's child or grandchild -> raking some nodes needed

	auto node = Nw;
	// Node representing v-w path may be child or grandchild of the root --> if so, we need to convert ancestors of this node
	// as rake clusters (they would be restored before next action with the Top Trees structure)
	while ((node->boundary_left != v || node->boundary_right != w) && (node->boundary_left != w || node->boundary_right != v)) {
		hard_expose_transformed_clusters.push_back(std::dynamic_pointer_cast<CompressCluster>(node));
		node = node->right_child;
	}
	// Rakerizing cluster nodes
	for (auto v: hard_expose_transformed_clusters) {
		v->do_split(&splitted_clusters);
		v->rakerized = true;
	}
	for (auto v: hard_expose_transformed_clusters) v->do_join();

	return Nw;
}

// Restore after hard expose
void TopTree::Restore() {
	for (auto v: internal->hard_expose_transformed_clusters) {
		v->do_split();
		v->rakerized = false;
	}
	for (auto v: internal->hard_expose_transformed_clusters) v->do_join();
	internal->hard_expose_transformed_clusters.clear();
}

////////////////////////////////////////////////////////////////////////////////
// Cut and Link

std::tuple<std::shared_ptr<ICluster>, std::shared_ptr<ICluster>, std::shared_ptr<EdgeData>> TopTree::Cut(int v_index, int w_index) {
	// Restore previous hard expose (if needed)
	Restore();

	if (v_index == w_index) {
		std::cerr << "Cannot cut edge from vertex to the same vertex" << std::endl;
		return std::make_tuple((std::shared_ptr<ICluster>)NULL, (std::shared_ptr<ICluster>)NULL, (std::shared_ptr<EdgeData>)NULL);
	}

	// Init array for clusters restoration
	internal->splitted_clusters.clear();

	// 1. Soft expose
	auto v = internal->base_tree->internal->vertices[v_index];
	auto w = internal->base_tree->internal->vertices[w_index];
	#ifdef DEBUG
		std::cerr << "[Cut of " << *v->data << ", " << *w->data << "]" << std::endl;
	#endif
	internal->soft_expose(v, w);
	auto Nw = internal->get_handle(w);
	auto Nv = internal->get_handle(v);

	// 2. Checks
	if (Nv == NULL || Nw == NULL) {
		#ifdef WARNINGS
			std::cerr << "WARNING: One of vertices " << *v << ", " << *w << " is an independent vertex, it cannot be cutted" << std::endl;
		#endif
		return std::make_tuple(Nw, Nv, (std::shared_ptr<EdgeData>)NULL); // They are already in different top trees
	}
	if (Nw != Nv && Nw->parent == NULL && Nv->parent == NULL) {
		#ifdef WARNINGS
			std::cerr << "WARNING: Vertices " << *v << " and " << *w << " are already in different top trees (not connected by edge)" << std::endl;
		#endif
		return std::make_tuple(Nw, Nv, (std::shared_ptr<EdgeData>)NULL); // They are already in different top trees
	}
	// Check if it is really an edge:
	auto node = Nw;
	while ((node->boundary_left != v || node->boundary_right != w) && (node->boundary_left != w || node->boundary_right != v)) {
		if (node->isBase()) {
			#ifdef WARNINGS
				std::cerr << "WARNING: There is no cluster with endpoint " << *v << ", " << *w << " (edge with these endpoints does not exists)" << std::endl;
			#endif
			return std::make_tuple(Nw, Nv, (std::shared_ptr<EdgeData>)NULL);
		}
		node = node->right_child;
	}
	if (!node->isBase()) {
		#ifdef WARNINGS
			std::cerr << "WARNING: Cut - it is not Base cluster: " << *node << std::endl;
		#endif
		return std::make_tuple(Nw, Nv, (std::shared_ptr<EdgeData>)NULL);
	}

	// Variables for new roots
	std::shared_ptr<TopCluster> first = NULL;
	std::shared_ptr<TopCluster> second = NULL;

	// Node representing v-w edge may be child or grandchild of the root --> from every node we came through
	// we remove the right child and find some new to its place
	node = Nw;
	while ((node->boundary_left != v || node->boundary_right != w) && (node->boundary_left != w || node->boundary_right != v)) {
		auto next = node->right_child;
		node->do_split(&internal->splitted_clusters);
		node->parent = NULL;

		if (first == NULL) first = node;
		else if (second == NULL) second = node;

		#ifdef DEBUG
			std::cerr << "Replacing right child of node " << *node << std::endl;
		#endif
		if (node->left_foster != NULL) {
			// Get leftmost node of the rake tree
			auto left_foster = node->left_foster;
			if (!left_foster->isRake()) {
				node->set_right_child(left_foster);
				node->left_foster = NULL;
			} else {
				while (left_foster->isRake()) left_foster = left_foster->left_child;
				left_foster->do_split();
				internal->guarded_splay(left_foster->parent, node);
				//node->left_foster->do_split();
				left_foster = node->left_foster;
				node->set_right_child(left_foster->left_child);
				node->set_left_foster(left_foster->right_child);
				left_foster->parent = NULL;
				left_foster->unregister();
			}
		} else if (node->right_foster != NULL) {
			// Get rightmost node of the rake tree
			auto right_foster = node->right_foster;
			if (!right_foster->isRake()) {
				node->set_right_child(right_foster);
				node->right_foster = NULL;
			} else {
				while (right_foster->isRake()) right_foster = right_foster->right_child;
				right_foster->do_split();
				internal->guarded_splay(right_foster->parent, node);
				//node->right_foster->do_split();
				right_foster = node->right_foster;
				node->set_right_child(right_foster->right_child);
				node->set_right_foster(right_foster->left_child);
				right_foster->parent = NULL;
				right_foster->unregister();
			}
		} else {
			// Left child is the new root and this cluster will be removed

			#ifdef DEBUG
				std::cerr << "... removing node " << *node << std::endl;
			#endif
			node->unregister();
			auto old_node = node;

			if (node == first) {
				// Unregister as root cluster
				internal->root_clusters.erase(node->root_clusters_iterator);
				// Push new root
				node = node->left_child;
				node->parent = NULL;
				internal->root_clusters.push_back(node);
				node->root_clusters_iterator = std::prev(internal->root_clusters.end());
				first = node;
			} else {
				node = node->left_child;
				node->parent = NULL;
				second = node;
			}
		}

		node->do_join();
		node = next;
	}

	// When removing this whole one edge tree
	if (first == NULL) {
		internal->root_clusters.erase(Nw->root_clusters_iterator);
	}
	if (second != NULL) {
		#ifdef DEBUG
			std::cerr << "Adding new root cluster " << *second << std::endl;
		#endif
		internal->root_clusters.push_back(second);
		second->root_clusters_iterator = std::prev(internal->root_clusters.end());
	}

	// Now node should be Base Cluster with edge
	node->do_split(&internal->splitted_clusters);
	auto edge_data = std::dynamic_pointer_cast<BaseCluster>(node)->edge->data;
	// Remove edge from underlying Base tree
	auto baseNode = std::dynamic_pointer_cast<BaseCluster>(node);
	baseNode->unregister();
	// Node will be deleted by garbage collector

	// Restore all clusters
	for (auto c: internal->splitted_clusters) c->do_join();
	internal->splitted_clusters.clear();

	return std::make_tuple(first, second, edge_data);
}

std::shared_ptr<ICluster> TopTree::Link(int v_index, int w_index, std::shared_ptr<EdgeData> edge_data) {
	// Restore previous hard expose (if needed)
	Restore();

	if (v_index == w_index) {
		#ifdef WARNINGS
			std::cerr << "WARNING: Cannot link vertex to itself" << std::endl;
		#endif
		return NULL;
	}

	// Init array for clusters restoration
	internal->splitted_clusters.clear();

	// 0. Get vertices and ensure degree of w is always >= degree of v
	auto v = internal->base_tree->internal->vertices[v_index];
	auto w = internal->base_tree->internal->vertices[w_index];
	if (v->degree > w->degree) {
		auto temp = v;
		v = w;
		w = temp;
	}
	#ifdef DEBUG
		std::cerr << "[Link of " << *v->data << ", " << *w->data << "]" << std::endl;
	#endif

	// 1. Soft expose handles
	internal->soft_expose(v, w);
	auto Nv = internal->get_handle(v);
	auto Nw = internal->get_handle(w);

	// 1.1 Checks
	if (Nv != NULL && Nw != NULL && (Nv == Nw || Nw == Nv->parent)) {
		#ifdef WARNINGS
			std::cerr << "WARNING: Vertices " << *v << " and " << *w << " are already in the same top tree, cannot link" << std::endl;
		#endif
		// std::cerr << *Nw << "---" << *Nv << std::endl;
		return NULL;
	}

	// 2. Make new base cluster
	auto edge = std::make_shared<BaseTree::Internal::Edge>(v, w, edge_data);
	edge->register_at_vertices();
	auto edge_cluster = BaseCluster::construct(edge);

	// 2. If joining solitary nodes return only the new cluster
	if (v->degree == 1 && w->degree == 1) {
		internal->root_clusters.push_back(edge_cluster);
		edge_cluster->root_clusters_iterator = std::prev(internal->root_clusters.end());
		return edge_cluster;
	}

	// 3. Joining normal nodes
	// 3.1 Remove Nv from the root list
	if (Nv != NULL && Nv->parent == NULL) {
		internal->root_clusters.erase(Nv->root_clusters_iterator);
	}

	// 3.2a Manage Nv
	std::shared_ptr<TopCluster> node = edge_cluster;
	if (v->degree == 1) {
		// v was independent node -> do nothing and leave node as edge cluster
	} else if (v->degree == 2) {
		// v had degree 1, it was endpoint of the Nv -> construct new compress node
		node = CompressCluster::construct(Nv, node);
	} else {
		Nv->do_split();
		if (Nv->right_foster == NULL) Nv->right_foster = Nv->right_child;
		else {
			Nv->right_foster = RakeCluster::construct(Nv->right_foster, Nv->right_child);
			Nv->right_foster->parent = Nv;
		}
		Nv->set_right_child(node);
		Nv->do_join();
		node = Nv;
	}
	// 3.2b Manage Nw
	// Nw cannot have degree one (this case where degree of both is one is special case above)
	if (w->degree == 2) {
		// w had degree 1, it was endpoint of the Nw -> construct new compress node
		node = CompressCluster::construct(Nw, node);
		// Nw is not longer root cluster, there is new root cluster
		// -> remove Nw from root clusters and push node there
		internal->root_clusters.erase(Nw->root_clusters_iterator);
		internal->root_clusters.push_back(node);
		node->root_clusters_iterator = std::prev(internal->root_clusters.end());
	} else {
		Nw->do_split();
		if (Nw->right_foster == NULL) Nw->right_foster = Nw->right_child;
		else {
			Nw->right_foster = RakeCluster::construct(Nw->right_foster, Nw->right_child);
			Nw->right_foster->parent = Nw;
		}
		Nw->set_right_child(node);
		Nw->do_join();
		node = Nw;
	}

	// Restore all clusters
	for (auto c: internal->splitted_clusters) c->do_join();

	return node;
}

////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<TopCluster> TopTree::Internal::construct_cluster(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> e) {
	std::queue<std::shared_ptr<TopCluster>> path;
	std::queue<std::shared_ptr<TopCluster>> rake_list;

	auto v_backup = v;

	// #ifdef DEBUG
	// 	std::cerr << "Constructing path from vertex " << *v->data << std::endl;
	// #endif

	auto next_v = v;
	auto next_e = e;

	// A. Find some path, create base clusters from edges of this path,
	// recursively construct all other clusters and rake them onto base clusters
	while (v != NULL) {
		v->used = true;
		next_v = NULL;

		// 1. Construct BaseCluster if there was edge given
		std::shared_ptr<TopCluster> path_cluster = (e != NULL ? BaseCluster::construct(e) : NULL);

		// 2. Select continuation and recursive construct top trees on subtrees
		for (auto n : v->neighbours) {
			if (auto ee = n.edge.lock()) {
				auto vv = ee->from;
				if (vv == v) vv = ee->to;
				if (vv->used) continue;
				if (next_v == NULL) {
					// Use this as continuation of path
					next_v = vv;
					next_e = ee;
				} else {
					// Recursively construct top tree on subtree
					rake_list.push(construct_cluster(vv, ee));
				}
			}
		}

		// 3. Rake all top trees from rake list to rake tree,
		// connect the rake tree as foster child to the path edge
		// and add path edge into path
		if (path_cluster != NULL) {
			// 3.1 Rake all edges into rake tree
			std::queue<std::shared_ptr<TopCluster>> rake_list_new;
			while (rake_list.size() > 1) {
				while (rake_list.size() > 0) {
					if (rake_list.size() == 1) {
						rake_list_new.push(rake_list.front());
						rake_list.pop();
					} else {
						auto first = rake_list.front();
						rake_list.pop();
						auto second = rake_list.front();
						rake_list.pop();
						rake_list_new.push(RakeCluster::construct(first, second));
					}
				}
				rake_list.swap(rake_list_new);
			}
			// 3.2 Prepare rake tree and save it into the vertex
			if (!rake_list.empty()) {
				auto rake_tree = rake_list.front();
				rake_list.pop();

				// Only use one side (left)
				v->rake_tree_left = rake_tree;
			}
			// 3.3 Push cluster with edge into path
			path.push(path_cluster);
		}

		// 4. Move to the next vertex on path
		v = next_v;
		e = next_e;
	}

	// B. Compress all clusters into one compress cluster
	std::queue<std::shared_ptr<TopCluster>> path_new;
	while (path.size() > 1) {
		while (path.size() > 0) {
			if (path.size() == 1) {
				path_new.push(path.front());
				path.pop();
			} else {
				auto left = path.front();
				path.pop();
				auto right = path.front();
				path.pop();

				// Construct compress cluster and push it into the path
				path_new.push(CompressCluster::construct(left, right));
			}
		}
		path.swap(path_new);
	}

	// #ifdef DEBUG
	// 	std::cerr << "Construction of path from vertex " << *v_backup->data << " ended" << std::endl;
	// #endif

	return path.front();
}


TopTree::~TopTree() {}
}
