#include <memory>
#include <string>
#include <algorithm>
#include <sstream>

#include "TopTreeInterface.hpp"

//#define DEBUG
//#define DEBUG_VERBOSE
//#define DEBUG_VERBOSE_GETTERS

//#define ASSERTS

class MyVertexData: public TopTree::VertexData {
public:
	MyVertexData(std::string label): label{label} {}
	std::string label;

	std::vector<int> incident; //incident_v,i = number of level >= i nontree edges with an endpoint in v

	virtual std::ostream& ToString(std::ostream& o) const { return o << label; }
};

class MyEdgeData: public TopTree::EdgeData {
public:
	MyEdgeData(uint from, uint to): from{from}, to{to} {
		std::ostringstream ss;
		ss << from << "-" << to;
		label = ss.str();
	}

	uint from; // index of vertex in top tree structure
	uint to;   // index of vertex in top tree structure
	std::list<std::shared_ptr<MyEdgeData>>::iterator from_incident_iterator;
	std::list<std::shared_ptr<MyEdgeData>>::iterator to_incident_iterator;
	bool registered;

	int cover = -1;
	std::shared_ptr<MyEdgeData> cover_edge = NULL;
	int level = 0;

	std::list<std::shared_ptr<MyEdgeData>>::iterator nontree_edges_iterator;

	std::string label;

	virtual std::ostream& ToString(std::ostream& o) const { return o << label << "(level " << level << ", cover " << cover << ")"; }
};

struct MyClusterData: public TopTree::ClusterData {
	int cover;

	int join_step;

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
		#ifdef DEBUG_VERBOSE_GETTERS
			std::cerr << "   getting size " << v << "," << i << "," << j << ": ";
		#endif
		#ifdef ASSERTS
			if (v != endpoint_a && v != endpoint_b) {
				std::cerr << "Cannot get size for vertex " << v << ", endpoints of this clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
				*(int*)0 = 0; // HACK: produce segfault to see "better" stacktrace output in valgrind
				exit(2);
			}
		#endif
		std::vector<std::vector<int>>& vector = size_a;
		if (v == endpoint_a) vector = size_a;
		else vector = size_b;


		// indexes are shifted +1
		uint i_index = i+1;
		uint j_index = j+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		if (vector[i_index].size() <= j_index) vector[i_index].resize(j_index+1);
		#ifdef DEBUG_VERBOSE_GETTERS
			std::cerr << vector[i_index][j_index] << std::endl;
		#endif
		return vector[i_index][j_index];
	}
	void set_size(int v, int i, int j, int value) {
		#ifdef DEBUG_VERBOSE_GETTERS
			std::cerr << "   setting size " << v << "," << i << "," << j << ": " << value;
		#endif
		#ifdef ASSERTS
			if (v != endpoint_a && v != endpoint_b) {
				std::cerr << "Cannot set size for vertex " << v << ", endpoints of this clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
				*(int*)0 = 0; // HACK: produce segfault to see "better" stacktrace output in valgrind
				exit(2);
			}
		#endif
		std::vector<std::vector<int>>& vector = size_a;
		if (v == endpoint_a) vector = size_a;
		else vector = size_b;

		// indexes are shifted +1
		uint i_index = i+1;
		uint j_index = j+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		if (vector[i_index].size() <= j_index) vector[i_index].resize(j_index+1);
		vector[i_index][j_index] = value;
	}
	int get_incident(int v, int i, int j) {
		#ifdef DEBUG_VERBOSE_GETTERS
			std::cerr << "   getting incident " << v << "," << i << "," << j << ": ";
		#endif
		#ifdef ASSERTS
			if (v != endpoint_a && v != endpoint_b) {
				std::cerr << "Cannot get incident for vertex " << v << ", endpoints of this clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
				*(int*)0 = 0; // HACK: produce segfault to see "better" stacktrace output in valgrind
				exit(2);
			}
		#endif
		std::vector<std::vector<int>>& vector = incident_a;
		if (v == endpoint_a) vector = incident_a;
		else vector = incident_b;

		// indexes are shifted +1
		uint i_index = i+1;
		uint j_index = j+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		if (vector[i_index].size() <= j_index) vector[i_index].resize(j_index+1);
		#ifdef DEBUG_VERBOSE_GETTERS
			std::cerr << vector[i_index][j_index] << std::endl;
		#endif
		return vector[i_index][j_index];
	}
	void set_incident(int v, int i, int j, int value) {
		#ifdef DEBUG_VERBOSE_GETTERS
			std::cerr << "   setting incident " << v << "," << i << "," << j << ": " << value;
		#endif
		#ifdef ASSERTS
			if (v != endpoint_a && v != endpoint_b) {
				std::cerr << "Cannot set incident for vertex " << v << ", endpoints of this clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
				*(int*)0 = 0; // HACK: produce segfault to see "better" stacktrace output in valgrind
				exit(2);
			}
		#endif
		std::vector<std::vector<int>>& vector = incident_a;
		if (v == endpoint_a) vector = incident_a;
		else vector = incident_b;

		// indexes are shifted +1
		uint i_index = i+1;
		uint j_index = j+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		if (vector[i_index].size() <= j_index) vector[i_index].resize(j_index+1);
		vector[i_index][j_index] = value;
	}

	int get_nonpath_size(int v, int i) {
		#ifdef DEBUG_VERBOSE_GETTERS
			std::cerr << "   getting nonpath size " << v << "," << i << ": ";
		#endif
		#ifdef ASSERTS
			if (v != endpoint_a && v != endpoint_b) {
				std::cerr << "Cannot get nonpath size for vertex " << v << ", endpoints of this clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
				*(int*)0 = 0; // HACK: produce segfault to see "better" stacktrace output in valgrind
				exit(2);
			}
		#endif

		std::vector<int>& vector = nonpath_size_a;
		if (v == endpoint_a) vector = nonpath_size_a;
		else vector = nonpath_size_b;

		// indexes are shifted +1
		uint i_index = i+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		#ifdef DEBUG_VERBOSE_GETTERS
			std::cerr << vector[i_index] << std::endl;
		#endif
		return vector[i_index];
	}
	void set_nonpath_size(int v, int i, int value) {
		#ifdef DEBUG_VERBOSE_GETTERS
			std::cerr << "   setting nonpath size " << v << "," << i << ": " << value;
		#endif
		#ifdef ASSERTS
			if (v != endpoint_a && v != endpoint_b) {
				std::cerr << "Cannot set nonpath size for vertex " << v << ", endpoints of this clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
				*(int*)0 = 0; // HACK: produce segfault to see "better" stacktrace output in valgrind
				exit(2);
			}
		#endif

		std::vector<int>& vector = nonpath_size_a;
		if (v == endpoint_a) vector = nonpath_size_a;
		else vector = nonpath_size_b;

		// indexes are shifted +1
		uint i_index = i+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		vector[i_index] = value;
	}
	int get_nonpath_incident(int v, int i) {
		#ifdef DEBUG_VERBOSE_GETTERS
			std::cerr << "   getting nonpath incident " << v << "," << i << ": ";
		#endif
		#ifdef ASSERTS
			if (v != endpoint_a && v != endpoint_b) {
				std::cerr << "Cannot get nonpath incident for vertex " << v << ", endpoints of this clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
				*(int*)0 = 0; // HACK: produce segfault to see "better" stacktrace output in valgrind
				exit(2);
			}
		#endif

		std::vector<int>& vector = nonpath_incident_a;
		if (v == endpoint_a) vector = nonpath_incident_a;
		else vector = nonpath_incident_b;

		// indexes are shifted +1
		uint i_index = i+1;
		if (vector.size() <= i_index) vector.resize(i_index+1);
		#ifdef DEBUG_VERBOSE_GETTERS
			std::cerr << vector[i_index] << std::endl;
		#endif
		return vector[i_index];
	}
	void set_nonpath_incident(int v, int i, int value) {
		#ifdef DEBUG_VERBOSE_GETTERS
			std::cerr << "   setting nonpath incident " << v << "," << i << ": " << value;
		#endif
		#ifdef ASSERTS
			if (v != endpoint_a && v != endpoint_b) {
				std::cerr << "Cannot set nonpath incident for vertex " << v << ", endpoints of this clusterData are " << endpoint_a << " and " << endpoint_b << std::endl;
				*(int*)0 = 0; // HACK: produce segfault to see "better" stacktrace output in valgrind
				exit(2);
			}
		#endif

		std::vector<int>& vector = nonpath_incident_a;
		if (v == endpoint_a) vector = nonpath_incident_a;
		else vector = nonpath_incident_b;

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
friend void TopTree::Join(std::shared_ptr<TopTree::ICluster> leftChild, std::shared_ptr<TopTree::ICluster> rightChild, std::shared_ptr<TopTree::ICluster> parent);
friend void TopTree::Split(std::shared_ptr<ICluster> leftChild, std::shared_ptr<ICluster> rightChild, std::shared_ptr<ICluster> parent);
friend void TopTree::Create(std::shared_ptr<ICluster> cluster, std::shared_ptr<EdgeData> edge);
friend void TopTree::Destroy(std::shared_ptr<ICluster> cluster, std::shared_ptr<EdgeData> edge);
friend void TopTree::CopyClusterData(std::shared_ptr<ICluster> from, std::shared_ptr<ICluster> to);
public:
	static DoubleConnectivity *dc;

	DoubleConnectivity(std::shared_ptr<TopTree::ITopTree> top_tree) {
		TT = top_tree;
		base_tree = std::make_shared<TopTree::BaseTree>();
		TT->InitFromBaseTree(base_tree); // empty for now
		dc = this;
	}

	// Decide if v and w are c-2-edge connected on level 0
	bool Double_edge_connected(int v, int w) {
		int vv = get_vertex(v);
		int ww = get_vertex(w);

		#ifdef DEBUG
			std::cerr << std::endl << "[Connected test] " << v << "(" << vv << ") and " << w << "(" << ww << ")" << std::endl;
		#endif

		auto cluster = TT->Expose(vv, ww);
		if (cluster == NULL) return false; // not even connected, cannot be double connected
		auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
		#ifdef DEBUG
			std::cerr << "data cover is " << data->cover << std::endl;
		#endif
		return (data->cover >= 0);
	} // COMPLETE

	std::shared_ptr<MyEdgeData> Insert(int v, int w) {
		if (v == w) return NULL;

		// 1. Try to Link v and w
		int vv = get_vertex(v);
		int ww = get_vertex(w);

		#ifdef DEBUG
			std::cerr << std::endl << "[Insert] " << v << "(" << vv << ") and " << w << "(" << ww << ")" << std::endl;
		#endif

		auto edge = std::make_shared<MyEdgeData>(vv, ww);
		auto result = TT->Link(vv, ww, edge);
		// 2. If link successful we end
		if (result != NULL) return edge;

		#ifdef DEBUG
			std::cerr << "[Insert B] Vertices are already connected, adding nontree edge" << std::endl;
		#endif

		// 3. Otherwise if cannot link (v and w are already connected)
		// set edge level
		edge->level = 0;
		// add edge to non tree edges
		nontree_edges.push_back(edge);
		register_at_vertices(edge);
		edge->nontree_edges_iterator = std::prev(nontree_edges.end());
		// call cover
		cover(edge, 0);

		return edge;
	}


	void Delete(std::shared_ptr<MyEdgeData> edge) {
		#ifdef DEBUG
			std::cerr << std::endl << "[Delete] " << edge->from << " and " << edge->to << std::endl;
		#endif
		// May be tree or nontree edge
		int vv = edge->from;
		int ww = edge->to;

		// 1. Expose v-w
		auto cluster = TT->Expose(vv, ww);
		if (cluster == NULL) {
			// This should never happen, but just for debug and to be sure
			std::cerr << "[Delete] Vertices " << vv << " and " << ww << " not even connected, cannot delete" << std::endl;
			unregister_at_vertices(edge);
			return;
		}
		auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);

		// 2. Delete edge
		if (data->edge != NULL) {
			// 2.1 If v-w is a bridge -> just delete it
			if (data->cover == -1) {
				TT->Cut(vv, ww);
				return;
			}
			// 2.2 Otherwise if it is an tree edge call Swap
			swap(vv, ww);
			// 2.3 Update cluster and data because they changed
			cluster = TT->Expose(vv, ww);
			if (cluster == NULL) {
				std::cerr << "[Delete] Vertices " << vv << " and " << ww << " - cannot get cluster after swap" << std::endl;
				unregister_at_vertices(edge);
				return;
			}
			data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
		}
		internal_uncover(cluster, edge->level);

		// If was nontree edge
		if (data->edge == NULL) unregister_at_vertices(edge);

		for (int i = edge->level; i >= 0; i--) recover(vv, ww, i);
	}

private:
	std::shared_ptr<TopTree::ITopTree> TT;
	std::shared_ptr<TopTree::BaseTree> base_tree;

	std::vector<bool> vertex_added;
	std::vector<uint> vertex_mapping; // from local indexes to the top trees structure indexes
	std::vector<uint> inverse_vertex_mapping; // from top trees structure indexes to local indexes

	std::list<std::shared_ptr<MyEdgeData>> nontree_edges;

	int get_vertex(uint v) {
		if (v >= vertex_added.size()) {
			vertex_added.resize(v+1);
			vertex_mapping.resize(v+1);
		}

		if (!vertex_added[v]) {
			N++;
			// Recompute max_l
			while (1<<max_l <= N) max_l++; // max_l = upper_part(log N)

			vertex_added[v] = true;
			auto vertex_data = std::make_shared<MyVertexData>(std::to_string(v));
			vertex_mapping[v] = base_tree->AddVertex(vertex_data);
			vertex_data->label = std::to_string(vertex_mapping[v]);

			if (vertex_mapping[v] >= inverse_vertex_mapping.size()) inverse_vertex_mapping.resize(vertex_mapping[v] + 1);
			inverse_vertex_mapping[vertex_mapping[v]] = v;
		}

		return vertex_mapping[v];
	}

	////////////////////////////////

	std::vector<std::vector<std::list<std::shared_ptr<MyEdgeData>>>> incident;
	std::vector<std::vector<int>> size;
	int max_l = 0;
	int N = 0;

	int join_counter = 0;

	int get_size(uint u, int i) {
		return 1; // citation: "For any vertex v and any level i: size[v][i]=1"
	}
	/*
	int get_size(uint u, int i) {
		// indexes are shifted +1
		uint i_index = i+1;
		if (size.size() <= u) size.resize(u+1);
		if (size[u].size() <= i_index) size[u].resize(i_index+1, 1);
		return size[u][i_index];
	}
	void set_size(uint u, int i, int value) {
		// indexes are shifted +1
		uint i_index = i+1;
		if (size.size() <= u) size.resize(u+1);
		if (size[u].size() <= i_index) size[u].resize(i_index+1, 1);
		size[u][i_index] = value;
	}*/
	int get_incident(uint u, int i) {
		// indexes are shifted +1
		uint i_index = i+1;
		if (incident.size() <= u) incident.resize(u+1);
		if (incident[u].size() <= i_index) incident[u].resize(i_index+1);
		return incident[u][i_index].size();
	}
	std::shared_ptr<MyEdgeData> get_incident_edge(uint u, int i) {
		// indexes are shifted +1
		uint i_index = i+1;
		// return first such edge
		return incident[u][i_index].front();
	}
	/*void set_incident(uint u, int i, int value) {
		// indexes are shifted +1
		uint i_index = i+1;
		if (incident.size() <= u) incident.resize(u+1);
		if (incident[u].size() <= i_index) incident[u].resize(i_index+1);
		incident[u][i_index] = value;
	}*/

	////////////////////////////////////////////////////////////////////////

	// For all e in v...w: if c(e) < i, set c(e)=i (lazy propagate down)
	void cover(std::shared_ptr<MyEdgeData> edge, int i) {
		#ifdef DEBUG
			std::cerr << "* Cover " << i << " for edge " << *edge << std::endl;
		#endif

		int vv = edge->from;
		int ww = edge->to;
		auto cluster = TT->Expose(vv, ww);
		if (cluster == NULL) {
			std::cerr << "Cover for edge " << *edge << " - cluster is NULL" << std::endl;
		}

		internal_cover(cluster, i, edge);
	}

	// For all e in v...w: if c(e)<=i, set c(e)=-1 (lazy propagate down)
	/*void uncover(int vv, int ww, int i) {
		auto cluster = TT->Expose(vv, ww);

		internal_uncover(cluster, i);
	}*/

	void register_at_vertices(std::shared_ptr<MyEdgeData> edge) {
		#ifdef DEBUG
			std::cerr << "* Registering edge " << *edge << " at vertices" << std::endl;
		#endif

		uint index = edge->level+1;

		if (incident.size() <= edge->from) incident.resize(edge->from+1);
		if (incident[edge->from].size() <= index) incident[edge->from].resize(index+1);
		incident[edge->from][index].push_back(edge);
		edge->from_incident_iterator = std::prev(incident[edge->from][index].end());

		if (incident.size() <= edge->to) incident.resize(edge->to+1);
		if (incident[edge->to].size() <= index) incident[edge->to].resize(index+1);
		incident[edge->to][index].push_back(edge);
		edge->to_incident_iterator = std::prev(incident[edge->to][index].end());

		edge->registered = true;
	}

	void unregister_at_vertices(std::shared_ptr<MyEdgeData> edge) {
		#ifdef DEBUG
			std::cerr << "* Unregistering edge " << *edge << " from vertices" << std::endl;
		#endif

		uint index = edge->level+1;

		if (edge->registered) {
			incident[edge->from][index].erase(edge->from_incident_iterator);
			incident[edge->to][index].erase(edge->to_incident_iterator);
			edge->registered = false;
		}
	}

	// INTERNAL:
	void internal_cover(std::shared_ptr<TopTree::ICluster> cluster, int i, std::shared_ptr<MyEdgeData> edge) {
		if (cluster == NULL) {
			std::cerr << "ERROR: No cluster for internal cover" << std::endl;
			return;
		}

		auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
		#ifdef DEBUG
			std::cerr << "* Internal cover " << i << " for edge ";
			if (edge != NULL) std::cerr << *edge;
			else std::cerr << "NULL";
			std::cerr << " at cluster " << cluster->getLeftBoundary() << "-" << cluster->getRightBoundary() << " joined in " << data->join_step << " with cover " << data->cover  << std::endl;
		#endif
		if (data->cover < i) {
			data->cover = i;
			data->cover_edge = edge;
		}
		// if (i < data->cover_set) // nothing
		if (data->cover_limit >= i && i >= data->cover_set) {
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

		#ifdef DEBUG
			std::cerr << "  -> resulting cover " << data->cover << std::endl;
		#endif
	} // COMPLETE

	void internal_uncover(std::shared_ptr<TopTree::ICluster> cluster, int i) {
		if (cluster == NULL) {
			std::cerr << "ERROR: No cluster for internal uncover" << std::endl;
			return;
		}

		auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
		#ifdef DEBUG
			std::cerr << "* Internal uncover " << i << " at cluster " << cluster->getLeftBoundary() << "-" << cluster->getRightBoundary() << " with cover " << data->cover  << std::endl;
		#endif
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
		// HOTFIX
		if ((data->endpoint_a != l && data->endpoint_b != l) || (data->endpoint_a != r && data->endpoint_b != r)) {
			data->endpoint_a = l;
			data->endpoint_b = r;
		}

		for (int j = -1; j <= i; j++) {
			for (int k = -1; k <= max_l; k++) {
				data->set_size(l, j, k, data->get_size(l, i+1, k));
				data->set_size(r, j, k, data->get_size(r, i+1, k));

				data->set_incident(l, j, k, data->get_incident(l, i+1, k));
				data->set_incident(r, j, k, data->get_incident(r, i+1, k));
			}
		}
	} // COMPLETE


	void clean(std::shared_ptr<TopTree::ICluster> leftChild, std::shared_ptr<TopTree::ICluster> rightChild, std::shared_ptr<TopTree::ICluster> parent) {
		// Part of the Split is an Clean method
		auto data = std::dynamic_pointer_cast<MyClusterData>(parent->data);

		#ifdef DEBUG_VERBOSE
			std::cerr << "Clean of cluster " << parent->getLeftBoundary() << "-" << parent->getRightBoundary() << " joined in step " << data->join_step;
		#endif

		// For each path child A of C call Uncover(A, cover_limit) and Cover(A, cover_set, cover_edge_set)
		if (isLeftRake(leftChild, rightChild, parent)) {
			#ifdef DEBUG_VERBOSE
				std::cerr << " - left rake" << std::endl;
			#endif
			// Only the right is a path child
			internal_uncover(rightChild, data->cover_limit);
			internal_cover(rightChild, data->cover_set, data->cover_edge_set);
		} else if (isRightRake(leftChild, rightChild, parent)) {
			#ifdef DEBUG_VERBOSE
				std::cerr << " - right rake" << std::endl;
			#endif
			// Only the left is a path child
			internal_uncover(leftChild, data->cover_limit);
			internal_cover(leftChild, data->cover_set, data->cover_edge_set);
		} else {
			#ifdef DEBUG_VERBOSE
				std::cerr << " - compress" << std::endl;
			#endif
			internal_uncover(leftChild, data->cover_limit);
			internal_cover(leftChild, data->cover_set, data->cover_edge_set);
			internal_uncover(rightChild, data->cover_limit);
			internal_cover(rightChild, data->cover_set, data->cover_edge_set);
		}

		data->cover_set = -1;
		data->cover_limit = -1;
		data->cover_edge_set = NULL;
	}

	void recover(int vv, int ww, int i) {
		#ifdef DEBUG
			std::cerr << "Recover " << i << " of path " << vv << "-" << ww << std::endl;
		#endif
		auto clusterC = TT->Expose(vv, ww);
		if (clusterC == NULL) {
			std::cerr << "Recover " << i << " of path " << vv << "-" << ww << " - cluster is NULL" << std::endl;
			return;
		}
		auto dataC = std::dynamic_pointer_cast<MyClusterData>(clusterC->data);

		std::shared_ptr<MyEdgeData> last_edge = NULL;

		// Firstly u=v
		auto u = clusterC->getLeftBoundary();
		auto uu = clusterC->getRightBoundary();
		while (true) {
			while (dataC->get_incident(u, -1, i) + get_incident(u, i) > 0) {
				#ifdef DEBUG
					std::cerr << "Recover step for " << clusterC->getLeftBoundary() << "-" << clusterC->getRightBoundary() << " with incident " << dataC->get_incident(u, -1, i) << " + " << get_incident(u, i) << std::endl;
				#endif
				std::shared_ptr<MyEdgeData> edge = find(u, clusterC, i);
				if (edge == last_edge) break;
				last_edge = edge;
				auto clusterD = TT->Expose(edge->from, edge->to);
				if (clusterD == NULL) {
					std::cerr << "Recover " << i << " of path " << vv << "-" << ww << " - clusterD is NULL" << std::endl;
					return;
				}
				auto dataD = std::dynamic_pointer_cast<MyClusterData>(clusterD->data);
				if ((dataD->get_size(edge->from, -1, i+1) + 2) > N/(1<<(i+1))) {
					internal_cover(clusterD, i, edge);
					break;
				} else {
					unregister_at_vertices(edge);
					edge->level = i+1;
					register_at_vertices(edge);
					internal_cover(clusterD, i+1, edge);
				}
				clusterC = TT->Expose(vv,ww);
				if (clusterC == NULL) {
					std::cerr << "Recover " << i << " of path " << vv << "-" << ww << " - clusterC is NULL" << std::endl;
					return;
				}
				dataC = std::dynamic_pointer_cast<MyClusterData>(clusterC->data);
			}
			if (u == uu) break;
			else u = uu; //end of the u=w run
		}
	}

	std::shared_ptr<MyEdgeData> find(int a, std::shared_ptr<TopTree::ICluster> cluster, int i) {
		#ifdef DEBUG
			std::cerr << "Find of " << a << " on level " << i << " in cluster " << cluster->getLeftBoundary() << "-" << cluster->getRightBoundary() << std::endl;
		#endif
		if (get_incident(a, i) > 0) {
			return get_incident_edge(a, i);
		} else {
			auto childs = TT->SplitRoot(cluster);
			if (childs.first == NULL || childs.second == NULL) {
				std::cerr << "ERROR: Cannot split cluster " << cluster->getLeftBoundary() << "-" << cluster->getRightBoundary() << " into childs" << std::endl;
			}

			clean(childs.first, childs.second, cluster);
			int common = childs.first->getLeftBoundary();
			if (common != childs.second->getLeftBoundary() && common != childs.second->getRightBoundary()) common = childs.first->getRightBoundary();

			auto A = childs.first;
			auto B = childs.second;
			int other_a = (A->getLeftBoundary() == common ? A->getRightBoundary() : A->getLeftBoundary());
			int other_b = (B->getLeftBoundary() == common ? B->getRightBoundary() : B->getLeftBoundary());

			if (a == other_b) {
				std::swap(other_a, other_b);
				std::swap(A, B);
			}

			auto data_a = std::dynamic_pointer_cast<MyClusterData>(A->data);
			//auto data_b = std::dynamic_pointer_cast<MyClusterData>(B->data);

			// If A is a nonpath child and ... or A is a path cluster and ...
			if (isLeftRake(A, B, cluster) && data_a->get_nonpath_incident(a, i) > 0) return find(a, A, i); // A is nonpath child
			else if (!isLeftRake(A, B, cluster) && data_a->get_incident(a, -1, i) > 0) return find(a, A, i); // A is path child
			else {
				// b is boundary vertex of B nearest to a -> common vertex
				return find(common, B, i);
			}
		}
	}

	void swap(int vv, int ww) {
		#ifdef DEBUG
			std::cerr << "[Swap] Swap of " << vv << "-" << ww << std::endl;
		#endif
		auto edgeCluster = TT->Expose(vv, ww);
		if (edgeCluster == NULL) {
			std::cerr << "[Swap] Swap of " << vv << "-" << ww << " - cluster is NULL" << std::endl;
		}
		auto data = std::dynamic_pointer_cast<MyClusterData>(edgeCluster->data);

		if (data->edge == NULL) {
			std::cerr << "Edge cluster " << vv << "-" << ww << " does not have underlying edge" << std::endl;
			return;
		}

		if (data->cover < 0) { // it is a bridge
			//std::cerr << "Edge " << *edgeCluster << " is a bridge, cannot swap" << std::endl;
			std::cerr << "Edge " << vv << "-" << ww << " is a bridge, cannot swap" << std::endl;
			return;
		}

		auto coverValue = data->cover;
		auto coverEdge = data->cover_edge;
		auto treeEdge = data->edge;

		#ifdef DEBUG
			std::cerr << "[Swap] swapping for " << coverEdge->from << "-" << coverEdge->to << std::endl;
		#endif

		// 1. Set level of the v-w to the cover
		treeEdge->level = coverValue;

		// 2. Replace tree edge v-w for coverEdge x-y
		// 2.1 Remove v-w from tree
		TT->Cut(vv, ww);
		// 2.2 Remove coverEdge from nontree_edges
		nontree_edges.erase(coverEdge->nontree_edges_iterator);
		unregister_at_vertices(coverEdge);
		// 2.3 Add x-y into tree
		TT->Link(coverEdge->from, coverEdge->to, coverEdge);
		// 2.5 Push v-w edge into nontree_edges
		nontree_edges.push_back(treeEdge);
		treeEdge->nontree_edges_iterator = std::prev(nontree_edges.end());
		register_at_vertices(treeEdge);

		// 3.1 InitTreeEdge(x,y)
		coverEdge->cover = -1;
		// 3.2 Call Cover (treeEdge is now nonTree edge)
		cover(treeEdge, coverValue);

		#ifdef DEBUG
			std::cerr << "Swap ended" << std::endl;
		#endif
	}

};
DoubleConnectivity* DoubleConnectivity::dc = NULL;


////////////////////////////////////////////////////////////////////////////////

// Merge
void TopTree::Join(std::shared_ptr<TopTree::ICluster> leftChild, std::shared_ptr<TopTree::ICluster> rightChild, std::shared_ptr<TopTree::ICluster> parent) {
	auto data = std::dynamic_pointer_cast<MyClusterData>(parent->data);
	auto left_data = std::dynamic_pointer_cast<MyClusterData>(leftChild->data);
	auto right_data = std::dynamic_pointer_cast<MyClusterData>(rightChild->data);

	auto dc = DoubleConnectivity::dc;

	#ifdef DEBUG
		std::cerr << "JOIN " << dc->join_counter <<  " of cluster " << leftChild->getLeftBoundary()  << "-" << leftChild->getRightBoundary() << "(" << left_data->cover << ")"
		<< " and " << rightChild->getLeftBoundary()  << "-" << rightChild->getRightBoundary() << "(" << right_data->cover << ")"
		<< " into " << parent->getLeftBoundary()  << "-" << parent->getRightBoundary() << std::endl;
	#endif

	// HOTFIX FIXUP
	/*
	if ((leftChild->getLeftBoundary() != left_data->endpoint_a && leftChild->getLeftBoundary() != left_data->endpoint_b)
		|| (leftChild->getRightBoundary() != left_data->endpoint_a && leftChild->getRightBoundary() != left_data->endpoint_b)) {
			left_data->endpoint_a = leftChild->getLeftBoundary();
			left_data->endpoint_b = leftChild->getRightBoundary();
		}

	// HOTFIX FIXUP
	if ((rightChild->getLeftBoundary() != right_data->endpoint_a && rightChild->getLeftBoundary() != right_data->endpoint_b)
		|| (rightChild->getRightBoundary() != right_data->endpoint_a && rightChild->getRightBoundary() != right_data->endpoint_b)) {
			right_data->endpoint_a = rightChild->getLeftBoundary();
			right_data->endpoint_b = rightChild->getRightBoundary();
		}
	*/

	/*
	std::cerr << "PARENT JOIN: " << parent->getLeftBoundary() << "-" << parent->getRightBoundary() << std::endl;

	std::cerr << "Left: " << leftChild->getLeftBoundary() << "-" << leftChild->getRightBoundary() << std::endl;
	std::cerr << "Left: " << left_data->endpoint_a << "-" << left_data->endpoint_b << std::endl;

	std::cerr << "Right: " << rightChild->getLeftBoundary() << "-" << rightChild->getRightBoundary() << std::endl;
	std::cerr << "Right: " << right_data->endpoint_a << "-" << right_data->endpoint_b << std::endl;
	*/

	/////////////////////////////////////////////////////////
	// Init endpoints in the new cluster - O(1) computations:
	data->endpoint_a = parent->getLeftBoundary();
	data->endpoint_b = parent->getRightBoundary();

	data->join_step = dc->join_counter++;

	// Find path child minimizing cover
	if (isLeftRake(leftChild, rightChild, parent)) {
		data->cover = right_data->cover;
		data->cover_edge = right_data->cover_edge;
		data->edge = right_data->edge; // copy the underlying edge if only from one child
	} else if (isRightRake(leftChild, rightChild, parent)) {
		data->cover = left_data->cover;
		data->cover_edge = left_data->cover_edge;
		data->edge = left_data->edge; // copy the underlying edge if only from one child
	} else {
		if (left_data->cover < right_data->cover) {
			data->cover = left_data->cover;
			data->cover_edge = left_data->cover_edge;
		} else {
			data->cover = right_data->cover;
			data->cover_edge = right_data->cover_edge;
		}
	}
	data->cover_set = -1;
	data->cover_limit = -1;
	data->cover_edge_set = NULL;

	///////////////////////////////////////////////////
	// Time consuming computations in O(log^2 N) below:

	int common = leftChild->getLeftBoundary();
	if (common != rightChild->getLeftBoundary() && common != rightChild->getRightBoundary()) common = leftChild->getRightBoundary();

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

	#ifdef DEBUG
		std::cerr << "JOIN result: cover " << data->cover << std::endl;
	#endif
} // COMPLETE
void TopTree::Split(std::shared_ptr<ICluster> leftChild, std::shared_ptr<ICluster> rightChild, std::shared_ptr<ICluster> parent) {
	DoubleConnectivity::dc->clean(leftChild, rightChild, parent);
	// delete C - not needed, it will be deleted by TopTrees structure
} // COMPLETE

// Creating and destroying Base clusters:
void TopTree::Create(std::shared_ptr<ICluster> cluster, std::shared_ptr<EdgeData> edge) {
	auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
	auto edge_data = std::dynamic_pointer_cast<MyEdgeData>(edge);

	#ifdef DEBUG
		std::cerr << "Creating cluster for edge " << edge_data->from << "-" << edge_data->to << " with cover " << edge_data->cover << std::endl;
	#endif

	data->endpoint_a = cluster->getLeftBoundary();
	data->endpoint_b = cluster->getRightBoundary();

	data->edge = edge_data;

	data->cover = edge_data->cover;

	// Backup (or defaults) from underlying edge
	data->cover = edge_data->cover;
	data->cover_edge = edge_data->cover_edge;
} // COMPLETE
void TopTree::Destroy(std::shared_ptr<ICluster> cluster, std::shared_ptr<EdgeData> edge) {
	auto data = std::dynamic_pointer_cast<MyClusterData>(cluster->data);
	auto edge_data = std::dynamic_pointer_cast<MyEdgeData>(edge);

	#ifdef DEBUG
		std::cerr << "Destroying cluster for edge " << edge_data->from << "-" << edge_data->to << " with cover " << data->cover << std::endl;
	#endif

	// Backup into cluster
	edge_data->cover = data->cover;
	edge_data->cover_edge = data->cover_edge;
} // COMPLETE

std::shared_ptr<TopTree::ClusterData> TopTree::InitClusterData() {
	return std::make_shared<MyClusterData>();
}

void TopTree::CopyClusterData(std::shared_ptr<ICluster> from, std::shared_ptr<ICluster> to) {
	#ifdef DEBUG
		//std::cerr << "COPY from cluster " << from->getLeftBoundary()  << "-" << from->getRightBoundary()
		//<< " to " << to->getLeftBoundary()  << "-" << to->getRightBoundary() << std::endl;
		std::cerr << "COPY from cluster " << from << " to " << to << std::endl;
	#endif

	auto fromData = std::dynamic_pointer_cast<MyClusterData>(from->data);
	auto toData = std::dynamic_pointer_cast<MyClusterData>(to->data);

	toData->join_step = fromData->join_step;

	toData->cover = fromData->cover;
	toData->cover_limit = fromData->cover_limit;
	toData->cover_set = fromData->cover_set;

	toData->edge = fromData->edge;
	toData->cover_edge = fromData->cover_edge;
	toData->cover_edge_set = fromData->cover_edge_set;

	toData->endpoint_a = fromData->endpoint_a;
	toData->endpoint_b = fromData->endpoint_b;

	auto dc = DoubleConnectivity::dc;

	if (dc->quick_expose && dc->quick_expose_running) return; // skip slow computations below

	auto a = toData->endpoint_a;
	auto b = toData->endpoint_b;

	for (int i = 0; i <= dc->max_l; i++) {
		for (int j = 0; j <= dc->max_l; j++) {
			toData->set_size(a, i, j, fromData->get_size(a, i, j));
			toData->set_size(b, i, j, fromData->get_size(b, i, j));
			toData->set_incident(a, i, j, fromData->get_incident(a, i, j));
			toData->set_incident(b, i, j, fromData->get_incident(b, i, j));
		}
		toData->set_nonpath_size(a, i, fromData->get_nonpath_size(a, i));
		toData->set_nonpath_size(b, i, fromData->get_nonpath_size(b, i));
		toData->set_nonpath_incident(a, i, fromData->get_nonpath_incident(a, i));
		toData->set_nonpath_incident(b, i, fromData->get_nonpath_incident(b, i));
	}
}
