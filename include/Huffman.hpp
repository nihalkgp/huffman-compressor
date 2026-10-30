#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP

#include "Node.hpp"
#include <array>
#include <string>
#include <cstdint>

class HuffmanCompressor {
private:
    std::array<uint64_t, 256> freqTable;
    std::array<std::string, 256> codeTable;
    Node* root;
    uint64_t totalBytes;

    void buildTree();
    void generateCodes(const Node* node, const std::string& currentCode);

public:
    HuffmanCompressor();
    ~HuffmanCompressor();

    bool analyzeFile(const std::string& inputPath);
    void printDebugTable() const;
};

#endif // HUFFMAN_HPP