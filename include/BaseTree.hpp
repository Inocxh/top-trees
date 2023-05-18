#include <memory>
#include <iostream>

#ifndef BASE_TREE_HPP
#define BASE_TREE_HPP

namespace TopTree {

//------------------------------------------------------------------------------

class VertexData {
static int v_counter;
public:
	int index = VertexData::v_counter++;
	virtual std::ostream& ToString(std::ostream& o) const { return o << "<v" << index << ">"; }
};
std::ostream& operator<<(std::ostream& o, const VertexData& v);

//------------------------------------------------------------------------------

class EdgeData {
static int e_counter;
public:
	int index = EdgeData::e_counter++;
	virtual std::ostream& ToString(std::ostream& o) const { return o << "<e" << index << ">"; }
};
std::ostream& operator<<(std::ostream& o, const EdgeData& e);

//------------------------------------------------------------------------------

class BaseTree {
friend class ICluster;
friend class STTopTree;
friend class STCluster;
friend class BaseCluster;
friend class CompressCluster;
friend class RakeCluster;
friend class TopologyTopTree;
friend class TopologyCluster;
friend class SimpleCluster;
public:
	BaseTree();
	~BaseTree();

	// Adding basic objects
	int AddVertex(std::shared_ptr<VertexData> v = NULL);
	int AddEdge(int from, int to, std::shared_ptr<EdgeData> e = NULL);

	// Shortcut for AddVertex and AddEdge, returns index of the vertex
	int AddLeaf(int parent, std::shared_ptr<EdgeData> e = NULL, std::shared_ptr<VertexData> v = NULL);

	// Getters
	std::shared_ptr<VertexData> GetVertexData(int index);
	std::shared_ptr<EdgeData> GetEdgeData(int index);

	// Testing functions
	void PrintRooted(int root);
private:
	class Internal;
	std::unique_ptr<Internal> internal;
};

//------------------------------------------------------------------------------

}
#endif // BASE_TREE_HPP
