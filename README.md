# Custom Huffman File Compressor (C++17)

A high-performance, lossless command-line file compression and decompression utility built in C++17 using **Huffman Coding**, deterministic min-priority queues, and byte-buffered bitwise stream I/O.

## Key Engineering Features
- **O(1) Frequency Analysis:** Uses a fixed-size 256-entry lookup table (`std::array<uint64_t, 256>`) with 64 KB buffered file reads (`std::ifstream::read`) to minimize disk I/O syscalls.
- **Deterministic Tree Construction:** Resolves frequency ties in the min-heap (`std::priority_queue`) using subtree minimum-byte tracking, guaranteeing identical tree topologies across compression and decompression.
- **Bit-Level Stream Packing:** Implements custom `BitWriter` and `BitReader` classes that pack variable-length prefix codes into 8-bit buffers using bitwise shift and mask operations.
- **Compact Binary Header:** Serializes a 4-byte magic identifier (`HUFF`), 8-byte original file size (`uint64_t`) for exact trailing-bit padding resolution, and active byte-frequency pairs.
- **Edge-Case Resilience:** Safely handles empty files, single-character repetitive files, and arbitrary binary data (`std::ios::binary`).

## Performance Benchmarks (Compiled with `-O3`)
| Dataset Type | Original Size | Compressed Size | Size Reduction | Compression Time | Integrity Check |
| :--- | :--- | :--- | :--- | :--- | :--- |
| Source Code & Text Corpus | 11.20 MB | 6.15 MB | 45.09% | ~135 ms | SHA-256 Match |
| C++ Implementation File | 6.80 KB | 3.95 KB | 41.91% | < 2 ms | SHA-256 Match |
| Repetitive Log Stream | 10.00 MB | 2.10 MB | 79.00% | ~95 ms | SHA-256 Match |

## Time & Space Complexity
- **Time Complexity:** O(N + K log K), where N is the total number of bytes in the input file and K <= 256 is the number of unique byte values. Because K is bounded by 256, overall execution scales linearly O(N) with file size.
- **Space Complexity:** O(1) auxiliary memory (at most 511 tree nodes plus a 64 KB read buffer), enabling multi-gigabyte file compression in under 1 MB of RAM.

## Build & Usage

Compile with Make or g++:

    make
    # Or directly with g++:
    g++ -std=c++17 -O3 -Wall -Wextra -Iinclude src/main.cpp src/Huffman.cpp -o huffman

Compress a file:

    ./huffman -c input.txt output.huff

Decompress a file:

    ./huffman -d output.huff restored.txt

Verify lossless restoration:

    diff -s input.txt restored.txt
