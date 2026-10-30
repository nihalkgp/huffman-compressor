CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -Iinclude
TARGET = huffman
SRCS = src/main.cpp src/Huffman.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe *.huff
