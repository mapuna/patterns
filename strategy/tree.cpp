#include "tree.h"

#include <queue>
#include <stdexcept>

template <typename T>
Node<T>::Node(const T& v) : data(v) {}

template <typename T>
void InOrderTraversal<T>::traverse(
    const Node<T>* node, const std::function<void(const T&)>& visit) const {
    if (!node) return;

    traverse(node->left.get( ), visit);
    visit(node->data);
    traverse(node->right.get( ), visit);
}

template <typename T>
void PreOrderTraversal<T>::traverse(
    const Node<T>* node, const std::function<void(const T&)>& visit) const {
    if (!node) return;

    visit(node->data);
    traverse(node->left.get( ), visit);
    traverse(node->right.get( ), visit);
}

template <typename T>
void PostOrderTraversal<T>::traverse(
    const Node<T>* node, const std::function<void(const T&)>& visit) const {
    if (!node) return;

    traverse(node->left.get( ), visit);
    traverse(node->right.get( ), visit);
    visit(node->data);
}

template <typename T>
void LevelOrderTraversal<T>::traverse(
    const Node<T>* node, const std::function<void(const T&)>& visit) const {
    if (!node) return;

    std::queue<const Node<T>*> q;
    q.push(node);

    while (!q.empty( )) {
        const Node<T>* cur = q.front( );
        q.pop( );
        visit(cur->data);
        if (cur->left) q.push(cur->left.get( ));
        if (cur->right) q.push(cur->right.get( ));
    }
}

// tree impl
template <typename T>
Tree<T>::Tree(std::unique_ptr<TraversalStrategy<T>> strategy)
    : _strategy(std::move(strategy)) {}

template <typename T>
void Tree<T>::set_root(std::unique_ptr<Node<T>> r) {
    _root = std::move(r);
}

template <typename T>
void Tree<T>::set_strategy(std::unique_ptr<TraversalStrategy<T>> strategy) {
    _strategy = std::move(strategy);
}

template <typename T>
void Tree<T>::traverse(const std::function<void(const T&)>& visit) const {
    if (!_strategy) throw std::runtime_error("Traversal strategy is not set");
    _strategy->traverse(_root.get( ), visit);
}

// since we are doing tree.h / tree.cpp split if templates' decl/def, we
// explicitly instantiate for the desired type. `int` here:
template struct Node<int>;
template class InOrderTraversal<int>;
template class PreOrderTraversal<int>;
template class PostOrderTraversal<int>;
template class LevelOrderTraversal<int>;
template class Tree<int>;
