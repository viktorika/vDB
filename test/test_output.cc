#include "../include/v_db.h"
#include <cstdio>
#include <string>
#include <functional>
#include <unordered_map>
#include <fcntl.h>
#include <iostream>

template<typename T>
bool check_result(T result1, T result2, int cmd_number, int cmd) {
	if (result1 != result2){
		printf("result is not same, cmd_number=%d, cmd=%d\n", cmd_number, cmd);
		std::cout<<"result1="<<result1<<" result2="<<result2<<std::endl;
		return false;
	}
	return true;
}

void test_output() {
	vDB::DB db;
	std::unordered_map<std::string, std::string> m;
	db.db_open("testdb", O_RDWR|O_CREAT|O_TRUNC);
	/*
	 * 通过input文件的输入数据同时来操作db和unorder_map
	 * 对比他们的输出是否不一致
	 */
	freopen("input", "r", stdin);
	int cmd;
	int cmd_number = 1;       //表示这是第几行的命令
	while (~scanf("%d", &cmd)) {
		printf("cmd =%d, cmd_number=%d\n",cmd, cmd_number);
		if (!cmd) {
			char key[4], value[5];
			int flag;
			scanf("%s%s%d", key, value, &flag);
			printf("key=%s,value=%s,flag=%d\n",key,value,flag);
			int db_result, map_result;
			/*
			 * store操作
			 * 对面map来说需要自己手动存
			 */
			db_result = db.db_store(key, value, flag);
			if (m.find(key) == m.end()) {
				if (flag != 2){
					//不是replace操作
					m[key] = value;
					map_result = 0;
				}
				else
					map_result = -1;
			}
			else{
				if (flag != 1){
					//不是insert操作
					m[key] = value;
					map_result = 0;
				}
				else if(1 == flag)
					map_result = 1;
			}
			if (!check_result<int>(db_result, map_result, cmd_number, cmd))
				return;
		}
		else {
			char key[10];
			scanf("%s", key);
			printf("key=%s\n",key);
			if (1 == cmd) {
				/*
				 * delete操作
				 * 对面map来说需要判一下是否存在这个key
				 */
				bool db_result, map_result;
				db_result = db.db_delete(key);
				auto element = m.find(key);
				if (element == m.end())
					map_result = 0;
				else {
					m.erase(element);
					map_result = 1;
				}
				if (!check_result<bool>(db_result, map_result, cmd_number, cmd))
					return;
			}
			else {
				/*
				 * fetch操作
				 * 同样需要查找是否存在
				 */
				std::string db_result, map_result;
				db_result = db.db_fetch(key);
				if (m.find(key) != m.end())
					map_result = m[key];
				if (!check_result<std::string>(db_result, map_result, cmd_number, cmd))
					return;
			}
		}
		cmd_number++;
	}
	db.db_close();
}

int main() {
	test_output();
}
