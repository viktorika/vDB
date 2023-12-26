#pragma once

#include <cstddef>

namespace VDB {

constexpr size_t kFour = 4;
constexpr size_t kFive = 5;
constexpr size_t kTen = 10;
constexpr size_t kSixteen = 16;
constexpr size_t kSeventeen = 17;
constexpr size_t kFortyEight = 48;
constexpr size_t kFortyNight = 49;
constexpr size_t kTwoFiveSize = 256;

constexpr size_t kMaxPrefixLength = 10;

enum ArtNodeType {
  Node4WithValue = 0,
  Node4 = 1,
  Node16WithValue = 2,
  Node16 = 3,
  Node48WithValue = 4,
  Node48 = 5,
  Node256WithValue = 6,
  Node256 = 7,
  LeafNode = 8,
};

}  // namespace VDB