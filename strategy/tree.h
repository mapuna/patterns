#ifndef TREE_H
#define TREE_H

#include <memory>
#include <functional>

// binary tree node
template<typename T>
struct Node {
    T data;
    std::unique_ptr<Node> left, right;
    explicit Node(const T& v);
};

// strategy interface 
template<typename T>
class TraversalStrategy {
public:
    virtual ~TraversalStrategy() = default;
    virtual void traverse(const Node<T>* node, const std::function<void(const T&)>& visit) const = 0;
};

// different traversal *declarations
// 1. in-order
template<typename T>
class InOrderTraversal : public TraversalStrategy<T> {
public:
    void traverse(const Node<T>* node, const std::function<void(const T&)>& visit) const override;
};

// 2. pre-order
template<typename T>
class PreOrderTraversal: public TraversalStrategy<T> {
public:
    void traverse(const Node<T>* node, const std::function<void(const T&)>& visit) const override;
};

// 3. post-order
template<typename T>
class PostOrderTraversal: public TraversalStrategy<T> {
public:
    void traverse(const Node<T>* node, const std::function<void(const T&)>& visit) const override;
};

// 4. level-order (breadth-first)
template<typename T>
class LevelOrderTraversal: public TraversalStrategy<T> {
public:
    void traverse(const Node<T>* node, const std::function<void(const T&)>& visit) const override;
};

// context is a tree holding a TraversalStrategy object reference
template<typename T>
class Tree {
    std::unique_ptr<Node<T>> _root;
    std::unique_ptr<TraversalStrategy<T>> _strategy;

public:
    Tree() = default;
    explicit Tree(std::unique_ptr<TraversalStrategy<T>> strategy);

    void set_root(std::unique_ptr<Node<T>> r);
    void set_strategy(std::unique_ptr<TraversalStrategy<T>> strategy);
    void traverse(const std::function<void(const T&)>& visit) const;
};

#endif // TREE_H
