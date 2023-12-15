#pragma once

#include <string.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "comm.h"

namespace VDB {

template <class ValueType>
class ArtNode;

template <class ValueType>
class ArtNode4;

template <class ValueType>
class ArtNode16;

template <class ValueType>
class ArtNode48;

template <class ValueType>
class ArtNode256;

template <class ValueType>
class ArtLeafNode;

template <class ValueType>
class Art;

template <class ValueType>
class ArtNode {
 public:
  ArtNode(ArtNodeType t, std::string_view key, uint8_t cc)
      : type(t), prefix_length(key.length()), child_cnt(cc) {
    memcpy(prefix.data(), key.data(), key.length());
  }

  [[nodiscard]] bool IsLeaf() const { return LeafNode == type; }

  // 检查当前节点的前缀是否匹配，返回前缀相同的长度
  [[nodiscard]] size_t CheckPrefix(std::string_view key) const {
    size_t same_prefix_length = 0;
    size_t cmp_size =
        std::min(static_cast<size_t>(prefix_length), key.length());
    while ((cmp_size--) != 0U) {
      if (prefix[same_prefix_length] != key[same_prefix_length]) {
        break;
      }
      same_prefix_length++;
    }
    return same_prefix_length;
  }

  // 根据字符串查找孩子节点，找不到返回nullptr
  std::unique_ptr<ArtNode<ValueType>> &FindChild(ArtNode<ValueType> *node,
                                                 char next_character) const {
    switch (node->type) {
      case Node4: {
        auto *node4 = static_cast<ArtNode4<ValueType> *>(node);
        for (int i = 0; i < node4->child_cnt; i++) {
          if (node4->keys[i] == next_character) {
            return node4->childs[i];
          }
        }
      } break;
      case Node16: {
        // TODO simd优化，或者考虑编译器自动向量化
        auto *node16 = static_cast<ArtNode16<ValueType> *>(node);
        for (int i = 0; i < node16->base.child_cnt; i++) {
          if (node16->keys[i] == next_character) {
            return node16->childs[i];
          }
        }
      } break;
      case Node48: {
        auto *node48 = static_cast<ArtNode48<ValueType> *>(node);
        if (auto index = node48->childs_index[next_character]; index != -1) {
          return node48->childs[index];
        }
      } break;
      case Node256: {
        auto *node256 = static_cast<ArtNode256<ValueType> *>(node);
        return node256->childs[next_character];
      } break;
      case LeafNode:
      default:
        assert(false);
    }
    return nullptr;
  }

  friend class Art<ValueType>;

 private:
  uint8_t type : 4;
  uint8_t prefix_length : 4;
  uint8_t child_cnt;
  std::array<char, kTen> prefix{};
};

template <class ValueType>
class ArtNode4 : public ArtNode<ValueType> {
 public:
  friend class Art<ValueType>;

 private:
  std::array<char, kFour> keys{};
  std::array<std::unique_ptr<ArtNode<ValueType>>, kFour> childs{};
};

template <class ValueType>
class ArtNode16 : public ArtNode<ValueType> {
 public:
  friend class Art<ValueType>;

 private:
  std::array<char, kSixteen> keys{};
  std::array<std::unique_ptr<ArtNode<ValueType>>, kSixteen> childs{};
};

template <class ValueType>
class ArtNode48 : public ArtNode<ValueType> {
 public:
  friend class Art<ValueType>;

 private:
  std::array<char, kTwoFiveSize> childs_index{};
  std::array<std::unique_ptr<ArtNode<ValueType>>, kFortyEight> childs{};
};

template <class ValueType>
class ArtNode256 : public ArtNode<ValueType> {
 public:
  friend class Art<ValueType>;

 private:
  std::array<std::unique_ptr<ArtNode<ValueType>>, kTwoFiveSize> childs{};
};

template <typename ValueType>
class ArtLeafNode : public ArtNode<ValueType> {
 public:
  template <class... Args>
  ArtLeafNode(std::string_view key, Args &&...args)
      : ArtNode<ValueType>(LeafNode, key, 0), value(std::forward(args)...) {}

  friend class Art<ValueType>;

  // 叶子节点是否匹配
  [[nodiscard]] bool LeafMatches(const std::string_view key) const {
    if (key.length() != this->prefix_length) {
      return false;
    }
    return 0 == memcmp(key.data(), this->prefix.data(), this->prefix_length);
  }

 private:
  ValueType value;
};

template <class ValueType>
class Art {
 public:
  // TODO copyconstruct

  // 查找
  bool Find(std::string_view key, ValueType *value);
  // 插入
  template <class... Args>
  bool Insert(std::string_view key, ValueType *old_value, Args &&...args);
  // 更新
  template <class... Args>
  bool Update(std::string_view key, Args &&...args);
  // 插入或更新
  template <class... Args>
  void Upsert(std::string_view key, Args &&...args);
  // 删除
  bool Delete(std::string_view key);

 private:
  bool FindImpl(ArtNode<ValueType> *node, std::string_view key,
                ValueType *value);

  template <class... Args>
  bool InsertImpl(ArtNode<ValueType> *node,
                  std::unique_ptr<ArtNode<ValueType>> &node_ref,
                  std::string_view key, ValueType *old_value, Args &&...args);

  std::unique_ptr<ArtNode<ValueType>> root;
};

template <class ValueType>
bool Art<ValueType>::Find(std::string_view key, ValueType *value) {
  return FindImpl(root.get(), key, value);
}

template <class ValueType>
bool Art<ValueType>::FindImpl(ArtNode<ValueType> *node, std::string_view key,
                              ValueType *value) {
  if (nullptr == node || key.empty()) {
    return false;
  }
  if (node->IsLeaf()) {
    if (node->LeafMatches(key)) {
      *value = static_cast<ArtLeafNode<ValueType> *>(node)->value;
      return true;
    }
    return false;
  }
  if (node->CheckPrefix(key) != node->prefix_length) {
    return false;
  }
  key.remove_prefix(node->prefix_length);
  if (key.empty()) {
    return false;
  }
  auto &child = node->FindChild(key[0]);
  return FindImpl(child.get(), key.remove_prefix(1), value);
}

template <class ValueType>
template <class... Args>
bool Art<ValueType>::Insert(std::string_view key, ValueType *old_value,
                            Args &&...args) {
  return InsertImpl(root.get(), &root, key, old_value, std::forward(args)...);
}

template <class ValueType>
template <class... Args>
bool Art<ValueType>::InsertImpl(ArtNode<ValueType> *node,
                                std::unique_ptr<ArtNode<ValueType>> &node_ref,
                                std::string_view key, ValueType *old_value,
                                Args &&...args) {
  if (node == nullptr) {
    node_ref =
        std::make_unique<ArtLeafNode<ValueType>>(key, std::forward(args)...);
    return true;
  }
  if (node->IsLeaf()) {
    size_t same_prefix_length = 0;
    if (same_prefix_length = node->CheckPrefix(key);
        key.length() == same_prefix_length &&
        node->prefix_length == same_prefix_length) {
      *old_value = static_cast<ArtLeafNode<ValueType *>>(node)->value;
      return false;
    }
    auto prefix_key = key;
    prefix_key.remove_suffix(key.length() - same_prefix_length);
    auto new_leaf = std::make_unique<ArtLeafNode<ValueType>>(
        prefix_key, std::forward(args)...);
    node->prefix_length -= same_prefix_length;
    memmove(node->prefix.data(), node->prefix.data() + same_prefix_length,
            node->prefix_length);
    auto new_node4 = std::make_unique<ArtNode4<ValueType>>();

    // ？？？这里已修改node_ref则node会无法使用
    node_ref = std::move(new_node4);
  }
  return true;
}

}  // namespace VDB