#include "v_db.h"
#include <cstdio>
#include <string>
#include <functional>
#include <unordered_map>

void generate_input() {
	//生成测试数据
	freopen("input", "w", stdout);
	const int kCmd_number = 10000;   //生成的命令数
	const int kKey_max = 1000;       //key的最大值
	const int kValue_max = 10000;    //value的最大值
	srand((unsigned)time(NULL));
	for (int i = 0; i < kCmd_number; ++i) {
		int cmd = rand() % 3;
		printf("%d", cmd);
		/*
		 * 0 store
		 * 1 delete
		 * 2 fetch
		 */
		if (!cmd) {
			int key = rand() % kKey_max;
			int value = rand() % kValue_max;
			int flag = rand() % 3 + 1;//范围在1-3之间
			printf("%d %d %d\n", key, value, flag);
		}
		else {
			int key = rand() % kKey_max;
			printf("%d\n", key);
		}
	}
}

template<typename T>
void check_result(T result1, T result2) {
	if (result1 != result2)
		printf("result is not same\n");
}

void test_output() {
	vDB::DB db;
	std::unordered_map<std::string, std::string> m;
	//db.db_open()
	/*
	 * 通过input文件的输入数据同时来操作db和unorder_map
	 * 对比他们的输出是否不一致
	 */
	freopen("input", "r", stdin);
	int cmd;
	while (~scanf("%d", &cmd)) {
		if (!cmd) {
			char key[4], value[5];
			int flag;
			scanf("%s%s%d", &key, value, flag);
			int db_result, map_result;
			/*
			 * store操作
			 * 对面map来说需要自己手动存
			 */
			db_result = db.db_store(key, value, flag);
			if (m.find(key) == m.end()) {
				m[key] = value;
				map_result = 0;
			}
			else
				map_result = 1;
			check_result<int>(db_result, map_result);
		}
		else {
			char key[4];
			scanf("%s", &key);
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
				check_result<bool>(db_result, map_result);
			}
			else {
				/*
				 * fetch操作
				 * 同样需要查找是否存在
				 */
				std::string db_result, map_result;
				db_result = db.db_fetch(key);
				if (m.find(key) == m.end())
					map_result = m[key];
				check_result<std::string>(db_result, map_result);
			}
		}
	}
	//db_close
}

int main() {
	
}