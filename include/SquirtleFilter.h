#ifndef SQUIRTLEFILTER_H
#define SQUIRTLEFILTER_H

#include <vector>
#include <atomic>
#include <cstdint>
#include <cmath>
#include <mutex>
#include <memory>
#include <shared_mutex>
#include <string>
#include <cstring>
#include <cereal/types/vector.hpp>
#include <cereal/archives/binary.hpp>

class BloomFilter {
public:

    BloomFilter(size_t expected_items = 1000, double false_positive_rate = 0.01, uint8_t hash_functions = 3);
    BloomFilter(const BloomFilter&) = delete;
    BloomFilter& operator=(const BloomFilter&) = delete;

    BloomFilter(BloomFilter&& other) noexcept;
    BloomFilter& operator=(BloomFilter&& other) noexcept;

    void insert(const void* key, size_t len);

    void insert(const std::string& key) {
        insert(key.data(), key.size());
    }

    void insert(double value) {
        insert(&value, sizeof(double));
    }

    bool contains(const void* key, size_t len) const;

    bool contains(const std::string& key) const {
        return contains(key.data(), key.size());
    }

    bool contains(double value) const {
    return contains(&value, sizeof(double));
    }

    void clear(); 

    typedef std::vector<std::atomic<uint64_t>> SQFilter;
    typedef std::vector<uint64_t> SQFilterRAW;

    std::shared_ptr<SQFilter> returnFilterReference() const; 

    SQFilterRAW returnFilter() const;

    void passFilterReference(std::shared_ptr<SQFilter> shared_bits);

    void writeSQFilter(const std::string& output_path) const;

    void loadSQFilter(const std::string& input_path);

    void printSummary() const;


    struct BloomFilterData {
        size_t bit_count;
        size_t capacity;
        size_t item_count;
        uint8_t hash_functions;
        double false_positive_rate;
        std::vector<uint64_t> bits;

        template <class Archive>
        void serialize(Archive& ar) {
            ar(bit_count, capacity, item_count, hash_functions, false_positive_rate, bits);
        }
    };

    BloomFilterData exportData() const;
    
    void importData(const BloomFilterData& data);

private:
    struct FilterSegment {
        size_t bit_count;            // number of bits in this filter
        size_t capacity;            // max number of items this filter was designed for
        double false_positive_rate; // target false positive rate for this filter when at capacity
        std::atomic<size_t> count;  // number of items inserted into this filter
        std::unique_ptr<std::atomic<uint64_t>[]> bits; // bit array (atomic for thread-safe bit set)
        FilterSegment* prev;        // pointer to previous (older) filter segment in the chain

        FilterSegment(size_t bitCount, size_t cap, double fpr)
            : bit_count(bitCount), capacity(cap), false_positive_rate(fpr),
            count(0), bits(new std::atomic<uint64_t>[(bitCount + 63) / 64]), prev(nullptr)
        {
            size_t word_count = (bit_count + 63) / 64;
            auto* rawBits = bits.get();
            for (size_t i = 0; i < word_count; ++i) {
                rawBits[i].store(0, std::memory_order_relaxed);
            }
        }
    };
    size_t bit_count;
    //std::vector<std::atomic<uint64_t>> bits;
    std::shared_ptr<std::vector<std::atomic<uint64_t>>> bits;
    size_t capacity;
    size_t item_count;

    // Hash a key to a 128-bit (two 64-bit) result using a high-performance hash (MurmurHash3).
    // We use two 64-bit hashes to generate multiple indices via double hashing.
    static void hash128(const void* key, size_t len, uint64_t seed, uint64_t& out1, uint64_t& out2);

    uint8_t k;                     // number of hash functions
    double target_false_positive;  // desired maximum false positive rate for the Bloom filter
    const double growth_factor = 2.0;      // factor to grow the filter size (capacity) when expanding
    const double error_decay = 0.5;        // factor to decrease false positive rate in each new filter
    mutable std::shared_mutex mutex; // mutex for thread-safe access (read for contains, write for resizing)
    FilterSegment* current; 
};

#endif
