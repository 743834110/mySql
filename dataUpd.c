#include <stdio.h>
#include <string.h>
#include "dataUpd.h"
#include "symtab.h"
#include "dataTab.h"
#include "result.h"

extern Meta tab_metas[100];
extern int tab_top;
extern int cond_top;
extern int showResult;
extern B_table medium;
extern int medium_row;
extern int medium_col;

static Table* table;
static void update_result(char* col_name, Token token){


	int row = 1;
	printf("\n    -----------------------------------------\n");
	//数据
	int count = 0;//统计
	for (; row <= medium_row; row++) {
		int index;
		//printf("cond_top = %d\n", cond_top);
		if (cond_top == 0 && strcmp(medium.table[row][0].value, "$$") != 0)
			continue;
		else if (cond_top != 0 && strcmp(medium.table[row][0].value, "$$") == 0)
			continue;
		count++;
		printf("更新的是第%d行数据\n", row);
		Field field = getField(table, col_name);
		update_field_by_row(row - 1, field, token -> ID);
		//printf("\n");
	}
	
	printf("\n                                  共更新到%d条记录\n", count);
	printf("    -----------------------------------------\n");	
}

//此前已有表名进栈：所以每次操作完毕就需要清空一下表栈
void update( char* col_name, Token token) {
	int success = 0;
	char* tab_name = tab_metas[0].tab_name;
	//printf("%d\n", tab_top);
	table = getTable(tab_name);
	if (isExistsCol(tab_name, col_name) == 0) 
		printf("所更新指定的列名'%s'无效\n", col_name, tab_name);
	else {
		//根据搜索结果进行结果的更新
		//printf("%s\n", buf);
		pre_do();//目标结果集进行预处理
		update_result(col_name, token);//根据结果集的状态位进行相应的列修改
	}
	
	
	
	//restore
	clear();
	cond_top = 0;
	tab_top = 0;
	showResult = 1;
}
