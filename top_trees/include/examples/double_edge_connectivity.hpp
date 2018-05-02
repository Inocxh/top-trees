#include <memory>
#include <string>
#include <algorithm>

#include "TopTreeInterface.hpp"

class MyVertexData: public TopTree::VertexData {
public:
	MyVertexData(std::string label): label{label} {}
	std::string label;

	std::vector<int> incident; //incident_v,i = number of level >= i nontree edges with an endpoint in v

	virtual std::ostream& ToString(std::ostream& o) const {
		//return o << "Vertex " << label;
		return o << label;
	}
};

class MyEdgeData: public TopTree::EdgeData {
public:
	MyEdgeData(std::string label): label{label} {}

	int from;
	int to;

	int cover = -1;
	int level = 0;

	std::list<std::shared_ptr<MyEdgeData>>::iterator nontree_edges_iterator;

	std::string label; // obsolete

	virtual std::ostream& ToString(std::ostream& o) const {
		//return o << "Edge " << label;
		return o << label;
	}
};

struct MyClusterData: public TopTree::ClusterData {
	int cover;

	int cover_limit; // in paper referred as cover^-
	int cover_set; // in paper referred as cover^+, should be cover_set <= cover_limit

	std::shared_ptr<MyEdgeData> edge = NULL; // only for base clusters
	std::shared_ptr<MyEdgeData> cover_edge; // representation of edge with l(cover_edge) = cover
	std::shared_ptr<MyEdgeData> cover_edge_set;

	int endpoint_a;
	int endpoint_b;

	// I_v,i,j = set of internal vertices of the cluster that are reachable from
	// v by a path P in F where c(P\cup\pi(C))>=i and c(P\\pi(C))>=j
	std::vector<std::vector<int>> size_a;  // size of the I_a,i,j
	std::vector<std::vector<int>> size_b; // size of the I_b,i,j

	// incident... = number of (directed) level j nontree edges (q,r) with q in I_v,i,j
	std::vector<std::vector<int>> incident_a;
	std::vector<std::vector<int>> incident_b;

	// For non-path clusters
	// I_v,i = set of internal vertices q from cluster such that c(v...q) >= i
	std::vector<int> nonpath_size_a;  // size of the I_a,i
	std::vector<int> nonpath_size_b; // size of the I_b,i
	// incident ... = number of directed level i nontree edges (q,r) with q in I_v,i
	std::vector<int> nonpath_incident_a;
	std::vector<int> nonpath_incident_b;

	int get_size(int v, int i, int j) {
		std::vector<std::vector<int>>& vector = size_a;
		if (v == endpoint_a) vector = size_a;
		else if (v == endpoint_b) vector = size_b;
		else {
			std::cerr << "Cannot get for vertex " << v << ", endpoints of edge clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
			exit(2);
		}

		// indexes are shifted +1
		uint i_index = i+1;
		uint j_index = j+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		if (vector[i_index].size() <= j_index) vector[i_index].resize(j_index+1);
		return vector[i_index][j_index];
	}
	void set_size(int v, int i, int j, int value) {
		std::vector<std::vector<int>>& vector = size_a;
		if (v == endpoint_a) vector = size_a;
		else if (v == endpoint_b) vector = size_b;
		else {
			std::cerr << "Cannot get for vertex " << v << ", endpoints of edge clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
			exit(2);
		}

		// indexes are shifted +1
		uint i_index = i+1;
		uint j_index = j+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		if (vector[i_index].size() <= j_index) vector[i_index].resize(j_index+1);
		vector[i_index][j_index] = value;
	}
	int get_incident(int v, int i, int j) {
		std::vector<std::vector<int>>& vector = incident_a;
		if (v == endpoint_a) vector = incident_a;
		else if (v == endpoint_b) vector = incident_b;
		else {
			std::cerr << "Cannot get for vertex " << v << ", endpoints of edge clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
			exit(2);
		}

		// indexes are shifted +1
		uint i_index = i+1;
		uint j_index = j+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		if (vector[i_index].size() <= j_index) vector[i_index].resize(j_index+1);
		return vector[i_index][j_index];
	}
	void set_incident(int v, int i, int j, int value) {
		std::vector<std::vector<int>>& vector = incident_a;
		if (v == endpoint_a) vector = incident_a;
		else if (v == endpoint_b) vector = incident_b;
		else {
			std::cerr << "Cannot get for vertex " << v << ", endpoints of edge clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
			exit(2);
		}

		// indexes are shifted +1
		uint i_index = i+1;
		uint j_index = j+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		if (vector[i_index].size() <= j_index) vector[i_index].resize(j_index+1);
		vector[i_index][j_index] = value;
	}

	int get_nonpath_size(int v, int i) {
		std::vector<int>& vector = nonpath_size_a;
		if (v == endpoint_a) vector = nonpath_size_a;
		else if (v == endpoint_b) vector = nonpath_size_b;
		else {
			std::cerr << "Cannot get for vertex " << v << ", endpoints of edge clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
			exit(2);
		}

		// indexes are shifted +1
		uint i_index = i+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		return vector[i_index];
	}
	void set_nonpath_size(int v, int i, int value) {
		std::vector<int>& vector = nonpath_size_a;
		if (v == endpoint_a) vector = nonpath_size_a;
		else if (v == endpoint_b) vector = nonpath_size_b;
		else {
			std::cerr << "Cannot get for vertex " << v << ", endpoints of edge clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
			exit(2);
		}

		// indexes are shifted +1
		uint i_index = i+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		vector[i_index] = value;
	}
	int get_nonpath_incident(int v, int i) {
		std::vector<int>& vector = nonpath_incident_a;
		if (v == endpoint_a) vector = nonpath_incident_a;
		else if (v == endpoint_b) vector = nonpath_incident_b;
		else {
			std::cerr << "Cannot get for vertex " << v << ", endpoints of edge clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
			exit(2);
		}

		// indexes are shifted +1
		uint i_index = i+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		return vector[i_index];
	}
	void set_nonpath_incident(int v, int i, int value) {
		std::vector<int>& vector = nonpath_incident_a;
		if (v == endpoint_a) vector = nonpath_incident_a;
		else if (v == endpoint_b) vector = nonpath_incident_b;
		else {
			std::cerr << "Cannot get for vertex " << v << ", endpoints of edge clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
			exit(2);
		}

		// indexes are shifted +1
		uint i_index = i+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		vector[i_index] = value;
	}

	// foreach child:
	//   if child.cover <= cover_limit:
	//     child.cover = cover.set
	//     child.cover_edge = cover_edge_set
};

////////////////////////////////////////////////////////////////////////////////

class DoubleConnectivity {
public:
std::shared_ptr<TopTree::ITopTree> TT;
std::vector<std::vector<int>> incident;
std::vector<std::vector<int>> size;
int max_l = 0;
int N = 0;

int get_size(uint u, int i) {
	// indexes are shifted +1
	uint i_index = i+1;
	if (size.size() <= u) size.resize(u+1);
	if (size[u].size() <= i_index) size[u].resize(i_index+1);
	return size[u][i_index];
}
void set_size(uint u, int i, int value) {
	// indexes are shifted +1
	uint i_index = i+1;
	if (size.size() <= u) size.resize(u+1);
	if (size[u].size() <= i_index) size[u].resize(i_index+1);
	size[u][i_index] = value;
}
int get_incident(uint u, int i) {
	// indexes are shifted +1
	uint i_index = i+1;
	if (incident.size() <= u) incident.resize(u+1);
	if (incident[u].size() <= i_index) incident[u].resize(i_index+1);
	return incident[u][i_index];
}
void set_incident(uint u, int i, int value) {
	// indexes are shifted +1
	uint i_index = i+1;
	if (incident.size() <= u) incident.resize(u+1);
	if (incident[u].size() <= i_index) incident[u].resize(i_index+1);
	incident[u][i_index] = value;
}

std::list<std::shared_ptr<MyEdgeData>> nontree_edges;

// Decide if v and w are c-2-edge connected on level 0
bool Double_edge_connected(int v, int w) {
	auto cluster = TT->Expose(v, w);
	auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
	return (data->cover > 0);
} // COMPLETE

// For all e in v...w: if c(e) < i, set c(e)=i (lazy propagate down)
void Cover(int v, int w, int i) {
	auto cluster = TT->Expose(v, w);

	internal_cover(cluster, i, v, w); // TODO: correctly send edge as MyEdgeData
}

// For all e in v...w: if c(e)<=i, set c(e)=-1 (lazy propagate down)
void Uncover(int v, int w, int i) {
	auto cluster = TT->Expose(v, w);

	internal_uncover(cluster, i);
}

// INTERNAL:
void internal_cover(std::shared_ptr<TopTree::ICluster> cluster, int i, std::shared_ptr<MyEdgeData> edge) {
	auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
	if (data->cover < i) {
		data->cover = i;
		data->cover_edge = edge;
	}
	// if (i < data->cover_set) // nothing
	if (i <= data->cover_limit && i >= data->cover_set) {
		data->cover_set = i;
		data->cover_edge_set = edge;
	} else if (i > data->cover_limit) {
		data->cover_limit = i;
		data->cover_set = i;
		data->cover_edge_set = edge;
	}

	auto l = cluster->getLeftBoundary();
	auto r = cluster->getRightBoundary();
	for (int j = -1; j <= i; j++) {
		for (int k = -1; k <= max_l; k++) {
			data->set_size(l, j, k, data->get_size(l, -1, k));
			data->set_size(r, j, k, data->get_size(r, -1, k));

			data->set_incident(l, j, k, data->get_incident(l, -1, k));
			data->set_incident(r, j, k, data->get_incident(r, -1, k));
		}
	}
} // COMPLETE

void internal_uncover(std::shared_ptr<TopTree::ICluster> cluster, int i) {
	auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
	if (data->cover <= i) {
		data->cover = -1;
		data->cover_edge = NULL;
	}
	// if (i < data->cover_set) // nothing
	if (i >= data->cover_set) {
		data->cover_set = -1;
		data->cover_edge_set = NULL;
		data->cover_limit = std::max(data->cover_limit, i);
	}

	int l = cluster->getLeftBoundary();
	int r = cluster->getRightBoundary();

	for (int j = -1; j <= i; j++) {
		for (int k = -1; k <= max_l; k++) {
			data->set_size(l, j, k, data->get_size(l, i+1, k));
			data->set_size(r, j, k, data->get_size(r, i+1, k));

			data->set_incident(l, j, k, data->get_incident(l, i+1, k));
			data->set_incident(r, j, k, data->get_incident(r, i+1, k));
		}
	}
} // COMPLETE


void Clean(std::shared_ptr<TopTree::ICluster> leftChild, std::shared_ptr<TopTree::ICluster> rightChild, std::shared_ptr<TopTree::ICluster> parent) {
	// Part of the Split is an Clean method
	auto data = std::dynamic_pointer_cast<MyClusterData>(parent->data);

	// For each path child A of C call Uncover(A, cover_limit) and Cover(A, cover_set, cover_edge_set)
	if (isLeftRake(leftChild, rightChild, parent))
		internal_uncover(rightChild, data->cover_limit); // Only the right is a path child
	else if (isRightRake(leftChild, rightChild, parent))
		internal_uncover(leftChild, data->cover_limit); // Only the left is a path child
	else {
		internal_uncover(leftChild, data->cover_limit);
		internal_uncover(rightChild, data->cover_limit);
	}

	data->cover_set = -1;
	data->cover_limit = -1;
	data->cover_edge_set = NULL;
}

void Recover(int v, int w, int i) {
	auto clusterC = TT->Expose(v, w);
	auto dataC = std::dynamic_pointer_cast<MyClusterData>(clusterC->data);

	// Firstly u=v
	auto u = clusterC->getLeftBoundary();
	while (true) {
		while (dataC->get_incident(u, -1, i) + get_incident(u, i) > 0) {
			std::shared_ptr<MyEdgeData> edge = Find(u, clusterC, i);
			auto clusterD = TT->Expose(edge->from, edge->to);
			auto dataD = std::dynamic_pointer_cast<MyClusterData>(clusterD->data);
			if ((dataD->get_size(edge->from, -1, i+1) + 2) > N/(1<<(i+1))) {
				internal_cover(clusterD, i, edge);
				break;
			} else {
				edge->level = i+1;
				incident[edge->from][i]--;
				incident[edge->to][i]--;
				incident[edge->from][i]++;
				incident[edge->to][i]++;
				internal_cover(clusterD, i+1, edge);
			}
			clusterC = TT->Expose(v,w);
			dataC = std::dynamic_pointer_cast<MyClusterData>(clusterC->data);
		}
		if (u == clusterC->getLeftBoundary()) u == clusterC->getRightBoundary();
		else break; //end of the u=w run
	}
}

std::shared_ptr<MyEdgeData> Find(int a, std::shared_ptr<TopTree::ICluster> cluster, int i) {
	if (incident[a][i] > 0) {
		// TODO: return a nontree edge incdent to a on level i
	} else {
		Clean(/*TODO childs*/ cluster);
	}
}

void Swap(int v, int w) {
	auto edgeCluster = TT->Expose(v, w);
	auto data = std::dynamic_pointer_cast<MyClusterData>(edgeCluster->data);

	if (data->edge == NULL) {
		std::cerr << "Edge cluster " << v << "-" << w << " does not have underlying edge" << std::endl;
		return;
	}

	if (data->cover < 0) { // it is a bridge
		//std::cerr << "Edge " << *edgeCluster << " is a bridge, cannot swap" << std::endl;
		std::cerr << "Edge " << v << "-" << w << " is a bridge, cannot swap" << std::endl;
		return;
	}

	auto cover = data->cover;
	auto coverEdge = data->cover_edge;

	// 1. Set level of the v-w to the cover
	data->edge->level = cover;
	// 1.1 Call InitTreeEdge(x,y)
	coverEdge->cover = -1;

	// 2. Replace tree edge v-w for coverEdge x-y
	// 2.1 Remove coverEdge from nontree_edges
	nontree_edges.erase(coverEdge->nontree_edges_iterator);
	// 2.2 Remove v-w from tree, push x-y into tree
	TT->Cut(v, w);
	TT->Link(coverEdge->from, coverEdge->to, coverEdge);
	// 2.3 Push v-w edge into nontree_edges
	nontree_edges.push_back(data->edge);
	data->edge->nontree_edges_iterator = std::prev(nontree_edges.end());

	// 3. Call Cover
	Cover(v, w, cover);
}

};


////////////////////////////////////////////////////////////////////////////////

DoubleConnectivity *dc; // TODO: save into this variable

// Merge
void TopTree::Join(std::shared_ptr<TopTree::ICluster> leftChild, std::shared_ptr<TopTree::ICluster> rightChild, std::shared_ptr<TopTree::ICluster> parent) {
	auto data = std::dynamic_pointer_cast<MyClusterData>(parent->data);
	auto left_data = std::dynamic_pointer_cast<MyClusterData>(leftChild->data);
	auto right_data = std::dynamic_pointer_cast<MyClusterData>(rightChild->data);

	/////////////////////////////////////////////////////////
	// Init endpoints in the new cluster - O(1) computations:
	data->endpoint_a = parent->getLeftBoundary();
	data->endpoint_b = parent->getRightBoundary();

	if (left_data->cover < right_data->cover) {
		data->cover = left_data->cover;
		data->cover_edge = left_data->cover_edge;
	} else {
		data->cover = right_data->cover;
		data->cover_edge = right_data->cover_edge;
	}
	data->cover_set = -1;
	data->cover_limit = -1;
	data->cover_edge_set = NULL;

	///////////////////////////////////////////////////
	// Time consuming computations in O(log^2 N) below:

	int common = leftChild->getLeftBoundary();
	if (common == rightChild->getLeftBoundary() || common == rightChild->getRightBoundary()) common = leftChild->getRightBoundary();

	int other_left = (leftChild->getLeftBoundary() == common ? leftChild->getRightBoundary() : leftChild->getLeftBoundary());
	int other_right = (rightChild->getLeftBoundary() == common ? rightChild->getRightBoundary() : rightChild->getLeftBoundary());

	// A) Computation of nonpath_size[a][j] for a in {a,b} and j in 0...max_l
	if (isCompress(leftChild, rightChild, parent)) {
		// Because we don't know which one of endpoint will be used compute for both a and c - figure 1(3)
		// For other vertex from leftChild
		int other = other_left;
		for (int j = 0; j <= dc->max_l; j++) {
			int size = left_data->get_size(other, j, j);
			int incident = left_data->get_size(other, j, j);
			if (left_data->cover >= j) {
				size += dc->get_size(common, j) + right_data->get_nonpath_size(common, j);
				incident += dc->get_incident(common, j) + right_data->get_nonpath_incident(common, j);
			}
			data->set_nonpath_size(other, j, size);
			data->set_nonpath_incident(other, j, incident);
		}

		// For oher vertex from rightChild
		other = other_right;
		for (int j = 0; j <= dc->max_l; j++) {
			int size = right_data->get_size(other, j, j);
			int incident = right_data->get_size(other, j, j);
			if (left_data->cover >= j) {
				size += dc->get_size(common, j) + right_data->get_nonpath_size(common, j);
				incident += dc->get_incident(common, j) + right_data->get_nonpath_incident(common, j);
			}
			data->set_nonpath_size(other, j, size);
			data->set_nonpath_incident(other, j, incident);
		}
	} else {
		// Some rake, not interesting which - figure 1(4)
		for (int j = 0; j <= dc->max_l; j++) {
			data->set_nonpath_size(common, j,
				left_data->get_nonpath_size(common, j) + right_data->get_nonpath_size(common, j)
			);
			data->set_nonpath_incident(common, j,
				left_data->get_nonpath_incident(common, j) + right_data->get_nonpath_incident(common, j)
			);
		}
	}

	// B) Computation of path size[a][i][j] for i,j in -1...max_l
	if (isLeftRake(leftChild, rightChild, parent)) {
		// left {common} is raked on the right one {common,other_right} - set for common and other_right
		for (int i = -1; i <= dc->max_l; i++) {
			for (int j = -1; j <= dc->max_l; j++) {
				data->set_size(common, i, j,
					left_data->get_nonpath_size(common, j) + right_data->get_size(common, i, j)
				);
				data->set_incident(common, i, j,
					left_data->get_nonpath_incident(common, j) + right_data->get_incident(common, i, j)
				);

				int size = right_data->get_size(other_right, i, j);
				int incident = right_data->get_incident(other_right, i, j);
				if (right_data->cover >= i) {
					size += left_data->get_nonpath_size(other_left, j);
					incident += left_data->get_nonpath_incident(other_left, j);
				}
				data->set_size(other_right, i, j, size);
				data->set_incident(other_right, i, j, incident);
			}
		}
	} else if (isRightRake(leftChild, rightChild, parent)) {
		// right {common} is raked on the left one {common,other_left} - set for common and other_left
		for (int i = -1; i <= dc->max_l; i++) {
			for (int j = -1; j <= dc->max_l; j++) {
				data->set_size(common, i, j,
					right_data->get_nonpath_size(common, j) + left_data->get_size(common, i, j)
				);
				data->set_incident(common, i, j,
					right_data->get_nonpath_incident(common, j) + left_data->get_incident(common, i, j)
				);

				int size = left_data->get_size(other_left, i, j);
				int incident = left_data->get_incident(other_left, i, j);
				if (left_data->cover >= i) {
					size += right_data->get_nonpath_size(other_right, j);
					incident += right_data->get_nonpath_incident(other_right, j);
				}
				data->set_size(other_left, i, j, size);
				data->set_incident(other_left, i, j, incident);
			}
		}
	} else { // Compress
		for (int i = -1; i <= dc->max_l; i++) {
			for (int j = -1; j <= dc->max_l; j++) {
				// for other_left
				int size = left_data->get_size(other_left, i, j);
				int incident = left_data->get_incident(other_left, i, j);
				if (left_data->cover >= i) {
					size += dc->get_size(common, j) + right_data->get_size(common, i, j);
					incident += dc->get_incident(common, j) + right_data->get_incident(common, i, j);
				}
				data->set_size(other_left, i, j, size);
				data->set_incident(other_left, i, j, incident);

				// for other_right
				size = right_data->get_size(other_right, i, j);
				incident = right_data->get_incident(other_right, i, j);
				if (right_data->cover >= i) {
					size += dc->get_size(common, j) + left_data->get_size(common, i, j);
					incident += dc->get_incident(common, j) + left_data->get_incident(common, i, j);
				}
				data->set_size(other_right, i, j, size);
				data->set_incident(other_right, i, j, incident);
			}
		}
	}
} // COMPLETE
void TopTree::Split(std::shared_ptr<ICluster> leftChild, std::shared_ptr<ICluster> rightChild, std::shared_ptr<ICluster> parent) {
	dc->Clean(leftChild, rightChild, parent);
	// delete C - not needed, it will be deleted by TopTrees structure
} // COMPLETE

// Creating and destroying Base clusters:
void TopTree::Create(std::shared_ptr<ICluster> cluster, std::shared_ptr<EdgeData> edge) {
	auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
	auto edge_data = std::dynamic_pointer_cast<MyEdgeData>(edge);
	data->cover = -1;
	edge_data->cover = -1;

	data->endpoint_a = cluster->getLeftBoundary();
	data->endpoint_b = cluster->getRightBoundary();

	data->edge = edge_data;
} // COMPLETE
void TopTree::Destroy(std::shared_ptr<ICluster> cluster, std::shared_ptr<EdgeData> edge) {
	// Nothing
} // COMPLETE

std::shared_ptr<TopTree::ClusterData> TopTree::InitClusterData() {
	return std::make_shared<MyClusterData>();
}
