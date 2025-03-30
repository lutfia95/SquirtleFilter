#include "gtest/gtest.h"
#include "../include/SquirtleFilter.h"

TEST(SquirtleFilterTest, BasicInsertContains) {
    BloomFilter bf(100, 0.01, 3);
    bf.insert("squirtle");
    EXPECT_TRUE(bf.contains("squirtle"));
    EXPECT_FALSE(bf.contains("charmander"));
}

TEST(SquirtleFilterTest, ResizeTest) {
    BloomFilter bf(10, 0.01, 3);
    for (int i = 0; i < 100; ++i) bf.insert("poke" + std::to_string(i));
    EXPECT_TRUE(bf.contains("poke50"));
    EXPECT_FALSE(bf.contains("nonexistent"));
}

TEST(SquirtleFilterTest, ThreadSafetyTest) {
    BloomFilter bf(1000, 0.01, 3);
    auto inserter = [&bf]() {
        for (int i = 0; i < 500; ++i)
            bf.insert("parallel" + std::to_string(i));
    };

    std::thread t1(inserter);
    std::thread t2(inserter);
    t1.join(); t2.join();
    EXPECT_TRUE(bf.contains("parallel1"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
