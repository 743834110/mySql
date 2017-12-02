#ifndef SYMTAB_H_
#define SYMTAB_H_
#include "lex.h"

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
	Token value;	// 这个表达式的值(字符串或者)
	Meta* _meta;	//判断非值连接时使用
}Cond;


void field_push(char* name, int status, int category);
void table_push(char* name, int category);
void find_data();
void col_tab_check();
void cond_check();
void code_binary(int sym);
void cond_push_token (int op, Token token);//弹一次栈
void cond_push_cond (char op);//弹两次栈
#endif
