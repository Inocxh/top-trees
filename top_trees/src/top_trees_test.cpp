#include <iostream>
#include <memory>
#include <string>

#include "TopTree.hpp"
#include "TopologyTopTree.hpp"

//#define DEBUG

struct MyClusterData: public TopTree::ClusterData {
	int weight;
	int total_weight;
	std::string label;
	std::string total_label;
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

void TopTree::Join(std::shared_ptr<ICluster> leftChild, std::shared_ptr<ICluster> rightChild, std::shared_ptr<ICluster> parent) {
	auto left_data = std::dynamic_pointer_cast<MyClusterData>(leftChild->data);
	auto right_data = std::dynamic_pointer_cast<MyClusterData>(rightChild->data);
	auto parent_data = std::dynamic_pointer_cast<MyClusterData>(parent->data);

	if (isLeftRake(leftChild, rightChild, parent)) {
		parent_data->weight = right_data->weight;
		parent_data->label = right_data->label;
	} else if (isRightRake(leftChild, rightChild, parent)) {
		parent_data->weight = left_data->weight;
		parent_data->label = left_data->label;
	} else {
		parent_data->weight = left_data->weight + right_data->weight;
		parent_data->label = left_data->label + "," + right_data->label;
	}

	parent_data->total_weight = left_data->total_weight + right_data->total_weight;
	parent_data->total_label = left_data->total_label + "," + right_data->total_label;

	#ifdef DEBUG
		std::cerr << "Joining " << left_data->total_weight << "(" << left_data->label << "/" << left_data->total_label << ") + " << right_data->total_weight << "(" << right_data->label << "/" << right_data->total_label << ")" << std::endl;
	#endif
}
void TopTree::Split(std::shared_ptr<ICluster> leftChild, std::shared_ptr<ICluster> rightChild, std::shared_ptr<ICluster> parent) {
	// Nothing
}

// Creating and destroying Base clusters:
void TopTree::Create(std::shared_ptr<ICluster> cluster, std::shared_ptr<EdgeData> edge) {
	auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
	auto edge_data = std::dynamic_pointer_cast<MyEdgeData>(edge);
	data->weight = 10;
	data->total_weight = 10;
	data->label = edge_data->label;
	data->total_label = edge_data->label;
}
void TopTree::Destroy(std::shared_ptr<ICluster> cluster, std::shared_ptr<EdgeData> edge) {
	// Nothing
}

void TopTree::CopyClusterData(std::shared_ptr<ICluster> from, std::shared_ptr<ICluster> to) {
	auto fromData = std::dynamic_pointer_cast<MyClusterData>(from->data);
	auto toData = std::dynamic_pointer_cast<MyClusterData>(to->data);

	toData->weight = fromData->weight;
	toData->total_weight = fromData->total_weight;
	toData->label = fromData->label;
	toData->total_label = fromData->total_label;
}

std::shared_ptr<TopTree::ClusterData> TopTree::InitClusterData() {
	return std::make_shared<MyClusterData>();
}

void print_node(std::shared_ptr<TopTree::ICluster> node) {
	if (node != NULL) {
		auto data = std::dynamic_pointer_cast<MyClusterData>(node->data);
		std::cerr << "[weight: " << data->weight << ", total_weight:" << data->total_weight << "]: " << data->label << std::endl;
	}
}

int main(int argc, char const *argv[]) {
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
	//auto TT = std::make_shared<TopTree::TopTree>(baseTree);
	std::cerr << "Top Tree builded" << std::endl;

	auto result = TT->Cut(c, w);
	std::cerr << "Two trees after cut:" << std::endl;
	print_node(std::get<0>(result));
	print_node(std::get<1>(result));

	auto result2 = TT->Link(b, p, std::make_shared<MyEdgeData>("B-P"));
	std::cerr << "After link:" << std::endl;
	print_node(result2);

	result2 = TT->Expose(z, r);
	//result2 = TT->Expose(h, c);
	//result2 = TT->Expose(c, b);
	std::cerr << "After expose:" << std::endl;
	print_node(result2);
	TT->Restore();

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
