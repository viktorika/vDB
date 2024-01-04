#include <iostream>
#include <string>
#include "art_node.h"

int main() {
  std::string key("123");
  auto *node = vDB::ArtNodeFactory::CreateNode16<std::string>(key, 1, "456");
  auto result = node->GetValue<std::string>();
  std::cout << result << std::endl;
  std::cout << "artnode size=" << sizeof(vDB::ArtNode) << std::endl;
  std::cout << "Node256size=" << sizeof(vDB::ArtNode256) << std::endl;
  return 0;
}