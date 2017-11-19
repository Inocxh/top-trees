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
friend class TopTree;
friend class BaseCluster;
friend class CompressCluster;
friend class RakeCluster;
public:
	std::shared_ptr<ClusterData> data = InitClusterData();
	virtual std::ostream& ToString(std::ostream& o) const = 0;
protected:
	std::shared_ptr<BaseTree::Internal::Vertex> boundary_left;
	std::shared_ptr<BaseTree::Internal::Vertex> boundary_right;

	std::shared_ptr<Cluster> parent = NULL;
	std::shared_ptr<Cluster> left_child = NULL;
	std::shared_ptr<Cluster> right_child = NULL;
	std::shared_ptr<Cluster> left_foster = NULL;
	std::shared_ptr<Cluster> right_foster = NULL;

	int root_vector_index = 0;

	virtual bool isBase() { return false; }
	virtual bool isCompress() { return false; }
	virtual bool isRake() { return false; }

	virtual void flip() = 0;
	virtual void normalize() = 0;

	virtual std::ostream& _short_name(std::ostream& o) const = 0; // Used only for debugging
};
std::ostream& operator<<(std::ostream& o, const Cluster& v);

class BaseCluster : public Cluster {
friend class TopTree;
public:
	virtual std::ostream& ToString(std::ostream& o) const;
protected:
	static std::shared_ptr<BaseCluster> construct(std::shared_ptr<BaseTree::Internal::Edge> edge);

	virtual bool isBase() { return true; }
	virtual void flip();
	virtual void normalize();

	virtual std::ostream& _short_name(std::ostream& o) const; // Used only for debugging
};

class CompressCluster : public Cluster {
friend class TopTree;
public:
	virtual std::ostream& ToString(std::ostream& o) const;
protected:
	static std::shared_ptr<CompressCluster> construct(std::shared_ptr<Cluster> left, std::shared_ptr<Cluster> right);

	virtual bool isCompress() { return true; }
	virtual void flip();
	virtual void normalize();

	virtual std::ostream& _short_name(std::ostream& o) const; // Used only for debugging

	std::shared_ptr<BaseTree::Internal::Vertex> common_vertex;
};

class RakeCluster : public Cluster {
friend class TopTree;
public:
	virtual std::ostream& ToString(std::ostream& o) const;
protected:
	static std::shared_ptr<RakeCluster> construct(std::shared_ptr<Cluster> rake_from, std::shared_ptr<Cluster> rake_to);

	virtual bool isRake() { return true; }
	virtual void flip();
	virtual void normalize();

	virtual std::ostream& _short_name(std::ostream& o) const; // Used only for debugging
};

}

#endif // CLUSTER_HPP
