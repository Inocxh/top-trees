#include <memory>
#include <string>
#include <sstream>

#include "TopTreeInterface.hpp"

class MyEdgeData: public TopTree::EdgeData {
public:
	MyEdgeData(int index, int weight, std::string label): index{index}, weight{weight}, label{label} {}

	int index;
	int weight;
	std::string label;

	// For testing:
	virtual std::ostream& ToString(std::ostream& o) const { return o << label; }
};

class MyVertexData: public TopTree::VertexData {
public:
	MyVertexData(std::string label): label{label} {}
	std::string label;

	// For testing:
	virtual std::ostream& ToString(std::ostream& o) const { return o << label; }
};

struct MyClusterData: public TopTree::ClusterData {
	int w_max;
	std::shared_ptr<MyEdgeData> w_max_edge;
	int w_extra;
};

////////////////////////////////////////////////////////////////////////////////

class MaximumEdgeWeight {
public:
	MaximumEdgeWeight(TopTree::ITopTree *top_tree): top_tree{top_tree}, base_tree{std::make_shared<TopTree::BaseTree>()} {}

	int add_vertex(std::string label) {
		// Add vertex adds vertex every time in the BaseTree
		int index = base_tree->AddVertex(std::make_shared<MyVertexData>(label));
		vertices.push_back(vertex{label, index});
		return vertices.size() - 1;
	}

	int add_edge(int a, int b, int weight) {
		edges.push_back(edge{a, b, weight});
		int index = edges.size() - 1;
		std::ostringstream ss;
		ss << vertices[a].index << "," << vertices[b].index;
		auto edge_data = std::make_shared<MyEdgeData>(index, weight, ss.str());

		// When not initialized add to BaseTree, otherwise call Link
		if (!initialized) base_tree->AddEdge(vertices[a].index, vertices[b].index, edge_data);
		else {
			auto result = top_tree->Link(vertices[a].index, vertices[b].index, edge_data);
			if (result == NULL) return -1; // cannot add edge
		}
		return index;
	}

	bool remove_edge(int a, int b) {
		if (!initialized) return false;
		auto cluster = top_tree->Cut(vertices[a].index, vertices[b].index);
		return (std::get<2>(cluster) != NULL); // if is NULL -> not cut happens
	}

	void initialize() {
		top_tree->InitFromBaseTree(base_tree);
		initialized = true;
	}

	// Functions that could be used after initialization:

	bool add_weight_on_path(int a, int b, int extra_weight) {
		auto cluster = top_tree->Expose(vertices[a].index, vertices[b].index);
		if (cluster == NULL) return false;

		auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
		data->w_extra += extra_weight;
		data->w_max += extra_weight;
		return true;
	}

	struct max_weight_result {
		bool exists;
		int max_weight;
		int edge_index;
	};
	struct max_weight_result get_max_weight_on_path(int a, int b) {
		auto cluster = top_tree->Expose(vertices[a].index, vertices[b].index);
		if (cluster == NULL) return max_weight_result{false, 0, 0};

		auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
		auto edge = std::dynamic_pointer_cast<MyEdgeData>(data->w_max_edge);
		return max_weight_result{true, data->w_max, edge->index};
	}

private:
	TopTree::ITopTree *top_tree;
	std::shared_ptr<TopTree::BaseTree> base_tree;

	bool initialized = false;

	struct vertex {
		std::string label;
		int index;
	};
	struct edge {
		int from;
		int to;
		int weight;
	};
	std::vector<vertex> vertices;
	std::vector<edge> edges;
};

////////////////////////////////////////////////////////////////////////////////

void TopTree::Join(std::shared_ptr<TopTree::ICluster> leftChild, std::shared_ptr<TopTree::ICluster> rightChild, std::shared_ptr<TopTree::ICluster> parent) {
	auto left_data = std::dynamic_pointer_cast<MyClusterData>(leftChild->data);
	auto right_data = std::dynamic_pointer_cast<MyClusterData>(rightChild->data);
	auto parent_data = std::dynamic_pointer_cast<MyClusterData>(parent->data);

	if (isLeftRake(leftChild, rightChild, parent)) {
		parent_data->w_max = right_data->w_max;
		parent_data->w_max_edge = right_data->w_max_edge;
	} else if (isRightRake(leftChild, rightChild, parent)) {
		parent_data->w_max = left_data->w_max;
		parent_data->w_max_edge = left_data->w_max_edge;
	} else {
		if (left_data->w_max > right_data->w_max) {
			parent_data->w_max = left_data->w_max;
			parent_data->w_max_edge = left_data->w_max_edge;
		} else {
			parent_data->w_max = right_data->w_max;
			parent_data->w_max_edge = right_data->w_max_edge;
		}
	}
	// There is no extra weight yet
	parent_data->w_extra = 0;
}
void TopTree::Split(std::shared_ptr<ICluster> leftChild, std::shared_ptr<ICluster> rightChild, std::shared_ptr<ICluster> parent) {
	auto left_data = std::dynamic_pointer_cast<MyClusterData>(leftChild->data);
	auto right_data = std::dynamic_pointer_cast<MyClusterData>(rightChild->data);
	auto parent_data = std::dynamic_pointer_cast<MyClusterData>(parent->data);

	// Distribute w_extra to childs
	if (isLeftRake(leftChild, rightChild, parent)) {
		right_data->w_extra += parent_data->w_extra;
		right_data->w_max += parent_data->w_extra;
	} else if (isRightRake(leftChild, rightChild, parent)) {
		left_data->w_extra += parent_data->w_extra;
		left_data->w_max += parent_data->w_extra;
	} else {
		// Left
		left_data->w_extra += parent_data->w_extra;
		left_data->w_max += parent_data->w_extra;
		// Right
		right_data->w_extra += parent_data->w_extra;
		right_data->w_max += parent_data->w_extra;
	}
}

// Creating and destroying Base clusters:
void TopTree::Create(std::shared_ptr<ICluster> cluster, std::shared_ptr<EdgeData> edge) {
	auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
	auto edge_data = std::dynamic_pointer_cast<MyEdgeData>(edge);
	data->w_max = edge_data->weight;
	data->w_max_edge = edge_data;
	data->w_extra = 0;
}
void TopTree::Destroy(std::shared_ptr<ICluster> cluster, std::shared_ptr<EdgeData> edge) {
	auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
	auto edge_data = std::dynamic_pointer_cast<MyEdgeData>(edge);
	edge_data->weight = data->w_max;
}

std::shared_ptr<TopTree::ClusterData> TopTree::InitClusterData() {
	return std::make_shared<MyClusterData>();
}
