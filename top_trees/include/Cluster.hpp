#include <memory>

#include "BaseTreeInternal.hpp"
#include "TopTree.hpp"

#ifndef CLUSTER_HPP
#define CLUSTER_HPP

namespace TopTree {

class Cluster {
public:
	std::shared_ptr<BaseTree::Internal::Vertex> boundary_a;
	std::shared_ptr<BaseTree::Internal::Vertex> boundary_b;

	std::shared_ptr<ClusterData> data = InitClusterData();
};

class BaseCluster : public Cluster {
public:
	BaseCluster(std::shared_ptr<BaseTree::Internal::Edge> edge);
};

class CompressCluster : public Cluster {
public:
	CompressCluster(std::shared_ptr<Cluster> left, std::shared_ptr<Cluster> right);
};

class RakeCluster : public Cluster {
public:
	RakeCluster(std::shared_ptr<Cluster> rake_from, std::shared_ptr<Cluster> rake_to);
};

}

#endif // CLUSTER_HPP
