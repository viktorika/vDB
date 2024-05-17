/*
 * @Author: viktorika
 * @Date: 2024-04-05 19:01:45
 * @Last Modified by: victorika
 * @Last Modified time: 2024-04-26 11:15:51
 */
#pragma once

#include <array>
#include <atomic>
#include <mutex>
#include <numeric>
#include <vector>

namespace vdb {

constexpr uint8_t kEpochSize = 3;
constexpr uint8_t kCacheLineSize = 64;

template <uint32_t kReadThreadNum>
class ThreadIDManager;

template <uint32_t kReadThreadNum>
struct ThreadID {
  ThreadID() { tid = ThreadIDManager<kReadThreadNum>::GetInstance().AcquireThreadID(); }
  ~ThreadID() { ThreadIDManager<kReadThreadNum>::GetInstance().ReleaseThreadID(tid); }
  uint32_t tid;
};

template <uint32_t kReadThreadNum>
class ThreadIDManager {
 public:
  ThreadIDManager() : tid_list_(kReadThreadNum) { std::iota(tid_list_.begin(), tid_list_.end(), 1); }
  ThreadIDManager(const ThreadIDManager &) = delete;
  ThreadIDManager(ThreadIDManager &&) = delete;
  ThreadIDManager &operator=(const ThreadIDManager &) = delete;
  ~ThreadIDManager() = default;

  static ThreadIDManager &GetInstance() {
    static ThreadIDManager inst;
    return inst;
  }

  uint32_t AcquireThreadID() {
    std::lock_guard<std::mutex> lock(tid_list_mutex_);
    auto tid = tid_list_.back();
    tid_list_.pop_back();
    return tid;
  }

  void ReleaseThreadID(const uint32_t tid) {
    std::lock_guard<std::mutex> lock(tid_list_mutex_);
    tid_list_.emplace_back(tid);
  }

 private:
  std::vector<uint32_t> tid_list_;
  std::mutex tid_list_mutex_;
};

struct TLS {
  TLS() : active(false), epoch(0) {}
  TLS(TLS &) = delete;
  TLS(TLS &&) = delete;
  void operator=(const TLS &) = delete;
  ~TLS() = default;
  std::atomic_flag active;
  std::atomic<uint8_t> epoch;
} __attribute__((aligned(kCacheLineSize)));

template <class RCObject, class DestroyClass, uint32_t kReadThreadNum>
class EbrManager {
 public:
  EbrManager() : tls_list_(), global_epoch_(0), update_(false), write_cnt_(0) {
    for (int i = 0; i < kEpochSize; i++) {
      retire_list_[i].store(nullptr, std::memory_order_release);
    }
  }
  EbrManager(const EbrManager<RCObject, DestroyClass, kReadThreadNum> &) = delete;
  EbrManager(EbrManager<RCObject, DestroyClass, kReadThreadNum> &&) = delete;
  EbrManager &operator=(const EbrManager<RCObject, DestroyClass, kReadThreadNum> &) = delete;
  ~EbrManager() { ClearAllRetireList(); }

  void ClearAllRetireList() {
    for (int i = 0; i < kEpochSize; i++) {
      ClearRetireList(i);
    }
  }

  inline void StartRead() {
    auto &tls = GetTLS();
    tls.active.test_and_set(std::memory_order_release);
    tls.epoch.store(global_epoch_.load(std::memory_order_acquire), std::memory_order_release);
  }

  inline void EndRead() { GetTLS().active.clear(std::memory_order_release); }

  inline void FreeObject(RCObject *object) {
    auto epoch = global_epoch_.load(std::memory_order_acquire);
    auto *node = new RetireNode;
    node->obj = object;
    do {
      node->next = retire_list_[epoch].load(std::memory_order_acquire);
    } while (!retire_list_[epoch].compare_exchange_weak(node->next, node, std::memory_order_acq_rel));

    auto write_cnt = write_cnt_.fetch_add(1, std::memory_order_relaxed);
    if (write_cnt > kReadThreadNum) {
      if (!update_.test_and_set(std::memory_order_acq_rel)) {
        TryGC();
        update_.clear(std::memory_order_release);
      }
    }
  }

 private:
  inline TLS &GetTLS() {
    thread_local ThreadID<kReadThreadNum> thread_id;
    return tls_list_[thread_id.tid];
  }

  inline void TryGC() {
    auto epoch = global_epoch_.load(std::memory_order_acquire);
    // TODO 记录上一次搜索到的位置
    for (int i = 0; i < tls_list_.size(); i++) {
      if (tls_list_[i].active.test(std::memory_order::memory_order_acquire) &&
          tls_list_[i].epoch.load(std::memory_order::memory_order_acquire) != epoch) {
        return;
      }
    }
    global_epoch_.store((epoch + 1) % kEpochSize, std::memory_order_release);
    ClearRetireList((epoch + 2) % kEpochSize);
    write_cnt_.store(0, std::memory_order_relaxed);
  }

  inline void ClearRetireList(int index) {
    auto *retire_node = retire_list_[index].load(std::memory_order_acquire);
    while (retire_node != nullptr) {
      DestroyClass destroy(retire_node->obj);
      auto *old_node = retire_node;
      retire_node = retire_node->next;
      delete old_node;
    }
    retire_list_[index].store(nullptr, std::memory_order_release);
  }

  struct RetireNode {
    RCObject *obj;
    RetireNode *next;
  };
  std::array<char, kCacheLineSize> start_padding_;
  std::array<TLS, kReadThreadNum> tls_list_;
  std::atomic<uint8_t> global_epoch_;
  std::array<char, kCacheLineSize> mid_padding_;
  std::atomic_flag update_;
  std::atomic<uint32_t> write_cnt_;
  std::atomic<RetireNode *> retire_list_[kEpochSize];
  std::array<char, kCacheLineSize> end_padding_;
};

}  // namespace vdb