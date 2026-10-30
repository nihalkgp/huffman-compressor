#ifndef BITIO_HPP
#define BITIO_HPP

#include <fstream>
#include <string>
#include <cstdint>

class BitWriter {
private:
    std::ofstream& out;
    uint8_t buffer;
    int bitCount;

public:
    explicit BitWriter(std::ofstream& os) : out(os), buffer(0), bitCount(0) {}

    inline void writeBit(int bit) {
        buffer = (buffer << 1) | (bit & 1);
        bitCount++;
        if (bitCount == 8) {
            out.put(static_cast<char>(buffer));
            buffer = 0;
            bitCount = 0;
        }
    }

    inline void writeBitString(const std::string& bits) {
        for (char c : bits) {
            writeBit(c - '0');
        }
    }

    // Pad leftover bits with trailing 0s to complete the final byte
    void flush() {
        if (bitCount > 0) {
            buffer <<= (8 - bitCount);
            out.put(static_cast<char>(buffer));
            buffer = 0;
            bitCount = 0;
        }
    }
};

class BitReader {
private:
    std::ifstream& in;
    uint8_t buffer;
    int bitCount;

public:
    explicit BitReader(std::ifstream& is) : in(is), buffer(0), bitCount(8) {}

    // Reads the next single bit (returns false if EOF is reached)
    inline bool readBit(int& bit) {
        if (bitCount == 8) {
            char c;
            if (!in.get(c)) {
                return false;
            }
            buffer = static_cast<uint8_t>(c);
            bitCount = 0;
        }
        bit = (buffer >> (7 - bitCount)) & 1;
        bitCount++;
        return true;
    }
};

#endif // BITIO_HPP