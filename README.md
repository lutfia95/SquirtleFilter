# SquirtleFilter

SquirtleFilter is a blazing-fast, memory-efficient, thread-safe Bloom Filter written in modern C++17. Designed for high-throughput insertions and lookups, with scalable performance and no external dependencies in the core.

Inspired by the mighty Squirtle — small, speedy, and packs a punch!

## Features

- Super fast insert & lookup
- Configurable hash functions (1 to 5)
- Dynamically grows to maintain low false-positive rate
- Thread-safe insertions and lookups using atomics
- Clean, modular CMake project structure
- Unit testing with Google Test
- Benchmarking with Google Benchmark

## Building the Project

```bash
git clone https://github.com/yourusername/SquirtleFilter.git
cd SquirtleFilter
mkdir build && cd build
cmake ..
make
```

This builds:
- `SquirtleMain`: example executable
- `SquirtleTests`: unit tests
- `SquirtleBenchmark`: benchmarking tool

## Running Unit Tests

```bash
ctest --output-on-failure
```

Or directly:

```bash
./SquirtleTests
```

## Running Benchmarks

```bash
./SquirtleBenchmark
```

This runs performance tests on insert and lookup operations.

## Running the Example

```bash
./SquirtleMain
```

Expected output:

```
Possibly in set
Definitely not in set
```

## Example Usage

```cpp
#include "SquirtleFilter.h"
#include <iostream>

int main() {
    BloomFilter bf(1000, 0.01, 3);
    bf.insert("squirtle");

    if (bf.contains("squirtle"))
        std::cout << "Possibly in set" << std::endl;

    if (!bf.contains("pikachu"))
        std::cout << "Definitely not in set" << std::endl;

    return 0;
}
```

## Dependencies

All dependencies are handled automatically via CMake:

- Google Test (for unit testing)
- Google Benchmark (for performance benchmarking)

You do not need to install them manually.

## License

This project is licensed under the Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0). Commercial use is not permitted. See [LICENSE](LICENSE) for details.

## Project Structure

```
SquirtleFilter/
├── CMakeLists.txt
├── LICENSE
├── README.md
├── benchmarks/
│   └── benchmark.cpp
├── include/
│   └── SquirtleFilter.h
├── main.cpp
├── src/
│   └── SquirtleFilter.cpp
└── tests/
    └── tests.cpp