#include <memory>

#ifndef CLUSTER_HPP
#define CLUSTER_HPP

namespace TopTree {
struct ClusterData {
	virtual ~ClusterData() {}
};
}

#include "BaseTreeInternal.hpp"
#include "UserFunctions.hpp"

namespace TopTree {

class Cluster {
public:
	std::shared_ptr<BaseTree::Internal::Vertex> boundary_a;
	std::shared_ptr<BaseTree::Internal::Vertex> boundary_b;

	std::shared_ptr<Cluster> left_child = NULL;
	std::shared_ptr<Cluster> right_child = NULL;
	std::shared_ptr<Cluster> left_foster = NULL;
	std::shared_ptr<Cluster> right_foster = NULL;

	std::shared_ptr<ClusterData> data = InitClusterData();

	virtual std::ostream& ToString(std::ostream& o) const = 0;
};
std::ostream& operator<<(std::ostream& o, const Cluster& v);

class BaseCluster : public Cluster {
public:
	BaseCluster(std::shared_ptr<BaseTree::Internal::Edge> edge);
	virtual std::ostream& ToString(std::ostream& o) const;
};

class CompressCluster : public Cluster {
public:
	CompressCluster(std::shared_ptr<Cluster> left, std::shared_ptr<Cluster> right);
	virtual std::ostream& ToString(std::ostream& o) const;
};

class RakeCluster : public Cluster {
public:
	RakeCluster(std::shared_ptr<Cluster> rake_from, std::shared_ptr<Cluster> rake_to);
	virtual std::ostream& ToString(std::ostream& o) const;
};

}

#endif // CLUSTER_HPP
