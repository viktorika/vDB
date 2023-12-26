#pragma once

#include <wchar.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "comm.h"

namespace VDB {

template <class ValueType>
class ArtNode;

template <class ValueType>
class ArtNode4WithValue;

template <class ValueType>
class ArtNode4;

template <class ValueType>
class ArtNode16WithValue;

template <class ValueType>
class ArtNode16;

template <class ValueType>
class ArtNode48WithValue;

template <class ValueType>
class ArtNode48;

template <class ValueType>
class ArtNode256WithValue;

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
  [[nodiscard]] bool HasValue() const { return (type & 1) == 0; }

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

  void RemovePrefix(size_t size) {
    prefix_length -= size;
    memmove(prefix.data(), prefix.data() + size, prefix_length);
  }

  friend class Art<ValueType>;

 private:
  uint8_t type : 4;
  uint8_t prefix_length : 4;
  uint8_t child_cnt;
  std::array<char, kTen> prefix{};
};

template <class ValueType>
class ArtNode4WithValue : public ArtNode4<ValueType> {
 public:
  template <class... Args>
  ArtNode4WithValue(std::string_view key, uint8_t child_cnt, Args &&...args)
      : ArtNode4<ValueType>(key, child_cnt), value(std::forward(args)...) {}

  template <class... Args>
  ArtNode4WithValue(ArtNode4<ValueType> &&node, Args &&...args)
      : ArtNode4<ValueType>(std::forward(node)), value(std::forward(args)...) {}

  friend class Art<ValueType>;

 private:
  ValueType value;
};

template <class ValueType>
class ArtNode4 : public ArtNode<ValueType> {
 public:
  ArtNode4(std::string_view key, uint8_t child_cnt)
      : ArtNode<ValueType>(Node4, key, child_cnt) {}

  friend class Art<ValueType>;

 private:
  std::array<char, kFour> keys{};
  std::array<std::unique_ptr<ArtNode<ValueType>>, kFour> childs{};
};

template <class ValueType>
class ArtNode16WithValue : public ArtNode16<ValueType> {
 public:
  template <class... Args>
  ArtNode16WithValue(std::string_view key, uint8_t child_cnt, Args &&...args)
      : ArtNode16<ValueType>(key, child_cnt), value(std::forward(args)...) {}

  template <class... Args>
  ArtNode16WithValue(ArtNode16<ValueType> &&node, Args &&...args)
      : ArtNode16<ValueType>(std::forward(node)),
        value(std::forward(args)...) {}

  friend class Art<ValueType>;

 private:
  ValueType value;
};

template <class ValueType>
class ArtNode16 : public ArtNode<ValueType> {
 public:
  ArtNode16(std::string_view key, uint8_t child_cnt)
      : ArtNode<ValueType>(Node16, key, child_cnt) {}

  friend class Art<ValueType>;

 private:
  std::array<char, kSixteen> keys{};
  std::array<std::unique_ptr<ArtNode<ValueType>>, kSixteen> childs{};
};

template <class ValueType>
class ArtNode48WithValue : public ArtNode48<ValueType> {
 public:
  template <class... Args>
  ArtNode48WithValue(std::string_view key, uint8_t child_cnt, Args &&...args)
      : ArtNode48<ValueType>(key, child_cnt), value(std::forward(args)...) {}

  template <class... Args>
  ArtNode48WithValue(ArtNode48<ValueType> &&node, Args &&...args)
      : ArtNode48<ValueType>(std::forward(node)),
        value(std::forward(args)...) {}

  friend class Art<ValueType>;

 private:
  ValueType value;
};

template <class ValueType>
class ArtNode48 : public ArtNode<ValueType> {
 public:
  ArtNode48(std::string_view key, uint8_t child_cnt)
      : ArtNode<ValueType>(Node48, key, child_cnt) {}

  friend class Art<ValueType>;

 private:
  std::array<char, kTwoFiveSize> childs_index{};
  std::array<std::unique_ptr<ArtNode<ValueType>>, kFortyEight> childs{};
};

template <class ValueType>
class ArtNode256WithValue : public ArtNode256<ValueType> {
 public:
  template <class... Args>
  ArtNode256WithValue(std::string_view key, uint8_t child_cnt, Args &&...args)
      : ArtNode256<ValueType>(key, child_cnt), value(std::forward(args)...) {}

  template <class... Args>
  ArtNode256WithValue(ArtNode256<ValueType> &&node, Args &&...args)
      : ArtNode256<ValueType>(std::forward(node)),
        value(std::forward(args)...) {}

  friend class Art<ValueType>;

 private:
  ValueType value;
};

template <class ValueType>
class ArtNode256 : public ArtNode<ValueType> {
 public:
  ArtNode256(std::string_view key, uint8_t child_cnt)
      : ArtNode<ValueType>(Node256, key, child_cnt) {}

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
  // [[nodiscard]] bool LeafMatches(const std::string_view key) const {
  //   if (key.length() != this->prefix_length) {
  //     return false;
  //   }
  //   return 0 == memcmp(key.data(), this->prefix.data(), this->prefix_length);
  // }

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
                  std::unique_ptr<ArtNode<ValueType>> *node_ptr,
                  std::string_view key, ValueType *old_value, Args &&...args);

  template <class... Args>
  bool UpdateImpl(ArtNode<ValueType> *node, std::string_view key,
                  Args &&...args);

  template <class... Args>
  void UpSertImpl(ArtNode<ValueType> *node,
                  std::unique_ptr<ArtNode<ValueType>> *node_ptr,
                  std::string_view key, Args &&...args);

  std::unique_ptr<ArtNode<ValueType>> *FindChild(ArtNode<ValueType> *node,
                                                 char next_character);
  ValueType GetValue(ArtNode<ValueType> *node);

  template <class... Args>
  void UpdateValue(ArtNode<ValueType> *node, Args &&...args);

  template <class... Args>
  std::unique_ptr<ArtNode<ValueType>> NewNodeWithValue(ArtNode<ValueType> *node,
                                                       Args &&...args);

  template <class... Args>
  std::unique_ptr<ArtNode<ValueType>> AddLeafChild(ArtNode<ValueType> *node,
                                                   std::string_view key,
                                                   Args &&...args);

  std::unique_ptr<ArtNode<ValueType>> root;
};

// 根据字符串查找孩子节点，找不到返回nullptr
template <class ValueType>
std::unique_ptr<ArtNode<ValueType>> *Art<ValueType>::FindChild(
    ArtNode<ValueType> *node, char next_character) {
  switch (node->type) {
    case Node4WithValue:
    case Node4: {
      auto *node4 = static_cast<ArtNode4<ValueType> *>(node);
      for (int i = 0; i < node4->child_cnt; i++) {
        if (node4->keys[i] == next_character) {
          return &node4->childs[i];
        }
      }
    } break;
    case Node16WithValue:
    case Node16: {
      // TODO simd优化，或者考虑编译器自动向量化
      auto *node16 = static_cast<ArtNode16<ValueType> *>(node);
      for (int i = 0; i < node16->base.child_cnt; i++) {
        if (node16->keys[i] == next_character) {
          return &node16->childs[i];
        }
      }
    } break;
    case Node48WithValue:
    case Node48: {
      auto *node48 = static_cast<ArtNode48<ValueType> *>(node);
      if (auto index = node48->childs_index[next_character]; index != -1) {
        return &node48->childs[index];
      }
    } break;
    case Node256WithValue:
    case Node256: {
      auto *node256 = static_cast<ArtNode256<ValueType> *>(node);
      return &node256->childs[next_character];
    } break;
    case LeafNode: {
      return nullptr;
    } break;
    default:
      assert(false);
  }
  return nullptr;
}

template <class ValueType>
ValueType Art<ValueType>::GetValue(ArtNode<ValueType> *node) {
  switch (node->type) {
    case Node4WithValue: {
      return static_cast<ArtNode4WithValue<ValueType> *>(node)->value;
    } break;
    case Node16WithValue: {
      return static_cast<ArtNode16WithValue<ValueType> *>(node)->value;
    } break;
    case Node48WithValue: {
      return static_cast<ArtNode48WithValue<ValueType> *>(node)->value;
    } break;
    case Node256WithValue: {
      return static_cast<ArtNode256WithValue<ValueType> *>(node)->value;
    } break;
    case LeafNode: {
      return static_cast<ArtLeafNode<ValueType> *>(node)->value;
    } break;
    default:
      assert(false);
  }
  assert(false);
  return ValueType();
}

template <class ValueType>
template <class... Args>
void Art<ValueType>::UpdateValue(ArtNode<ValueType> *node, Args &&...args) {
  switch (node->type) {
    case Node4WithValue: {
      static_cast<ArtNode4WithValue<ValueType> *>(node)->value =
          ValueType(std::forward(args)...);
    } break;
    case Node16WithValue: {
      static_cast<ArtNode16WithValue<ValueType> *>(node)->value =
          ValueType(std::forward(args)...);
    } break;
    case Node48WithValue: {
      static_cast<ArtNode48WithValue<ValueType> *>(node)->value =
          ValueType(std::forward(args)...);
    } break;
    case Node256WithValue: {
      static_cast<ArtNode256WithValue<ValueType> *>(node)->value =
          ValueType(std::forward(args)...);
    } break;
    case LeafNode: {
      static_cast<ArtLeafNode<ValueType> *>(node)->value =
          ValueType(std::forward(args)...);
    } break;
    default:
      assert(false);
  }
  assert(false);
}

template <class ValueType>
template <class... Args>
std::unique_ptr<ArtNode<ValueType>> Art<ValueType>::NewNodeWithValue(
    ArtNode<ValueType> *node, Args &&...args) {
  switch (node->type) {
    case Node4WithValue: {
      return std::unique_ptr<ArtNode<ValueType>>(
          new ArtNode4WithValue<ValueType>(
              std::move(*static_cast<ArtNode4<ValueType> *>(node))),
          std::forward(args)...);
    } break;
    case Node16WithValue: {
      return std::unique_ptr<ArtNode<ValueType>>(
          new ArtNode16WithValue<ValueType>(
              std::move(*static_cast<ArtNode16<ValueType> *>(node))),
          std::forward(args)...);
    } break;
    case Node48WithValue: {
      return std::unique_ptr<ArtNode<ValueType>>(
          new ArtNode48WithValue<ValueType>(
              std::move(*static_cast<ArtNode48<ValueType> *>(node))),
          std::forward(args)...);
    } break;
    case Node256WithValue: {
      return std::unique_ptr<ArtNode<ValueType>>(
          new ArtNode256WithValue<ValueType>(
              std::move(*static_cast<ArtNode256<ValueType> *>(node))),
          std::forward(args)...);
    } break;
    default:
      assert(false);
  }
  assert(false);
  return nullptr;
}

// TODO
template <class ValueType>
template <class... Args>
std::unique_ptr<ArtNode<ValueType>> Art<ValueType>::AddLeafChild(
    ArtNode<ValueType> *node, std::string_view key, Args &&...args) {
  switch (node->type) {
    case Node4: {
      auto *node4 = static_cast<ArtNode4<ValueType> *>(node);
      if (node4->child_cnt < kFour) {
        node4->keys[node4->child_cnt] = key[0];
        node4->childs[node4->child_cnt++] = std::make_unique<LeafNode>(
            key.remove_prefix(1), std::forward(args)...);
        return nullptr;
      }
      auto new_node16 =
          std::make_unique<ArtNode16<ValueType>>(node4->prefix, kFive);
      memcpy(new_node16->keys.data(), node4->keys.data(), kFour);
      new_node16->keys[kFive] = key[0];
      std::copy_n(std::make_move_iterator(node4->childs.begin()), kFour,
                  new_node16->childs.begin());
      new_node16->childs[kFive] = std::make_unique<LeafNode>(
          key.remove_prefix(1), std::forward(args)...);
      return new_node16;
    } break;
    case Node4WithValue: {
      auto *node4withvalue = static_cast<ArtNode4WithValue<ValueType> *>(node);
      if (node4withvalue->child_cnt < kFour) {
        node4withvalue->keys[node4withvalue->child_cnt] = key[0];
        node4withvalue->childs[node4withvalue->child_cnt++] =
            std::make_unique<LeafNode>(key.remove_prefix(1),
                                       std::forward(args)...);
        return nullptr;
      }
      // node4->node16
    } break;
    case Node16: {
      auto *node16 = static_cast<ArtNode16<ValueType> *>(node);
      if (node16->child_cnt < kSixteen) {
        node16->keys[node16->child_cnt] = key[0];
        node16->childs[node16->child_cnt++] = std::make_unique<LeafNode>(
            key.remove_prefix(1), std::forward(args)...);
      }
      // node16->node48;
    } break;
    case Node16WithValue: {
    } break;
    case Node48: {
    }
    case Node48WithValue: {
    } break;
    case Node256: {
    } break;
    case Node256WithValue: {
    } break;
    default:
      assert(false);
  }
  assert(false);
  return nullptr;
}

template <class ValueType>
bool Art<ValueType>::Find(std::string_view key, ValueType *value) {
  if (key.empty()) {
    return false;
  }
  return FindImpl(root.get(), key, value);
}

template <class ValueType>
bool Art<ValueType>::FindImpl(ArtNode<ValueType> *node, std::string_view key,
                              ValueType *value) {
  size_t same_prefix_length = node->CheckPrefix(key);
  if (same_prefix_length < node->prefix_length) {
    return false;
  }
  if (key.length() == same_prefix_length) {
    if (node->HasValue()) {
      *value = GetValue(node);
      return true;
    }
    return false;
  }
  key.remove_prefix(same_prefix_length);
  auto *child = FindChild(node, key[0]);
  if (nullptr == child) {
    return false;
  }
  return FindImpl(child->get(), key.remove_prefix(1), value);
}

template <class ValueType>
template <class... Args>
bool Art<ValueType>::Insert(std::string_view key, ValueType *old_value,
                            Args &&...args) {
  if (key.empty()) {
    return false;
  }
  return InsertImpl(root.get(), &root, key, old_value, std::forward(args)...);
}

template <class ValueType>
template <class... Args>
bool Art<ValueType>::InsertImpl(ArtNode<ValueType> *node,
                                std::unique_ptr<ArtNode<ValueType>> *node_ptr,
                                std::string_view key, ValueType *old_value,
                                Args &&...args) {
  auto same_prefix_length = node->CheckPrefix(key);
  // 只能匹配节点前缀一部分
  if (same_prefix_length < node->prefix_length) {
    // 只能匹配key一部分
    if (same_prefix_length < key.length()) {
      // 有共同前缀，把共同前缀的部分拆成一个新的node4，剩余前缀作为孩子，并插入一个新的叶子节点
      auto same_prefix_key = key;
      same_prefix_key.remove_suffix(key.length() - same_prefix_length);
      auto new_node4 =
          std::make_unique<ArtNode4<ValueType>>(same_prefix_key, 2);
      // add child
      new_node4->keys[0] = node->prefix[same_prefix_length];
      node->RemovePrefix(same_prefix_length + 1);
      new_node4->childs[0] = std::move(*node_ptr);

      // add child
      key.remove_prefix(same_prefix_length);
      new_node4->keys[1] = key[0];
      new_node4->childs[1] = std::make_unique<ArtLeafNode<ValueType>>(
          key.remove_prefix(1), std::forward(args)...);

      *node_ptr = std::move(new_node4);
      return true;
    }
    // 可以匹配key的全部
    // 插入一个中间节点，旧的节点作为新的中间节点的一个孩子。
    auto same_prefix_key = key;
    same_prefix_key.remove_suffix(key.length() - same_prefix_length);
    auto new_node4 = std::make_unique<ArtNode4WithValue<ValueType>>(
        same_prefix_key, 1, std::forward(args)...);
    new_node4->keys[0] = node->prefix[same_prefix_length];
    node->RemovePrefix(same_prefix_length + 1);
    new_node4->childs[0] = std::move(*node_ptr);
    *node_ptr = std::move(new_node4);
    return true;
  }
  // 可以匹配节点的全部
  key.remove_prefix(same_prefix_length);
  // 同时key也可以全匹配
  if (key.empty()) {
    if (node->HasValue()) {
      *old_value = GetValue(node);
      return false;
    }
    *node_ptr = NewNodeWithValue(node, std::forward(args)...);
    return true;
  }
  // 还要继续搜索key
  auto *child = FindChild(node, key[0]);
  // 找不到相应的孩子
  if (nullptr == child) {
    // 添加一个叶子节点
    // TODO???
  }
  return InsertImpl(child->get(), child, key, old_value, std::forward(args)...);
}

template <class ValueType>
template <class... Args>
bool Art<ValueType>::Update(std::string_view key, Args &&...args) {
  return UpdateImpl(root.get(), key, std::forward(args)...);
}

template <class ValueType>
template <class... Args>
bool Art<ValueType>::UpdateImpl(ArtNode<ValueType> *node, std::string_view key,
                                Args &&...args) {
  size_t same_prefix_length = node->CheckPrefix(key);
  if (same_prefix_length < node->prefix_length) {
    return false;
  }
  if (key.length() == same_prefix_length) {
    if (node->HasValue()) {
      UpdateValue(node, std::forward(args)...);
      return true;
    }
    return false;
  }
  key.remove_prefix(same_prefix_length);
  auto *child = FindChild(node, key[0]);
  if (nullptr == node) {
    return false;
  }
  return UpdateImpl(child->get(), key.remove_prefix(1), std::forward(args)...);
}

template <class ValueType>
template <class... Args>
void Art<ValueType>::Upsert(std::string_view key, Args &&...args) {
  return UpSertImpl(root.get(), &root, key, std::forward(args)...);
}

template <class ValueType>
template <class... Args>
void Art<ValueType>::UpSertImpl(ArtNode<ValueType> *node,
                                std::unique_ptr<ArtNode<ValueType>> *node_ptr,
                                std::string_view key, Args &&...args) {
  // if (node == nullptr) {
  //   *node_ptr =
  //       std::make_unique<ArtLeafNode<ValueType>>(key, std::forward(args)...);
  //   return;
  // }
  // auto same_prefix_length = node->CheckPrefix(key);
  // if (same_prefix_length < node->prefix_length) {
  //   if (same_prefix_length < key.length()) {
  //     //
  //     有共同前缀，把共同前缀的部分拆成一个新的node4，剩余前缀作为孩子，并插入一个新的叶子节点
  //     auto same_prefix_key = key;
  //     same_prefix_key.remove_suffix(key.length() - same_prefix_length);
  //     auto new_node4 =
  //         std::make_unique<ArtNode4<ValueType>>(same_prefix_key, 2);
  //     // add child
  //     new_node4->keys[0] = node->prefix[same_prefix_length];
  //     node->RemovePrefix(same_prefix_length + 1);
  //     new_node4->childs[0] = std::move(*node_ptr);

  //     // add child
  //     key.remove_prefix(same_prefix_length);
  //     new_node4->keys[1] = key[0];
  //     new_node4->childs[1] = std::make_unique<ArtLeafNode<ValueType>>(
  //         key.remove_prefix(1), std::forward(args)...);

  //     *node_ptr = std::move(new_node4);
  //     return;
  //   }
  //   // 插入一个中间节点，旧的节点作为新的中间节点的一个孩子。
  //   auto same_prefix_key = key;
  //   same_prefix_key.remove_suffix(key.length() - same_prefix_length);
  //   auto new_node4 = std::make_unique<ArtNode4WithValue<ValueType>>(
  //       same_prefix_key, 1, std::forward(args)...);
  //   new_node4->keys[0] = node->prefix[same_prefix_length];
  //   node->RemovePrefix(same_prefix_length + 1);
  //   new_node4->childs[0] = std::move(*node_ptr);
  //   *node_ptr = std::move(new_node4);
  //   return;
  // }
  // key.remove_prefix(same_prefix_length);
  // if (key.empty()) {
  //   if (node->HasValue()) {
  //     UpdateValue(node, std::forward(args)...);
  //     return;
  //   }
  //   *node_ptr = NewNodeWithValue(node, std::forward(args)...);
  //   return;
  // }
  // auto *child = FindChild(node, key[0]);
  // return UpSertImpl(child->get(), child, key, std::forward(args)...);
}

}  // namespace VDB