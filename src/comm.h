/*
 * @Author: viktorika
 * @Date: 2024-04-05 19:01:40
 * @Last Modified by: victorika
 * @Last Modified time: 2024-05-15 17:06:25
 */
#pragma once

#include <cstddef>
#include <cstdint>

namespace vdb {

#ifdef __GNUC__
#  define likely(x) __builtin_expect(!!(x), 1)
#  define unlikely(x) __builtin_expect(!!(x), 0)
#else
#  define likely(x) (x)
#  define unlikely(x) (x)
#endif

constexpr size_t kFour = 4;
constexpr size_t kFive = 5;
constexpr size_t kTen = 10;
constexpr size_t kSixteen = 16;
constexpr size_t kSeventeen = 17;
constexpr size_t kFortyEight = 48;
constexpr size_t kFortyNight = 49;
constexpr size_t kTwoFiveSix = 256;

constexpr size_t kNodeChildMaxCnt = 256;
constexpr size_t kMaxKeySize = 1 << 20;

constexpr uint8_t kFree = 0;
constexpr uint8_t kLocked = 1;

}  // namespace vdb