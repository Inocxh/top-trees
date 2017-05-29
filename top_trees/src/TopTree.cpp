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

	void print_rooted_prefix(const std::shared_ptr<Cluster> cluster, const std::string prefix = "", bool last_child = true) const;
};

////////////////////////////////////////////////////////////////////////////////

TopTree::TopTree() : internal{std::make_unique<Internal>()} {}

TopTree::TopTree(std::shared_ptr<BaseTree> from_base_tree) : TopTree() {
	internal->base_tree = from_base_tree;

	for (auto v : internal->base_tree->internal->vertices) v->used = false;

	for (auto v : internal->base_tree->internal->vertices) {
		if (v->used || v->degree != 1) continue;
		internal->root_clusters.push_back(internal->construct_cluster(v));
	}
}

std::vector<std::shared_ptr<Cluster> > TopTree::GetTopTrees() {
	return internal->root_clusters;
}

void TopTree::PrintRooted(const std::shared_ptr<Cluster> root) {
	internal->print_rooted_prefix(root);

	for (auto v: internal->base_tree->internal->vertices) {
		if (v->handle != NULL)
			std::cout << *v->data << ": " << *v->handle << std::endl;
		else std::cout << *v->data << ": NONE" << std::endl;
	}
}

////////////////////////////////////////////////////////////////////////////////

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
