#include <memory>

#ifndef BASE_TREE_HPP
#define BASE_TREE_HPP

namespace TopTree {

class BaseTree {
public:
	BaseTree();
	~BaseTree();

	int AddVertex();
	int AddEdge(int a, int b);
private:
	class BaseTreeData;
	std::unique_ptr<BaseTreeData> data;
};

}

#endif // BASE_TREE_HPP
