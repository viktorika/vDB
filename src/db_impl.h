/*
 * @Author: victorika
 * @Date: 2024-05-15 16:23:26
 * @Last Modified by: victorika
 * @Last Modified time: 2024-05-15 16:59:29
 */
#pragma once

#include "db.h"

namespace vdb {

class DBImpl : public DB {
 public:
  DBImpl(const Options& options, const std::string_view name);
  DBImpl(const DBImpl&) = delete;
  DBImpl& operator=(const DBImpl&) = delete;
  ~DBImpl() override;

  Status Put(const WriteOptions& options, const std::string_view key, const std::string& value) override;

 private:
  std::string db_name_;
  Vrt<std::string, false, 1> vrt;
};

}  // namespace vdb