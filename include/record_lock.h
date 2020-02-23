#pragma once

#include <fcntl.h>

/*
 * RAII封装的记录锁
 * 构造时调用对应的lock函数锁定，离开作用域后自动解锁
 * 注意，不应该自己手动解锁
 */
class RecordLock {
public:
	/*
	 * 第一个参数是fd，对应操作的文件描述符
	 * 第二个参数是offset，相对偏移量
	 * 第三个参数是操作的起点SEEK_SET,SEEK_CUR,SEEK_END
	 * 第四个参数是长度
	 * 构造函数会调用lock上锁，需要重写lock函数
	 */
	explicit RecordLock(int, off_t, int, off_t);
	/*
	 * 析构函数调用un_lock解锁
	 */
	virtual ~RecordLock();
	int lock_result();
	int unlock_result();

protected:
	char *lockname_;        //对应的锁的名字
	int fd_;                //对应操作的文件fd	
	off_t offset_;          //偏移量
	int whence_;            //起点
	off_t len_;             //长度
	int lock_result_;        //调用lock函数的返回结果
	int unlock_result_;      //调用unlock函数的返回结果
	/*
	 * 不同锁对应的锁定函数
	 */
	virtual int lock();
	/*
	 * 解锁函数，锁不同但是解锁是一样的故该函数不需要重写
	 */
	int un_lock();
	/*
	 * 上锁失败的处理函数
	 */
	void fail_lock_process();
	int lock_reg(int, int, int, off_t, int, off_t);

private:
	void fail_unlock_process();
};

/*
 * 非阻塞读锁
 */
class RecordReadLock :public RecordLock {
public:
	explicit RecordReadLock(int, off_t, int, off_t);
protected:
	virtual int lock();
};

/*
 * 阻塞读锁
 */
class RecordReadwLock :public RecordLock {
public:
	explicit RecordReadwLock(int, off_t, int, off_t);
protected:
	virtual int lock();
};

/*
 * 非阻塞写锁
 */
class RecordWriteLock :public RecordLock {
public:
	explicit RecordWriteLock(int, off_t, int, off_t);
protected:
	virtual int lock();
};

/*
 * 阻塞写锁
 */
class RecordWritewLock :public RecordLock {
public:
	explicit RecordWritewLock(int, off_t, int, off_t);
protected:
	virtual int lock();
};
