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
	// Get root of above compress tree
	if (node->parent->isRake() && node->parent->parent->isRake()) {
		auto root = node->parent->parent->parent;
	}
}

// C. Soft expose itself
void TopTree::Internal::soft_expose(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Vertex> w) {
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

	// 2. Perform a series of splices from N_w to the root, making N_w part of the topmost compress subtree

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
