#pragma once

#include <utility>
#include "art_node.h"

namespace vDB {

template <class ValueType>
class Art {
 public:
  // TODO copyconstruct

  // 查找
  // bool Find(std::string_view key, ValueType *value);
  // 插入
  template <class... Args>
  bool Insert(std::string_view key, ValueType *old_value, Args &&...args);
  // 更新
  // template <class... Args>
  // bool Update(std::string_view key, Args &&...args);
  // // 插入或更新
  // template <class... Args>
  // void Upsert(std::string_view key, Args &&...args);
  // // 删除
  // bool Delete(std::string_view key);

 private:
  // bool FindImpl(ArtNode<ValueType> *node, std::string_view key, ValueType *value);

  template <class... Args>
  bool InsertImpl(ArtNode *&node, std::string_view key, ValueType *old_value, Args &&...args);

  // template <class... Args>
  // bool UpdateImpl(ArtNode<ValueType> *node, std::string_view key, Args &&...args);

  // template <class... Args>
  // void UpSertImpl(ArtNode<ValueType> *node, std::unique_ptr<ArtNode<ValueType>> *node_ptr, std::string_view key,
  //                 Args &&...args);

  // std::unique_ptr<ArtNode<ValueType>> *FindChild(ArtNode<ValueType> *node, char next_character);
  // ValueType GetValue(ArtNode<ValueType> *node);

  // template <class... Args>
  // void UpdateValue(ArtNode<ValueType> *node, Args &&...args);

  // template <class... Args>
  // std::unique_ptr<ArtNode<ValueType>> NewNodeWithValue(ArtNode<ValueType> *node, Args &&...args);

  // template <class... Args>
  // std::unique_ptr<ArtNode<ValueType>> AddLeafChild(ArtNode<ValueType> *node, std::string_view key, Args &&...args);

  ArtNode *root;
};

template <class ValueType>
template <class... Args>
bool Art<ValueType>::Insert(std::string_view key, ValueType *old_value, Args &&...args) {
  if (key.empty() || key.size() >= kMaxKeySize) {
    return false;
  }
  return InsertImpl(root, key, old_value, std::forward<Args>(args)...);
}

template <class ValueType>
template <class... Args>
bool Art<ValueType>::InsertImpl(ArtNode *&node, std::string_view key, ValueType *old_value, Args &&...args) {
  auto *node_key_ptr = ArtNodeHelper::GetKeyPtr(node, sizeof(ValueType));
  auto same_prefix_length = ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, key);
  if (same_prefix_length < key.length() && same_prefix_length < node->key_length_) {
    // 同前缀部分作为父节点，旧节点去掉相同部分后作为child1，key剩余部分新建叶子结点作为child2。
    std::string_view new_father_key = key.substr(0, same_prefix_length);
    ArtNode4 *new_father = ArtNodeFactory::CreateNode4(new_father_key, 2);

    new_father->edge[0] = node_key_ptr[same_prefix_length];
    ArtNodeHelper::RemoveKeyPrefix(node, node_key_ptr, same_prefix_length + 1);
    new_father->childs[0] = node;

    new_father->edge[1] = key[same_prefix_length];
    key.remove_prefix(same_prefix_length + 1);
    ArtLeafNode *new_leaf = ArtNodeFactory::CreateLeafNode<ValueType>(key, 0, std::forward<Args>(args)...);
    new_father->childs[1] = reinterpret_cast<ArtNode *>(new_leaf);

    node = reinterpret_cast<ArtNode *>(new_father);
    return true;
  }
  if (same_prefix_length == key.length() && same_prefix_length < node->key_length_) {
    // 同前缀部分作为父节点并且插入值，旧节点去掉相同部分后作为child1
    std::string_view new_father_key = key.substr(0, same_prefix_length);
    ArtNode4 *new_father = ArtNodeFactory::CreateNode4(new_father_key, 1, std::forward<Args>(args)...);
    new_father->edge[0] = node_key_ptr[same_prefix_length];
    ArtNodeHelper::RemoveKeyPrefix(node, node_key_ptr, same_prefix_length + 1);
    new_father->childs[0] = node;
    node = reinterpret_cast<ArtNode *>(new_father);
    return true;
  }
  if (same_prefix_length == key.length() && same_prefix_length == node->key_length_) {
    // 值挂在当前节点上
    if (node->HasValue()) {
      *old_value = ArtNodeHelper::GetValue<ValueType>(node_key_ptr);
      return false;
    }
    auto *old_node = node;
    node = ArtNodeHelper::CopyNewNodeWithValue(node, node_key_ptr, std::forward<Args>(args)...);
    ArtNodeHelper::DestroyNode<ValueType>(old_node, node_key_ptr);
    return true;
  }
  // 继续搜索，没有找到对应子节点，增新增叶子挂在当前节点上
  ArtNode **next_node;
  if (ArtNodeHelper::FindChild(node, key[same_prefix_length], next_node)) {
    key.remove_prefix(same_prefix_length + 1);
    return InsertImpl(*next_node, key, old_value, std::forward<Args>(args)...);
  }
  char next_char = key[same_prefix_length];
  key.remove_prefix(same_prefix_length + 1);
  ArtNode *leaf_node =
      reinterpret_cast<ArtNode *>(ArtNodeFactory::CreateLeafNode<ValueType>(key, 0, std::forward<Args>(args)...));
  node = ArtNodeHelper::AddChild<ValueType>(node, next_char, leaf_node);
  return true;
}

}  // namespace vDB