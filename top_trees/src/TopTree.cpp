#include <queue>
#include <vector>

#include "TopTree.hpp"
#include "BaseTreeInternal.hpp"
#include "Cluster.hpp"

namespace TopTree {

// Hide data from .hpp file using PIMP idiom
class TopTree::Internal {
public:
	std::vector<std::shared_ptr<Cluster> > root_clusters;
	std::shared_ptr<BaseTree> base_tree;

	std::shared_ptr<Cluster> construct_cluster(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> e=NULL);

	void soft_expose(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w);

	// Debug methods:
	void print_rooted_prefix(const std::shared_ptr<Cluster> cluster, const std::string prefix = "", bool last_child = true) const;
	void print_graphviz_recursive(std::shared_ptr<Cluster> parent, std::shared_ptr<Cluster> node) const;
	void print_graphviz_child(std::shared_ptr<Cluster> from, std::shared_ptr<Cluster> to) const;
private:
	void guarded_splay(std::shared_ptr<Cluster> node, std::shared_ptr<Cluster> guard = NULL);
	void adjust_parent(std::shared_ptr<Cluster> parent, std::shared_ptr<Cluster> old_child, std::shared_ptr<Cluster> new_child);
	void rotate_left(std::shared_ptr<Cluster> x);
	void rotate_right(std::shared_ptr<Cluster> x);

	void splice(std::shared_ptr<Cluster> node);

	std::vector<std::shared_ptr<Cluster>> splitted_clusters;
};

////////////////////////////////////////////////////////////////////////////////

TopTree::TopTree() : internal{std::make_unique<Internal>()} {}

TopTree::TopTree(std::shared_ptr<BaseTree> from_base_tree) : TopTree() {
	internal->base_tree = from_base_tree;

	for (auto v : internal->base_tree->internal->vertices) v->used = false;

	int i = 0;
	for (auto v : internal->base_tree->internal->vertices) {
		if (v->used || v->degree != 1) continue;
		internal->root_clusters.push_back(internal->construct_cluster(v));
		internal->root_clusters[i]->root_vector_index = i;
		i++;
	}
}

std::vector<std::shared_ptr<Cluster> > TopTree::GetTopTrees() {
	return internal->root_clusters;
}

////////////////////////////////////////////////////////////////////////////////
// Debug output - console

void TopTree::Internal::print_rooted_prefix(const std::shared_ptr<Cluster> cluster, const std::string prefix, bool last_child) const {
	std::cout << *cluster << std::endl;
	auto prefix_child = prefix + (last_child ? "   " : "|  ");

	if (cluster->left_foster != NULL) {
		std::cout << prefix_child << "|-Foster left: ";
		print_rooted_prefix(cluster->left_foster, prefix_child, false);
	}
	if (cluster->left_child != NULL) {
		std::cout << prefix_child << "|-Left: ";
		print_rooted_prefix(cluster->left_child, prefix_child, false);
	}
	if (cluster->right_child != NULL) {
		std::cout << prefix_child << "|-Right: ";
		print_rooted_prefix(cluster->right_child, prefix_child, (cluster->right_foster == NULL));
	}
	if (cluster->right_foster != NULL) {
		std::cout << prefix_child << "|-Foster right: ";
		print_rooted_prefix(cluster->right_foster, prefix_child, true);
	}
}

void TopTree::PrintRooted(const std::shared_ptr<Cluster> root) const {
	internal->print_rooted_prefix(root);

	for (auto v: internal->base_tree->internal->vertices) {
		if (v->handle != NULL)
			std::cout << *v->data << ": " << *v->handle << std::endl;
		else std::cout << *v->data << ": NONE" << std::endl;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Debug outpu - Graphviz

void TopTree::Internal::print_graphviz_child(std::shared_ptr<Cluster> from, std::shared_ptr<Cluster> to) const {
	to->_short_name(std::cout << "\t\"" << to << "\" [label=\"") << "\",shape=";
	if (to->isCompress()) std::cout << "box";
	else if (to->isRake()) std::cout << "diamond";
	else std::cout << "circle";
	std::cout << "]" << std::endl;

	if (from == NULL) return;
	std::cout << "\t\"" << from << "\" -> \"" << to << "\"";
	if (to == from->left_foster || to == from->right_foster) std::cout << " [style=dashed]";
	std::cout << std::endl;
}

void TopTree::Internal::print_graphviz_recursive(std::shared_ptr<Cluster> parent, std::shared_ptr<Cluster> node) const {
	if (node == NULL) return;
	print_graphviz_child(parent, node);

	print_graphviz_recursive(node, node->left_foster);
	print_graphviz_recursive(node, node->left_child);
	print_graphviz_recursive(node, node->right_child);
	print_graphviz_recursive(node, node->right_foster);
}

void TopTree::PrintGraphviz(const std::shared_ptr<Cluster> root) const {
	std::cout << "digraph \"" << root << "\" {" << std::endl;
	internal->print_graphviz_recursive(NULL, root);
	std::cout << "}" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// Soft expose related functions

void TopTree::Expose(int v, int w) {
	auto vertexV = internal->base_tree->internal->vertices[v];
	auto vertexW = internal->base_tree->internal->vertices[w];

	internal->soft_expose(vertexV, vertexW);
}

// void TopTree::Internal::fix_endpoints(std::shared_ptr<Cluster> node) {
//
// }

// A. Splaying
void TopTree::Internal::adjust_parent(std::shared_ptr<Cluster> parent, std::shared_ptr<Cluster> old_child, std::shared_ptr<Cluster> new_child) {
	// Ensure that both childs are splitted before any action
	new_child->do_split(&splitted_clusters);
	old_child->do_split(&splitted_clusters);

	if (parent != NULL) {
		if (parent->left_child == old_child) parent->left_child = new_child;
		else if (parent->right_child == old_child) parent->right_child = new_child;
		else if (parent->left_foster == old_child) parent->left_foster = new_child;
		else if (parent->right_foster == old_child) parent->right_foster = new_child;
		else exit(1); // Something bad happens
	} else {
		// x was one of the roots, replace it in place in the vector
		new_child->root_vector_index = old_child->root_vector_index;
		root_clusters[old_child->root_vector_index] = new_child;
	}
}

//     x               y
//   A   y    ->    x    C
//      B C        A B
void TopTree::Internal::rotate_left(std::shared_ptr<Cluster> x) {
	auto parent = x->parent;
	auto y = x->right_child;

	adjust_parent(parent, x, y);

	// Adjust x:
	x->right_child = y->left_child;
	x->parent = y;

	// Adjust y:
	y->left_child = x;
	y->parent = parent;
}

//     x               y
//   y   C    ->    A    x
//  A B                 B C
void TopTree::Internal::rotate_right(std::shared_ptr<Cluster> x) {
	auto parent = x->parent;
	auto y = x->left_child;

	adjust_parent(parent, x, y);

	// Adjust x:
	x->left_child = y->right_child;
	x->parent = y;

	// Adjust y:
	y->right_child = x;
	y->parent = parent;
}

void TopTree::Internal::guarded_splay(std::shared_ptr<Cluster> node, std::shared_ptr<Cluster> guard) {
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
void TopTree::Internal::splice(std::shared_ptr<Cluster> node) {
	auto left_nodes = std::vector<std::shared_ptr<Cluster>>();
	auto right_nodes = std::vector<std::shared_ptr<Cluster>>();

	// 1. Go up to the root of a compress tree and split other nodes to left
	// and right siblings
	auto root = node->parent;
	auto current = node;
	while (root->isRake()) {
		// The most generic variant is when there are two rake nodes to the nearest
		// root of a compress tree
		if (current == root->left_child) right_nodes.push_back(root->right_child);
		else left_nodes.push_back(root->left_child);

		// Inner clusters will be deleted by garbage collection after
		current = root;
		root = root->parent;
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
	std::shared_ptr<Cluster> new_left_foster = NULL;
	if (!left_nodes.empty()) {
		new_left_foster = left_nodes.back();
		left_nodes.pop_back();

		while (!left_nodes.empty()) {
			auto right = left_nodes.back();
			left_nodes.pop_back();

			auto temp = std::make_shared<RakeCluster>();

			temp->left_child = new_left_foster;
			new_left_foster->parent = temp;
			temp->right_child = right;
			right->parent = temp;

			// This cluster will need joining:
			splitted_clusters.push_back(temp);

			new_left_foster = temp;
		}
	}
	// The same for right nodes
	std::shared_ptr<Cluster> new_right_foster = NULL;
	if (!right_nodes.empty()) {
		new_right_foster = right_nodes.back();
		right_nodes.pop_back();

		while (!right_nodes.empty()) {
			auto left = right_nodes.back();
			right_nodes.pop_back();

			auto temp = std::make_shared<RakeCluster>();

			if (new_right_foster == NULL) std::cerr << "What?" << std::endl;

			temp->right_child = new_right_foster;
			new_right_foster->parent = temp;
			temp->left_child = left;
			left->parent = temp;

			// This cluster will need joining:
			splitted_clusters.push_back(temp);

			new_right_foster = temp;
		}
	}


	// 4. Connect everything in place
	root->left_child = node;
	node->parent = root;
	root->left_foster = new_left_foster;
	if (new_left_foster != NULL) new_left_foster->parent = root;
	root->right_foster = new_right_foster;
	if (new_right_foster != NULL) new_right_foster->parent = root;
}

// C. Soft expose itself
void TopTree::Internal::soft_expose(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w) {
	// Init array for clusters restoration
	splitted_clusters = std::vector<std::shared_ptr<Cluster>>();

	// A. Making handle of w root node of its top tree

	auto Nw = w->handle;
	Nw->normalize();

	// 1. Splay within each compress and rake subtree in the path from N_w to the root
	auto node = Nw;
	while (node != NULL) {
		// 1.1 Make node N root of its compress tree (and leaf of a rake tree)
		// Get guard
		auto guard = node->parent;
		while (guard != NULL && !guard->isRake()) guard = guard->parent;
		// Splay within this compress tree
		guarded_splay(node, guard);

		if (node->parent == NULL) break;
		// 1.2 Splay on N's parent (which is a rake cluster) within rake tree
		auto orig_parent = node->parent;
		if (orig_parent->isRake()) {
			guard = orig_parent->parent;
			while (guard != NULL && guard->isRake()) guard = guard->parent;
			guarded_splay(orig_parent, guard);
		}

		// 1.3 If N_w have different parent than orig_parent:
		if (node->parent != orig_parent) guarded_splay(node->parent, orig_parent);

		// For next run - node in above compress tree under which Nw is
		node = orig_parent -> parent;
	}

	for (auto root : root_clusters) {
		std::cout << "digraph \"" << root << "\" {" << std::endl;
		print_graphviz_recursive(NULL, root);
		std::cout << "}" << std::endl;
	}

	// 2. Perform a series of splices from N_w to the root, making N_w part of the topmost compress subtree
	node = Nw;
	while (node->parent != NULL) {
		splice(node);
		node = node->parent;
	}

	// 3. Splay Nw and making it the root of the entire tree
	guarded_splay(Nw);

	// 4. Restore all clusters
	for (auto c: splitted_clusters) c->do_join();
}

////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Cluster> TopTree::Internal::construct_cluster(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> e) {
	std::queue<std::shared_ptr<Cluster> > path;
	std::queue<std::shared_ptr<Cluster> > rake_list;

	auto next_v = v;
	auto next_e = e;

	// A. Find some path, create base clusters from edges of this path,
	// recursively construct all other clusters and rake them onto base clusters
	while (v != NULL) {
		v->used = true;
		next_v = NULL;

		// 1. Construct BaseCluster if there was edge given
		std::shared_ptr<Cluster> path_cluster = (e != NULL ? BaseCluster::construct(e) : NULL);

		// 2. Select continuation and recursive construct top trees on subtrees
		for (auto n : v->neighbours) {
			if (auto vv = n.vertex.lock()) if (auto ee = n.edge.lock()) {
				if (vv->used) continue;
				if (next_v == NULL) {
					// Use this as continuation of path
					next_v = vv;
					next_e = ee;
				} else {
					// Recursively consruct top tree on subtree
					rake_list.push(construct_cluster(vv, ee));
				}
			}
		}

		// 3. Rake all top trees from rake list to rake tree,
		// connect the rake tree as foster child to the path edge
		// and add path edge into path
		if (path_cluster != NULL) {
			// 3.1 Rake all edges into rake tree
			std::queue<std::shared_ptr<Cluster> > rake_list_new;
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
	std::queue<std::shared_ptr<Cluster> > path_new;
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

	return path.front();
}


TopTree::~TopTree() {}
}
