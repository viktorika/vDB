#include "art.h"

#include <cstdlib>
#include <cwchar>
#include <string_view>

namespace VDB {

inline __attribute((always_inline)) ArtNode *FindChild(ArtNode *node, char next_character) {
  switch (node->type) {
    case Node4: {
      auto *node4 = reinterpret_cast<ArtNode4*>(node);
      for (int i = 0; i < node4->base.child_cnt; i++) {
        if (node4->keys[i] == next_character) {
          return node4->childs[i];
        }
      }
    } break;
    case Node16: {
      // TODO simd优化
      auto *node16 = reinterpret_cast<ArtNode16*>(node);
      for (int i = 0; i < node16->base.child_cnt; i++) {
        if (node16->keys[i] == next_character) {
          return node16->childs[i];
        }
      }
    } break;
    case Node48: {
    } break;
    case Node256: {
    } break;
    case LeafNode: {
    } break;
    default:
      return nullptr;
  }
}

}  // namespace VDB