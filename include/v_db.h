#pragma once

#include <string>
#include <sys/uio.h>
#include <functional>

namespace vDB {

/*
 * db_store的合法标志
 * INSERT插入
 * REPLACE替换
 * STORE替换或插入
 */
enum DB_STORE_FLAG{STORE_MIN_FLAG, DB_INSERT, DB_REPLACE, DB_STORE, STORE_MAX_FLAG};

const int kIndex_min = 6;     //index长度最小为6，key，sep，start，sep，length，\n
const int kIndex_max = 1024;  //index最大长度，这个可以自己调节
const int kData_min = 2;      //data的最小长度为2，一个字节加一个换行符
const int kData_max = 1024;   //data的最大长度，可以自己调节

using std::string;

typedef unsigned int DBHASH;       //hash值类型

/*
 * 一个key->value数据库，数据库打开后会生成两个文件.idx和.dat文件
 * .idx存储key和其他相关的信息，.dat存储真正的数据
 * key和value均为string类型
 * 注意，里面有许多lseek操作，所以对同一个对象来说这些接口都是不可重入的
 * key不允许带:符号
 */
class DB {
public:
	explicit DB();
	DB(const DB&) = delete;
	virtual ~DB();
	/*
	 * 打开或者创建数据库，参数与open系统调用一致
	 * 成功返回True失败返回False
	 */
	virtual bool db_open(const string&, int, ...);
	/*
	 * 放弃对数据库的访问
	 */
	virtual void db_close();
	/*
	 * 用key获取对应的value，不存在则返回空string
	 */
	virtual string db_fetch(const string&);
	/*
	 * 删除指定key的记录
	 * 成功返回true失败返回false
	 */
	virtual bool db_delete(const string&);
	/*
	 * 把记录存储到数据库中
	 * 第一个参数是key，第二个参数是value，第三个是操作的标志
	 * 成功返回0，错误返回-1，如果存在而且指定了DB_INSERT则返回1
	 */
	virtual int db_store(const string&, const string&, int);
private:
	string pathname_;          //数据库路径
	//最后一次读写index记录时的前一个节点和后一个节点的偏移量
	off_t pre_offset_, next_offset_;
	//db_store不同flag对应的映射函数
	std::function<int(const string&, const string&, bool, off_t)> store_function_map[STORE_MAX_FLAG];

	struct Handle {
		int fd;                //文件描述符
		off_t offset;          //最后一次读写记录时的偏移量
		int length;            //最后一次读写的记录长度
		char *buffer;          //读写记录时用的缓冲区
	}index_, data_;            //idx文件和dat文件

	void _db_bind_function();
	bool _db_allocate();
	void _db_free();
	DBHASH _db_hash(const string&);
	bool _db_find(const string&, off_t);
	off_t _db_read_ptr(off_t);
	off_t _db_read_idx(off_t);
	char *_db_read_data();
	bool _db_do_delete();
	bool _db_write_data(const char*, off_t, int);
	bool _db_lock_and_write_data(const char*, off_t, int);
	bool _db_write_idx(const char*, off_t, int, off_t);
	bool _db_lock_and_write_idx(const char*, off_t, int, off_t);
	bool _db_pre_write_idx(const char*, off_t, struct iovec*, char*);
	bool _db_do_write_idx(off_t, int, struct iovec*);
	bool _db_write_ptr(off_t, off_t);
	int _db_store_insert(const string&, const string&, bool, off_t);
	int _db_store_replace(const string&, const string&, bool, off_t);
	int _db_store_ins_or_rep(const string&, const string&, bool, off_t);
	bool _db_find_and_delete_free(int, int);
};

}
