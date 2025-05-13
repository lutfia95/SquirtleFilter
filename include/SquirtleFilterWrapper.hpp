#ifndef SQUIRTLEFILTERWRAPPER_H
#define SQUIRTLEFILTERWRAPPER_H

#include "SquirtleFilter.h"
#include <string>

class SquirtleFilterWrapper {
public:
    SquirtleFilterWrapper(uint64_t expected_items, double false_positive_rate, uint8_t hash_functions)
        : bf(expected_items, false_positive_rate, hash_functions) {}

    void insert(const std::string& key) {
        bf.insert(key.data(), key.size());
    }

    bool contains(const std::string& key) const {
        return bf.contains(key.data(), key.size());
    }

private:
    BloomFilter bf;
};

#endif
