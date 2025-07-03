#include "../include/SquirtleFilter.h"
#include <fstream>
#include <vector>
#include <atomic>
#include <stdexcept>
#include <cassert>
#include <cstring>
#include <cereal/archives/binary.hpp>

// Rotate left 64-bit value
static inline uint64_t rotl64(uint64_t x, int8_t r) {
    return (x << r) | (x >> (64 - r));
}

// Finalization mix function for 64-bit (from MurmurHash3)
static inline uint64_t fmix64(uint64_t k) {
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ULL;
    k ^= k >> 33;
    return k;
}

// Implementation of 128-bit MurmurHash3 (x64 variant) to produce two 64-bit hash outputs.
// Check: https://github.com/judwhite/Grassfed.MurmurHash3/blob/master/Grassfed.MurmurHash3/MurmurHash3.cs
void BloomFilter::hash128(const void* key, size_t len, uint64_t seed, uint64_t& out1, uint64_t& out2) {
    const uint8_t* data = static_cast<const uint8_t*>(key);
    const int nblocks = len / 16;

    uint64_t h1 = seed;
    uint64_t h2 = seed;

    const uint64_t c1 = 0x87c37b91114253d5ULL;
    const uint64_t c2 = 0x4cf5ad432745937fULL;

    // Body - process 16-byte blocks
    const uint64_t* blocks = reinterpret_cast<const uint64_t*>(data);
    for (int i = 0; i < nblocks; i++) {
        uint64_t k1 = blocks[2*i];
        uint64_t k2 = blocks[2*i + 1];

        k1 *= c1;
        k1 = rotl64(k1, 31);
        k1 *= c2;
        h1 ^= k1;

        h1 = rotl64(h1, 27);
        h1 += h2;
        h1 = h1 * 5 + 0x52dce729;

        k2 *= c2;
        k2 = rotl64(k2, 33);
        k2 *= c1;
        h2 ^= k2;

        h2 = rotl64(h2, 31);
        h2 += h1;
        h2 = h2 * 5 + 0x38495ab5;
    }

    // Tail - process remaining bytes
    const uint8_t* tail = data + nblocks * 16;
    uint64_t k1 = 0;
    uint64_t k2 = 0;
    switch (len & 15) {
    case 15: k2 ^= (uint64_t)tail[14] << 48;
    case 14: k2 ^= (uint64_t)tail[13] << 40;
    case 13: k2 ^= (uint64_t)tail[12] << 32;
    case 12: k2 ^= (uint64_t)tail[11] << 24;
    case 11: k2 ^= (uint64_t)tail[10] << 16;
    case 10: k2 ^= (uint64_t)tail[9]  << 8;
    case 9:  k2 ^= (uint64_t)tail[8]  << 0;
             k2 *= c2;
             k2 = rotl64(k2, 33);
             k2 *= c1;
             h2 ^= k2;
    case 8: k1 ^= (uint64_t)tail[7] << 56;
    case 7: k1 ^= (uint64_t)tail[6] << 48;
    case 6: k1 ^= (uint64_t)tail[5] << 40;
    case 5: k1 ^= (uint64_t)tail[4] << 32;
    case 4: k1 ^= (uint64_t)tail[3] << 24;
    case 3: k1 ^= (uint64_t)tail[2] << 16;
    case 2: k1 ^= (uint64_t)tail[1] << 8;
    case 1: k1 ^= (uint64_t)tail[0] << 0;
            k1 *= c1;
            k1 = rotl64(k1, 31);
            k1 *= c2;
            h1 ^= k1;
    }

    // Finalization
    h1 ^= len;
    h2 ^= len;
    h1 += h2;
    h2 += h1;
    h1 = fmix64(h1);
    h2 = fmix64(h2);
    h1 += h2;
    h2 += h1;
    out1 = h1;
    out2 = h2;
}

BloomFilter::BloomFilter(size_t expected_items, double false_positive_rate, uint8_t hash_functions)
    : k(hash_functions), target_false_positive(false_positive_rate), item_count(0) {
    
    if (k < 1) k = 1;
    if (k > 5) k = 5;
    if (target_false_positive <= 0.0) target_false_positive = 0.0001;
    if (target_false_positive >= 1.0) target_false_positive = 0.999;

    // m = - (n * ln(p)) / (ln(2)^2)
    double m_calc = -(double)expected_items * std::log(target_false_positive) / (std::log(2) * std::log(2));
    bit_count = static_cast<size_t>(std::ceil(m_calc));
    size_t num_words = (bit_count + 63) / 64;
    // bits = std::vector<std::atomic<uint64_t>>(num_words);
    bits = std::make_shared<std::vector<std::atomic<uint64_t>>>(num_words);
    for (auto& word : *bits) word.store(0, std::memory_order_relaxed);

    capacity = expected_items;
}

// Insert an element (thread-safe). Uses double hashing to set k bits.
void BloomFilter::insert(const void* key, size_t len) {
    // Generate two 64-bit hash values for the key

    uint64_t h1, h2;
    hash128(key, len, 0, h1, h2);
    uint64_t base_index = h1 % bit_count;
    uint64_t hash2_mod = h2 % bit_count;
    for (uint8_t i = 0; i < k; ++i) {
        uint64_t index = (base_index + i * hash2_mod) % bit_count;
    (*bits)[index / 64].fetch_or(1ULL << (index % 64), std::memory_order_relaxed); //  atomicity only, no synchronization with other thread
    }
    ++item_count;
}

BloomFilter& BloomFilter::operator=(BloomFilter&& other) noexcept {
    if (this != &other) {
        std::unique_lock<std::shared_mutex> lock_this(mutex);
        std::unique_lock<std::shared_mutex> lock_other(other.mutex);
        k = other.k;
        target_false_positive = other.target_false_positive;
        // Delete existing filters in *this
        FilterSegment* seg = current;
        while (seg) {
            FilterSegment* prev = seg->prev;
            delete seg;
            seg = prev;
        }
        current = other.current;
        other.current = nullptr;
    }
    return *this;
}


// (thread-safe).
bool BloomFilter::contains(const void* key, size_t len) const {
    uint64_t h1, h2;
    hash128(key, len, 0, h1, h2);
    uint64_t base_index = h1 % bit_count;
    uint64_t hash2_mod = h2 % bit_count;
    for (uint8_t i = 0; i < k; ++i) {
        uint64_t index = (base_index + i * hash2_mod) % bit_count;
        if (((*bits)[index / 64].load(std::memory_order_relaxed) & (1ULL << (index % 64))) == 0)
            return false;
    }
    return true;
}

std::shared_ptr<BloomFilter::SQFilter> BloomFilter::returnFilterReference() const{
    return bits; 
}

BloomFilter::SQFilterRAW BloomFilter::returnFilter() const{
    SQFilterRAW snapshot;
    snapshot.reserve(bits->size());
    for (const auto& atomic_val : *bits) {
        snapshot.push_back(atomic_val.load(std::memory_order_relaxed));
    }
    return snapshot;
}

void BloomFilter::passFilterReference(std::shared_ptr<SQFilter> shared_bits){

    bits = shared_bits;
}

void BloomFilter::clear(){
    for (auto& word : *bits) {
        word.store(0, std::memory_order_relaxed);
    }
    item_count = 0;
}

void BloomFilter::writeSQFilter(const std::string& output_path) const {
    BloomFilterData data;
    data.bit_count = bit_count;
    data.capacity = capacity;
    data.item_count = item_count;
    data.hash_functions = k;
    data.false_positive_rate = target_false_positive;

    data.bits.reserve(bits->size());
    for (const auto& val : *bits) {
        data.bits.push_back(val.load(std::memory_order_relaxed));
    }

    std::ofstream ofs(output_path, std::ios::binary);
    if (!ofs) throw std::runtime_error("Failed to open file for writing: " + output_path);
    cereal::BinaryOutputArchive archive(ofs);
    archive(data);
}

void BloomFilter::loadSQFilter(const std::string& input_path) {
    std::ifstream ifs(input_path, std::ios::binary);
    if (!ifs) throw std::runtime_error("Failed to open file for reading: " + input_path);
    cereal::BinaryInputArchive archive(ifs);

    BloomFilterData data;
    archive(data);

    // Set internal config
    bit_count = data.bit_count;
    capacity = data.capacity;
    item_count = data.item_count;
    k = data.hash_functions;
    target_false_positive = data.false_positive_rate;

    // Reconstruct atomic bit vector
    bits = std::make_shared<std::vector<std::atomic<uint64_t>>>(data.bits.size());
    for (size_t i = 0; i < data.bits.size(); ++i) {
        (*bits)[i].store(data.bits[i], std::memory_order_relaxed);
    }
}

inline uint64_t popcount(uint64_t x) {
    x = x - ((x >> 1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    x = x + (x >> 8);
    x = x + (x >> 16);
    x = x + (x >> 32);
    return x & 0x7F;
}

void BloomFilter::printSummary() const {
    std::cout << "=== Bloom Filter Summary ===\n";
    std::cout << "Bit count             : " << bit_count << '\n';
    std::cout << "Capacity              : " << capacity << '\n';
    std::cout << "Item count            : " << item_count << '\n';
    std::cout << "Hash functions (k)    : " << static_cast<int>(k) << '\n';
    std::cout << "False positive rate   : " << target_false_positive << '\n';
    std::cout << "Bit vector size       : " << bits->size() << " words (64-bit each)\n";

    size_t set_bits = 0;
    for (const auto& word : *bits) {
        set_bits += popcount(word.load(std::memory_order_relaxed));
    }
    std::cout << "Total bits set        : " << set_bits << '\n';
    std::cout << "============================\n";
}


BloomFilter::BloomFilterData BloomFilter::exportData() const {
    BloomFilterData data;
    data.bit_count = bit_count;
    data.capacity = capacity;
    data.item_count = item_count;
    data.hash_functions = k;
    data.false_positive_rate = target_false_positive;

    data.bits.reserve(bits->size());
    for (const auto& word : *bits) {
        data.bits.push_back(word.load(std::memory_order_relaxed));
    }
    return data;
}

void BloomFilter::importData(const BloomFilterData& data) {
    bit_count = data.bit_count;
    capacity = data.capacity;
    item_count = data.item_count;
    k = data.hash_functions;
    target_false_positive = data.false_positive_rate;

    size_t word_count = (bit_count + 63) / 64;
    bits = std::make_shared<std::vector<std::atomic<uint64_t>>>(word_count);

    for (size_t i = 0; i < data.bits.size(); ++i) {
        (*bits)[i].store(data.bits[i], std::memory_order_relaxed);
    }
}

BloomFilter::BloomFilter(BloomFilter&& other) noexcept
    : bit_count(other.bit_count),
      bits(std::move(other.bits)),
      capacity(other.capacity),
      item_count(other.item_count),
      k(other.k),
      target_false_positive(other.target_false_positive),
      current(other.current)
{
    std::unique_lock<std::shared_mutex> lock_other(other.mutex);
    other.current = nullptr;
}