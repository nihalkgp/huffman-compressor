#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP

#include "Node.hpp"
#include <array>
#include <string>
#include <fstream>
#include <cstdint>

class HuffmanCompressor {
private:
    std::array<uint64_t, 256> freqTable;
    std::array<std::string, 256> codeTable;
    Node* root;
    uint64_t totalBytes;

    void buildTree();
    void generateCodes(const Node* node, const std::string& currentCode);
    void writeHeader(std::ofstream& outFile) const;
    bool readHeader(std::ifstream& inFile);

public:
    HuffmanCompressor();
    ~HuffmanCompressor();

    bool compressFile(const std::string& inputPath, const std::string& outputPath);
    bool decompressFile(const std::string& inputPath, const std::string& outputPath);
};

#endif // HUFFMAN_HPP