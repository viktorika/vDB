/*
 * @Author: victorika
 * @Date: 2024-05-15 17:06:22
 * @Last Modified by:   victorika
 * @Last Modified time: 2024-05-15 17:06:22
 */
#include "db_impl.h"
#include "memtable/vrt.h"

namespace vdb {

Status DB::Open(const Options& options, const std::string_view name, std::unique_ptr<DB>* dbptr) {
  *dbptr = std::make_unique<DBImpl>(options, name);
  // TODO 数据复原等操作
  return Status::OK();
}

DBImpl::DBImpl(const Options& options, const std::string_view name) : db_name_(name) {}

DBImpl::~DBImpl() {}

Status DBImpl::Put(const WriteOptions& options, const std::string_view key, const std::string& value) {
  Vrt<std::string, false, 1> vrt;
  return Status::OK();
}

}  // namespace vdb