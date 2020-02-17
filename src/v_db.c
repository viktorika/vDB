#include "v_db.h"
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/uio.h>
#include <stdio.h>
#include <unistd.h>

/*
 * 内部索引文件常量
 * 这些常量被用在构造索引文件记录和数据文件记录时
 */
#define IDXLEN_SZ    4      //索引记录长度
#define SEP         ':'     //索引记录中的分隔符
#define SPACE       ' '     //空格符
#define NEWLINE     '\n'    //换行符

/*
 * 下面的定义用于在索引文件中hash链和空闲列表链
 */
#define PTR_SZ       7      //hash链中ptr字段的大小
#define PTR_MAX 999999      //最大文件偏移 = 10 ** PTR_SZ - 1
#define NHASH_DEF  137      //默认hash表的大小
#define FREE_OFF     0      //在索引文件中空闲列表的偏移
#define HASH_OFF PTR_SZ     //在索引文件中hash表的偏移

typedef unsigned long DBHASH   //hash值
typedef unsigned long COUNT    //无符号计数器

/*
 * 数据库的私有数据结构表示
 */
typedef struct {
	int idxfd;              //索引文件fd
	int datfd;              //数据文件fd
	char *idxbuf;           //用于索引记录的已分配的缓冲区
	char *datbuf;           //用于数据记录的已分配的缓冲区
	char *name;             //已打开的数据库的名字
	off_t idxoff;           //索引记录在索引文件中的偏移量
							//key在(idxoff + PTR_SZ + IDXLEN_SZ)
	size_t idxlen;          //索引记录的长度
	                        //包括记录开头的IDXLEN_SZ和索引结尾的\n
	off_t datoff;           //数据记录在数据文件中的偏移量
	size_t datlen;          //数据记录的长度，包括结尾的\n
	off_t ptrval;           //索引记录中ptr链的内容
	off_t ptroff;           //指向此idx记录的链ptr的偏移量
	off_t chainoff;         //这个索引记录的hash链的偏移量
	off_t hashoff;          //在索引文件中hash表的偏移量
	DBHASH nhash;           //当前hash表的大小
	COUNT cnt_delok;        //删除成功的计数器
	COUNT cnt_delerr;       //删除错误的计数器
	COUNT cnt_fetchok;      //取数据成功的计数器
	COUNT cnt_fetcherr;     //取数据失败的计数器
	COUNT cnt_nextrec;      //下一条记录的计数器? TODO待定
	COUNT cnt_store1;       //DB_INSERT标志，no empty, appended
	COUNT cnt_store2;       //DB_INSERT标志，found empty, reused
	COUNT cnt_store3;       //DB_REPLACE标志，diff len，appended
	COUNT cnt_store4;       //DB_REPLACE标志，smae len, overwrote
	COUNT cnt_store5;       //store错误
} DB;

//内部函数
static DB *_db_alloc(int);
static void _db_dodelete(DB *);
static int _db_find_and_lock(DB *, const char *, int);
static int _db_findfree(DB *, int, int);
static void _db_free(DB *);
static DBHASH _db_hash(DB *, const char *);
static char *_db_readdat(DB *);
static off_t _db_readidx(DB *, off_t);
static off_t _db_readptr(DB *, off_t);
static void _db_writedat(DB *, const char *, off_t, int);
static void _db_writeidx(DB *, const char *, off_t, int, off_t);
static void _db_writeptr(DB *, off_t, off_t);

/*
 *打开或者创建一个数据库.参数与open一致
 */
DBHANDLE db_open(const char *pathname, int oflag, ...){
	DB *db;
	int len, mode;
	size_t i;
	char asciiptr[PTR_SZ + 1], hash[(NHASH_DEF + 1) * PTR_SZ + 2];//+2是为了换行符和null
	struct stat statbuff;

	/*
	 *分配一个DB结构，还有它需要的buffers
	 */
	len = strlen(pathname);
	if ((db = _db_alloc(len)) == NULL){
		printf("db_open: _db_alloc error for DB");
		exit(1);
	}
	db->nhash = NHASH_DEF; //hash表的大小
	db->hashoff = HASH_OFF; //hash表的索引文件偏移量
	strcpy(db->name, pathname);
	strcat(db->name, ".idx");

	if(oflag & O_CREAT){
		va_list ap;
		va_start(ap, oflag);
		mode = va_arg(ap, int);
		va_end(ap);
		
		//打开索引文件和数据文件
		db->idxfd = open(db->name, oflag, mode);
		strcpy(db->name + len, ".dat");
		db->datfd = open(db->name, oflag, mode);
	}else{
		//打开索引文件和数据文件
		db->idxfd = open(db->name, oflag);
		strcpy(db->name + len, ".dat");
		db->datfd = open(db->name, oflag);
	}
	if (db->idxfd < 0 || db->datfd < 0){
		_db_free(db);
		return NULL;
	}
	if ((oflag & (O_CREAT | O_TRUNC)) == (O_CREAT | O_TRUNC)){
		/*
		 * 如果数据库被创建，我们必须初始化它
		 * 写锁住整个文件，以至于我们能统计它
		 * 检查它的大小，并且自动初始化它
		 */
		if (writew_lock(db->idxfd, 0, SEEK_SET, 0) < 0){
			printf("db_open: writew_lock error\n");
			exit(1);
		}
		if (fstat(db->idxfd, &statbuff) < 0){
			printf("db_open: fstat error\n");
			exit(1);
		}

		if (0 == statbuff.st_size){
			/*
			 * 我们必须构建一个值为0的NASH_DEF + 1的链表
			 * + 1表示hash表之前的空闲列表指针
			 */
			sprintf(asciiptr, "%*d", PTR_SZ, 0);
			hash[0] = 0;
			for (int i = 0; i< NHASH_DEF + 1; i++)
				strcat(hash, asciiptr);
			strcat(hash, "\n");
			i = strlen(hash);
			if (write(db->idxfd, hash, i) != i){
				printf("db_open: index file init write error\n");
				exit(0);
			}
		}
		if (un_lock(db->idxfd, 0, SEEK_SET, 0) < 0){
			printf("db_open: un_lock error\n");
			exit(0);
		}
	}
	db_rewind(db);
	return db;
}

/*
 * 分配&初始化一个DB结构和它的buffers
 */
static DB *_db_alloc(int namelen){
	DB *db;
	/*
	 * 使用calloc，用0去初始化结构
	 */
	if (!(db = calloc(1, sizeof(DB)))){
		printf("_db_alloc: calloc error for DB\n");
		exit(0);
	}
	db->idxfd = db->datfd = -1;
	/*
	 * 为名字分配空间
	 * +5是为了".idx"or".dat"+字符串结束符
	 */
	if (!(db->name = malloc(namelen + 5))){
		printf("_db_alloc: malloc error for name\n");
		exit(0);
	}
	/*
	 * 分配一个index buffer和一个data buffer
	 * +2是为了换行和字符串结束符
	 */
	if (!(db->idxbuf = malloc(IDXLEN_MAX + 2))){
		printf("_db_alloc: malloc error for index buffer\n");
		exit(0);
	}
	if (!(db->datbuf = malloc(DATLEN_MAX + 2))){
		printf("_db_alloc: malloc error for data buffer\n");
		exit(0);
	}
}

/*
 * 放弃对数据库的访问
 */
void db_close(DBHANDLE h){
	_db_free((DB *) h);
}

/*
 * 释放数据库结构以及它可能指向的所有已分配的buffers
 * 同时关闭打开了的fd
 */
static void _db_free(DB *db){
	if (db->idxfd >=0 )
		close(db->idxfd);
	if (db->datfd >= 0)
		close(db->datfd);
	if (db->idxbuf != NULL)
		free(db->idxbuf);
	if (db->datbuf != NULL)
		free(db->datbuf);
	if (db->name !=NULL)
		free(db->name);
	free(db);
}

/*
 * 获取一条记录，返回一个以空值结尾的日期的指针
 */
char *db_fetch(DBHANDLE h, const char *key){
	DB *db = h;
	char *ptr;
	if (_db_find_and_lock(db, key, 0) < 0){
		ptr = NULL;   //错误，没找到记录
		db->cnt_fetcherr++;
	}else{
		ptr = _db_readdat(db);  //返回一个指向data的指针
		db->cnt_fetchok++;
	}

	/*
	 * 解锁hash链_db_find_and_lock的锁 
	 */
	if (un_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0){
		printf("_db_fetch: un_lock error\n");
		exit(0);
	}
	return ptr;
}

/*
 * 找到指定的记录， e可以呗db_delete,db_fetch和db_store调用
 * hash链锁定返回
 */
static int _db_find_and_lock(DB *db, const char *key, int writelock){
	off_t offset, nextoffset;
	/*
	 * 计算key的hash值，然后计算hash表中相应链ptr的字节偏移。
	 * 这是我们开始搜索的地方，首先我们计算这个key在hash表中的偏移
	 */
	db->chainoff = (_db_hash(db, key) * PTR_SZ) + db->hashoff;
	db->ptroff = db->chainoff;

	/*
	 * 我们在这里锁定hash链，调用者必须事后自行解锁
	 * 注意我们只在第一个字节锁和解锁
	 */
	if (writelock){
		if (writew_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0){
			printf("_db_find_and_lock: writew_lock error\n");
			exit(0);
		}
	}else{
		if (readw_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0){
			printf("_db_find_and_lock: readw_lock error\n");
			exit(0);
		}
	}
	/*
	 * 获取index文件中的偏移量或hash链上的第一条记录
	 */
	offset = _db_readptr(db, db->ptroff);
	while(offset){
		nextoffset = _db_readidx(db, offset);
		if (!strcmp(db->idxbuf, key))
			break;               //找到一个匹配
		db->ptroff = offset;     //这个记录的偏移量
		offset = nextoffset;     //下一个要比较的偏移量
	}
	/*
	 * 错误时offset == 0
	 */
	return !offset? -1: 0;
}

/*
 * 计算key的hash值
 */
static DBHASH _db_hash(DB *db, const char *key){
	DBHASH hval = 0;
	char c;
	int i;
	for (i = 1; (c = *key++) != 0; i++)
		hval += c * i;
	return hval % db->nhash;
}

/*
 * 从index文件中的任意地方读取一个链ptr字段:
 * 空闲链表指针，一个hash链ptr或者一个index记录链ptr
 */
static off_t _db_readptr(DB *db, off_t offset){
	char asciiptr[PTR_SZ + 1];
	if (lseek(db->idxfd, offset, SEEK_SET) == -1){
		printf("_db_readptr: lseek error to ptr field\n");
		exit(0);
	}
	if (read(db->idxfd, asciiptr, PTR_SZ) != PTR_SZ){
		printf("_db_readptr: read error of ptr field\n");
		exit(0);
	}
	asciiptr[PTR_SZ] = 0;
	return atol(asciiptr);
}

/*
 * 读取下一个index记录.我们从index文件中指定的偏移量开始.
 * 我们将index记录读入db->idxbuf并将分隔符替换为null字节
 * 如果一切正常，我们将db->datoff和db->datlen设置为data文件相应数据记录的偏移量和长度
 */
static off_t _db_readidx(DB* db, off_t offset){
	ssize_t i;
	char *ptr1, *ptr2;
	char asciiptr[PTR_SZ + 1], asciilen(IDXLEN_SZ + 1);
	struct iovec iov[2];
	/*
	 * 定位index文件并且记录偏移量
	 * db_nextrec用参数offset==0调用这个函数意味着从当前偏移量读取
	 * 我们仍然需要调用lseek去记录当前偏移量
	 */
	if (-1 == (db->idxoff = lseek(db->idxfd, offset, !offset?SEEK_CUR:SEEK_SET))){
		printf("_db_readidx: leek error\n");
		exit(0);
	}
	iov[0].iov_base = asciiptr;
	iov[0].iov_len = PTR_SZ;
	iov[1].iov_base = asciilen;
	iov[1].iov_len = IDXLEN_SZ;
	if ((i = readv(db->idxfd, &iov[0], 2)) != PTR_SZ + IDXLEN_SZ){
		if (!i && !offset)
			return -1; //EOF
		printf("_db_readidx: readv error of index record\n");
	}

	/*
	 * 这是我们的返回值，通常>=0
	 */
	asciiptr[PTR_SZ] = 0;        //空终止
	db->ptrval = toll(asciiptr); //链中下一个key的偏移量
	asciilen[IDXLEN_SZ] = 0;     //空终止
	if ((db->idxlen = atoi(asciilen)) < IDXLEN_MIN || db->idxlen > IDXLEN_MAX){
		printf("_db_readidx: invalid length\n");
		exit(0);
	}
	/*
	 * 现在读取真正的index记录,我们把它读到打开数据库时分配了空间的key buffer
	 */
	if ((i = read(db->idxfd, db->idxbuf, db->idxlen)) != db->idxlen){
		printf("_db_readidx: read error of index record\n");
		exit(0);
	}
	if (db->idxbuf(db->idxlen - 1) != NEWLINE){
		//完整性检查
		printf("_db_readidx: missing newline\n");
		exit(0);
	}
	db->idxbuf[db->idxlen - 1] = 0;    //替换换行为null
	/*
	 * 在index文件中找到分隔符
	 */
	if (!(ptr1 = strchr(db->idxbuf, SEP))){
		printf("_db_readidx: missing first separator\n");
		exit(0);
	}
	*ptr1++ = 0;      //替换SEP为null
	if (!(ptr2 = strchr(ptr1, SEP))){
		printf("_db_readidx: missing second separator\n");
		exit(0);
	}
	*ptr2++ = 0;      //替换SEP为null
	if (strchr(ptr2, SEP) != NULL){
		printf("_db_readix: too many separators\n");
		exit(0);
	}
	/*
	 * 获取data记录的起始偏移量和长度
	 */
	if ((db->datoff = atol(ptr1)) < 0){
		printf("_db_readidx: starting offset < 0\n");
		exit(0);
	}
	if ((db->datlen = atol(ptr2)) <= 0){
		printf("_db_readidx: invalid length\n");
		exit(0);
	}
	return db->ptrval;
}

/*
 * 将当前的data记录读到data buffer
 * 返回以null值结尾的数据缓冲区的指针
 */
static char *_db_readdat(DB *db){
	if (-1 == lseek(db->datfd, db->datoff, SEEK_SET)){
		printf("_db_readdat: lseek error\n");
		exit(0);
	}
	if (read(db->datfd, db->datbuf, db->datlen) != db->datlen){
		printf("_db_readdat: read error\n");
		exit(0);
	}
	if (db->datbuf[db->datlen - 1] != NEWLINE){
		//完整性检查
		printf("_db_readdat: missing newline\n");
		exit(0);
	}
	db->datbuf[db->datlen - 1] = 0; //用null替换换行符
	return db->datbuf;
}

/*
 * 删除指定的记录
 */
int db_delete(DBHANDLE h, const char *key){
	DB *db = h;
	int rc = 0;
	if (!_db_find_and_lock(db, key, 1)){
		_db_dodelete(db);
		db->cnt_delock++;
	}else{
		rc = -1;          //没有找到
		db->cnt_delerr++;
	}
	if (un_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0){
		printf("db_delete: un_lock error\n");
		exit(0);
	}
	return rc;
}

/*
 * 删除db结构指定的当前记录
 * 在记录已由_db_find_and_lock查找之后，该函数由db_delete和db_store调用
 */
static void _db_dodelete(DB *db){
	int i;
	char *ptr;
	off_t freeptr, saveptr;
	/*
	 * 将data buffer和key用b空白填充
	 */
	for (ptr = db->datbuf, i = 0; i < db->datlen - 1; i++)
		*ptr++ = SPACE;
	*ptr = 0;      //_db_writedat的null终止符
	ptr = db->idxbuf;
	while(*ptr)
		*ptr++ = SPACE;
	/*
	 * 我们必须锁住空闲链表
	 */
	if (writew_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0){
		printf("_db_dodelete: write_lock error\n");
		exit(0);
	}
	/*
	 * 写数据时全空白
	 */
	_db_writedat(db, db->datbuf, db->datoff, SEEK_SET);
	/*
	 * 读取空闲链表指针
	 * 它的值成为已删除索引记录的chain ptr字段
	 * 这意味着删除的记录变成了空闲链表的头
	 */
	freeptr = _db_readptr(db, FREE_OFF);
	/*
	 * 保存index记录链ptr的内容
	 * 然后由_db_writeidx重写
	 */
	saveptr = db->ptrval;
	/*
	 * 重写index记录
	 * 重写index记录的长度，数据偏移量和数据长度
	 */
	_db_writeidx(db, db->idxbuf, db->idxoff, SEEK_SET, freeptr);
	/*
	 * 编写新的空闲链表指针
	 */
	_db_writeptr(db, FREE_OFF, db->idxoff);
	/*
	 * 重写指向该记录被删除的链ptr
	 * 回想一下_db_find_and_lock设置db->ptroff指向此链ptr
	 * 我们将此链条ptr设置为已删除记录的链ptr,saveptr的内容
	 */
	_db_writeptr(db, db->ptroff, saveptr);
	if (un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0){
		printf("_db_dodelete: un_lock error\n");
		exit(0);
	}
}

/*
 * 写一个data记录. 由_db_dodelete调用（用空白写入记录）和db_store
 */
static void _db_writedat(DB *DB, const char *data, off_t offset, int whence){
	struct lovec iov[2];
	static char newline = NEWLINE;
	/*
	 * 如果要追加，则必须在进行lseek之前锁定并进行写入以使两者成为原子操作
	 * 如果我们要覆盖现有记录，则不必锁定
	 */
	if (whence == SEEK_END){
		// 追加，锁定整个文件
		if (writew_lock(db->datfd, 0, SEEK_SET, 0) < 0){
			printf("_db_writedat: write_lock error\n");
			exit(0);
		}
	}
	if (-1 == (db->datoff = lseek(db->datfd, offset, whence))){
		printf("_db_writedat: writew_lock error\n");
		exit(0);
	}

	if (-1 == (db->datoff = lseek(db->datfd, offset, whence))){
		printf("_db_writedat: lseek error\n");
	}
	db->datlen = strlen(data) + 1; //长度包括换行
	iov[0].lov_base = (char *)data;
	iov[0].iov_len = db->datlen - 1;
	iov[1].iov_base = &newline;
	iov[1].iov_len = 1;
	if (writev(db->datfd, &iov[0], 2) != db->datlen){
		printf("_db_writedat: writev error of data record\n");
		exit(0);
	}
	
	if (whence == SEEK_END)
		if (un_lock(db->datfd, 0, SEEK_SET, 0) < 0){
			printf("_db_writedat: unlock error\n");
			exit(0);
		}
}


/*
 * 写index记录
 * 在此函数之前调用_db_writedat来设置我们写的index记录的db结构中的datoff和datlen字段
 */
static void _db_writeidx(DB *db, const char *key, off_t offset, int whence, off_t ptrval){
	struct iovec iov[2];
	char asciiptrlen[PTR_SZ + IDXLEN_SZ + 1];
	int len;
	if ((db->ptrval = ptrval) < 0 || ptrval > PTR_MAX){
		printf("_db_writeidx: invalid ptr: %d", ptrval);
		exit(0);
	}
	sprintf(db->idxbuf, "%s%c%lld%c%ld\n", key, SEP, (long long)db->datoff, SEP, (long)db->datlen);
	len = strlen(db->idxbuf);
	if (len < IDXLEN_MIN || len > IDXLEN_MAX){
		printf("_db_writeidx: invalid length\n");
		exit(0);
	}
	sprintf(asciiptrlen, "%*lld%*d", PTR_SZ, (long long)ptrval, IDXLEN_SZ, len);
	/*
	 * 如果要追加，则必须在进行lseek之前锁定并进行写入以使两者成为原子操作
	 * 如果我们要覆盖现有记录，则不必锁定
	 */
	if (whence == SEEK_END){
		//追加
		if (writew_lock(db->idxfd, ((db->nhash + 1) * PTR_SZ) + 1, SEEK_SET, 0) < 0){
			printf("_db_writeidx: writew_lock error\n");
			exit(0);
		}
	}
	/*
	 * 标记index文件并且记录偏移量
	 */
	if (-1 == (db->idxoff = lseek(db->idxfd, offset, whence))){
		printf("_db_writeidx: lseek error\n");
		exit(0);
	}
	iov[0].iov_base = asciiptrlen;
	iov[0].iov_len = PTR_SZ + IDXLEN_SZ;
	iov[1].iov_base = db->idxbuf;
	iov[1].iov_len = len;
	if (writev(db->idxbuf, &iov[0], 2) != PTR_SZ + IDXLEN_SZ + len){
		printf("_db_writeidx: writev error of index record\n");
		exit(0);
	}
	if (SEEK_END == whence){
		if (un_lock(db->idxfd, ((db->nhash + 1) * PTR_SZ) + 1, SEEK_SET, 0) < 0)
			printf("_db_writeidx: un_lock error\n");
	}
}

/*
 * 在索引文件的某个位置（自由列表，哈希表或索引记录）中写一个链ptr字段
 */
static void _db_writeptr(DB *db, off_t offset, off_t ptrval){
	char asciiptr[PTR_SZ + 1];
	if (ptrval < 0 || ptrval > PTR_MAX){
		printf("_db_writeptr: invalid ptr: %d", ptrval);
		exit(0);
	}
	sprintf(asciiptr, "%*lld", PTR_SZ, (long long)ptrval);

	if (-1 == lseek(db->idxfd, offset, SEEK_SET)){
		printf("_db_writeptr: lseek error to ptr field\n");
		exit(0);
	}
	if (write(db->idxfd, asciiptr, PTR_SZ) != PTR_SZ){
		printf("_db_writeptr: write error of ptr field\n");
		exit(0);
	}
}

/*
 * 将记录存储在数据库中。 如果OK，则返回0；如果记录存在并且指定了DB_INSERT，则返回1；如果错误，则返回-1。
 */
int db_store(DBHANDLE h, const char *key, const char *data, int flag){
	DB *db = h;
	int rc, keylen, datlen;
	off_t ptrval;
	if (flag != DB_INSERT && flag != DB_REPLACE && flag != DB_STORE){
		errno = EINVAL;
		return -1;
	}
	keylen = strlen(key);
	datlen = strlen(data) + 1;    //+1为了结束的换行
	if (datlen < DATLEN_MIN || datlen > DATLEN_MAX){
		printf("db_store: invalid data length\n");
		exit(0);
	}
	/*
	 * db_find_and_lock计算此新记录进入db-> chainoff的哈希表，而不管它是否已经存在。
	 * 以下对_db_writeptr的调用将更改此链的哈希表条目以指向新记录。新记录被添加到 哈希链的前面
	 */
	if (_db_find_and_lock(db, key, 1) < 0){
		//没有找到记录
		if (flag == DB_REPLACE){
			rc = -1;
			db->cnt_storerr++;
			errno = ENOENT;          //错误，记录不存在
			goto doreturn;
		}
		/*
		 * _db_find_and_lock为我们锁定了哈希链
		 *  将链ptr读取到哈希链上的第一个索引记录
		 */
		ptrval = _db_readptr(db, db->chainoff);
		if (_db_findfree(db, keylen, datlen) < 0){
			/*
			 * 找不到足够大的空记录。将新记录追加到索引和数据文件的末尾
			 */
			_db_writedat(db, data, 0, SEEK_END);
			_db_writeidx(db, key, 0, SEEK_END, ptrval);
			/*
			 * db-> idxoff由_db_writeidx设置。新记录进入哈希链的最前面
			 */
			_db_writeptr(db, db->chainoff, db->idxoff);
			db->cnt_stor1++;
		}else{
			/*
			 * 重用一个空记录。 _
			 * db_findfree从可用列表中将其删除，并设置db->datoff和db->idxoff.重用记录位于哈希链的最前面
			 */
			_db_writedat(db, data, db->datoff, SEEK_SET);
			_db_writeidx(db, key, db->idxoff, SEEK_SET, ptrval);
			_db_writeptr(db, db->chainoff, db->idxoff);
			db->cnt_store2++;
		}	
	}else{
		//找到记录
		if (flag == DB_INSERT){
			rc = 1; //错误，记录已经存在db
			db->cnt_storerr++;
			goto doreturn;
		}
		/*
		 * 我们正在替换现有记录
		 * 我们知道新key等于现有key，但是我们需要检查数据记录的大小是否相同
		 */
		if (datlen != db->datlen){
			_db_dodelete(db); //删除存在的记录
			/*
			 * 重新读取哈希表中的链ptr（可能会随着删除而改变）
			 */
			ptrval = _db_readptr(db, db->chainoff);
			/*
			 * 将新的index和data记录追加到文件结尾
			 */
			_db_writedat(db, data, 0, SEEK_END);
			_db_writeidx(db, key, 0, SEEK_END, ptrval);

			/*
			 * 新记录走在hash链的最前面
			 */
			_db_writeptr(db, db->chainoff, db->idxoff);
			db->cnt_stor3++;
		}else{
			/*
			 * 同样大小的数据，仅仅替换数据记录
			 */
			_db_writedat(db, data, db->datoff, SEEK_SET);
			db->cnt_stor4++;
		}
	}
	rc = 0;   //OK

doreturn:   //解锁由_db_find_and_lock锁定的哈希链
	if (un_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0){
		printf("db_store: unlock error\n");
		exit(0);
	}
	return rc;
}

/*
 * 尝试找到正确大小的空闲index记录和随附的data记录
 * 仅由db_store调用
 */
static int _db_findfree(DB *db, int keylen, int datlen){
	int rc;
	off_t offset, nextoffset, saveoffset;
	/*
	 * 锁定空闲链表
	 */
	if (writew_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0){
		printf("_db_findfree: writew_lock error\n");
		exit(0);
	}
	/*
	 * 读空闲链表指针
	 */
	saveoffset = FREE_OFF;
	offset = _db_readptr(db, saveoffset);
	while (offset){
		nextoffset = _db_readidx(db, offset);
		if (strlen(db->idxbuf) == keylen && db->datlen == datlen)
			break;           //找到一个匹配的
		saveoffset = offset;
		offset = nextoffset;
	}
	if (!offset)
		rc = -1;   //没有找到匹配的
	else{
		/*
		 * 找到一个大小匹配的空闲记录
		 * 索引记录由上面的_db_readidx读取，它设置db-> ptrval
		 * 同样，saveoffset指向指向空闲列表上此空记录的链ptr
		 * 我们将此链ptr设置为db-> ptrval，这将从空闲列表中删除空记录
		 */
		_db_writeptr(db, saveoffset, db->ptrval);
		rc = 0;
		/*
		 * 还要注意，_db_readidx设置了db-> idxoff和db-> datoff
		 * 调用者db_store使用它来写入新的索引记录和数据记录
		 */
	}
	/*
	 * 解锁空闲链表
	 */
	if (un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0){
		printf("_db_findfree: unlock error\n");
		exit(0);
	}
	return rc;
}

/*
 * 倒带db_nextrec的索引文件
 * 由db_open自动调用。 必须在第一个db_nextrec之前调用
 */
void db_rewind(DBHANDLE h){
	DB *db = h;
	off_t offset;
	offset = (db->nhash + 1) * PTR_SZ;  //+1为了空闲链表ptr
	/*
	 * 我们只是将这个过程的文件偏移量设置为index记录的开始.
	 * hash表末尾的换行符不需要锁定+ 1
	 */
	if ((-1 == db->idxoff = lseek(db->idxfd, offset + 1, SEEK_SET))){
		printf("db_rewind: lseek error\n");
		exit(0);
	}
}
/*
 * 返回下一个连续记录
 * 我们只是逐步浏览index文件，忽略删除的记录
 * 必须在第一次调用此函数之前调用db_rewind
 */
char *db_nextrec(DBHANDLE h, char *key){
	DB *db = h;
	char c;
	char *ptr;
	/*
	 * 我们读取锁定空闲列表，以便我们不会在删除记录的中间读取记录
	 */
	if (readw_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0){
		printf("db_nextrec: readw_lock error\n");
		exit(0);
	}
	do {
		/*
		 * 读取下一个顺序索引记录
		 */
		if (_db_readidx(db, 0) < 0){
			ptr = NULL;  //index文件的结束,EOF
			goto doreturn;
		}
		/*
		 * 检查key是否全部是空（空记录）
		 */
		ptr = db->idxbuf;
		while((c = *ptr++) !=0 && SPACE == c)
			;  //跳过知道空字节或者非空白
	}while(!c); //循环知道找到非空白key
	if (key)
		strcpy(key, db->idxbuf);   //返回key
	ptr = _db_readdat(db);         //返回一个指针指向data buffer
	db->cnt_nextrec++;
doreturn:
	if (un_lock(db->idxfd, FREE_OFF,SEEK_SET, 1) < 0){
		printf("db_nextrec: unlock error\n");
		exit(0);
	}
	return ptr;
}
