#include <memory>

#ifndef CLUSTER_HPP
#define CLUSTER_HPP

namespace TopTree {
struct ClusterData {
	virtual ~ClusterData() {}
};

class Cluster;
}

#include "BaseTreeInternal.hpp"
#include "UserFunctions.hpp"

namespace TopTree {

class Cluster {
public:
	std::shared_ptr<BaseTree::Internal::Vertex> boundary_a;
	std::shared_ptr<BaseTree::Internal::Vertex> boundary_b;

	std::shared_ptr<Cluster> parent = NULL;
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
	static std::shared_ptr<BaseCluster> construct(std::shared_ptr<BaseTree::Internal::Edge> edge);

	virtual std::ostream& ToString(std::ostream& o) const;
};

class CompressCluster : public Cluster {
public:
	static std::shared_ptr<CompressCluster> construct(std::shared_ptr<Cluster> left, std::shared_ptr<Cluster> right);

	std::shared_ptr<BaseTree::Internal::Vertex> common_vertex;

	virtual std::ostream& ToString(std::ostream& o) const;
};

class RakeCluster : public Cluster {
public:
	static std::shared_ptr<RakeCluster> construct(std::shared_ptr<Cluster> rake_from, std::shared_ptr<Cluster> rake_to);

	virtual std::ostream& ToString(std::ostream& o) const;
};

}

#endif // CLUSTER_HPP
