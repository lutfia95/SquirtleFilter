#ifndef SQUIRTLEFILTER_H
#define SQUIRTLEFILTER_H

#include <vector>
#include <atomic>
#include <cstdint>
#include <cmath>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <cstring>

// A highly optimized Bloom Filter implementation supporting dynamic scaling and thread safety.
class BloomFilter {
public:
    // Construct a BloomFilter with given expected number of items, target false positive rate, and number of hash functions.
    // If not sure about expected items, the filter will grow dynamically to maintain the false positive rate.
    BloomFilter(size_t expected_items = 1000, double false_positive_rate = 0.01, uint8_t hash_functions = 3);

    // Disable copying to avoid accidental expensive operations (could implement if needed)
    BloomFilter(const BloomFilter&) = delete;
    BloomFilter& operator=(const BloomFilter&) = delete;

    // Allow moving
    BloomFilter(BloomFilter&& other) noexcept;
    BloomFilter& operator=(BloomFilter&& other) noexcept;

    // Insert an element (by raw data pointer and length) into the Bloom filter.
    void insert(const void* key, size_t len);

    // Overload: Insert a std::string key.
    void insert(const std::string& key) {
        insert(key.data(), key.size());
    }

    // Check whether an element (by raw data pointer and length) might be in the set.
    bool contains(const void* key, size_t len) const;

    // Overload: Check membership of std::string key.
    bool contains(const std::string& key) const {
        return contains(key.data(), key.size());
    }

    // Destructor to release allocated memory.
    ~BloomFilter();

private:
    // Internal structure representing one Bloom filter segment (for scalable Bloom filter).
    struct FilterSegment {
        size_t bit_count;            // number of bits in this filter
        size_t capacity;            // max number of items this filter was designed for
        double false_positive_rate; // target false positive rate for this filter when at capacity
        std::atomic<size_t> count;  // number of items inserted into this filter
        std::unique_ptr<std::atomic<uint64_t>[]> bits; // bit array (atomic for thread-safe bit set)
        FilterSegment* prev;        // pointer to previous (older) filter segment in the chain

        FilterSegment(size_t bits, size_t cap, double fpr)
            : bit_count(bits), capacity(cap), false_positive_rate(fpr),
              count(0), bits(new std::atomic<uint64_t>[ (bits + 63) / 64 ] ), prev(nullptr)
        {
            // Initialize bit array to 0
            size_t word_count = (bit_count + 63) / 64;
            for (size_t i = 0; i < word_count; ++i) {
                bits[i].store(0, std::memory_order_relaxed);
            }
        }
    };

    // Hash a key to a 128-bit (two 64-bit) result using a high-performance hash (MurmurHash3).
    // We use two 64-bit hashes to generate multiple indices via double hashing.
    static void hash128(const void* key, size_t len, uint64_t seed, uint64_t& out1, uint64_t& out2);

    // Add a new filter segment when the current one is at capacity.
    void addFilterSegment();

    uint8_t k;                     // number of hash functions
    double target_false_positive;  // desired maximum false positive rate for the Bloom filter
    const double growth_factor = 2.0;      // factor to grow the filter size (capacity) when expanding
    const double error_decay = 0.5;        // factor to decrease false positive rate in each new filter

    FilterSegment* current;        // pointer to the current (latest) filter segment
    mutable std::shared_mutex mutex; // mutex for thread-safe access (read for contains, write for resizing)
};

#endif
