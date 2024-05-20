/*
 * @Author: victorika
 * @Date: 2024-05-15 16:00:54
 * @Last Modified by: victorika
 * @Last Modified time: 2024-05-15 16:50:56
 */
#pragma once

#include <memory>
#include "options.h"
#include "status.h"

namespace vdb {

// A DB is a persistent ordered map from keys to values.
// A DB is safe for concurrent access from multiple threads without
// any external synchronization.
class DB {
 public:
  static Status Open(const Options& options, const std::string_view name, std::unique_ptr<DB>* dbptr);

  DB() = default;

  DB(const DB&) = delete;
  DB& operator=(const DB&) = delete;

  virtual ~DB();

  virtual Status Put(const WriteOptions& options, const std::string_view key, const std::string& value) = 0;
};

}  // namespace vdb