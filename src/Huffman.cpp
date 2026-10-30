#include "Huffman.hpp"
#include "BitIO.hpp"
#include <iostream>
#include <queue>
#include <vector>
#include <iomanip>
#include <chrono>

HuffmanCompressor::HuffmanCompressor() : root(nullptr), totalBytes(0) {
    freqTable.fill(0);
    codeTable.fill("");
}

HuffmanCompressor::~HuffmanCompressor() {
    deleteTree(root);
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

    if (pq.empty()) return;

    // Single unique byte edge case (e.g., "aaaaaaa")
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
        pq.push(new Node(left, right));
    }

    root = pq.top();
}

void HuffmanCompressor::generateCodes(const Node* node, const std::string& currentCode) {
    if (!node) return;

    if (node->isLeaf()) {
        codeTable[node->byte] = currentCode.empty() ? "0" : currentCode;
        return;
    }

    generateCodes(node->left, currentCode + "0");
    generateCodes(node->right, currentCode + "1");
}

void HuffmanCompressor::writeHeader(std::ofstream& outFile) const {
    // 1. Magic bytes "HUFF" (4 bytes)
    outFile.write("HUFF", 4);

    // 2. Original uncompressed size (8 bytes)
    outFile.write(reinterpret_cast<const char*>(&totalBytes), sizeof(totalBytes));

    // 3. Number of unique bytes (2 bytes)
    uint16_t uniqueCount = 0;
    for (int i = 0; i < 256; ++i) {
        if (freqTable[i] > 0) uniqueCount++;
    }
    outFile.write(reinterpret_cast<const char*>(&uniqueCount), sizeof(uniqueCount));

    // 4. Pairs of (1-byte character, 8-byte frequency)
    for (int i = 0; i < 256; ++i) {
        if (freqTable[i] > 0) {
            uint8_t b = static_cast<uint8_t>(i);
            outFile.write(reinterpret_cast<const char*>(&b), sizeof(b));
            outFile.write(reinterpret_cast<const char*>(&freqTable[i]), sizeof(freqTable[i]));
        }
    }
}

bool HuffmanCompressor::readHeader(std::ifstream& inFile) {
    char magic[4];
    if (!inFile.read(magic, 4) || std::string(magic, 4) != "HUFF") {
        std::cerr << "Error: Invalid .huff file (magic bytes mismatch).\n";
        return false;
    }

    inFile.read(reinterpret_cast<char*>(&totalBytes), sizeof(totalBytes));

    uint16_t uniqueCount = 0;
    inFile.read(reinterpret_cast<char*>(&uniqueCount), sizeof(uniqueCount));

    freqTable.fill(0);
    for (uint16_t i = 0; i < uniqueCount; ++i) {
        uint8_t b = 0;
        uint64_t f = 0;
        inFile.read(reinterpret_cast<char*>(&b), sizeof(b));
        inFile.read(reinterpret_cast<char*>(&f), sizeof(f));
        freqTable[b] = f;
    }
    return inFile.good() || inFile.eof();
}

bool HuffmanCompressor::compressFile(const std::string& inputPath, const std::string& outputPath) {
    auto startTime = std::chrono::high_resolution_clock::now();

    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error: Cannot open input file '" << inputPath << "'\n";
        return false;
    }

    freqTable.fill(0);
    totalBytes = 0;

    char buffer[65536];
    while (inFile.read(buffer, sizeof(buffer)) || inFile.gcount() > 0) {
        std::streamsize n = inFile.gcount();
        for (std::streamsize i = 0; i < n; ++i) {
            freqTable[static_cast<uint8_t>(buffer[i])]++;
        }
        totalBytes += n;
        if (inFile.eof()) break;
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Error: Cannot open output file '" << outputPath << "'\n";
        return false;
    }

    // Handle empty file gracefully
    if (totalBytes == 0) {
        writeHeader(outFile);
        std::cout << "Compressed empty file (0 bytes).\n";
        return true;
    }

    buildTree();
    codeTable.fill("");
    generateCodes(root, "");

    // Write header and track header size
    writeHeader(outFile);
    uint64_t headerBytes = static_cast<uint64_t>(outFile.tellp());

    // Rewind input file to beginning for encoding pass
    inFile.clear();
    inFile.seekg(0, std::ios::beg);

    BitWriter bitWriter(outFile);
    while (inFile.read(buffer, sizeof(buffer)) || inFile.gcount() > 0) {
        std::streamsize n = inFile.gcount();
        for (std::streamsize i = 0; i < n; ++i) {
            uint8_t b = static_cast<uint8_t>(buffer[i]);
            bitWriter.writeBitString(codeTable[b]);
        }
        if (inFile.eof()) break;
    }
    bitWriter.flush();
    outFile.flush();

    uint64_t compressedTotal = static_cast<uint64_t>(outFile.tellp());
    uint64_t payloadBytes = compressedTotal - headerBytes;

    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    double payloadReduction = (1.0 - (static_cast<double>(payloadBytes) / totalBytes)) * 100.0;
    double totalReduction = (1.0 - (static_cast<double>(compressedTotal) / totalBytes)) * 100.0;

    std::cout << "=== Compression Complete ===\n"
              << "Original Size   : " << totalBytes << " bytes\n"
              << "Header Overhead : " << headerBytes << " bytes\n"
              << "Encoded Payload : " << payloadBytes << " bytes ("
              << std::fixed << std::setprecision(2) << payloadReduction << "% payload reduction)\n"
              << "Total .huff Size: " << compressedTotal << " bytes ("
              << totalReduction << "% net reduction)\n"
              << "Time Taken      : " << elapsedMs << " ms\n";

    return true;
}

bool HuffmanCompressor::decompressFile(const std::string& inputPath, const std::string& outputPath) {
    auto startTime = std::chrono::high_resolution_clock::now();

    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error: Cannot open compressed file '" << inputPath << "'\n";
        return false;
    }

    if (!readHeader(inFile)) return false;

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Error: Cannot open output file '" << outputPath << "'\n";
        return false;
    }

    if (totalBytes == 0) return true;

    buildTree();

    BitReader bitReader(inFile);
    const Node* curr = root;
    uint64_t bytesWritten = 0;
    int bit = 0;

    while (bytesWritten < totalBytes && bitReader.readBit(bit)) {
        curr = (bit == 0) ? curr->left : curr->right;

        if (curr->isLeaf()) {
            outFile.put(static_cast<char>(curr->byte));
            bytesWritten++;
            curr = root;
        }
    }
    outFile.flush();

    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    std::cout << "=== Decompression Complete ===\n"
              << "Restored Size   : " << bytesWritten << " / " << totalBytes << " bytes\n"
              << "Time Taken      : " << elapsedMs << " ms\n";

    return bytesWritten == totalBytes;
}