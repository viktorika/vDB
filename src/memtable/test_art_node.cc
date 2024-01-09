#include <array>
#include <iostream>
#include <string>

#include "art_node.h"
#include "comm.h"
#include "gtest/gtest.h"

TEST(NodeCreateTest, Node4CreateTest) {
  std::string key("123");
  auto *node = vDB::ArtNodeFactory::CreateNode4<std::string>(key, 1, "456");
  EXPECT_EQ(std::string("456"), node->GetValue<std::string>());
  EXPECT_EQ(key, node->GetKey<std::string>());
  EXPECT_EQ(node->IsLeaf(), false);
  EXPECT_EQ(node->HasValue(), true);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(std::string("456"), vDB::ArtNodeHelper::GetValue<std::string>(node_key_ptr));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode4(key, 1);
  EXPECT_EQ(key, node->GetKey<std::string>());
  EXPECT_EQ(node->IsLeaf(), false);
  EXPECT_EQ(node->HasValue(), false);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(NodeCreateTest, Node16CreateTest) {
  std::string key("123");
  auto *node = vDB::ArtNodeFactory::CreateNode16<std::string>(key, 1, "456");
  EXPECT_EQ(std::string("456"), node->GetValue<std::string>());
  EXPECT_EQ(key, node->GetKey<std::string>());
  EXPECT_EQ(node->IsLeaf(), false);
  EXPECT_EQ(node->HasValue(), true);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode16(key, 1);
  EXPECT_EQ(key, node->GetKey<std::string>());
  EXPECT_EQ(node->IsLeaf(), false);
  EXPECT_EQ(node->HasValue(), false);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(NodeCreateTest, Node48CreateTest) {
  std::string key("123");
  auto *node = vDB::ArtNodeFactory::CreateNode48<std::string>(key, 1, "456");
  EXPECT_EQ(std::string("456"), node->GetValue<std::string>());
  EXPECT_EQ(key, node->GetKey<std::string>());
  EXPECT_EQ(node->IsLeaf(), false);
  EXPECT_EQ(node->HasValue(), true);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode48(key, 1);
  EXPECT_EQ(key, node->GetKey<std::string>());
  EXPECT_EQ(node->IsLeaf(), false);
  EXPECT_EQ(node->HasValue(), false);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(NodeCreateTest, Node256CreateTest) {
  std::string key("123");
  auto *node = vDB::ArtNodeFactory::CreateNode256<std::string>(key, 1, "456");
  EXPECT_EQ(std::string("456"), node->GetValue<std::string>());
  EXPECT_EQ(key, node->GetKey<std::string>());
  EXPECT_EQ(node->IsLeaf(), false);
  EXPECT_EQ(node->HasValue(), true);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode256(key, 1);
  EXPECT_EQ(key, node->GetKey<std::string>());
  EXPECT_EQ(node->IsLeaf(), false);
  EXPECT_EQ(node->HasValue(), false);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(NodeCreateTest, LeafNodeCreateTest) {
  std::string key("123");
  auto *node = vDB::ArtNodeFactory::CreateLeafNode<std::string>(key, 0, "456");
  EXPECT_EQ(std::string("456"), node->GetValue<std::string>());
  EXPECT_EQ(key, node->GetKey<std::string>());
  EXPECT_EQ(node->IsLeaf(), true);
  EXPECT_EQ(node->HasValue(), true);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(CheckSamePrefixLengthTest, Node4Test) {
  std::string key1("123");
  auto *node = vDB::ArtNodeFactory::CreateNode4<std::string>(key1, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "12"), 2);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "1234"), 3);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "456"), 0);
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode4(key1, 0);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "12"), 2);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "1234"), 3);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "456"), 0);
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(CheckSamePrefixLengthTest, Node16Test) {
  std::string key1("123");
  auto *node = vDB::ArtNodeFactory::CreateNode16<std::string>(key1, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "12"), 2);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "1234"), 3);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "456"), 0);
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode16(key1, 0);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "12"), 2);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "1234"), 3);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "456"), 0);
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(CheckSamePrefixLengthTest, Node48Test) {
  std::string key1("123");
  auto *node = vDB::ArtNodeFactory::CreateNode48<std::string>(key1, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "12"), 2);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "1234"), 3);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "456"), 0);
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode48(key1, 0);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "12"), 2);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "1234"), 3);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "456"), 0);
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(CheckSamePrefixLengthTest, Node256Test) {
  std::string key1("123");
  auto *node = vDB::ArtNodeFactory::CreateNode256<std::string>(key1, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "12"), 2);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "1234"), 3);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "456"), 0);
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode256(key1, 0);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "12"), 2);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "1234"), 3);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "456"), 0);
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(CheckSamePrefixLengthTest, LeafNodeTest) {
  std::string key1("123");
  auto *node = vDB::ArtNodeFactory::CreateLeafNode<std::string>(key1, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "12"), 2);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "1234"), 3);
  EXPECT_EQ(vDB::ArtNodeHelper::CheckSamePrefixLength(node, node_key_ptr, "456"), 0);
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(RemovePrefixTest, Node4Test) {
  std::string key1("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode4<std::string>(key1, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode4 *>(vDB::ArtNodeHelper::RemoveKeyPrefix<std::string>(node, node_key_ptr, 3));
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(node->GetKey<std::string>(), "4567");
  EXPECT_EQ(node->GetValue<std::string>(), "456");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode4(key1, 0);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode4 *>(vDB::ArtNodeHelper::RemoveKeyPrefix<std::string>(node, node_key_ptr, 3));
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(node->GetKey<std::string>(), "4567");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(RemovePrefixTest, Node16Test) {
  std::string key1("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode16<std::string>(key1, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode16 *>(vDB::ArtNodeHelper::RemoveKeyPrefix<std::string>(node, node_key_ptr, 3));
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(node->GetKey<std::string>(), "4567");
  EXPECT_EQ(node->GetValue<std::string>(), "456");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode16(key1, 0);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode16 *>(vDB::ArtNodeHelper::RemoveKeyPrefix<std::string>(node, node_key_ptr, 3));
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(node->GetKey<std::string>(), "4567");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(RemovePrefixTest, Node48Test) {
  std::string key1("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode48<std::string>(key1, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode48 *>(vDB::ArtNodeHelper::RemoveKeyPrefix<std::string>(node, node_key_ptr, 3));
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(node->GetKey<std::string>(), "4567");
  EXPECT_EQ(node->GetValue<std::string>(), "456");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode48(key1, 0);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode48 *>(vDB::ArtNodeHelper::RemoveKeyPrefix<std::string>(node, node_key_ptr, 3));
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(node->GetKey<std::string>(), "4567");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(RemovePrefixTest, Node256Test) {
  std::string key1("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode256<std::string>(key1, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode256 *>(vDB::ArtNodeHelper::RemoveKeyPrefix<std::string>(node, node_key_ptr, 3));
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(node->GetKey<std::string>(), "4567");
  EXPECT_EQ(node->GetValue<std::string>(), "456");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode256(key1, 0);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode256 *>(vDB::ArtNodeHelper::RemoveKeyPrefix<std::string>(node, node_key_ptr, 3));
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(node->GetKey<std::string>(), "4567");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(RemovePrefixTest, LeafNodeTest) {
  std::string key1("1234567");
  auto *node = vDB::ArtNodeFactory::CreateLeafNode<std::string>(key1, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtLeafNode *>(vDB::ArtNodeHelper::RemoveKeyPrefix<std::string>(node, node_key_ptr, 3));
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  EXPECT_EQ(node->GetKey<std::string>(), "4567");
  EXPECT_EQ(node->GetValue<std::string>(), "456");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(CopyNewNodeTest, Node4Test) {
  std::string key1("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode4(key1, 0);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  auto *new_node = reinterpret_cast<vDB::ArtNode4 *>(
      vDB::ArtNodeHelper::CopyNewNodeWithValue<std::string>(node, node_key_ptr, "456"));
  EXPECT_EQ(new_node->GetKey<std::string>(), "1234567");
  EXPECT_EQ(new_node->GetValue<std::string>(), "456");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
  auto *new_node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(new_node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(new_node, new_node_key_ptr);
}

TEST(CopyNewNodeTest, Node16Test) {
  std::string key1("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode16(key1, 0);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  auto *new_node = reinterpret_cast<vDB::ArtNode16 *>(
      vDB::ArtNodeHelper::CopyNewNodeWithValue<std::string>(node, node_key_ptr, "456"));
  EXPECT_EQ(new_node->GetKey<std::string>(), "1234567");
  EXPECT_EQ(new_node->GetValue<std::string>(), "456");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
  auto *new_node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(new_node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(new_node, new_node_key_ptr);
}

TEST(CopyNewNodeTest, Node48Test) {
  std::string key1("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode48(key1, 0);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  auto *new_node = reinterpret_cast<vDB::ArtNode48 *>(
      vDB::ArtNodeHelper::CopyNewNodeWithValue<std::string>(node, node_key_ptr, "456"));
  EXPECT_EQ(new_node->GetKey<std::string>(), "1234567");
  EXPECT_EQ(new_node->GetValue<std::string>(), "456");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
  auto *new_node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(new_node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(new_node, new_node_key_ptr);
}

TEST(CopyNewNodeTest, Node256Test) {
  std::string key1("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode256(key1, 0);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  auto *new_node = reinterpret_cast<vDB::ArtNode256 *>(
      vDB::ArtNodeHelper::CopyNewNodeWithValue<std::string>(node, node_key_ptr, "456"));
  EXPECT_EQ(new_node->GetKey<std::string>(), "1234567");
  EXPECT_EQ(new_node->GetValue<std::string>(), "456");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
  auto *new_node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(new_node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(new_node, new_node_key_ptr);
}

TEST(AddAndFindChildTest, Node4FatherTest) {
  std::string key1("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode4(key1, 0);
  auto *child1 = vDB::ArtNodeFactory::CreateNode4(key1, 0);
  auto *child1_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(child1, sizeof(std::string));
  auto *child2 = vDB::ArtNodeFactory::CreateNode16(key1, 0);
  auto *child2_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(child1, sizeof(std::string));
  vDB::ArtNodeHelper::AddChild<std::string>(node, 'a', child1);
  vDB::ArtNodeHelper::AddChild<std::string>(node, 'b', child2);
  vDB::ArtNode **next_node1;
  vDB::ArtNode **next_node2;
  vDB::ArtNode **next_node3;
  vDB::ArtNode **next_node4;
  vDB::ArtNode **next_node5;
  vDB::ArtNode **next_node6;
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, 'a', next_node1), true);
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, 'b', next_node2), true);
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, 'c', next_node3), false);
  EXPECT_EQ(*next_node1, child1);
  EXPECT_EQ(*next_node2, child2);
  auto *child3 = vDB::ArtNodeFactory::CreateNode48(key1, 0);
  auto *child3_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(child3, sizeof(std::string));
  auto *child4 = vDB::ArtNodeFactory::CreateNode256(key1, 0);
  auto *child4_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(child4, sizeof(std::string));
  auto *child5 = vDB::ArtNodeFactory::CreateNode4(key1, 0);
  auto *child5_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(child5, sizeof(std::string));
  vDB::ArtNode *new_father = vDB::ArtNodeHelper::AddChild<std::string>(node, 'c', child3);
  new_father = vDB::ArtNodeHelper::AddChild<std::string>(new_father, 'd', child4);
  new_father = vDB::ArtNodeHelper::AddChild<std::string>(new_father, 'e', child5);
  auto *new_father_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(new_father, sizeof(std::string));
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(new_father, 'c', next_node3), true);
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(new_father, 'd', next_node4), true);
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(new_father, 'e', next_node5), true);
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(new_father, 'f', next_node6), false);
  EXPECT_EQ(*next_node3, child3);
  EXPECT_EQ(*next_node4, child4);
  EXPECT_EQ(*next_node5, child5);
  vDB::ArtNodeHelper::DestroyNode<std::string>(new_father, new_father_key_ptr);
  vDB::ArtNodeHelper::DestroyNode<std::string>(child1, child1_key_ptr);
  vDB::ArtNodeHelper::DestroyNode<std::string>(child2, child2_key_ptr);
  vDB::ArtNodeHelper::DestroyNode<std::string>(child3, child3_key_ptr);
  vDB::ArtNodeHelper::DestroyNode<std::string>(child4, child4_key_ptr);
  vDB::ArtNodeHelper::DestroyNode<std::string>(child5, child5_key_ptr);
}

TEST(AddAndFindChildTest, Node16FatherTest) {
  std::string key1("1234567");
  auto *node = reinterpret_cast<vDB::ArtNode *>(vDB::ArtNodeFactory::CreateNode16(key1, 0));

  vDB::ArtNode *childs[17];
  for (int i = 0; i < 10; i++) {
    childs[i] = vDB::ArtNodeFactory::CreateNode4(key1, 0);
    node = vDB::ArtNodeHelper::AddChild<std::string>(node, i, childs[i]);
  }
  vDB::ArtNode **next_node;
  for (int i = 0; i < 10; i++) {
    ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, i, next_node), true);
    EXPECT_EQ(*next_node, childs[i]);
  }
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, 10, next_node), false);
  for (int i = 10; i < 17; i++) {
    childs[i] = vDB::ArtNodeFactory::CreateNode4(key1, 0);
    node = vDB::ArtNodeHelper::AddChild<std::string>(node, i, childs[i]);
  }
  for (int i = 10; i < 17; i++) {
    ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, i, next_node), true);
    EXPECT_EQ(*next_node, childs[i]);
  }
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, 17, next_node), false);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
  for (auto &child : childs) {
    auto *child_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(child, sizeof(std::string));
    vDB::ArtNodeHelper::DestroyNode<std::string>(child, child_key_ptr);
  }
}

TEST(AddAndFindChildTest, Node48FatherTest) {
  std::string key1("1234567");
  auto *node = reinterpret_cast<vDB::ArtNode *>(vDB::ArtNodeFactory::CreateNode48<std::string>(key1, 0, "456"));

  vDB::ArtNode *childs[49];
  for (int i = 0; i < 40; i++) {
    childs[i] = vDB::ArtNodeFactory::CreateNode4(key1, 0);
    node = vDB::ArtNodeHelper::AddChild<std::string>(node, i, childs[i]);
  }
  vDB::ArtNode **next_node;
  for (int i = 0; i < 40; i++) {
    ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, i, next_node), true);
    EXPECT_EQ(*next_node, childs[i]);
  }
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, 40, next_node), false);
  for (int i = 40; i < 49; i++) {
    childs[i] = vDB::ArtNodeFactory::CreateNode4(key1, 0);
    node = vDB::ArtNodeHelper::AddChild<std::string>(node, i, childs[i]);
  }
  for (int i = 40; i < 49; i++) {
    ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, i, next_node), true);
    EXPECT_EQ(*next_node, childs[i]);
  }
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, 49, next_node), false);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
  for (auto &child : childs) {
    auto *child_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(child, sizeof(std::string));
    vDB::ArtNodeHelper::DestroyNode<std::string>(child, child_key_ptr);
  }
}

TEST(AddAndFindChildTest, Node256FatherTest) {
  std::string key1("1234567");
  auto *node = reinterpret_cast<vDB::ArtNode *>(vDB::ArtNodeFactory::CreateNode256<std::string>(key1, 0, "456"));

  vDB::ArtNode *childs[257];
  for (int i = 0; i < 250; i++) {
    childs[i] = vDB::ArtNodeFactory::CreateNode4(key1, 0);
    node = vDB::ArtNodeHelper::AddChild<std::string>(node, i, childs[i]);
  }
  vDB::ArtNode **next_node;
  for (int i = 0; i < 250; i++) {
    ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, i, next_node), true);
    EXPECT_EQ(*next_node, childs[i]);
  }
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, -6, next_node), false);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
  for (int i = 0; i < 250; i++) {
    auto *child_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(childs[i], sizeof(std::string));
    vDB::ArtNodeHelper::DestroyNode<std::string>(childs[i], child_key_ptr);
  }
}

TEST(AddAndFindChildTest, LeafNodeFatherTest) {
  std::string key1("1234567");
  auto *node = reinterpret_cast<vDB::ArtNode *>(vDB::ArtNodeFactory::CreateLeafNode<std::string>(key1, 0, "456"));

  vDB::ArtNode *child;
  child = vDB::ArtNodeFactory::CreateNode4(key1, 0);
  node = vDB::ArtNodeHelper::AddChild<std::string>(node, 'a', child);
  vDB::ArtNode **next_node;
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, 'a', next_node), true);
  EXPECT_EQ(*next_node, child);
  ASSERT_EQ(vDB::ArtNodeHelper::FindChild(node, 'b', next_node), false);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
  auto *child_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(child, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(child, child_key_ptr);
}

TEST(AddPrefixTest, Node4Test) {
  std::string key("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode4<std::string>(key, 0, "456");
  node = reinterpret_cast<vDB::ArtNode4 *>(vDB::ArtNodeHelper::AddKeyPrefix<std::string>(node, "abc"));
  EXPECT_EQ(node->GetKey<std::string>(), "abc1234567");
  EXPECT_EQ(node->GetValue<std::string>(), "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode4(key, 0);
  node = reinterpret_cast<vDB::ArtNode4 *>(vDB::ArtNodeHelper::AddKeyPrefix<std::string>(node, "abc"));
  EXPECT_EQ(node->GetKey<std::string>(), "abc1234567");
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(AddPrefixTest, Node16Test) {
  std::string key("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode16<std::string>(key, 0, "456");
  node = reinterpret_cast<vDB::ArtNode16 *>(vDB::ArtNodeHelper::AddKeyPrefix<std::string>(node, "abc"));
  EXPECT_EQ(node->GetKey<std::string>(), "abc1234567");
  EXPECT_EQ(node->GetValue<std::string>(), "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode16(key, 0);
  node = reinterpret_cast<vDB::ArtNode16 *>(vDB::ArtNodeHelper::AddKeyPrefix<std::string>(node, "abc"));
  EXPECT_EQ(node->GetKey<std::string>(), "abc1234567");
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(AddPrefixTest, Node48Test) {
  std::string key("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode48<std::string>(key, 0, "456");
  node = reinterpret_cast<vDB::ArtNode48 *>(vDB::ArtNodeHelper::AddKeyPrefix<std::string>(node, "abc"));
  EXPECT_EQ(node->GetKey<std::string>(), "abc1234567");
  EXPECT_EQ(node->GetValue<std::string>(), "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode48(key, 0);
  node = reinterpret_cast<vDB::ArtNode48 *>(vDB::ArtNodeHelper::AddKeyPrefix<std::string>(node, "abc"));
  EXPECT_EQ(node->GetKey<std::string>(), "abc1234567");
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(AddPrefixTest, Node256Test) {
  std::string key("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode256<std::string>(key, 0, "456");
  node = reinterpret_cast<vDB::ArtNode256 *>(vDB::ArtNodeHelper::AddKeyPrefix<std::string>(node, "abc"));
  EXPECT_EQ(node->GetKey<std::string>(), "abc1234567");
  EXPECT_EQ(node->GetValue<std::string>(), "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode256(key, 0);
  node = reinterpret_cast<vDB::ArtNode256 *>(vDB::ArtNodeHelper::AddKeyPrefix<std::string>(node, "abc"));
  EXPECT_EQ(node->GetKey<std::string>(), "abc1234567");
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(AddPrefixTest, LeafNodeTest) {
  std::string key("1234567");
  auto *node = vDB::ArtNodeFactory::CreateLeafNode<std::string>(key, 0, "456");
  node = reinterpret_cast<vDB::ArtLeafNode *>(vDB::ArtNodeHelper::AddKeyPrefix<std::string>(node, "abc"));
  EXPECT_EQ(node->GetKey<std::string>(), "abc1234567");
  EXPECT_EQ(node->GetValue<std::string>(), "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(DeleteValueTest, Node4Test) {
  std::string key("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode4<std::string>(key, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode4 *>(vDB::ArtNodeHelper::DeleteValue<std::string>(node, node_key_ptr));
  EXPECT_EQ(node->GetKey<std::string>(), "1234567");
  EXPECT_EQ(node->HasValue(), false);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(DeleteValueTest, Node16Test) {
  std::string key("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode16<std::string>(key, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode16 *>(vDB::ArtNodeHelper::DeleteValue<std::string>(node, node_key_ptr));
  EXPECT_EQ(node->GetKey<std::string>(), "1234567");
  EXPECT_EQ(node->HasValue(), false);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(DeleteValueTest, Node48Test) {
  std::string key("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode48<std::string>(key, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode48 *>(vDB::ArtNodeHelper::DeleteValue<std::string>(node, node_key_ptr));
  EXPECT_EQ(node->GetKey<std::string>(), "1234567");
  EXPECT_EQ(node->HasValue(), false);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(DeleteValueTest, Node256Test) {
  std::string key("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode256<std::string>(key, 0, "456");
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode256 *>(vDB::ArtNodeHelper::DeleteValue<std::string>(node, node_key_ptr));
  EXPECT_EQ(node->GetKey<std::string>(), "1234567");
  EXPECT_EQ(node->HasValue(), false);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(PathCompressionTest, Node4Test) {
  std::string key("1234567");
  auto *node = vDB::ArtNodeFactory::CreateNode4<std::string>(key, 0, "456");
  auto *child = vDB::ArtNodeFactory::CreateNode4<std::string>(key, 0, "789");
  node = reinterpret_cast<vDB::ArtNode4 *>(vDB::ArtNodeHelper::AddChild<std::string>(node, 'a', child));
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode4 *>(vDB::ArtNodeHelper::PathCompression<std::string>(node, node_key_ptr));
  EXPECT_EQ(node->GetKey<std::string>(), "1234567a1234567");
  EXPECT_EQ(node->GetValue<std::string>(), "789");
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);

  node = vDB::ArtNodeFactory::CreateNode4(key, 0);
  child = vDB::ArtNodeFactory::CreateNode4(key, 0);
  node = reinterpret_cast<vDB::ArtNode4 *>(vDB::ArtNodeHelper::AddChild<std::string>(node, 'a', child));
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = reinterpret_cast<vDB::ArtNode4 *>(vDB::ArtNodeHelper::PathCompression<std::string>(node, node_key_ptr));
  EXPECT_EQ(node->GetKey<std::string>(), "1234567a1234567");
  EXPECT_EQ(node->HasValue(), false);
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(RemoveChildTest, Node4FatherTest) {
  std::string key("1234567");
  vDB::ArtNode *node = reinterpret_cast<vDB::ArtNode *>(vDB::ArtNodeFactory::CreateNode4<std::string>(key, 0, "456"));
  auto *child = vDB::ArtNodeFactory::CreateNode4<std::string>(key, 0, "789");
  node = vDB::ArtNodeHelper::AddChild<std::string>(node, 'a', child);
  node = vDB::ArtNodeHelper::AddChild<std::string>(node, 'b', child);
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 'a');
  vDB::ArtNode **next_node;
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 'a', next_node), false);
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 'b', next_node), true);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 'b');
  EXPECT_EQ(node->IsLeaf(), true);
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 'a', next_node), false);
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 'b', next_node), false);
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(RemoveChildTest, Node16FatherTest) {
  std::string key("1234567");
  vDB::ArtNode *node = reinterpret_cast<vDB::ArtNode *>(vDB::ArtNodeFactory::CreateNode16<std::string>(key, 0, "456"));
  std::array<vDB::ArtNode4 *, 8> childs;
  for (int i = 0; i < 8; i++) {
    childs[i] = vDB::ArtNodeFactory::CreateNode4<std::string>(key, 0, "789");
    node = vDB::ArtNodeHelper::AddChild<std::string>(node, i, childs[i]);
  }
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 7);
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 6);
  vDB::ArtNode **next_node;
  for (int i = 0; i < 6; i++) {
    EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, i, next_node), true);
  }
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 7, next_node), false);
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 6, next_node), false);
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 5);
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 4);
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 5, next_node), false);
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 4, next_node), false);
  for (int i = 0; i < 4; i++) {
    EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, i, next_node), true);
  }
  for (int i = 0; i < 8; i++) {
    auto *child_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(childs[i], sizeof(std::string));
    vDB::ArtNodeHelper::DestroyNode<std::string>(childs[i], child_key_ptr);
  }
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(RemoveChildTest, Node48FatherTest) {
  std::string key("1234567");
  vDB::ArtNode *node = reinterpret_cast<vDB::ArtNode *>(vDB::ArtNodeFactory::CreateNode48<std::string>(key, 0, "456"));
  std::array<vDB::ArtNode4 *, 20> childs;
  for (int i = 0; i < 20; i++) {
    childs[i] = vDB::ArtNodeFactory::CreateNode4<std::string>(key, 0, "789");
    node = vDB::ArtNodeHelper::AddChild<std::string>(node, i, childs[i]);
  }
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 19);
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 18);
  vDB::ArtNode **next_node;
  for (int i = 0; i < 18; i++) {
    EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, i, next_node), true);
  }
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 18, next_node), false);
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 19, next_node), false);
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 17);
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 16);
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 17, next_node), false);
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 16, next_node), false);
  for (int i = 0; i < 16; i++) {
    EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, i, next_node), true);
  }
  for (int i = 0; i < 20; i++) {
    auto *child_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(childs[i], sizeof(std::string));
    vDB::ArtNodeHelper::DestroyNode<std::string>(childs[i], child_key_ptr);
  }
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

TEST(RemoveChildTest, Node256FatherTest) {
  std::string key("1234567");
  vDB::ArtNode *node = reinterpret_cast<vDB::ArtNode *>(vDB::ArtNodeFactory::CreateNode256<std::string>(key, 0, "456"));
  std::array<vDB::ArtNode4 *, 50> childs;
  for (int i = 0; i < 50; i++) {
    childs[i] = vDB::ArtNodeFactory::CreateNode4<std::string>(key, 0, "789");
    node = vDB::ArtNodeHelper::AddChild<std::string>(node, i, childs[i]);
  }
  auto *node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 49);
  vDB::ArtNode **next_node;
  for (int i = 0; i < 49; i++) {
    EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, i, next_node), true);
  }
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 49, next_node), false);
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 48);
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 47);
  node = vDB::ArtNodeHelper::RemoveChild<std::string>(node, node_key_ptr, 46);
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 48, next_node), false);
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 47, next_node), false);
  EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, 46, next_node), false);
  for (int i = 0; i < 46; i++) {
    EXPECT_EQ(vDB::ArtNodeHelper::FindChild(node, i, next_node), true);
  }
  for (int i = 0; i < 50; i++) {
    auto *child_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(childs[i], sizeof(std::string));
    vDB::ArtNodeHelper::DestroyNode<std::string>(childs[i], child_key_ptr);
  }
  node_key_ptr = vDB::ArtNodeHelper::GetKeyPtr(node, sizeof(std::string));
  vDB::ArtNodeHelper::DestroyNode<std::string>(node, node_key_ptr);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}