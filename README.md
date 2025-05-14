# SquirtleFilter

SquirtleFilter is a fast, memory-efficient, thread-safe Bloom Filter written in modern C++17.

## Building the Project

```bash
git clone https://github.com/lutfia95/SquirtleFilter.git
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
C++ 
```cpp
#include "SquirtleFilter.h"
#include <iostream>

int main() {
    BloomFilter bf(1000, 0.01, 3);
    bf.insert("squirtle");

    if (bf.contains("squirtle"))
        std::cout << "Possibly in set" << std::endl;

    if (!bf.contains("ditto"))
        std::cout << "Definitely not in set" << std::endl;

    return 0;
}
```
Python
```py
import squirtlefilter

# Initialize Bloom filter
bf = squirtlefilter.SquirtleFilter(1000000, 0.01, 3)
bf.insert("squirtle")

# Query
print(bf.contains("squirtle"))    # True
print(bf.contains("ditto"))
```

## Dependencies

All dependencies are handled automatically via CMake:

- Google Test (for unit testing)
- Google Benchmark (for performance benchmarking)

You do not need to install them manually.

## License

This project is licensed under the GNU Affero General Public License (AGPL). Commercial use must comply with AGPL requirements. See [LICENSE](LICENSE) for details.

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
