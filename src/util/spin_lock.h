/*
 * @Author: viktorika
 * @Date: 2024-04-05 19:01:42
 * @Last Modified by:   viktorika
 * @Last Modified time: 2024-04-05 19:01:42
 */
#pragma once

#include <unistd.h>
#include <atomic>
#include <cstdint>
#include "comm.h"

namespace vdb {

class Sleeper {
  static const uint32_t kMaxActiveSpin = 4000;
  uint32_t spin_count_;

 public:
  Sleeper() noexcept : spin_count_(0) {}

  static void sleep() noexcept {
    struct timespec ts = {0, 500000};
    nanosleep(&ts, nullptr);
  }

  void wait() noexcept {
    if (spin_count_ < kMaxActiveSpin) {
      ++spin_count_;
#if defined(__x86_64__) || defined(__i386__)
      asm volatile("pause");
#elif defined(__aarch64__) || defined(__arm__)
      asm volatile("yield");
#else
#  error "Unsupported architecture"
#endif
    } else {
      sleep();
    }
  }
};

class SpinLock {
 public:
  SpinLock() : lock_(kFree) {};
  void operator=(const SpinLock&) = delete;
  SpinLock(const SpinLock&) = delete;

  bool try_lock() noexcept { return cas(kFree, kLocked); }

  void lock() noexcept {
    Sleeper sleeper;
    while (!try_lock()) {
      do {
        sleeper.wait();
      } while (lock_.load(std::memory_order_relaxed) == kLocked);
    }
  }

  void unlock() noexcept { lock_.store(kFree, std::memory_order_release); }

 private:
  std::atomic<uint8_t> lock_;

  bool cas(uint8_t compare, uint8_t new_val) noexcept {
    return lock_.compare_exchange_strong(compare, new_val, std::memory_order_acquire, std::memory_order_relaxed);
  }
};

}  // namespace vdb
