#ifndef RESULT_H_
#define RESULT_H_

//定义结果集结构体
typedef struct {
	char* value;
}item;

typedef struct {
	item table[1000][1000];
} B_table;

typedef struct {
	int num;
	char* key;
	char* alias;
}OutputOrder;

void clear();
void pre_do();
void common_search();
int getTab_nameByAlias(char* alias);
void generateMediumResult();
void merge(int op);//两个括号结果的合并
#endif
