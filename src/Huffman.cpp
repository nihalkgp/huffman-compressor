#include "hauf.hpp"
#include <fstream>
#include <iostream>
#include <queue>
#include <vector>
#include <iomanip>
#include <cctype>

HuffmanCompressor::HuffmanCompressor() : root(nullptr), totalBytes(0) {
    freqTable.fill(0);
    codeTable.fill("");
}

HuffmanCompressor::~HuffmanCompressor() {
    deleteTree(root);
}

bool HuffmanCompressor::analyzeFile(const std::string& inputPath) {
    // Always open in binary mode 
    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open file '" << inputPath << "'\n";
        return false;
    }

    freqTable.fill(0);
    totalBytes = 0;

    // Read file in 64 kb chunks
    char buffer[65536];
    while (inFile.read(buffer, sizeof(buffer)) || inFile.gcount() > 0) {
        std::streamsize bytesRead = inFile.gcount();
        for (std::streamsize i = 0; i < bytesRead; ++i) {
            uint8_t byte = static_cast<uint8_t>(buffer[i]);
            freqTable[byte]++;
        }
        totalBytes += bytesRead;
        if (inFile.eof()) break;
    }

    if (totalBytes == 0) {
        std::cerr << "Error: Input file is empty.\n";
        return false;
    }

    buildTree();
    codeTable.fill("");
    generateCodes(root, "");
    return true;
}

void HuffmanCompressor::buildTree() {
    deleteTree(root);
    root = nullptr;

    std::priority_queue<Node*, std::vector<Node*>, CompareNode> pq;

    for (int i = 0; i < 256; ++i) {
        if (freqTable[i] > 0) {
            pq.push(new Node(static_cast<uint8_t>(i), freqTable[i]));
        }
    }

    // Edge case: File contains only 1 unique character
    if (pq.size() == 1) {
        Node* onlyChild = pq.top();
        pq.pop();
        root = new Node(onlyChild->byte, onlyChild->freq);
        root->left = onlyChild;
        return;
    }

    while (pq.size() > 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();

        Node* parent = new Node(left, right);
        pq.push(parent);
    }

    root = pq.top();
}

void HuffmanCompressor::generateCodes(const Node* node, const std::string& currentCode) {
    if (!node) return;

    if (node->isLeaf()) {
        // If single-character edge case, ensure at least bit "0" is assigned
        codeTable[node->byte] = currentCode.empty() ? "0" : currentCode;
        return;
    }

    generateCodes(node->left, currentCode + "0");
    generateCodes(node->right, currentCode + "1");
}

void HuffmanCompressor::printDebugTable() const {
    std::cout << "\nTotal File Size: " << totalBytes << " bytes\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << "Byte  | Char | Frequency    | Huffman Binary Code\n";
    std::cout << "--------------------------------------------------------\n";

    uint64_t estimatedBits = 0;
    for (int i = 0; i < 256; ++i) {
        if (freqTable[i] > 0) {
            estimatedBits += freqTable[i] * codeTable[i].length();
            char displayChar = std::isprint(i) ? static_cast<char>(i) : '.';
            std::cout << std::setw(5) << i << " |  "
                      << displayChar << "   | "
                      << std::setw(12) << freqTable[i] << " | "
                      << codeTable[i] << "\n";
        }
    }
    std::cout << "--------------------------------------------------------\n";
    double compressedBytes = (estimatedBits + 7) / 8;
    double savings = (1.0 - (compressedBytes / totalBytes)) * 100.0;
    std::cout << "Theoretical Payload Size: " << static_cast<uint64_t>(compressedBytes)
              << " bytes (" << std::fixed << std::setprecision(2) << savings << "% reduction)\n\n";
}