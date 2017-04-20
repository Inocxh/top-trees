#include <memory>
#include <iostream>

#ifndef BASE_TREE_HPP
#define BASE_TREE_HPP

namespace TopTree {

//------------------------------------------------------------------------------

class VertexData {
public:
	virtual std::ostream& ToString(std::ostream& o) const { return o << "V"; }
};
std::ostream& operator<<(std::ostream& o, const VertexData& v);

//------------------------------------------------------------------------------

class EdgeData {
public:
	virtual std::ostream& ToString(std::ostream& o) const { return o; }
};
std::ostream& operator<<(std::ostream& o, const EdgeData& e);

//------------------------------------------------------------------------------

class BaseTree {
public:
	BaseTree();
	~BaseTree();

	// Adding basic objects
	int AddVertex(std::shared_ptr<VertexData> v = NULL);
	int AddEdge(int a, int b, std::shared_ptr<EdgeData> e = NULL);

	// Shortcut for AddVertex and AddEdge, returns index of the vertex
	int AddLeaf(int parent, std::shared_ptr<EdgeData> e = NULL, std::shared_ptr<VertexData> v = NULL);

	// Getters
	std::shared_ptr<VertexData> GetVertexData(int index);
	std::shared_ptr<EdgeData> GetEdgeData(int index);

	// Testing functions
	void PrintRooted(int root);
private:
	class BaseTreeData;
	std::unique_ptr<BaseTreeData> data;
};

//------------------------------------------------------------------------------

}
#endif // BASE_TREE_HPP
