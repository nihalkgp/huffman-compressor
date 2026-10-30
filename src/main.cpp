#include "Huffman.hpp"
#include <iostream>
#include <string>

void printUsage() {
    std::cout << "Usage:\n"
              << "  Compress  : ./huffman -c <input_file> <output.huff>\n"
              << "  Decompress: ./huffman -d <input.huff> <restored_file>\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printUsage();
        return 1;
    }

    std::string mode = argv[1];
    std::string inputPath = argv[2];
    std::string outputPath = argv[3];

    HuffmanCompressor compressor;

    if (mode == "-c") {
        return compressor.compressFile(inputPath, outputPath) ? 0 : 1;
    } else if (mode == "-d") {
        return compressor.decompressFile(inputPath, outputPath) ? 0 : 1;
    } else {
        std::cerr << "Error: Unknown flag '" << mode << "'\n";
        printUsage();
        return 1;
    }
}