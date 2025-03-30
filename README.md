# SquirtleFilter
A blazing-fast, memory-efficient, thread-safe Bloom Filter implementation in modern C++. Powered by the precision of hash functions and the spirit of Squirtle.

# 🐢 SquirtleFilter

**SquirtleFilter** is a high-performance, thread-safe Bloom Filter written in modern C++17, designed for fast lookups, efficient memory use, and scalable insertions. Just like its namesake, SquirtleFilter is small, fast, and ready to battle false positives!

## 🚀 Features
- Fast insert & lookup
- Configurable number of hash functions (1 to 5)
- Dynamic sizing to maintain low false-positive rates
- Thread-safe with atomic bit operations
- Built with modern C++17 and CMake
- Uses high-speed hashing (MurmurHash3 by default)

##  Building

```bash
git clone https://github.com/lutfia95/SquirtleFilter.git
cd SquirtleFilter
mkdir build && cd build
cmake ..
make
