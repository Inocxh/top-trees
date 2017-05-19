#include <queue>

#include "TopTree.hpp"
#include "BaseTreeInternal.hpp"
#include "Cluster.hpp"

namespace TopTree {

// Hide data from .hpp file using PIMP idiom
class TopTree::Internal {
public:
	std::shared_ptr<Cluster> construct_cluster(std::shared_ptr<BaseTree::Internal::Vertex> v, std::shared_ptr<BaseTree::Internal::Edge> e=NULL);
	// TODO
};

////////////////////////////////////////////////////////////////////////////////

TopTree::TopTree() : internal{std::make_unique<Internal>()} {}

TopTree::TopTree(std::shared_ptr<BaseTree> baseTree) : TopTree() {
	for (auto v : baseTree->internal->vertices) v->used = false;

	for (auto v : baseTree->internal->vertices) {
		if (v->used || v->degree != 1) continue;
		std::cout << "Pokus" << std::endl;
		std::cout << internal->construct_cluster(v);
	}


	// TODO - rozlozit na cesty a rekurzivne postavit clustery
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
		std::shared_ptr<Cluster> path_cluster = (e != NULL ? std::make_shared<BaseCluster>(e) : NULL);

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

		// 3. Rake all top trees from rake list, rake them onto given edge
		// and then add edge into path
		if (path_cluster != NULL) {
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
						rake_list_new.push(std::make_shared<RakeCluster>(first, second));
					}
				}
				rake_list.swap(rake_list_new);
			}
			// Rake onto given edge
			if (!rake_list.empty()) {
				auto rake_tree = rake_list.front();

				path_cluster = std::make_shared<RakeCluster>(rake_tree, path_cluster);
			}
			// Push cluster with edge into path
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
				path_new.push(std::make_shared<CompressCluster>(left, right));
			}
		}
		path.swap(path_new);
	}

	return path.front();
}


TopTree::~TopTree() {}


}
