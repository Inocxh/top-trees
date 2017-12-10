#include <iostream>
#include <memory>
#include <string>

#include "TopTree.hpp"
#include "TopologyTopTree.hpp"

struct MyClusterData: public TopTree::ClusterData {
	int weight;
	int total_weight;
	std::string label;
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

	virtual std::ostream& ToString(std::ostream& o) const {
		//return o << "Edge " << label;
		return o << label;
	}
};

void TopTree::Join(std::shared_ptr<ClusterData> leftChild, std::shared_ptr<ClusterData> rightChild, std::shared_ptr<ClusterData> parent, bool isRake) {
	auto leftChild2 = std::dynamic_pointer_cast<MyClusterData>(leftChild);
	auto rightChild2 = std::dynamic_pointer_cast<MyClusterData>(rightChild);
	auto parent2 = std::dynamic_pointer_cast<MyClusterData>(parent);

	if (isRake) parent2->weight = rightChild2->weight;
	else parent2->weight = leftChild2->weight + rightChild2->weight;

	parent2->total_weight = leftChild2->total_weight + rightChild2->total_weight;

	// For debug:
	// parent2->label = leftChild2->label + "," + rightChild2->label;
	// std::cerr << "Joining " << leftChild2->total_weight << "(" << leftChild2->label << ") + " << rightChild2->total_weight << "(" << rightChild2->label << ")" << std::endl;
}
void TopTree::Split(std::shared_ptr<ClusterData> leftChild, std::shared_ptr<ClusterData> rightChild, std::shared_ptr<ClusterData> parent, bool isRake) {
	// Nothing
}

// Creating and destroying Base clusters:
void TopTree::Create(std::shared_ptr<ClusterData> cluster, std::shared_ptr<EdgeData> edge) {
	auto cluster2 = std::dynamic_pointer_cast<MyClusterData>(cluster);
	auto edge2 = std::dynamic_pointer_cast<MyEdgeData>(edge);
	cluster2->weight = 10;
	cluster2->total_weight = 10;
	cluster2->label = edge2->label;
}
void TopTree::Destroy(std::shared_ptr<ClusterData> cluster, std::shared_ptr<EdgeData> edge) {
	// Nothing
}

std::shared_ptr<TopTree::ClusterData> TopTree::InitClusterData() {
	return std::make_shared<MyClusterData>();
}

void print_node(std::shared_ptr<TopTree::Cluster> node) {
	if (node != NULL) {
		auto data = std::dynamic_pointer_cast<MyClusterData>(node->data);
		std::cerr << "[weight: " << data->weight << ", total_weight:" << data->total_weight << "]" << std::endl;
	}
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
	auto w = baseTree->AddLeaf(p, std::make_shared<MyEdgeData>("p-w"), std::make_shared<MyVertexData>("w"));
	auto c = baseTree->AddLeaf(w, std::make_shared<MyEdgeData>("w-c"), std::make_shared<MyVertexData>("c"));
	auto b = baseTree->AddLeaf(c, std::make_shared<MyEdgeData>("c-b"), std::make_shared<MyVertexData>("b"));
	auto a = baseTree->AddLeaf(b, std::make_shared<MyEdgeData>("b-a"), std::make_shared<MyVertexData>("a"));

	auto t = baseTree->AddLeaf(b, std::make_shared<MyEdgeData>("b-t"), std::make_shared<MyVertexData>("t"));

	auto d = baseTree->AddLeaf(c, std::make_shared<MyEdgeData>("c-d"), std::make_shared<MyVertexData>("d"));
	auto r = baseTree->AddLeaf(d, std::make_shared<MyEdgeData>("d-r"), std::make_shared<MyVertexData>("r"));

	auto e = baseTree->AddLeaf(c, std::make_shared<MyEdgeData>("c-e"), std::make_shared<MyVertexData>("e"));
	auto q = baseTree->AddLeaf(e, std::make_shared<MyEdgeData>("e-q"), std::make_shared<MyVertexData>("q"));

	auto s = baseTree->AddLeaf(e, std::make_shared<MyEdgeData>("e-s"), std::make_shared<MyVertexData>("s"));

	baseTree->AddLeaf(c, std::make_shared<MyEdgeData>("c-f"), std::make_shared<MyVertexData>("f"));

	auto g = baseTree->AddLeaf(c, std::make_shared<MyEdgeData>("c-g"), std::make_shared<MyVertexData>("g"));
	auto j = baseTree->AddLeaf(g, std::make_shared<MyEdgeData>("g-j"), std::make_shared<MyVertexData>("j"));

	auto h = baseTree->AddLeaf(g, std::make_shared<MyEdgeData>("g-h"), std::make_shared<MyVertexData>("h"));

	auto n = baseTree->AddLeaf(c, std::make_shared<MyEdgeData>("c-n"), std::make_shared<MyVertexData>("n"));
	auto k = baseTree->AddLeaf(n, std::make_shared<MyEdgeData>("n-k"), std::make_shared<MyVertexData>("k"));

	auto v = baseTree->AddLeaf(n, std::make_shared<MyEdgeData>("n-v"), std::make_shared<MyVertexData>("v"));
	baseTree->AddLeaf(v, std::make_shared<MyEdgeData>("v-u"), std::make_shared<MyVertexData>("u"));

	baseTree->AddLeaf(v, std::make_shared<MyEdgeData>("v-l"), std::make_shared<MyVertexData>("l"));

	baseTree->AddLeaf(n, std::make_shared<MyEdgeData>("n-i"), std::make_shared<MyVertexData>("i"));

	auto y = baseTree->AddLeaf(w, std::make_shared<MyEdgeData>("w-y"), std::make_shared<MyVertexData>("y"));

	auto o = baseTree->AddLeaf(w, std::make_shared<MyEdgeData>("w-o"), std::make_shared<MyVertexData>("o"));

	baseTree->AddLeaf(p, std::make_shared<MyEdgeData>("p-x"), std::make_shared<MyVertexData>("x"));

	// baseTree->PrintRooted(z);

	////////////////

	auto TT = std::make_shared<TopTree::TopologyTopTree>(baseTree);

	/*
	auto T = std::make_shared<TopTree::TopTree>(baseTree);

	for (auto root : T->GetTopTrees()) T->PrintGraphviz(root, "Initial Top Tree");
	auto node = T->Expose(s, e);
	print_node(node);

	auto nodes = T->Cut(c, w);
	print_node(std::get<0>(nodes));
	print_node(std::get<1>(nodes));
	for (auto root : T->GetTopTrees()) T->PrintGraphviz(root, "After Cut");

	node = T->Link(k, y, std::make_shared<MyEdgeData>("k-y"));
	print_node(node);
	//T->Expose(v, e);
	for (auto root : T->GetTopTrees()) T->PrintGraphviz(root, "After Link");
	*/

	return 0;
}
