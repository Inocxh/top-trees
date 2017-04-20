#include <iostream>
#include <memory>

#include "TopTree.hpp"

struct EdgeValues {
	int weight;
};

class MyTopTree : public TopTree::AbstractTopTree<EdgeValues> {
public:
	virtual void Join(std::shared_ptr<EdgeValues> leftChild, std::shared_ptr<EdgeValues> rightChild, std::shared_ptr<EdgeValues> parent) {
		parent->weight = leftChild->weight + rightChild->weight;
	}
	virtual void Split(std::shared_ptr<EdgeValues> leftChild, std::shared_ptr<EdgeValues> rightChild, std::shared_ptr<EdgeValues> parent) {
		// Nothing
	}

	// Creating and destroying Base clusters:
	virtual void Create(std::shared_ptr<EdgeValues> edgeData) {
		edgeData->weight = 10;
	}
	virtual void Destroy(std::shared_ptr<EdgeValues> edgeData) {
		// Nothing
	}
};

class MyVertexData: public TopTree::VertexData {
public:
	MyVertexData(std::string label): label{label} {}
	std::string label;

	virtual std::ostream& ToString(std::ostream& o) const {
		return o << "Vertex " << label;
	}
};

class MyEdgeData: public TopTree::EdgeData {
public:
	MyEdgeData(std::string label): label{label} {}
	std::string label;

	virtual std::ostream& ToString(std::ostream& o) const {
		return o << "Edge " << label;
	}
};

int main(int argc, char const *argv[]) {
	/*
	std::shared_ptr<EdgeValues> p1(new EdgeValues);
	std::shared_ptr<EdgeValues> p2(new EdgeValues);
	std::shared_ptr<EdgeValues> p3(new EdgeValues);
	auto T = MyTopTree();
	T.Create(p1);
	T.Create(p2);
	T.Join(p1,p2,p3);
	std::cout << p3->weight << std::endl;
	*/

	auto baseTree = new TopTree::BaseTree();

	// Example from article:
	auto z = baseTree->AddVertex(std::make_shared<MyVertexData>("z"));
	auto p = baseTree->AddLeaf(z, NULL, std::make_shared<MyVertexData>("p"));
	auto w = baseTree->AddLeaf(p, NULL, std::make_shared<MyVertexData>("w"));
	auto c = baseTree->AddLeaf(w, NULL, std::make_shared<MyVertexData>("c"));
	auto b = baseTree->AddLeaf(c, NULL, std::make_shared<MyVertexData>("b"));
	baseTree->AddLeaf(b, NULL, std::make_shared<MyVertexData>("a"));

	baseTree->AddLeaf(b, NULL, std::make_shared<MyVertexData>("t"));

	auto d = baseTree->AddLeaf(c, NULL, std::make_shared<MyVertexData>("d"));
	baseTree->AddLeaf(d, NULL, std::make_shared<MyVertexData>("r"));

	auto e = baseTree->AddLeaf(c, NULL, std::make_shared<MyVertexData>("e"));
	baseTree->AddLeaf(e, NULL, std::make_shared<MyVertexData>("q"));

	baseTree->AddLeaf(e, NULL, std::make_shared<MyVertexData>("s"));

	baseTree->AddLeaf(c, NULL, std::make_shared<MyVertexData>("f"));

	auto g = baseTree->AddLeaf(c, NULL, std::make_shared<MyVertexData>("g"));
	baseTree->AddLeaf(g, NULL, std::make_shared<MyVertexData>("j"));

	baseTree->AddLeaf(g, NULL, std::make_shared<MyVertexData>("h"));

	auto n = baseTree->AddLeaf(c, NULL, std::make_shared<MyVertexData>("n"));
	baseTree->AddLeaf(n, NULL, std::make_shared<MyVertexData>("k"));

	auto v = baseTree->AddLeaf(n, NULL, std::make_shared<MyVertexData>("v"));
	baseTree->AddLeaf(v, NULL, std::make_shared<MyVertexData>("u"));

	baseTree->AddLeaf(v, NULL, std::make_shared<MyVertexData>("l"));

	baseTree->AddLeaf(n, NULL, std::make_shared<MyVertexData>("i"));

	baseTree->AddLeaf(w, NULL, std::make_shared<MyVertexData>("y"));

	baseTree->AddLeaf(w, NULL, std::make_shared<MyVertexData>("o"));

	baseTree->AddLeaf(p, NULL, std::make_shared<MyVertexData>("x"));

	baseTree->PrintRooted(z);

	delete baseTree;
	return 0;
}
