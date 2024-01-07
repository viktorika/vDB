#include <array>
#include <iostream>
#include <random>
#include <string>

#include "art.h"
#include "gtest/gtest.h"

TEST(NoormalInsertFindTest, LeafNodeFatherTest) {
  std::array<std::string, 6> keys = {
      "abcdefg", "ab", "abcght", "abqert", "abcghq", "abcgh",
  };
  vDB::Art<std::string> art_tree;
  for (int i = 0; i < 6; i++) {
    EXPECT_EQ(art_tree.Insert(keys[i], nullptr, std::to_string(i)), true);
  }
  for (int i = 0; i < 6; i++) {
    std::string value;
    EXPECT_EQ(art_tree.Find(keys[i], &value), true);
    EXPECT_EQ(value, std::to_string(i));
  }
}

TEST(RandomTest, RandomTest) {
  std::cout << "size string" << sizeof(std::string) << std::endl;
  constexpr uint32_t kMaxKeyLength = 20;
  constexpr uint32_t kMaxKey = 100000000;
  std::vector<std::string> keys(kMaxKey);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> distrib(-128, 127);
  vDB::Art<std::string> art_tree;
  for (int i = 0; i < kMaxKey; i++) {
    keys[i].resize(kMaxKeyLength);
    for (int j = 0; j < kMaxKeyLength; j++) {
      keys[i][j] = distrib(gen);
    }
    EXPECT_EQ(art_tree.Insert(keys[i], nullptr, std::to_string(i)), true);
  }
  for (int i = 0; i < kMaxKey; i++) {
    std::string value;
    EXPECT_EQ(art_tree.Find(keys[i], &value), true);
    EXPECT_EQ(value, std::to_string(i));
  }
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}