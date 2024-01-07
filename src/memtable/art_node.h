#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string_view>
#include <utility>

#include "comm.h"


namespace vDB {

// 注意，key最多只能2^20次方的长度，否则会出错

// memory layout
// ---- type 3bit ---- has_value 1bit ---- child_cnt 4bit ---- key_length 24bit ---- node_unique_attribute ----
// value(if has) ---- key ----

class ArtNode {
 public:
  ArtNode() = default;
  ArtNode(ArtNodeType type, bool has_value, uint8_t child_cnt, uint32_t key_length)
      : type_(type), has_value_(has_value), child_cnt_(child_cnt), key_length_(key_length) {}

  [[nodiscard]] bool IsLeaf() const { return LeafNode == type_; }
  [[nodiscard]] bool HasValue() const { return has_value_ > 0; }

  ART_FRIEND_CLASS

 protected:
  uint32_t type_ : 3;
  uint32_t has_value_ : 1;
  uint32_t child_cnt_ : 8;
  uint32_t key_length_ : 20;
};

class ArtNode4 : public ArtNode {
 public:
  NONCOPY(ArtNode4);
  GET_FUNC
  ART_FRIEND_CLASS

 private:
  ArtNode4() = default;
  ArtNode4(bool has_value, uint8_t child_cnt, uint32_t key_length) : ArtNode(Node4, has_value, child_cnt, key_length) {}

  char edge_[kFour];
  ArtNode *childs_[kFour];
  char data_[0];
};

class ArtNode16 : public ArtNode {
 public:
  NONCOPY(ArtNode16);
  GET_FUNC
  ART_FRIEND_CLASS

 private:
  ArtNode16() = default;
  ArtNode16(bool has_value, uint8_t child_cnt, uint32_t key_length)
      : ArtNode(Node16, has_value, child_cnt, key_length) {}

  char edge_[kSixteen];
  ArtNode *childs_[kSixteen];
  char data_[0];
};

class ArtNode48 : public ArtNode {
 public:
  NONCOPY(ArtNode48);
  GET_FUNC
  ART_FRIEND_CLASS

 private:
  ArtNode48() = default;
  ArtNode48(bool has_value, uint8_t child_cnt, uint32_t key_length)
      : ArtNode(Node48, has_value, child_cnt, key_length) {
    memset(childs_index_, -1, sizeof(childs_index_));
  }

  char childs_index_[kTwoFiveSix];
  ArtNode *childs_[kFortyEight];
  char data_[0];
};

class ArtNode256 : public ArtNode {
 public:
  NONCOPY(ArtNode256);
  GET_FUNC
  ART_FRIEND_CLASS

 private:
  ArtNode256() = default;
  ArtNode256(bool has_value, uint8_t child_cnt, uint32_t key_length)
      : ArtNode(Node256, has_value, child_cnt, key_length) {
    memset(childs_, 0, sizeof(childs_));
  }

  ArtNode *childs_[kTwoFiveSix];
  char data_[0];
};

class ArtLeafNode : public ArtNode {
 public:
  NONCOPY(ArtLeafNode);
  GET_FUNC
  ART_FRIEND_CLASS

 private:
  ArtLeafNode() = default;
  ArtLeafNode(bool has_value, uint8_t child_cnt, uint32_t key_length)
      : ArtNode(LeafNode, has_value, child_cnt, key_length) {}
  char data_[0];
};

class ArtNodeFactory {
 public:
  CREATE_NODE_WITH_VALUE_FUNC(ArtNode4, Node4);
  CREATE_NODE_WITH_VALUE_FUNC(ArtNode16, Node16);
  CREATE_NODE_WITH_VALUE_FUNC(ArtNode48, Node48);
  CREATE_NODE_WITH_VALUE_FUNC(ArtNode256, Node256);
  CREATE_NODE_WITH_VALUE_FUNC(ArtLeafNode, LeafNode);
  CREATE_NODE_FUNC(ArtNode4, Node4);
  CREATE_NODE_FUNC(ArtNode16, Node16);
  CREATE_NODE_FUNC(ArtNode48, Node48);
  CREATE_NODE_FUNC(ArtNode256, Node256);
};

class ArtNodeHelper {
 public:
  static char *GetKeyPtr(ArtNode *node, size_t value_size) {
    switch (node->type_) {
      case Node4: {
        if (node->HasValue()) {
          return reinterpret_cast<char *>(node) + sizeof(ArtNode4) + value_size;
        }
        return reinterpret_cast<char *>(node) + sizeof(ArtNode4);
      } break;
      case Node16: {
        if (node->HasValue()) {
          return reinterpret_cast<char *>(node) + sizeof(ArtNode16) + value_size;
        }
        return reinterpret_cast<char *>(node) + sizeof(ArtNode16);
      } break;
      case Node48: {
        if (node->HasValue()) {
          return reinterpret_cast<char *>(node) + sizeof(ArtNode48) + value_size;
        }
        return reinterpret_cast<char *>(node) + sizeof(ArtNode48);
      } break;
      case Node256: {
        if (node->HasValue()) {
          return reinterpret_cast<char *>(node) + sizeof(ArtNode256) + value_size;
        }
        return reinterpret_cast<char *>(node) + sizeof(ArtNode256);
      } break;
      case LeafNode: {
        return reinterpret_cast<char *>(node) + sizeof(ArtLeafNode) + value_size;
      } break;
    }
    assert(false);
    return nullptr;
  }

  static size_t CheckSamePrefixLength(ArtNode *node, char *node_key_ptr, std::string_view key) {
    size_t same_prefix_length = 0;
    size_t cmp_size = std::min(static_cast<size_t>(node->key_length_), key.length());
    while ((cmp_size--) != 0U) {
      if (node_key_ptr[same_prefix_length] != key[same_prefix_length]) {
        break;
      }
      same_prefix_length++;
    }
    return same_prefix_length;
  }

  static void RemoveKeyPrefix(ArtNode *node, char *node_key_ptr, size_t remove_size) {
    node->key_length_ -= remove_size;
    memmove(node_key_ptr, node_key_ptr + remove_size, node->key_length_);
    // TODO，待确认realloc的问题
    // node =
    //     reinterpret_cast<ArtNode *>(realloc(node, node_key_ptr - reinterpret_cast<char *>(node) + node->key_length_));
    // assert(node != nullptr);
  }

  template <class ValueType, class... Args>
  static ArtNode *CopyNewNodeWithValue(ArtNode *node, char *node_key_ptr, Args &&...args) {
    size_t node_header_size = node_key_ptr - reinterpret_cast<char *>(node);
    size_t new_node_size = node_key_ptr + node->key_length_ - reinterpret_cast<char *>(node) + sizeof(ValueType);
    char *new_node = reinterpret_cast<char *>(malloc(new_node_size));
    memcpy(new_node, node, node_key_ptr - reinterpret_cast<char *>(node));
    memcpy(new_node + (node_key_ptr - reinterpret_cast<char *>(node)) + sizeof(ValueType), node_key_ptr,
           node->key_length_);
    new (new_node + (node_key_ptr - reinterpret_cast<char *>(node))) ValueType(std::forward<Args>(args)...);
    auto *return_new_node = reinterpret_cast<ArtNode *>(new_node);
    return_new_node->has_value_ = 1;
    return return_new_node;
  }

  template <class ValueType>
  static void DestroyNode(ArtNode *node, char *node_key_ptr) {
    if (node->HasValue()) {
      auto *value = reinterpret_cast<ValueType *>(node_key_ptr - sizeof(ValueType));
      value->~ValueType();
    }
    free(node);
  }

  // TODO函数待优化
  static bool FindChild(ArtNode *node, char find_char, ArtNode **&next_node) {
    switch (node->type_) {
      case Node4: {
        auto *node4 = static_cast<ArtNode4 *>(node);
        for (int i = 0; i < node4->child_cnt_; i++) {
          if (node4->edge_[i] == find_char) {
            next_node = &node4->childs_[i];
            return true;
          }
        }
      } break;
      case Node16: {
        auto *node16 = static_cast<ArtNode16 *>(node);
        for (int i = 0; i < node16->child_cnt_; i++) {
          if (node16->edge_[i] == find_char) {
            next_node = &node16->childs_[i];
            return true;
          }
        }
      } break;
      case Node48: {
        auto *node48 = static_cast<ArtNode48 *>(node);
        if (auto index = node48->childs_index_[static_cast<uint8_t>(find_char)]; index != -1) {
          next_node = &node48->childs_[index];
          return true;
        }
      } break;
      case Node256: {
        auto *node256 = static_cast<ArtNode256 *>(node);
        auto index = static_cast<uint8_t>(find_char);
        if (node256->childs_[index] == nullptr) {
          return false;
        }
        next_node = &node256->childs_[index];
        return true;
      } break;
      case LeafNode: {
        return false;
      } break;
      default: {
        assert(false);
      }
    }
    return false;
  }

  // TODO 待优化
  template <class ValueType>
  static ArtNode *AddChild(ArtNode *node, char next_char, ArtNode *next_node) {
    switch (node->type_) {
      case Node4: {
        auto *node4 = static_cast<ArtNode4 *>(node);
        if (node4->child_cnt_ < kFour) {
          node4->edge_[node4->child_cnt_] = next_char;
          node4->childs_[node4->child_cnt_] = next_node;
          node4->child_cnt_++;
          return node;
        }
        ArtNode16 *node16;
        char *node_key_ptr;
        if (node4->HasValue()) {
          node_key_ptr = node4->data_ + sizeof(ValueType);
          std::string_view key(node_key_ptr, node4->key_length_);
          auto *value_ptr = reinterpret_cast<ValueType *>(node4->data_);
          node16 = ArtNodeFactory::CreateNode16<ValueType>(key, 5, std::move(*value_ptr));
        } else {
          node_key_ptr = node4->data_;
          std::string_view key(node_key_ptr, node4->key_length_);
          node16 = ArtNodeFactory::CreateNode16(key, 5);
        }
        for (int i = 0; i < kFour; i++) {
          node16->edge_[i] = node4->edge_[i];
          node16->childs_[i] = node4->childs_[i];
        }
        node16->edge_[kFour] = next_char;
        node16->childs_[kFour] = next_node;
        DestroyNode<ValueType>(node, node_key_ptr);
        return reinterpret_cast<ArtNode *>(node16);
      } break;
      case Node16: {
        auto *node16 = static_cast<ArtNode16 *>(node);
        if (node16->child_cnt_ < kSixteen) {
          node16->edge_[node16->child_cnt_] = next_char;
          node16->childs_[node16->child_cnt_] = next_node;
          node16->child_cnt_++;
          return node;
        }
        ArtNode48 *node48;
        char *node_key_ptr;
        if (node16->HasValue()) {
          node_key_ptr = node16->data_ + sizeof(ValueType);
          std::string_view key(node_key_ptr, node16->key_length_);
          auto *value_ptr = reinterpret_cast<ValueType *>(node16->data_);
          node48 = ArtNodeFactory::CreateNode48<ValueType>(key, 17, std::move(*value_ptr));
        } else {
          node_key_ptr = node16->data_;
          std::string_view key(node_key_ptr, node16->key_length_);
          node48 = ArtNodeFactory::CreateNode48(key, 17);
        }
        for (int i = 0; i < kSixteen; i++) {
          node48->childs_index_[static_cast<uint8_t>(node16->edge_[i])] = i;
          node48->childs_[i] = node16->childs_[i];
        }
        node48->childs_index_[static_cast<uint8_t>(next_char)] = kSixteen;
        node48->childs_[kSixteen] = next_node;
        DestroyNode<ValueType>(node, node_key_ptr);
        return reinterpret_cast<ArtNode *>(node48);
      } break;
      case Node48: {
        auto *node48 = static_cast<ArtNode48 *>(node);
        auto next_char_index = static_cast<uint8_t>(next_char);
        if (node48->child_cnt_ < kFortyEight) {
          node48->childs_index_[next_char_index] = node48->child_cnt_;
          node48->childs_[node48->child_cnt_] = next_node;
          node48->child_cnt_++;
          return node;
        }
        ArtNode256 *node256;
        char *node_key_ptr;
        if (node48->HasValue()) {
          node_key_ptr = node48->data_ + sizeof(ValueType);
          std::string_view key(node_key_ptr, node48->key_length_);
          auto *value_ptr = reinterpret_cast<ValueType *>(node48->data_);
          node256 = ArtNodeFactory::CreateNode256<ValueType>(key, 49, std::move(*value_ptr));
        } else {
          node_key_ptr = node48->data_;
          std::string_view key(node_key_ptr, node48->key_length_);
          node256 = ArtNodeFactory::CreateNode256(key, 49);
        }
        for (int i = 0; i < kTwoFiveSix; i++) {
          if (node48->childs_index_[i] == -1) {
            continue;
          }
          node256->childs_[i] = node48->childs_[node48->childs_index_[i]];
        }
        node256->childs_[next_char_index] = next_node;
        DestroyNode<ValueType>(node, node_key_ptr);
        return reinterpret_cast<ArtNode *>(node256);
      } break;
      case Node256: {
        auto *node256 = static_cast<ArtNode256 *>(node);
        auto next_char_index = static_cast<uint8_t>(next_char);
        if (node256->child_cnt_ < kTwoFiveSix) {
          node256->childs_[next_char_index] = next_node;
          node256->child_cnt_++;
          return node;
        }
      } break;
      case LeafNode: {
        auto *leaf_node = static_cast<ArtLeafNode *>(node);
        char *node_key_ptr = leaf_node->data_ + sizeof(ValueType);
        std::string_view key(node_key_ptr, leaf_node->key_length_);
        auto *value_ptr = reinterpret_cast<ValueType *>(leaf_node->data_);
        ArtNode4 *node4 = ArtNodeFactory::CreateNode4<ValueType>(key, 1, std::move(*value_ptr));
        node4->edge_[0] = next_char;
        node4->childs_[0] = next_node;
        DestroyNode<ValueType>(node, node_key_ptr);
        return reinterpret_cast<ArtNode *>(node4);
      } break;
      default: {
        assert(false);
      }
    }
    assert(false);
    return nullptr;
  }

  template <class ValueType>
  static ValueType GetValue(char *node_key_ptr) {
    return *(reinterpret_cast<ValueType *>(node_key_ptr - sizeof(ValueType)));
  }
};

}  // namespace vDB