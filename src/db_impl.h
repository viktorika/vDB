/*
 * @Author: victorika
 * @Date: 2024-05-15 16:23:26
 * @Last Modified by: victorika
 * @Last Modified time: 2024-05-20 15:35:58
 */
#pragma once

#include <string>
#include "db.h"
#include "memtable/vrt.h"
#include "status.h"

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
  Vrt<std::string, false, 100> vrt_;  // TODO需要把第三个参数写进编译配置里
};

}  // namespace vdb