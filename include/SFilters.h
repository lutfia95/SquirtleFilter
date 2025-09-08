#ifndef SFILTERS_H
#define SFILTERS_H

#include "SquirtleFilter.h"
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>


/**
 * @brief Manages a collection of Bloom filter instances, enabling operations across multiple filters.
 *
 * The `SFilters` class (likely short for "Scalable Filters" or "Set of Filters")
 * provides a higher-level abstraction for working with multiple `BloomFilter` objects.
 * It allows for initialization of a specified number of filters, inserting items
 * into specific filters, serializing/deserializing the entire collection,
 * and querying the presence of items across all managed filters. This can be
 * useful for tiered Bloom filters, sharded filters, or scenarios requiring
 * multiple independent filters.
 */
class SFilters {
public:
    SFilters() = default;

    // Create N filters
    void initialize(size_t num_filters, size_t expected_items, double false_positive_rate, uint8_t hash_functions);

    // Insert into a specific filter
    void insert(size_t index, const std::string& key);
    void insert(size_t index, double value);

    // Write all filters to file
    void writeToFile(const std::string& output_path) const;

    // Load filters from file
    void loadFromFile(const std::string& input_path);

    // Search across all filters
    template <typename T>
    bool contains(const T& key) const;

    // Get underlying filters (const)
    const std::vector<BloomFilter>& getFilters() const;

    template <typename T>
    std::vector<size_t> matchFilters(const T& key) const;

    std::vector<int> matchBitVector(const std::string& key) const;
    std::vector<int> matchBitVector(double value) const;

    void printSummary() const;

private:
    std::vector<BloomFilter> filters;
};

#include "SFilters.tpp"  // for template impl

#endif // SFILTERS_H
