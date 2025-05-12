#include "tree.h"
#include <iostream>

int main() {
    /**
     * Build a small tree:
     *        4
     *      /   \
     *     2     5
     *    / \
     *   1   3
     */

    auto n1 = std::make_unique<Node<int>>(1);
    auto n2 = std::make_unique<Node<int>>(2);
    auto n3 = std::make_unique<Node<int>>(3);
    n2->left = std::move(n1);
    n2->right = std::move(n3);
    auto n5 = std::make_unique<Node<int>>(5);
    auto root = std::make_unique<Node<int>>(4);
    root->left = std::move(n2);
    root->right = std::move(n5);

    Tree<int> tree;
    tree.set_root(std::move(root));

    // our visit function just pronts the value stored at a node
    auto printer = [](const int& v) { std::cout << v << ' '; };
    tree.set_strategy(std::make_unique<InOrderTraversal<int>>());
    std::cout << "In-order: ";
    tree.traverse(printer);
    std::cout << std::endl;

    tree.set_strategy(std::make_unique<PreOrderTraversal<int>>());
    std::cout << "Pre-order: ";
    tree.traverse(printer);
    std::cout << std::endl;

    tree.set_strategy(std::make_unique<PostOrderTraversal<int>>());
    std::cout << "Post-order: ";
    tree.traverse(printer);
    std::cout << std::endl;

    tree.set_strategy(std::make_unique<LevelOrderTraversal<int>>());
    std::cout << "Level-order: ";
    tree.traverse(printer);
    std::cout << std::endl;

    return 0;
}
