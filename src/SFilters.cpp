#include "SFilters.h"
#include <fstream>
#include <cereal/archives/binary.hpp>

void SFilters::initialize(size_t num_filters, size_t expected_items, double false_positive_rate, uint8_t hash_functions) {
    filters.clear();
    filters.reserve(num_filters);
    for (size_t i = 0; i < num_filters; ++i) {
        filters.emplace_back(expected_items, false_positive_rate, hash_functions);
    }
}

void SFilters::insert(size_t index, const std::string& key) {
    if (index >= filters.size()) throw std::out_of_range("insert: index out of bounds");
    filters[index].insert(key);
}

void SFilters::insert(size_t index, double value) {
    if (index >= filters.size()) throw std::out_of_range("insert: index out of bounds");
    filters[index].insert(value);
}

void SFilters::writeToFile(const std::string& output_path) const {
    std::ofstream ofs(output_path, std::ios::binary);
    if (!ofs) throw std::runtime_error("Cannot open output file: " + output_path);

    cereal::BinaryOutputArchive archive(ofs);
    std::vector<BloomFilter::BloomFilterData> data;
    for (const auto& f : filters) data.push_back(f.exportData());
    archive(data);
}

void SFilters::loadFromFile(const std::string& input_path) {
    std::ifstream ifs(input_path, std::ios::binary);
    if (!ifs) throw std::runtime_error("Cannot open input file: " + input_path);

    cereal::BinaryInputArchive archive(ifs);
    std::vector<BloomFilter::BloomFilterData> data;
    archive(data);

    filters.clear();
    filters.reserve(data.size());
    for (const auto& d : data) {
        BloomFilter f;
        f.importData(d);
        filters.push_back(std::move(f));
    }
}

const std::vector<BloomFilter>& SFilters::getFilters() const {
    return filters;
}

std::vector<int> SFilters::matchBitVector(const std::string& key) const {
    std::vector<int> presence;
    for (const auto& filter : filters) {
        presence.push_back(filter.contains(key) ? 1 : 0);
    }
    return presence;
}

std::vector<int> SFilters::matchBitVector(double value) const {
    std::vector<int> presence;
    for (const auto& filter : filters) {
        presence.push_back(filter.contains(value) ? 1 : 0);
    }
    return presence;
}
