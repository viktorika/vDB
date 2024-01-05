#pragma once

#include <cstddef>
#include <string_view>

namespace vDB {

constexpr size_t kFour = 4;
constexpr size_t kFive = 5;
constexpr size_t kTen = 10;
constexpr size_t kSixteen = 16;
constexpr size_t kSeventeen = 17;
constexpr size_t kFortyEight = 48;
constexpr size_t kFortyNight = 49;
constexpr size_t kTwoFiveSix = 256;

constexpr size_t kMaxKeySize = 1 << 20;

enum ArtNodeType {
  Node4 = 1,
  Node16 = 2,
  Node48 = 3,
  Node256 = 4,
  LeafNode = 5,
};

#define NONCOPY(type)          \
  type(const type &) = delete; \
  type &operator=(const type &) = delete;

#define GET_FUNC                                                                    \
  template <class ValueType>                                                        \
  ValueType GetValue() {                                                            \
    return *(reinterpret_cast<ValueType *>(data_));                                 \
  }                                                                                 \
                                                                                    \
  template <class ValueType>                                                        \
  std::string_view GetKey() {                                                       \
    if (HasValue()) {                                                               \
      return std::string_view(data_ + sizeof(ValueType), key_length_);              \
    }                                                                               \
    return std::string_view(data_, key_length_);                                    \
  }

#define CREATE_NODE_WITH_VALUE_FUNC(class_type, node_type)                                                            \
  template <class ValueType, class... Args>                                                                           \
  static class_type *Create##node_type(std::string_view key, uint8_t child_cnt, Args &&...args) {                     \
    class_type *node = reinterpret_cast<class_type *>(malloc(sizeof(class_type) + sizeof(ValueType) + key.length())); \
    new (node) class_type(1, child_cnt, key.length());                                                                \
    ValueType *value_ptr = reinterpret_cast<ValueType *>(node->data_);                                                \
    new (value_ptr) ValueType(std::forward<Args>(args)...);                                                           \
    memcpy(node->data_ + sizeof(ValueType), key.data(), key.length());                                                \
    return node;                                                                                                      \
  }

#define CREATE_NODE_FUNC(class_type, node_type)                                                    \
  static class_type *Create##node_type(std::string_view key, uint8_t child_cnt) {                  \
    class_type *node = reinterpret_cast<class_type *>(malloc(sizeof(class_type) + +key.length())); \
    new (node) class_type(0, child_cnt, key.length());                                             \
    memcpy(node->data_, key.data(), key.length());                                                 \
    return node;                                                                                   \
  }

#define ART_FRIEND_CLASS       \
  friend class ArtNodeFactory; \
  friend class ArtNodeHelper;  \
  template <class ValueType>   \
  friend class Art;

}  // namespace vDB