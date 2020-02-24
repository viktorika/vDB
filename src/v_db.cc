#include "../include/v_db.h"
#include "../include/record_lock.h"

#include <cstring>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/stat.h>

const int kPtr_size = 7;                 //idx文件中的ptr结构的大小
const off_t kPtr_max = 9999999;          //ptr的最大值，7位
const int kHash_table_size = 137;        //默认的hash表大小
const off_t kHash_offset = kPtr_size;    //idx文件中hash表的偏移量
const int kHash_multipy_factor = 31;     //计算hash值时的累乘因子
const int kIndex_length_size = 4;        //存储index记录长度的字节数
const off_t kFree_offset = 0;            //空闲链表偏移量

const char kNew_line = '\n';             //换行符
const char kSeparate = ':';              //分隔符
const char kSpace = ' ';                 //空格符

namespace vDB {

DB::DB() {
	//初始化映射函数
	_db_bind_function();
}
DB::~DB() {
	/*
	 * 离开作用域会释放资源
	 * 最好还是自己调用close吧
	 */
	_db_free();
}

bool DB::db_open(const string &pathname, int oflag, ...) {
	//检查fpathname，不允许为空
	if (!pathname.length()){
		printf("db_open: pathname can not be blank\n");
		return false;
	}
	//分配资源
	if (!_db_allocate())
		return false;
	/*
	 * 初始化路径和fd
	 */
	pathname_ = pathname;
	index_.fd = data_.fd = -1;
	//处理oflag
	if (oflag & O_CREAT) {
		va_list ap;
		va_start(ap, oflag);
		int mode = va_arg(ap, int);
		va_end(ap);
		
		//打开idx文件和dat文件
		index_.fd = open((pathname_ + ".idx").c_str(), oflag, mode);
		data_.fd = open((pathname_ + ".dat").c_str(), oflag, mode);
	}
	else {
		//打开idx文件和dat文件
		index_.fd = open((pathname_ + ".idx").c_str(), oflag);
		data_.fd = open((pathname_ + ".dat").c_str(), oflag);
	}
	if (index_.fd < 0 || data_.fd < 0) {
		//打开失败
		_db_free();
		return false;
	}
	if ((oflag & (O_CREAT | O_TRUNC)) == (O_CREAT | O_TRUNC)) {
		/*
		 * 如果数据库是重新创建的，我们必须初始化它
		 * 写锁住整个文件，我们才能统计它并且进行初始化
		 * 检查它的大小，并且自动初始化它
		 */
		struct stat statbuff;
		char asciiptr[kPtr_size + 1], hash[(kHash_table_size + 1) * kPtr_size + 2];   //+2是为了null和换行符
		asciiptr[kPtr_size] = 0;
		RecordWritewLock writew_lock(index_.fd, 0, SEEK_SET, 0);
		if (fstat(index_.fd, &statbuff) < 0) {
			printf("db_open: fstat error\n");
			return false;
		}
		if (!statbuff.st_size) {
			/*
			 * 我们必须构建一个值为0的kHashtablesize + 1的链表
			 * + 1表示hash表之前的空闲列表指针
			 */
			sprintf(asciiptr, "%*d", kPtr_size, 0);
			hash[0] = 0;
			for (int i = 0; i < kHash_table_size + 1; i++)
				strcat(hash, asciiptr);
			strcat(hash, "\n");
			int size = strlen(hash);
			if (write(index_.fd, hash, size) != size) {
				printf("db_open: index file init write error\n");
				return false;
			}
		}
	}
	return true;
}

void DB::_db_bind_function() {
	store_function_map[DB_INSERT] = std::bind(&DB::_db_store_insert, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	store_function_map[DB_REPLACE] = std::bind(&DB::_db_store_replace, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	store_function_map[DB_STORE] = std::bind(&DB::_db_store_ins_or_rep, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
}

bool DB::_db_allocate() {
	if (!(index_.buffer = new char[kIndex_max + 2])) {
		printf("_db_allocate: malloc error for index buffer\n");
		goto allocate_fail;
	}
	if (!(data_.buffer = new char[kData_max + 2])) {
		printf("_db_allocate: malloc error for data buffer\n");
		goto allocate_fail;
	}
	return true;
allocate_fail:
	_db_free();
	return false;
}

/*
 * 释放资源
 */
void DB::_db_free() {
	if (index_.fd >= 0)
		close(index_.fd);
	if (data_.fd >= 0)
		close(data_.fd);
	if (index_.buffer)
		delete index_.buffer;
	if (data_.buffer)
		delete data_.buffer;
}

void DB::db_close() {
	_db_free();
}

string DB::db_fetch(const string &key) {
	off_t start_offset = _db_hash(key) * kPtr_size + kHash_offset;
	string value;
	//加个读锁，只锁第一个字节
	RecordReadwLock readw_lock(index_.fd, start_offset, SEEK_SET, 1);
	if (_db_find(key, start_offset)) 
		//查找成功
		value = _db_read_data();
	return value;
}

/*
 * 计算key的hash值
 */
DBHASH DB::_db_hash(const string &key) {
	DBHASH hash_value = 0;
	for (int i = 0; i < key.length(); ++i)
		//字符串hash
		hash_value = hash_value * kHash_multipy_factor + key[i];
	hash_value %= kHash_table_size;
	return hash_value;
}

/*
 * 查找是否存在这个key
 * 用的是hash表存储的结构，offset为对应的hash链表的起始偏移量，也就是查找的起点
 * 调用此函数前需要自行上锁
 * 成功返回true，失败返回false
 * 查找成功后相关的结果存储在index_和data_中
 */
bool DB::_db_find(const string& key, off_t offset) {
	pre_offset_ = offset;
	offset = _db_read_ptr(offset);
	while (offset) {
		off_t next_offset = _db_read_idx(offset);
		if (!strcmp(index_.buffer, key.c_str())) 
			//分隔符的第一个元素是key
			return true;
		pre_offset_ = offset;                  //记录最后一次read_idx的前一个节点
		offset = next_offset;
	}
	return false;
}

/*
 * 在.idx文件中的offset偏移量里读取一个kPtr_size的一个pointer（就是下一个节点的偏移量）
 * 返回0则是读取失败或者是尾节点了，成功则返回对应的偏移量
 */
off_t DB::_db_read_ptr(off_t offset) {
	char asciiptr[kPtr_size + 1];
	if (-1 == lseek(index_.fd, offset, SEEK_SET)) {
		printf("_db_read_ptr: lseek error to ptr field\n");
		return 0;
	}
	if (read(index_.fd, asciiptr, kPtr_size) != kPtr_size) {
		printf("_db_read_ptr: read error of ptr field\n");
		return 0;
	}
	asciiptr[kPtr_size] = 0;
	return atol(asciiptr);
}

/*
 * 把idx文件里offset里的节点信息读入Handle的结构里
 * 返回下一个index在idx文件里的偏移量，失败返回0
 */
off_t DB::_db_read_idx(off_t offset) {
	/*
	 * 定位idx文件并且记录偏移量
	 * 0 == offset表示从当前偏移量读取
	 */
	if (-1 == (index_.offset = lseek(index_.fd, offset, !offset ? SEEK_CUR : SEEK_SET))) {
		printf("_db_read_idx: leek error\n");
		return 0;
	}
	//分成两部分读取，第一部分是指向下一个节点的ptr，第二个部分是index记录的长度
	char asciiptr[kPtr_size + 1], ptr_length[kIndex_length_size + 1];
	struct iovec iov[2];
	iov[0].iov_base = asciiptr;
	iov[0].iov_len = kPtr_size;
	iov[1].iov_base = ptr_length;
	iov[1].iov_len = kIndex_length_size;
	size_t read_length;
	if ((read_length = readv(index_.fd, iov, 2)) != kPtr_size + kIndex_length_size) {
		/*
		 * SEEK_CUR和offset==0时可能会碰见EOF
		 */
		if (!read_length && !offset)
			return 0;
		printf("_db_read_idx: readv error of index record\n");
		return 0;
	}
	//终止符
	asciiptr[kPtr_size] = 0;
	ptr_length[kIndex_length_size] = 0;
	next_offset_ = atol(asciiptr); //记录最后一次read_idx的下一个节点
	index_.length = atoi(ptr_length);
	//长度条件判定
	if ((index_.length < kIndex_min || index_.length > kIndex_max)) {
		printf("_db_read_idx: index length =%d, index length not in range\n", index_.length);
		return 0;
	}
	//读取index记录
	if (read(index_.fd, index_.buffer, index_.length) != index_.length) {
		printf("_db_read_idx: read error of index record\n");
		return 0;
	}
	//完整性检查
	if (index_.buffer[index_.length - 1] != kNew_line) {
		printf("_db_read_idx: missing newline\n");
		return 0;
	}
	index_.buffer[index_.length - 1] = 0;  //换行符替换为null
	char *ptr1, *ptr2;
	ptr2 = index_.buffer + index_.length - 2;
	while (ptr2 >= index_.buffer && kSeparate != *ptr2)
		ptr2--;
	if (ptr2 < index_.buffer){
		printf("_db_read_idx: missing second separator\n");
		return 0;
	}
	*ptr2++ = 0;    //替换分割符为null
	ptr1 = ptr2 - 2;
	while (ptr1 >= index_.buffer && kSeparate != *ptr1)
		ptr1--;
	if (ptr1 < index_.buffer){
		printf("_db_read_idx: missing first separator\n");
		return 0;
	}
	*ptr1++ = 0;
	/*
	 * 分隔符分开的三个数据分别是key，data的偏移量，data的长度
	 *这里把data的偏移量和长度读到data_里
	 */
	if ((data_.offset = atol(ptr1)) < 0) {
		printf("_db_read_idx: starting offset < 0\n");
		return 0;
	}
	if ((data_.length = atol(ptr2)) <= 0) {
		printf("_db_read_idx: invalid length\n");
		return 0;
	}
	return next_offset_;
}

/*
 * 将data读到data_.buffer里并返回
 * 失败返回""字符串
 */
char *DB::_db_read_data() {
	if (-1 == lseek(data_.fd, data_.offset, SEEK_SET)) {
		printf("_db_read_dat: lseek error\n");
		return "";
	}
	if (read(data_.fd, data_.buffer, data_.length) != data_.length) {
		printf("_db_read_dat: read error\n");
		return "";
	}
	if (data_.buffer[data_.length - 1] != kNew_line) {
		//完整性检查
		printf("_db_read_dat: missing newline\n");
		return "";
	}
	data_.buffer[data_.length - 1] = 0; //用null替换换行符
	return data_.buffer;
}

bool DB::db_delete(const string &key) {
	off_t start_offset = _db_hash(key) * kPtr_size + kHash_offset;
	//因为要删除所以加个写锁，同样只锁第一个字节
	RecordReadwLock writew_lock(index_.fd, start_offset, SEEK_SET, 1);
	if (_db_find(key, start_offset)) 
		//存在这个key
		return _db_do_delete();
	return false;
}

/*
 * 真正的删除操作
 * 成功返回true，失败返回false
 */
bool DB::_db_do_delete() {
	//先清空databuffer
	memset(data_.buffer, kSpace, data_.length);
	data_.buffer[data_.length - 1] = 0;
	//再清空indexbuffer
	char *ptr = index_.buffer;
	while (*ptr)
		*ptr++ = kSpace;
	//锁住空闲链表
	RecordWritewLock writew_lock(index_.fd, kFree_offset, SEEK_SET, 1);
	//将空的databuffer写入
	if (!_db_write_data(data_.buffer, data_.offset, SEEK_SET)) {
		printf("_db_do_delete: db_write_data error\n");
		return false;
	}
	off_t save_ptr = next_offset_;     //保存一下原本节点的下一个节点的偏移量，因为下面的调用会修改这个值
	/*
	 * 将该节点放到空闲链表里
	 * 具体操作为将该节点的ptr部分指向空闲链表的第一个节点
	 * 再将空闲链表头的ptr设置为该节点的偏移量
	 */
	off_t free_ptr = _db_read_ptr(kFree_offset);
	if (!_db_write_idx(index_.buffer, index_.offset, SEEK_SET, free_ptr)) {
		printf("_db_do_delete: db write idx error\n");
		return false;
	}
	if (!_db_write_ptr(kFree_offset, index_.offset)) {
		printf("_db_do_delete: db write ptr error\n");
		return false;
	}
	/*
	 * 把原来该节点在hash链中的前一个节点的ptr指向原来该节点的后一个节点
	 * 也就是把他从hash链中删除
	 */
	if (!_db_write_ptr(pre_offset_, save_ptr)) {
		printf("_db_do_delete: db write ptr error\n");
		return false;
	}
	return true;
}

/*
 * 写入一个data记录，偏移量为offset，操作起点whence
 * 成功返回true，失败返回false
 * 此版本是无锁版本
 */
bool DB::_db_write_data(const char* data, off_t offset, int whence) {
	if (-1 == (data_.offset = lseek(data_.fd, offset, whence))) {
		printf("_db_write_data: lseek error\n");
		return false;
	}
	struct iovec iov[2];
	char newline = kNew_line;
	//将data写入进.dat文件，每个data记录都是用换行符隔开的
	data_.length = strlen(data) + 1;
	iov[0].iov_base = (char *)data;
	iov[0].iov_len = data_.length - 1;
	iov[1].iov_base = &newline;
	iov[1].iov_len = 1;
	if (writev(data_.fd, iov, 2) != data_.length) {
		printf("_db_write_data: writev error of data record\n");
		return false;
	}
	return true;
}

/*
 * 上面的方法的加锁版
 * 自行根据情况需要调用哪个函数
 */
bool DB::_db_lock_and_write_data(const char* data, off_t offset, int whence) {
	//锁住整个data文件
	RecordWritewLock writew_lock(data_.fd, 0, SEEK_SET, 0);
	return _db_write_data(data, offset, whence);
}

/*
 * 在offset这个位置写一个index记录
 * key是存的key值，offset为写入的位置的偏移量，whence起点，next_offset为该节点的下一个偏移量
 * 成功返回true，失败返回false
 * 此版本为无锁版本
 */
bool DB::_db_write_idx(const char* key, off_t offset, int whence, off_t next_offset) {
	struct iovec iov[2];
	char prefix[kPtr_size + kIndex_length_size + 1];
	if (!_db_pre_write_idx(key, next_offset, iov, prefix)) {
		printf("_db_writeidx: pre write idx error\n");
		return false;
	}
	if (!_db_do_write_idx(offset, whence, iov)) {
		printf("_db_writeidx: do write idx error\n");
		return false;
	}
	return true;
}

/*
 * _db_write_idx的加锁版
 */
bool DB::_db_lock_and_write_idx(const char* key, off_t offset, int whence, off_t next_offset) {
	struct iovec iov[2];
	char prefix[kPtr_size + kIndex_length_size + 1];
	if (!_db_pre_write_idx(key, next_offset, iov, prefix)) {
		printf("_db_writeidx: pre write idx error\n");
		return false;
	}
	//只锁住后面的内容，如需锁住某条hash链请在之前自行加锁
	RecordWritewLock writew_lock(index_.fd, ((kHash_table_size + 1) * kPtr_size) + 1, SEEK_SET, 0);
	if (!_db_do_write_idx(offset, whence, iov)) {
		printf("_db_writeidx: do write idx error\n");
		return false;
	}
	return true;
}

/*
 * 真正写index记录之前的预处理
 * 主要是做合理性判定以及返回iovec结构用于写index记录
 * 参数参考db_write_idx，最后两个参数是用于结果返回
 * 成功返回true，失败返回false，iov存散列写的结构
 */
bool DB::_db_pre_write_idx(const char* key, off_t next_offset, struct iovec *iov, char *prefix) {
	next_offset_ = next_offset;     //记录一下最后一次write_idx记录的下一个节点
	if (next_offset < 0 || next_offset > kPtr_max) {
		printf("_db_writeidx: invalid next_offset: %d", next_offset);
		return false;
	}
	//结构是key:data的偏移量:data的长度 \n是每个记录的分隔符
	sprintf(index_.buffer, "%s%c%lld%c%d\n", key, kSeparate, (long long)data_.offset, kSeparate, data_.length);
	index_.length = strlen(index_.buffer);
	if (index_.length < kIndex_min || index_.length > kIndex_max) {
		printf("_db_writeidx: invalid length\n");
		return false;
	}
	//index记录的前缀，结构是next_offset+index_length
	sprintf(prefix, "%*lld%*d", kPtr_size, (long long)next_offset, kIndex_length_size, index_.length);
	iov[0].iov_base = prefix;
	iov[0].iov_len = kPtr_size + kIndex_length_size;
	iov[1].iov_base = index_.buffer;
	iov[1].iov_len = index_.length;
	return true;
}

/*
 * 真正写入index文件的函数，参数参考db_write_idx和_db_pre_write_idx
 * 成功返回true，失败返回false
 */
bool DB::_db_do_write_idx(off_t offset, int whence, struct iovec *iov) {
	//记录一下当前index记录的偏移量
	if (-1 == (index_.offset = lseek(index_.fd, offset, whence))) {
		printf("_db_writeidx: lseek error\n");
		return false;
	}
	if (writev(index_.fd, iov, 2) != kPtr_size + kIndex_length_size + index_.length) {
		printf("_db_writeidx: writev error of index record\n");
		return false;
	}
	return true;
}

/*
 * 在当前位置写入ptr
 * 成功返回true，失败返回false
 */
bool DB::_db_write_ptr(off_t offset, off_t ptr) {
	char asciiptr[kPtr_size + 1];
	if (ptr < 0 || ptr > kPtr_max) {
		printf("_db_writeptr: invalid ptr: %d\n", ptr);
		return false;
	}
	sprintf(asciiptr, "%*lld", kPtr_size, (long long)ptr);

	if (-1 == lseek(index_.fd, offset, SEEK_SET)) {
		printf("_db_write_ptr: lseek error to ptr field\n");
		return false;
	}
	if (write(index_.fd, asciiptr, kPtr_size) != kPtr_size) {
		printf("_db_write_ptr: write error of ptr field\n");
		return false;
	}
	return true;
}

int DB::db_store(const string &key, const string &data, int flag) {
	//检查标志合法性
	if (flag <= STORE_MIN_FLAG || flag >= STORE_MAX_FLAG) {
		printf("_db_store: flag is invalid\n");
		return -1;
	}
	//检查数据长度
	int data_length = data.length() + 1;    //记得留个换行符
	if (data_length < kData_min || data_length > kData_max) {
		printf("db_store: invalid data length\n");
		return -1;
	}
	//先对这个key的hash链上个写锁
	off_t start_offset = _db_hash(key) * kPtr_size + kHash_offset;
	RecordWritewLock writew_lock(index_.fd, start_offset, SEEK_SET, 1);
	bool can_find = _db_find(key, start_offset);
	//不同的flag调用不同的函数
	return store_function_map[flag](key, data, can_find, start_offset);
}

/*
 * insert操作，参数只比db_store多了个是否存在该key的标记(can_find)
 * 返回值与db_store一致
 */
int DB::_db_store_insert(const string &key, const string &data, bool can_find, off_t start_offset) {
	if (can_find) {
		printf("_db_store_insert: key is exist in db\n");
		return 1;
	}
	int key_length = key.length();
	int data_length = data.length() + 1;    //记得留换行符
	off_t ptr = _db_read_ptr(start_offset);    //记录当前hash链的第一个节点的偏移量
	//先找是否有合适的空闲节点
	if (!_db_find_and_delete_free(key_length, data_length)) {
		/*
		 * 没有找到则把记录追加到.idx文件和.dat文件结尾
		 * 而且这个记录必须放到这个hash链的头
		 * 注意，这里必须要用上锁版
		 */
		if (!_db_lock_and_write_data(data.c_str(), 0, SEEK_END)) {
			printf("_db_store_insert: db lock and write data error\n");
			return -1;
		}
		if (!_db_lock_and_write_idx(key.c_str(), 0, SEEK_END, ptr)) {
			printf("_db_store_insert: db lock and write idx error\n");
			return -1;
		}
	}
	else {
		/*
		 * 找到则把该记录写到这个节点的位置
		 * 这个节点已经不存在空闲链表，而且hash链也锁住了，所以只需要调用不上锁版
		 */
		if (!_db_write_data(data.c_str(), data_.offset, SEEK_SET)) {
			printf("_db_store_insert: db write data error\n");
			return -1;
		}
		if (!_db_write_idx(key.c_str(), index_.offset, SEEK_SET, ptr)) {
			printf("_db_store_insert: db write idx error\n");
			return -1;
		}
	}
	if (!_db_write_ptr(start_offset, index_.offset)) {
		printf("_db_store_insert: db write ptr error\n");
		return -1;
	}
	return 0;
}

/*
 * 参数和返回值参考_db_store_insert
 * replace操作
 */
int DB::_db_store_replace(const string &key, const string &data, bool can_find, off_t start_offset) {
	if (!can_find) {
		printf("_db_store_replace: db can not find key\n");
		return -1;
	}
	//检查data的长度是否符合
	int data_length = data.length() + 1;      //记得留换行符
	if (data_length != data_.length) {
		/*
		 * 长度不一致
		 * 先删除这个数据，然后再调用insert插入
		 */
		if (!_db_do_delete()) {
			printf("_db_store_replace: db do delete error\n");
			return -1;
		}
		return _db_store_insert(key, data, false, start_offset);
	}
	else {
		/*
		 * 长度一致
		 * 直接利用这个节点重写数据
		 */
		if (!_db_write_data(data.c_str(), data_.offset, SEEK_SET)) {
			printf("_db_store_replace: db write data error\n");
			return -1;
		}
		return 0;
	}
}

/*
 * 参数参考db_store_insert
 * insert或者replace操作
 */
int DB::_db_store_ins_or_rep(const string &key, const string &data, bool can_find, off_t start_offset) {
	//没什么好说的，存在key就调用replace，不存在调用insert
	if (can_find)
		return _db_store_replace(key, data, can_find, start_offset);
	else
		return _db_store_insert(key, data, can_find, start_offset);
}

/*
 * 查找是否有合适的空闲节点
 * 如果有则把他从空闲链表中删除
 * 且将相应的信息写到index_和data_
 * 成功返回true，失败返回false
 */
bool DB::_db_find_and_delete_free(int key_length, int data_length) {
	off_t offset, next_offset;
	//先上个写锁
	RecordWritewLock writew_lock(index_.fd, kFree_offset, SEEK_SET, 1);
	pre_offset_ = kFree_offset;
	offset = _db_read_ptr(kFree_offset);
	while (offset) {
		next_offset = _db_read_idx(offset);
		if (strlen(index_.buffer) == key_length && data_.length == data_length)
			//找到了合适的空闲节点
			break;
		pre_offset_ = offset;    //记录前一个节点
		offset = next_offset;
	}
	if (!offset)
		return false;
	//找到了就把这个节点从空闲链表里去掉
	return _db_write_ptr(pre_offset_, next_offset_);
}

}
