#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dataIns.h"
#include "symtab.h"
#include "dataTab.h"

extern Meta col_metas[100];
extern Meta tab_metas[100];
extern int col_top;
extern int tab_top;
extern int showResult;
//存放将要插入的值:列的最大值为100
Token tokens[100];
int tokens_top = 0;
static int count = 0;//更新的记录数目

void clear_of_dataIns(){
	printf("\n    ----------------------------------\n\n");
	printf("\t\t\t\t成功插入%d条记录\n", count);
	printf("    ----------------------------------\n");
	clear_token();
	col_top = 0;
	tab_top = 0;
	showResult = 1;
	count = 0;
}

void add_token(Token token){

	tokens[tokens_top++] = token;
}

void clear_token() {
	while (tokens_top != 0)
		free(tokens[--tokens_top]);
}

static void insertBytokens(Table* table) {
	int i = 0;
	for (i = 0; i < table -> col_num; i++) {
		Field field = table -> fields[i];
		int j = 0, isInsert = 0;
		for (j = 0; j < col_top; j++) {
			if (strcmp(field -> col_name, col_metas[j].col_name) == 0) {
				insert_value_in_field(field,tokens[j] -> ID);
				isInsert = 1;
				break;
			}
		}
		//没有被选择插入的列插入null值
		if (!isInsert){
			insert_value_in_field(field,"");
		}
	}
	//更新表的行总数
	table -> row_num += 1;
	count++;
	
}
//输入插入的顺序和列原本的顺序有关
static void insert_order_by_table(Table* table){
	int i = 0;
	//当类数目与要求相匹配的时候择进行数据的插入的操作
	for (i = 0; i < table -> col_num; i++) {
		Field field = table -> fields[i];
		insert_value_in_field(field,tokens[i] -> ID);
	}
	//更新表的行总数
	table -> row_num += 1;
	count++;
}

void insert(){
	//前面检查出现问题时，则没有后面的操作
	if (!showResult)
		return ;
	if (col_top  != 0 && col_top != tokens_top){
		printf("\t列数量不相匹配,请检查\n");
		clear_token();//不影响下次操作的正确性
		return ;
	}

	Table* table = getTable(tab_metas[0].tab_name);
	if (col_top == 0) {
		if (table -> col_num != tokens_top){
			printf("\t列数量不相匹配,请检查\n");
			clear_token();
			return ;
		}
		insert_order_by_table(table);
	}
	else {
		insertBytokens(table);
	}
	clear_token();
}


