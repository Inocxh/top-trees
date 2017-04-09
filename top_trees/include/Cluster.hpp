#include <memory>

#ifndef CLUSTER_HPP
#define CLUSTER_HPP

namespace TopTree {

template <class TData>
class Cluster {
public:
	int edge_vertex_a;
	int edge_vertex_b;

	std::shared_ptr<TData> data;
};


}

template <class TData>
class BaseCluster : public Cluster<TData> {
public:

};

template <class TData>
class CompressCluster : public Cluster<TData> {
public:

};

template <class TData>
class RakeCluster : public Cluster<TData> {
public:

};

#endif // CLUSTER_HPP
