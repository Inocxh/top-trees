#include <iostream>
#include <memory>

#include "TopTree.hpp"

struct MyClusterData: public TopTree::ClusterData {
	int weight;
};

class MyVertexData: public TopTree::VertexData {
public:
	MyVertexData(std::string label): label{label} {}
	std::string label;

	virtual std::ostream& ToString(std::ostream& o) const {
		//return o << "Vertex " << label;
		return o << label;
	}
};

class MyEdgeData: public TopTree::EdgeData {
public:
	MyEdgeData(std::string label): label{label} {}

	std::string label;
	int weight;

	virtual std::ostream& ToString(std::ostream& o) const {
		//return o << "Edge " << label;
		return o << label;
	}
};

void TopTree::Join(std::shared_ptr<ClusterData> leftChild, std::shared_ptr<ClusterData> rightChild, std::shared_ptr<ClusterData> parent, bool isRake) {
	auto leftChild2 = std::dynamic_pointer_cast<MyClusterData>(leftChild);
	auto rightChild2 = std::dynamic_pointer_cast<MyClusterData>(rightChild);
	auto parent2 = std::dynamic_pointer_cast<MyClusterData>(parent);
	parent2->weight = leftChild2->weight + rightChild2->weight;
}
void TopTree::Split(std::shared_ptr<ClusterData> leftChild, std::shared_ptr<ClusterData> rightChild, std::shared_ptr<ClusterData> parent, bool isRake) {
	// Nothing
}

// Creating and destroying Base clusters:
void TopTree::Create(std::shared_ptr<ClusterData> cluster, std::shared_ptr<EdgeData> edge) {
	auto cluster2 = std::dynamic_pointer_cast<MyClusterData>(cluster);
	cluster2->weight = 10;
}
void TopTree::Destroy(std::shared_ptr<ClusterData> cluster, std::shared_ptr<EdgeData> edge) {
	// Nothing
}

std::shared_ptr<TopTree::ClusterData> TopTree::InitClusterData() {
	return std::make_shared<MyClusterData>();
}

int main(int argc, char const *argv[]) {
	/*
	std::shared_ptr<MyEdgeData> p1(new MyEdgeData);
	std::shared_ptr<MyEdgeData> p2(new MyEdgeData);
	std::shared_ptr<MyEdgeData> p3(new MyEdgeData);
	auto T = MyTopTree();
	T.Create(p1);
	T.Create(p2);
	T.Join(p1,p2,p3);
	std::cout << p3->weight << std::endl;
	*/

	auto baseTree = std::make_shared<TopTree::BaseTree>();

	// Example from article:
	auto z = baseTree->AddVertex(std::make_shared<MyVertexData>("z"));
	auto p = baseTree->AddLeaf(z, std::make_shared<MyEdgeData>("z-p"), std::make_shared<MyVertexData>("p"));
	auto w = baseTree->AddLeaf(p, NULL, std::make_shared<MyVertexData>("w"));
	auto c = baseTree->AddLeaf(w, NULL, std::make_shared<MyVertexData>("c"));
	auto b = baseTree->AddLeaf(c, NULL, std::make_shared<MyVertexData>("b"));
	baseTree->AddLeaf(b, NULL, std::make_shared<MyVertexData>("a"));

	baseTree->AddLeaf(b, NULL, std::make_shared<MyVertexData>("t"));

	auto d = baseTree->AddLeaf(c, NULL, std::make_shared<MyVertexData>("d"));
	baseTree->AddLeaf(d, NULL, std::make_shared<MyVertexData>("r"));

	auto e = baseTree->AddLeaf(c, NULL, std::make_shared<MyVertexData>("e"));
	baseTree->AddLeaf(e, NULL, std::make_shared<MyVertexData>("q"));

	auto s = baseTree->AddLeaf(e, NULL, std::make_shared<MyVertexData>("s"));

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

	// baseTree->PrintRooted(z);

	////////////////

	auto T = new TopTree::TopTree(baseTree);

	auto roots = T->GetTopTrees();
	for (auto root : roots) {
		// T->PrintRooted(root);
		T->PrintGraphviz(root, "Initial Top Tree");
	}
	T->Expose(v, d);
	roots = T->GetTopTrees();
	for (auto root : roots) {
		// T->PrintRooted(root);
		T->PrintGraphviz(root, "Final Top Tree");
	}

	delete T;

	return 0;
}
