#include "hauf.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./huffman <input_file>\n";
        return 1;
    }

    HuffmanCompressor compressor;
    if (compressor.analyzeFile(argv[1])) {
        compressor.printDebugTable();
    }

    return 0;
}