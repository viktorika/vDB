#pragma once

typedef void * DBHANDLE;

DBHANDLE db_open(const char *, int, ...);
void db_close(DBHANDLE);
char *db_fetch(DBHANDLE, const char *);
int db_store(DBHANDLE, const char *, const char *, int);
int db_delete(DBHANDLE, const char *);

/*
 * db_store的合法标志
 */
#define DB_INSERT 1  //只插入一个新的记录
#define DB_REPLACE 2 //替换一个已存在的记录
#define DB_STORE 3   //替换或者插入

/*
 * 实现限制
 */
#define IDXLEN_MIN 6     //索引长度最小为6，key，sep，start，sep，length，\n
#define IDXLEN_MAX 1024  //索引最大长度，这个可以自己调节
#define DATLEN_MIN 2     //数据长度最小为2，data byte，\n
#define DATLEN_MAX 1024  //数据最大长度，随意调节
