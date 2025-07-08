#include "SFilters.h"
#include <fstream>
#include <cereal/archives/binary.hpp>


/**
 * @brief Initializes a collection of Bloom filters.
 *
 * This function prepares a container to hold a specified number of Bloom filter instances.
 * It first clears any existing filters, then reserves memory for the new filters,
 * and finally constructs each individual Bloom filter with the provided configuration
 * parameters (expected items, false positive rate, and number of hash functions).
 *
 * @param num_filters The total number of Bloom filter instances to create and manage.
 * @param expected_items The approximate maximum number of items each individual Bloom filter
 * is expected to hold, used to calculate its optimal size.
 * @param false_positive_rate The desired maximum false positive rate for each Bloom filter,
 * influencing their bit array size.
 * @param hash_functions The number of hash functions to be used by each Bloom filter.
 */
void SFilters::initialize(size_t num_filters, size_t expected_items, double false_positive_rate, uint8_t hash_functions) {
    filters.clear();
    filters.reserve(num_filters);
    for (size_t i = 0; i < num_filters; ++i) {
        filters.emplace_back(expected_items, false_positive_rate, hash_functions);
    }
}

/**
 * @brief Inserts a key into a specific Bloom filter within the collection.
 *
 * This function provides a way to add an item to one of the Bloom filter instances
 * managed by the `SFilters` object. It first performs a bounds check to ensure
 * the provided `index` is valid before attempting to insert the key into the
 * corresponding Bloom filter.
 *
 * @param index The zero-based index of the Bloom filter in the collection
 * where the key should be inserted.
 * @param key The string key to be inserted into the specified Bloom filter.
 * @throws std::out_of_range If the provided `index` is greater than or equal to
 * the number of filters in the collection.
 */
void SFilters::insert(size_t index, const std::string& key) {
    if (index >= filters.size()) throw std::out_of_range("insert: index out of bounds");
    filters[index].insert(key);
}

/**
 * @brief Inserts a double-precision floating-point value into a specific Bloom filter.
 *
 * This overloaded function allows for the insertion of a `double` value into
 * one of the Bloom filter instances. It performs a bounds check on the provided
 * `index` and then delegates the insertion to the corresponding Bloom filter's
 * `insert` method, which will handle the conversion of the double to a suitable
 * format for hashing.
 *
 * @param index The zero-based index of the Bloom filter in the collection
 * where the double value should be inserted.
 * @param value The double-precision floating-point value to be inserted.
 * @throws std::out_of_range If the provided `index` is greater than or equal to
 * the number of filters in the collection.
 */
void SFilters::insert(size_t index, double value) {
    if (index >= filters.size()) throw std::out_of_range("insert: index out of bounds");
    filters[index].insert(value);
}

/**
 * @brief Writes the entire collection of Bloom filters to a single binary file.
 *
 * This function serializes the state of all Bloom filter instances managed
 * by the `SFilters` object into a binary file at the specified path.
 * Each individual Bloom filter's data (configuration and bit array) is
 * extracted and then the entire vector of these data structures is saved
 * using the Cereal library. This allows for persistent storage of the
 * complete filter collection.
 *
 * @param output_path The file path where the serialized collection of Bloom filters will be written.
 * @throws std::runtime_error If the output file cannot be opened for writing.
 */
void SFilters::writeToFile(const std::string& output_path) const {
    std::ofstream ofs(output_path, std::ios::binary);
    if (!ofs) throw std::runtime_error("Cannot open output file: " + output_path);

    cereal::BinaryOutputArchive archive(ofs);
    std::vector<BloomFilter::BloomFilterData> data;
    for (const auto& f : filters) data.push_back(f.exportData());
    archive(data);
}

/**
 * @brief Loads a collection of Bloom filters from a binary file.
 *
 * This function reads a previously serialized collection of Bloom filter data
 * from a binary file at the specified path. It deserializes the data using
 * the Cereal library, then reconstructs the individual Bloom filter objects
 * within the `SFilters` collection based on the loaded data. Any existing
 * filters in the current `SFilters` object are cleared before loading.
 *
 * @param input_path The file path from which the serialized collection of Bloom filters will be read.
 * @throws std::runtime_error If the input file cannot be opened for reading.
 */
void SFilters::loadFromFile(const std::string& input_path) {
    std::ifstream ifs(input_path, std::ios::binary);
    if (!ifs) throw std::runtime_error("Cannot open input file: " + input_path);

    cereal::BinaryInputArchive archive(ifs);
    std::vector<BloomFilter::BloomFilterData> data; // should we change from vector to arr? 
    archive(data);

    filters.clear();
    filters.reserve(data.size());
    for (const auto& d : data) {
        BloomFilter f;
        f.importData(d);
        filters.push_back(std::move(f));
    }
}

/**
 * @brief Returns a constant reference to the underlying vector of Bloom filter objects.
 *
 * This function provides read-only access to the collection of Bloom filters
 * managed by the `SFilters` object. It allows external code to iterate over,
 * inspect, or query the individual Bloom filters without being able to modify
 * the `filters` vector itself or the BloomFilter objects within it directly
 * (unless the BloomFilter objects themselves provide non-const access).
 *
 * @return A constant reference to a `std::vector<BloomFilter>`,
 * containing all the Bloom filter instances.
 */
const std::vector<BloomFilter>& SFilters::getFilters() const {
    return filters;
}

/**
 * @brief Checks the presence of a key across all Bloom filters in the collection.
 *
 * This function iterates through each Bloom filter managed by `SFilters` and
 * checks if the given `key` might be present in that specific filter. It returns
 * a vector of integers, where each integer corresponds to a Bloom filter in the
 * collection. A `1` indicates that the key might be present in that filter, and
 * a `0` indicates that the key is definitively not present.
 *
 * @param key The string key to check for presence across the filters.
 * @return A `std::vector<int>` where each element is 1 if the key `contains`
 * in the corresponding filter, and 0 otherwise. The order of elements in the
 * returned vector matches the order of filters in the internal collection.
 */
std::vector<int> SFilters::matchBitVector(const std::string& key) const {
    std::vector<int> presence;
    for (const auto& filter : filters) {
        presence.push_back(filter.contains(key) ? 1 : 0);
    }
    return presence;
}

/**
 * @brief Checks the presence of a double value across all Bloom filters in the collection.
 *
 * This overloaded function iterates through each Bloom filter managed by `SFilters` and
 * checks if the given `double` value might be present in that specific filter. It returns
 * a vector of integers, where each integer corresponds to a Bloom filter in the
 * collection. A `1` indicates that the value might be present in that filter, and
 * a `0` indicates that the value is definitively not present.
 *
 * @param value The double-precision floating-point value to check for presence across the filters.
 * @return A `std::vector<int>` where each element is 1 if the `value` `contains`
 * in the corresponding filter, and 0 otherwise. The order of elements in the
 * returned vector matches the order of filters in the internal collection.
 *//**
 * @brief Checks the presence of a double value across all Bloom filters in the collection.
 *
 * This overloaded function iterates through each Bloom filter managed by `SFilters` and
 * checks if the given `double` value might be present in that specific filter. It returns
 * a vector of integers, where each integer corresponds to a Bloom filter in the
 * collection. A `1` indicates that the value might be present in that filter, and
 * a `0` indicates that the value is definitively not present.
 *
 * @param value The double-precision floating-point value to check for presence across the filters.
 * @return A `std::vector<int>` where each element is 1 if the `value` `contains`
 * in the corresponding filter, and 0 otherwise. The order of elements in the
 * returned vector matches the order of filters in the internal collection.
 */
std::vector<int> SFilters::matchBitVector(double value) const {
    std::vector<int> presence;
    for (const auto& filter : filters) {
        presence.push_back(filter.contains(value) ? 1 : 0);
    }
    return presence;
}
