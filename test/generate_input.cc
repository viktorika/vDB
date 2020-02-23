#include "../include/v_db.h"
#include <cstdio>
#include <string>
#include <functional>
#include <unordered_map>
#include <fcntl.h>
#include <iostream>

void generate_input() {
	//Éú³É²âÊÔÊý¾Ý
	freopen("input", "w", stdout);
	const int kCmd_number = 10000;   //Éú³ÉµÄÃüÁîÊý
	const int kKey_max = 1000;       //keyµÄ×î´óÖµ
	const int kValue_max = 10000;    //valueµÄ×î´óÖµ
	srand((unsigned)time(NULL));
	for (int i = 0; i < kCmd_number; ++i) {
		int cmd = rand() % 3;
		printf("%d ", cmd);
		/*
		 * 0 store
		 * 1 delete
		 * 2 fetch
		 */
		if (!cmd) {
			int key = rand() % kKey_max;
			int value = rand() % kValue_max;
			int flag = rand() % 3 + 1;//·¶Î§ÔÚ1-3Ö®¼ä
			printf("%d %d %d\n", key, value, flag);
		}
		else {
			int key = rand() % kKey_max;
			printf("%d\n", key);
		}
	}
}

int main(){
	generate_input();	
}
