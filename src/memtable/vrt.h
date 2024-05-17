/*
 * @Author: viktorika
 * @Date: 2024-04-05 19:01:23
 * @Last Modified by: viktorika
 * @Last Modified time: 2024-04-05 19:45:36
 */
#pragma once

#include "util/ebr.h"
#include "comm.h"
#include "vrt_node.h"
#ifdef MEM_DEBUG
#  include <iostream>
#endif

namespace vdb {

/**
 * @param ValueType
 * @param kWriteLock: whether a write lock is needed or not. It is not necessary to enable it for read-only or
 * single-write, multi-read scenarios.
 * @param kReadThreadNum: The number of read threads, which is very important, must be set to the maximum possible
 * number of read threads. Setting it too low will cause a core dump, while setting it too high will affect write
 * performance.
 */
template <class ValueType, bool kWriteLock, uint32_t kReadThreadNum>
class Vrt {
 public:
  Vrt() : root_(nullptr), ebr_mgr_(), root_parent_() {};
  Vrt(const Vrt &) = delete;
  Vrt(Vrt &&) = default;
  ~Vrt();
  Vrt &operator=(const Vrt &) = delete;

  // only read, find the key and return the value
  bool Find(std::string_view key, ValueType *value);
  // If the key does not exist, insert it; otherwise, return the value from vrt
  template <class... Args>
  bool Insert(std::string_view key, ValueType *old_value, Args &&...args);
  // if the key exist, update it
  template <class... Args>
  bool Update(std::string_view key, Args &&...args);
  template <class... Args>
  // if the key does not exist, insert it; otherwise, update it
  bool Upsert(std::string_view key, Args &&...args);
  // if the key exist, delete it
  bool Delete(std::string_view key);

 private:
  bool FindImpl(VrtNode<kWriteLock> *node, std::string_view key, ValueType *value);
  template <class... Args>
  bool InsertImpl(VrtNode<kWriteLock> *&node, VrtNode<kWriteLock> *parent, std::string_view key, ValueType *old_value,
                  Args &&...args);
  template <class... Args>
  bool UpdateImpl(VrtNode<kWriteLock> *&node, VrtNode<kWriteLock> *parent, std::string_view key, Args &&...args);
  template <class... Args>
  bool UpsertImpl(VrtNode<kWriteLock> *&node, VrtNode<kWriteLock> *parent, std::string_view key, Args &&...args);
  bool DeleteImpl(VrtNode<kWriteLock> *&node, VrtNode<kWriteLock> *parent, std::string_view key);

  void FreeNode(VrtNode<kWriteLock> *node);

  VrtNode<kWriteLock> *root_;
  EbrManager<VrtNode<kWriteLock>, VrtNodeDestroy<ValueType, kWriteLock>, kReadThreadNum> ebr_mgr_;
  VrtNode<kWriteLock> root_parent_;
};

template <class ValueType, bool kWriteLock, uint32_t kReadThreadNum>
Vrt<ValueType, kWriteLock, kReadThreadNum>::~Vrt() {
  if (root_ == nullptr) {
    return;
  }
  VrtNodeHelper<kWriteLock>::template DestroyTree<ValueType>(root_);
#ifdef MEM_DEBUG
  ebr_mgr_.ClearAllRetireList();
  std::cout << "create_node_cnt = " << VrtNodeHelper<kWriteLock>::GetCreateNodeCnt() << std::endl;
  std::cout << "destroy_node_cnt = " << VrtNodeHelper<kWriteLock>::GetDestroyNodeCnt() << std::endl;
#endif
}

template <class ValueType, bool kWriteLock, uint32_t kReadThreadNum>
void Vrt<ValueType, kWriteLock, kReadThreadNum>::FreeNode(VrtNode<kWriteLock> *node) {
  ebr_mgr_.FreeObject(node);
}

template <class ValueType, bool kWriteLock, uint32_t kReadThreadNum>
bool Vrt<ValueType, kWriteLock, kReadThreadNum>::Find(std::string_view key, ValueType *value) {
  if (unlikely(key.empty())) {
    return false;
  }
  ebr_mgr_.StartRead();
  if (unlikely(nullptr == root_)) {
    return false;
  }
  auto ret = FindImpl(root_, key, value);
  ebr_mgr_.EndRead();
  return ret;
}

template <class ValueType, bool kWriteLock, uint32_t kReadThreadNum>
bool Vrt<ValueType, kWriteLock, kReadThreadNum>::FindImpl(VrtNode<kWriteLock> *node, std::string_view key,
                                                          ValueType *value) {
  auto same_prefix_length = VrtNodeHelper<kWriteLock>::CheckSamePrefixLength(node, key);
  if (same_prefix_length < node->key_length) {
    return false;
  }
  if (key.length() == same_prefix_length) {
    if (node->has_value) {
      *value = VrtNodeHelper<kWriteLock>::template GetValue<ValueType>(node);
      return true;
    }
    return false;
  }
  if (VrtNode<kWriteLock> *&next_node = VrtNodeHelper<kWriteLock>::FindChild(node, key[same_prefix_length]);
      next_node != nullptr) {
    key.remove_prefix(same_prefix_length + 1);
    return FindImpl(next_node, key, value);
  }
  return false;
}

template <class ValueType, bool kWriteLock, uint32_t kReadThreadNum>
template <class... Args>
bool Vrt<ValueType, kWriteLock, kReadThreadNum>::Insert(std::string_view key, ValueType *old_value, Args &&...args) {
  if (unlikely(key.empty() || key.size() >= kMaxKeySize)) {
    return false;
  }
  root_parent_.Lock();
  if (nullptr == root_) {
    root_ = VrtNodeHelper<kWriteLock>::template CreateVrtNode<LeafNode, ValueType>(key, std::forward<Args>(args)...);
    root_parent_.Unlock();
    return true;
  }
  return InsertImpl(root_, &root_parent_, key, old_value, std::forward<Args>(args)...);
}

template <class ValueType, bool kWriteLock, uint32_t kReadThreadNum>
template <class... Args>
bool Vrt<ValueType, kWriteLock, kReadThreadNum>::InsertImpl(VrtNode<kWriteLock> *&node, VrtNode<kWriteLock> *parent,
                                                            std::string_view key, ValueType *old_value,
                                                            Args &&...args) {
  node->Lock();
  auto same_prefix_length = VrtNodeHelper<kWriteLock>::CheckSamePrefixLength(node, key);
  if (same_prefix_length < key.length() && same_prefix_length < node->key_length) {
    // 同前缀部分作为父节点，旧节点去掉相同部分后作为child1，key剩余部分新建结点作为child2。
    std::string_view new_node_key = key.substr(0, same_prefix_length);
    auto *new_node = VrtNodeHelper<kWriteLock>::template CreateVrtNodeWithoutValue<Node4>(new_node_key);
    auto *child =
        VrtNodeHelper<kWriteLock>::template CreateVrtNodeByRemovePrefix<ValueType>(node, same_prefix_length + 1);
    VrtNodeHelper<kWriteLock>::template AddChild<ValueType>(
        new_node, VrtNodeHelper<kWriteLock>::GetKeyIndexChar(node, same_prefix_length), child);
    char next_char = key[same_prefix_length];
    key.remove_prefix(same_prefix_length + 1);
    child = VrtNodeHelper<kWriteLock>::template CreateVrtNode<LeafNode, ValueType>(key, std::forward<Args>(args)...);
    VrtNodeHelper<kWriteLock>::template AddChild<ValueType>(new_node, next_char, child);
    auto *old_node = node;
    node = new_node;
    parent->Unlock();
    FreeNode(old_node);
    return true;
  }
  if (same_prefix_length == key.length() && same_prefix_length < node->key_length) {
    // 同前缀部分作为父节点并且插入值，旧节点去掉相同部分后作为child1
    std::string_view new_node_key = key.substr(0, same_prefix_length);
    auto *new_node =
        VrtNodeHelper<kWriteLock>::template CreateVrtNode<Node4, ValueType>(new_node_key, std::forward<Args>(args)...);
    auto *child =
        VrtNodeHelper<kWriteLock>::template CreateVrtNodeByRemovePrefix<ValueType>(node, same_prefix_length + 1);
    VrtNodeHelper<kWriteLock>::template AddChild<ValueType>(
        new_node, VrtNodeHelper<kWriteLock>::GetKeyIndexChar(node, same_prefix_length), child);
    auto *old_node = node;
    node = new_node;
    parent->Unlock();
    FreeNode(old_node);
    return true;
  }
  if (same_prefix_length == key.length() && same_prefix_length == node->key_length) {
    // 值挂在当前节点上
    if (node->has_value) {
      *old_value = VrtNodeHelper<kWriteLock>::template GetValue<ValueType>(node);
      node->Unlock();
      parent->Unlock();
      return false;
    }
    auto *old_node = node;
    auto *new_node =
        VrtNodeHelper<kWriteLock>::template CreateVrtNodeByAddValue<ValueType>(node, std::forward<Args>(args)...);
    node = new_node;
    parent->Unlock();
    FreeNode(old_node);
    return true;
  }
  // 继续搜索，没有找到对应子节点，增新增叶子挂在当前节点上
  if (VrtNode<kWriteLock> *&next_node = VrtNodeHelper<kWriteLock>::FindChild(node, key[same_prefix_length]);
      next_node != nullptr) {
    parent->Unlock();
    key.remove_prefix(same_prefix_length + 1);
    return InsertImpl(next_node, node, key, old_value, std::forward<Args>(args)...);
  }
  char next_char = key[same_prefix_length];
  key.remove_prefix(same_prefix_length + 1);
  auto *new_node =
      VrtNodeHelper<kWriteLock>::template CreateVrtNode<LeafNode, ValueType>(key, std::forward<Args>(args)...);
  auto *node_pre_add_child = node;
  node = VrtNodeHelper<kWriteLock>::template AddChild<ValueType>(node, next_char, new_node);
  if (node != node_pre_add_child) {
    FreeNode(node_pre_add_child);
  }
  parent->Unlock();
  node->Unlock();
  return true;
}

template <class ValueType, bool kWriteLock, uint32_t kReadThreadNum>
template <class... Args>
bool Vrt<ValueType, kWriteLock, kReadThreadNum>::Update(std::string_view key, Args &&...args) {
  if (unlikely(key.empty() || key.size() >= kMaxKeySize)) {
    return false;
  }
  root_parent_.Lock();
  if (nullptr == root_) {
    root_parent_.Unlock();
    return false;
  }
  return UpdateImpl(root_, &root_parent_, key, std::forward<Args>(args)...);
}

template <class ValueType, bool kWriteLock, uint32_t kReadThreadNum>
template <class... Args>
bool Vrt<ValueType, kWriteLock, kReadThreadNum>::UpdateImpl(VrtNode<kWriteLock> *&node, VrtNode<kWriteLock> *parent,
                                                            std::string_view key, Args &&...args) {
  node->Lock();
  auto same_prefix_length = VrtNodeHelper<kWriteLock>::CheckSamePrefixLength(node, key);
  if (same_prefix_length < node->key_length) {
    node->Unlock();
    parent->Unlock();
    return false;
  }
  if (key.length() == same_prefix_length) {
    if (node->has_value) {
      auto *new_node =
          VrtNodeHelper<kWriteLock>::template CreateVrtNodeByAddValue<ValueType>(node, std::forward<Args>(args)...);
      auto *old_node = node;
      node = new_node;
      parent->Unlock();
      FreeNode(old_node);
      return true;
    }
    node->Unlock();
    parent->Unlock();
    return false;
  }
  parent->Unlock();
  if (VrtNode<kWriteLock> *&next_node = VrtNodeHelper<kWriteLock>::FindChild(node, key[same_prefix_length]);
      next_node != nullptr) {
    key.remove_prefix(same_prefix_length + 1);
    return UpdateImpl(next_node, node, key, std::forward<Args>(args)...);
  }
  node->Unlock();
  return false;
}

template <class ValueType, bool kWriteLock, uint32_t kReadThreadNum>
template <class... Args>
bool Vrt<ValueType, kWriteLock, kReadThreadNum>::Upsert(std::string_view key, Args &&...args) {
  if (unlikely(key.empty() || key.size() >= kMaxKeySize)) {
    return false;
  }
  root_parent_.Lock();
  if (nullptr == root_) {
    root_ = VrtNodeHelper<kWriteLock>::template CreateVrtNode<LeafNode, ValueType>(key, std::forward<Args>(args)...);
    root_parent_.Unlock();
    return true;
  }
  return UpsertImpl(root_, &root_parent_, key, std::forward<Args>(args)...);
}

template <class ValueType, bool kWriteLock, uint32_t kReadThreadNum>
template <class... Args>
bool Vrt<ValueType, kWriteLock, kReadThreadNum>::UpsertImpl(VrtNode<kWriteLock> *&node, VrtNode<kWriteLock> *parent,
                                                            std::string_view key, Args &&...args) {
  node->Lock();
  auto same_prefix_length = VrtNodeHelper<kWriteLock>::CheckSamePrefixLength(node, key);
  if (same_prefix_length < key.length() && same_prefix_length < node->key_length) {
    // 同前缀部分作为父节点，旧节点去掉相同部分后作为child1，key剩余部分新建结点作为child2。
    std::string_view new_node_key = key.substr(0, same_prefix_length);
    auto *new_node = VrtNodeHelper<kWriteLock>::template CreateVrtNodeWithoutValue<Node4>(new_node_key);
    auto *child =
        VrtNodeHelper<kWriteLock>::template CreateVrtNodeByRemovePrefix<ValueType>(node, same_prefix_length + 1);
    VrtNodeHelper<kWriteLock>::template AddChild<ValueType>(
        new_node, VrtNodeHelper<kWriteLock>::GetKeyIndexChar(node, same_prefix_length), child);

    char next_char = key[same_prefix_length];
    key.remove_prefix(same_prefix_length + 1);
    child = VrtNodeHelper<kWriteLock>::template CreateVrtNode<LeafNode, ValueType>(key, std::forward<Args>(args)...);
    VrtNodeHelper<kWriteLock>::template AddChild<ValueType>(new_node, next_char, child);

    auto *old_node = node;
    node = new_node;
    parent->Unlock();
    FreeNode(old_node);
    return true;
  }
  if (same_prefix_length == key.length() && same_prefix_length < node->key_length) {
    // 同前缀部分作为父节点并且插入值，旧节点去掉相同部分后作为child1
    std::string_view new_node_key = key.substr(0, same_prefix_length);
    auto *new_node =
        VrtNodeHelper<kWriteLock>::template CreateVrtNode<Node4, ValueType>(new_node_key, std::forward<Args>(args)...);
    auto *child =
        VrtNodeHelper<kWriteLock>::template CreateVrtNodeByRemovePrefix<ValueType>(node, same_prefix_length + 1);
    VrtNodeHelper<kWriteLock>::template AddChild<ValueType>(
        new_node, VrtNodeHelper<kWriteLock>::GetKeyIndexChar(node, same_prefix_length), child);
    auto *old_node = node;
    node = new_node;
    parent->Unlock();
    FreeNode(old_node);
    return true;
  }
  if (same_prefix_length == key.length() && same_prefix_length == node->key_length) {
    // 值挂在当前节点上
    auto *old_node = node;
    auto *new_node =
        VrtNodeHelper<kWriteLock>::template CreateVrtNodeByAddValue<ValueType>(node, std::forward<Args>(args)...);
    node = new_node;
    parent->Unlock();
    FreeNode(old_node);
    return true;
  }
  // 继续搜索，没有找到对应子节点，增新增叶子挂在当前节点上
  if (VrtNode<kWriteLock> *&next_node = VrtNodeHelper<kWriteLock>::FindChild(node, key[same_prefix_length]);
      next_node != nullptr) {
    parent->Unlock();
    key.remove_prefix(same_prefix_length + 1);
    return UpsertImpl(next_node, node, key, std::forward<Args>(args)...);
  }
  char next_char = key[same_prefix_length];
  key.remove_prefix(same_prefix_length + 1);
  auto *new_node =
      VrtNodeHelper<kWriteLock>::template CreateVrtNode<LeafNode, ValueType>(key, std::forward<Args>(args)...);
  auto *node_pre_add_child = node;
  node = VrtNodeHelper<kWriteLock>::template AddChild<ValueType>(node, next_char, new_node);
  if (node != node_pre_add_child) {
    FreeNode(node_pre_add_child);
  }
  parent->Unlock();
  node->Unlock();
  return true;
}

// 删除
template <class ValueType, bool kWriteLock, uint32_t kReadThreadNum>
bool Vrt<ValueType, kWriteLock, kReadThreadNum>::Delete(std::string_view key) {
  if (unlikely(key.empty() || key.size() >= kMaxKeySize)) {
    return false;
  }
  root_parent_.Lock();
  if (nullptr == root_) {
    root_parent_.Unlock();
    return false;
  }
  return DeleteImpl(root_, &root_parent_, key);
}

template <class ValueType, bool kWriteLock, uint32_t kReadThreadNum>
bool Vrt<ValueType, kWriteLock, kReadThreadNum>::DeleteImpl(VrtNode<kWriteLock> *&node, VrtNode<kWriteLock> *parent,
                                                            std::string_view key) {
  node->Lock();
  auto same_prefix_length = VrtNodeHelper<kWriteLock>::CheckSamePrefixLength(node, key);
  if (same_prefix_length < node->key_length) {
    node->Unlock();
    parent->Unlock();
    return false;
  }
  if (key.length() == same_prefix_length) {
    if (node->has_value) {
      // TODO，考虑路径压缩
      auto *new_node = VrtNodeHelper<kWriteLock>::template CreateVrtNodeByDeleteValue<ValueType>(node);
      auto *old_node = node;
      node = new_node;
      parent->Unlock();
      FreeNode(old_node);
      return true;
    }
    node->Unlock();
    parent->Unlock();
    return false;
  }
  parent->Unlock();
  if (VrtNode<kWriteLock> *&next_node = VrtNodeHelper<kWriteLock>::FindChild(node, key[same_prefix_length]);
      next_node != nullptr) {
    key.remove_prefix(same_prefix_length + 1);
    return DeleteImpl(next_node, node, key);
  }
  node->Unlock();
  return false;
}

}  // namespace vdb
