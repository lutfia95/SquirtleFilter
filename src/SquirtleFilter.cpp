#include "../include/SquirtleFilter.h"
#include <fstream>
#include <vector>
#include <atomic>
#include <stdexcept>
#include <cassert>
#include <cstring>

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
    bits = std::vector<std::atomic<uint64_t>>(num_words);
    for (auto& word : bits) word.store(0, std::memory_order_relaxed);

    capacity = expected_items;
}

/* 
// Constructor: initialize Bloom filter with one filter segment
BloomFilter::BloomFilter(size_t expected_items, double false_positive_rate, uint8_t hash_functions)
    : k(hash_functions), target_false_positive(false_positive_rate) {
    if (k < 1) k = 1;
    if (k > 5) k = 5;
    if (target_false_positive <= 0.0) target_false_positive = 0.0001; // ensure positive
    if (target_false_positive >= 1.0) target_false_positive = 0.999;  // ensure < 1

    // Determine the target false positive rate for the initial filter segment.
    // Use error_decay factor to ensure overall false positive does not exceed target.
    double initial_fpr = target_false_positive * (1.0 - error_decay);
    if (initial_fpr <= 0.0) {
        initial_fpr = target_false_positive; // if decay is 0 or such, just use target
    }

    // Calculate number of bits (m) required to store expected_items with given fpr and k.
    // Formula: m = - (k * expected_items) / ln(1 - p^(1/k))
    double p_term = pow(1.0 - initial_fpr, 1.0 / k);
    double denom = log(p_term);
    size_t m = 0;
    if (denom != 0) {
        m = (size_t) std::ceil(- (double)expected_items * k / denom);
    }
    if (m < 64) m = 64; // minimum size to have some bits (64 bits)
    // Determine capacity for initial filter (use expected_items as capacity)
    size_t cap = expected_items;
    if (cap < 1) cap = 1;

    // Create initial filter segment
    current = new FilterSegment(m, cap, initial_fpr);
    current->prev = nullptr;
}*/

// Move constructor
BloomFilter::BloomFilter(BloomFilter&& other) noexcept {
    std::unique_lock<std::shared_mutex> lock(other.mutex);
    k = other.k;
    target_false_positive = other.target_false_positive;
    current = other.current;
    other.current = nullptr;
}

// Move assignment
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

// Safely add a new filter segment when current filter is at capacity.
// This function should be called with mutex (unique_lock) held.
void BloomFilter::addFilterSegment() {
    // current is full, create a new segment with larger size and smaller false positive rate
    FilterSegment* old = current;
    // Compute new capacity and fpr
    size_t new_capacity = (size_t) std::ceil(old->capacity * growth_factor);
    // The new filter's false positive target is reduced by factor error_decay
    double new_fpr = old->false_positive_rate * error_decay;
    if (new_fpr < 1e-9) {
        new_fpr = old->false_positive_rate; // avoid extreme reduction leading to very large bit vector
    }
    // Compute required bits for new_capacity and new_fpr
    double p_term = pow(1.0 - new_fpr, 1.0 / k);
    double denom = log(p_term);
    size_t m = 0;
    if (denom != 0) {
        m = (size_t) std::ceil(- (double)new_capacity * k / denom);
    }
    if (m < 64) m = 64;
    FilterSegment* newSeg = new FilterSegment(m, new_capacity, new_fpr);
    newSeg->prev = old;
    current = newSeg;
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
        bits[index / 64].fetch_or(1ULL << (index % 64), std::memory_order_relaxed); //  atomicity only, no synchronization with other thread
    }
    ++item_count;
    /*
    while (true) {
        // Acquire shared access to current filter pointer (mutex as shared).
        // Use shared_mutex to allow concurrent contains while inserting.
        std::shared_lock<std::shared_mutex> readLock(mutex);
        FilterSegment* seg = current;
        // Reserve a slot in this filter by incrementing count (atomic)
        size_t curCount = seg->count.load(std::memory_order_relaxed);
        if (curCount >= seg->capacity) {
            // Need to add new filter segment, upgrade to unique lock
            readLock.unlock();
            std::unique_lock<std::shared_mutex> writeLock(mutex);
            // Double-check in case another thread already added a new segment
            if (seg == current && seg->count.load(std::memory_order_relaxed) >= seg->capacity) {
                addFilterSegment();
                seg = current;
            } else {
                // Another thread resized, use the new current
                seg = current;
            }
            // Now we have correct segment in seg, but we haven't inserted yet.
            writeLock.unlock();
            // Proceed to actually insert into seg (which is the new current filter).
        } else {
            // No resizing needed; can proceed with current segment
            readLock.unlock();
        }

        // Now we have a filter segment (seg) to insert into, and no lock held.
        // Attempt to increment the count for seg. If it fails due to reaching capacity (race with another thread),
        // we'll loop and try again on the new filter.
        size_t prevCount = seg->count.fetch_add(1, std::memory_order_relaxed);
        if (prevCount >= seg->capacity) {
            // This segment turned out to be full (or over capacity) - undo the increment and retry with a new segment
            seg->count.fetch_sub(1, std::memory_order_relaxed);
            continue;
        }

        // Set the k bits in the chosen filter segment
        // Use double hashing: generate k indices from h1 and h2
        uint64_t hash2_mod = h2 % seg->bit_count;
        uint64_t base_index = h1 % seg->bit_count;
        for (uint8_t i = 0; i < k; ++i) {
            uint64_t index = (base_index + i * hash2_mod) % seg->bit_count;
            // Set the corresponding bit (atomic OR)
            size_t wordIndex = index / 64;
            uint64_t bitMask = 1ULL << (index % 64);
            seg->bits[wordIndex].fetch_or(bitMask, std::memory_order_relaxed);
        }
        return;
    }*/
}

// (thread-safe).
bool BloomFilter::contains(const void* key, size_t len) const {
    uint64_t h1, h2;
    hash128(key, len, 0, h1, h2);
    uint64_t base_index = h1 % bit_count;
    uint64_t hash2_mod = h2 % bit_count;
    for (uint8_t i = 0; i < k; ++i) {
        uint64_t index = (base_index + i * hash2_mod) % bit_count;
        if ((bits[index / 64].load(std::memory_order_relaxed) & (1ULL << (index % 64))) == 0)
            return false;
    }
    return true;
    /*
    // Compute two hash values for double hashing
    uint64_t h1, h2;
    hash128(key, len, 0, h1, h2);

    // Traverse through all filter segments (from newest to oldest)
    std::shared_lock<std::shared_mutex> lock(mutex);
    const FilterSegment* seg = current;
    
    while (seg) {
        bool found = true;
        // Calculate first index and offset for double hashing
        uint64_t hash2_mod = h2 % seg->bit_count;
        uint64_t base_index = h1 % seg->bit_count;
        for (uint8_t i = 0; i < k; ++i) {
            uint64_t index = (base_index + i * hash2_mod) % seg->bit_count;
            size_t wordIndex = index / 64;
            uint64_t bitMask = 1ULL << (index % 64);
            // If any required bit is not set, then the item is definitely not in this segment
            if ((seg->bits[wordIndex].load(std::memory_order_relaxed) & bitMask) == 0ULL) {
                found = false;
                break;
            }
        }
        if (found) {
            return true; // possibly present (either a real match or a false positive)
        }
        seg = seg->prev;
    }
    return false;*/
}

// Destructor - free all filter segments
/*BloomFilter::~BloomFilter() {
    FilterSegment* seg = current;
    while (seg) {
        FilterSegment* prev = seg->prev;
        delete seg;
        seg = prev;
    }
}*/
