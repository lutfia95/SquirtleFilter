#include "gtest/gtest.h"
#include "../include/SquirtleFilter.h"
#include <thread>

TEST(SquirtleFilterTest, BasicInsertContains) {
    BloomFilter bf(100, 0.01, 3);
    bf.insert("squirtle");
    EXPECT_TRUE(bf.contains("squirtle"));
    EXPECT_FALSE(bf.contains("charmander"));
}

TEST(SquirtleFilterTest, ThreadSafetyTest) {
    BloomFilter bf(1000, 0.01, 3);
    auto inserter = [&bf]() {
        for (int i = 0; i < 500; ++i)
            bf.insert("parallel" + std::to_string(i));
    };

    std::thread t1(inserter);
    std::thread t2(inserter);
    t1.join();
    t2.join();

    EXPECT_TRUE(bf.contains("parallel1"));
    EXPECT_TRUE(bf.contains("parallel499"));
    EXPECT_FALSE(bf.contains("unseen"));
}



int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
