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

int main(int argc, char const *argv[]) {
	std::cout << "Test" << std::endl;

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
	auto a = baseTree->AddVertex();
	auto b = baseTree->AddVertex();
	baseTree->AddEdge(a, b);

	delete baseTree;
	return 0;
}
