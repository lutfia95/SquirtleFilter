#include <iostream>
#include "include/SquirtleFilter.h"

int main() {
    BloomFilter bf(1000, 0.01, 3);

    // Insert some sample keys
    bf.insert("hello");
    bf.insert("world");
    bf.insert("bloom");
    bf.insert("filter");

    std::cout << "\"hello\" -> " << (bf.contains("hello") ? "possibly in set" : "not in set") << std::endl;
    std::cout << "\"world\" -> " << (bf.contains("world") ? "possibly in set" : "not in set") << std::endl;

    std::string testKey = "test";
    std::cout << "\"test\" -> " << (bf.contains(testKey) ? "possibly in set" : "not in set") << std::endl;

    for (int i = 0; i < 2000; ++i) {
        std::string key = "key" + std::to_string(i);
        bf.insert(key);
    }
    std::cout << "Inserted 2000 additional elements." << std::endl;
    std::cout << "\"key100\" -> " << (bf.contains("key100") ? "possibly in set" : "not in set") << std::endl;
    std::cout << "\"nonexistent\" -> " << (bf.contains("nonexistent") ? "possibly in set" : "not in set") << std::endl;

    return 0;
}
