#ifndef DATASEAR_H_
#define DATASESR_H_

enum {
	COL, TAB,
	ALIAS, NON
};

typedef struct {
	char* alias;		// 别名
	char* col_name;		// 列名
	char* tab_name;		// 表名
}Meta;

typedef struct {
	Meta meta;	// 只是列的名字或者是带表名的类的名字
	int op;			// 条件判断的符号
	char* value;	// 这个表达式的值(字符串或者)
}Cond;



void find_data(char* table_name);
void field_push(char* name, int status, int category);
void table_push(char* name, int category);
void col_tab_check();

#endif
