#include <memory>

#include "BaseTree.hpp"

#ifndef TOP_TREE_HPP
#define TOP_TREE_HPP

namespace TopTree {

template <typename TData>
class AbstractTopTree {
public:
	AbstractTopTree();
	AbstractTopTree(std::shared_ptr<BaseTree> baseTree); // Construct from underlying tree

	// USER DEFINED FUNCTIONS:
	// (abstract functions to be defined in custom Top Trees inherited from this class)

	// Joining and splitting of compress/rake clusters:
	virtual void Join(std::shared_ptr<TData> leftChild, std::shared_ptr<TData> rightChild, std::shared_ptr<TData> parent) = 0;
	virtual void Split(std::shared_ptr<TData> leftChild, std::shared_ptr<TData> rightChild, std::shared_ptr<TData> parent) = 0;

	// Creating and destroying Base clusters:
	virtual void Create(std::shared_ptr<TData> edgeData) = 0;
	virtual void Destroy(std::shared_ptr<TData> edgeData) = 0;
	// END OF USER DEFINED FUNCTIONS

	// Functions that could be called on Top Tree:
};

}

#endif // TOP_TREE_HPP
