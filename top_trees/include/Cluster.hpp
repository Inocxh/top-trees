#include <memory>

#ifndef CLUSTER_HPP
#define CLUSTER_HPP

namespace TopTree {
class Cluster;
}

#include "BaseTreeInternal.hpp"
#include "UserFunctions.hpp"

namespace TopTree {

class Cluster : public std::enable_shared_from_this<Cluster> {
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
	std::shared_ptr<BaseTree::Internal::Vertex> common_vertex;

	std::shared_ptr<Cluster> parent = NULL;
	std::shared_ptr<Cluster> left_child = NULL;
	std::shared_ptr<Cluster> right_child = NULL;
	std::shared_ptr<Cluster> left_foster = NULL;
	std::shared_ptr<Cluster> right_foster = NULL;

	void set_left_child(std::shared_ptr<Cluster> child);
	void set_right_child(std::shared_ptr<Cluster> child);
	void set_left_foster(std::shared_ptr<Cluster> child);
	void set_right_foster(std::shared_ptr<Cluster> child);

	int root_vector_index = -1;
	bool is_splitted = true; // Initially clusters are in state that they need do_join method (which is called during construction)

	virtual bool isBase() { return false; }
	virtual bool isCompress() { return false; }
	virtual bool isRake() { return false; }

	virtual void do_join() = 0;
	virtual void do_split(std::vector<std::shared_ptr<Cluster>>* splitted_clusters = NULL) = 0;
	virtual void correct_endpoints() = 0;
	virtual void flip() = 0;
	virtual void normalize_for_splay() = 0;

	virtual std::ostream& _short_name(std::ostream& o) const = 0; // Used only for debugging
};
std::ostream& operator<<(std::ostream& o, const Cluster& v);

class BaseCluster : public Cluster {
friend class TopTree;
public:
	virtual std::ostream& ToString(std::ostream& o) const;
protected:
	static std::shared_ptr<BaseCluster> construct(std::shared_ptr<BaseTree::Internal::Edge> edge);

	std::shared_ptr<BaseTree::Internal::Edge> edge;

	virtual bool isBase() { return true; }
	virtual void do_join();
	virtual void do_split(std::vector<std::shared_ptr<Cluster>>* splitted_clusters = NULL);
	virtual void correct_endpoints() {};
	virtual void flip();
	virtual void normalize_for_splay();

	virtual std::ostream& _short_name(std::ostream& o) const; // Used only for debugging
};

class CompressCluster : public Cluster {
friend class TopTree;
public:
	virtual std::ostream& ToString(std::ostream& o) const;
protected:
	static std::shared_ptr<CompressCluster> construct(std::shared_ptr<Cluster> left, std::shared_ptr<Cluster> right);

	virtual bool isCompress() { return !rakerized; }
	virtual bool isRake() { return rakerized; }
	virtual void do_join();
	virtual void do_split(std::vector<std::shared_ptr<Cluster>>* splitted_clusters = NULL);
	virtual void correct_endpoints();
	virtual void flip();
	virtual void normalize_for_splay();

	virtual std::ostream& _short_name(std::ostream& o) const; // Used only for debugging

	bool rakerized = false; // modyfied to rake node during hard_expose
};

class RakeCluster : public Cluster {
friend class TopTree;
public:
	virtual std::ostream& ToString(std::ostream& o) const;
protected:
	static std::shared_ptr<RakeCluster> construct(std::shared_ptr<Cluster> rake_from, std::shared_ptr<Cluster> rake_to);

	virtual bool isRake() { return true; }
	virtual void do_join();
	virtual void do_split(std::vector<std::shared_ptr<Cluster>>* splitted_clusters = NULL);
	virtual void correct_endpoints();
	virtual void flip();
	virtual void normalize_for_splay();

	virtual std::ostream& _short_name(std::ostream& o) const; // Used only for debugging
};

}

#endif // CLUSTER_HPP
