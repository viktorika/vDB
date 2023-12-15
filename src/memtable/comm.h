#pragma once

#include <cstddef>

namespace VDB {

constexpr size_t kFour = 4;
constexpr size_t kTen = 10;
constexpr size_t kSixteen = 16;
constexpr size_t kFortyEight = 48;
constexpr size_t kTwoFiveSize = 256;

constexpr size_t kMaxPrefixLength = 10;

enum ArtNodeType {
  Node4 = 1,
  Node16 = 2,
  Node48 = 3,
  Node256 = 4,
  LeafNode = 5,
};

}  // namespace VDB