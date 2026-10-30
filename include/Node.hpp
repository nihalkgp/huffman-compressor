#ifndef NODE_HPP
#define NODE_HPP

#include <cstdint>

struct Node {
    uint8_t byte;       
    uint64_t freq;      
    uint8_t minByte;    
    Node* left;
    Node* right;

    // Const for leaf node
    Node(uint8_t b, uint64_t f)
        : byte(b), freq(f), minByte(b), left(nullptr), right(nullptr) {}

    // Constructor for an Internal Node (merged parent of two nodes)
    Node(Node* l, Node* r)
        : byte(0), freq(l->freq + r->freq),
          minByte(l->minByte < r->minByte ? l->minByte : r->minByte),
          left(l), right(r) {}

    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }
};

//comparator
struct CompareNode {
    bool operator()(const Node* a, const Node* b) const {
        if (a->freq != b->freq) {
            return a->freq > b->freq; // Smaller frequency gets higher priority
        }
        // Deterministic tie-breaker so compression and decompression build the exact same tree
        return a->minByte > b->minByte;
    }
};

//memory cleanup recoursive
inline void deleteTree(Node* root) {
    if (!root) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

#endif 