#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "dataSear.h"
#include "dataTab.h"
#include "mysql.tab.h"
#include "IOUtil.h"
#include "result.h"

//列栈
 Meta col_metas[100];
 int col_top = 0;

//表栈
 Meta tab_metas[100];
 int tab_top = 0;

//表达式栈
 Cond conds[100];
 int cond_top = 0;

//0表示所写的语句误,无结果输出,1表示有输出
static int showResult = 1;

//检查表名是否存在:初步检查,确保表的正确存在性
static int tab_check(char* tab_name,char* alias) {
	int result = 1;
	if (alias == NULL) {
		if (!isExistsTable(tab_name)) {
			fprintf(stderr, "表名%s不可用\n", tab_name);
			result = 0;
		}
	}
	else {
		int i = 0,count = 0;
		for (; i < tab_top; i++) {
			if (strcmp(tab_name, tab_metas[i].alias) == 0) 
				count++;
		}
		if (count == 2 || count == 0)
			return 0;
	}
	return result;
}

//检查表的别名的重复性
static int alias_tab_check(char* alias){
	int i = 0,count = 0;
	
	for (; i < tab_top; i++) {
		if (tab_metas[i].alias == NULL)
			continue;
		if (strcmp(tab_metas[i].alias, alias) == 0)
			count++;
	}
	if (count == 0)
		return 1;
	else
		return 0;
		
}

static int col_check(char* tab_name, char* col_name){
	int col_repeat_count = 0, tab_i = 0;
	if (tab_name != NULL) {
		//找一下列名有没有出现在表名内
		if (!tab_check (tab_name,"g")) {
			showResult = 0;
			printf("所指定的表名在语句当中不合法\n");
		}
		else {
			int index = getTab_nameByAlias(tab_name);
			//if (*col_name == '*')
			//	return 1;
			//printf("%s\n", tab_metas[index].tab_name);
			if (isExistsCol(tab_metas[index].tab_name, col_name))
				col_repeat_count++;
			if (col_repeat_count == 0) {
				showResult = 0;
				printf("在所指定的列%s在给出的表%s中未出现相关列\n", col_name, tab_name);
			}
			
		}		
	}
	else {
		for (; tab_i < tab_top; tab_i++) {
			char* table_name = tab_metas[tab_i].tab_name;
			if (*col_name == '*')//当列名为*号的是否则不必进行合格性检查
				return 1;
			if (isExistsCol(table_name, col_name))
				col_repeat_count++;
			//printf("%s\t%s\n", table_name, col_name);		
		}
		//当列名不出现在任何表结构时
		if (col_repeat_count == 0) {
			showResult = 0;
			printf("在所指定的列%s在给出的表中未出现相关列\n", col_name);
		}
		//检查有没有表名字制定	
		else if (col_repeat_count >=2 ) {
			showResult = 0;
			printf("列%s出现在多个表中,请为其指定所属\n", col_name);
		}
	}
	return showResult;
}

//检查所出现的列对于表名是否合法
static int col_to_tab_check(){
	
	int col_i = 0, tab_i = 0;
	 for (; col_i < col_top; col_i++){
	//可能会出现多个相同名字的列在多个表中出现,这时候检查有没有别名或者表名字制定

		//当有表前缀时：只进行一次搜索就结束本次循环
		//当没有表前缀时,按照此循环走
		char* table_name = col_metas[col_i].tab_name;
		char* col_name = col_metas[col_i].col_name;
		col_check(table_name, col_name);
	}
}

//处理结果:table.* 没有处理
void code_binary(int sym){
	Cond cond;
	cond.op = sym;
	//printf("conds:%d\n", cond_top);
	Cond cond2 = conds[--cond_top];
	conds[cond_top++] = cond;
	conds[cond_top++] = cond2;
	
}

//表达式进栈_token
void cond_push_token (int op, Token token) {
	//把meta从col_metas中弹出
	if (showResult == 0)
		return;
	
	Meta meta = col_metas[--col_top];
	//printf("key:%s.%s\n", meta.tab_name, meta.col_name);
	printf("op:%d\n", op);
	printf("showResult:%d\n", showResult);
	printf("col_top:%d\n", col_top);
	//printf("value:%s\n", token -> ID);
	//检查meta信息的合法性
	char* tab_name  = meta.tab_name;
	char* col_name = meta.col_name;
	
	if (!col_check(tab_name, col_name))
		return ;
	Cond cond ;
	cond.op = op;
	cond.value = token;
	cond.meta = meta;
	cond._meta = NULL;
	conds[cond_top++] = cond;
}

//自然、不等连接用
void cond_push_cond (char op) {
	Meta _meta = col_metas[--col_top];
	Meta meta = col_metas[--col_top];
	
	//检查meta的合法性
	char* tab_name  = meta.tab_name;
	char* col_name = meta.col_name;
	if (!col_check(tab_name, col_name))
		return ;
	tab_name = _meta.tab_name;
	col_name = _meta.col_name;
	if (!col_check(tab_name, col_name))
		return ;
	Cond cond;
	cond.op = op;
	cond.meta = meta;
	cond._meta = &_meta;//取地址，然后将该结构体首地址给cond._meta
}

//查询列名和表名的正确性:最后关口
void col_tab_check () {
	
	//当前面检查有错时,下面就不再运行
	if (showResult == 0)
		return;
	col_to_tab_check();
	//int i = 0;
	//for (i = 0; i < col_top; i++) {
		//printf("表名:%s\n", col_metas[i].tab_name);
		//printf("列名:%s\n", col_metas[i].col_name);
		//printf("别名:%s\n", col_metas[i].alias);
	//}

}

//0表示为要新建一个metaData类型的数据, 1表示已经存在了不需要进行新建操作

static Meta newMeta(int sym, char* tab_name,char* col_name){
	Meta meta;
	if (sym == SELECT || sym == FROM) {
	
		meta.tab_name = tab_name;
		meta.col_name = col_name;
		meta.alias = NULL;
	}
	else if (sym == WHERE) {
		
	}
	else {
		printf("误操作\n");
	}
	return meta;
}

void field_push(char* name, int status,int category){
	Meta meta_data;
	//printf("%s\n", name);
	
	if (status == 1){
		if (category == COL){
			meta_data = newMeta(SELECT, NULL, name);
		}
		else {
			//if (tab_check (TAB, name)) {
				meta_data = newMeta(SELECT, name, NULL);
				//printf("ff%s", meta_data.tab_name);
			//}
		}
		col_metas[col_top++] = meta_data;
	}
	// 检查带前缀的列名是否存在
	else if (category == NON){
		meta_data = col_metas[--col_top];
		meta_data.col_name = name;
		
		col_metas[col_top++] = meta_data;
		//printf("带前缀的表%s的列名的表名为：%s\n", meta_data.tab_name, meta_data.col_name);
	}
	else {
		meta_data = col_metas[--col_top];
		meta_data.alias = name;
		col_metas[col_top++] = meta_data;
	}
}

void table_push(char* name,int category){
	
	Meta meta_data;
	if (category == TAB) {
		//检查表名可用
		if (tab_check (name, NULL)) {	
			meta_data = newMeta(FROM, name, NULL);
			meta_data.alias = name;//别名就是列名
			
		}
		else{
			showResult = 0;
			//printf("fdfdfdfdfdfdfdf\n");
		}
	}
	else {
		meta_data = tab_metas[--tab_top];
		meta_data.alias = name;
		
		//printf("%s表的别名:%s\n", meta_data.tab_name,meta_data.alias);
	}
	tab_metas[tab_top++] = meta_data;
}

void find_data(){
	//首先要清空栈里面的内容，保证下次交互的正确性
	
	if (showResult == 0){
		showResult = 1;
		col_top = tab_top = cond_top = 0;
		return ;	
	}
	common_search();
		
	col_top = tab_top = cond_top = 0;
//	int k;
//	Table *table = tables[0];
//	Value* node = table -> fields[2] -> data;
//	for (k = 0; k < table -> row_num; k++){
//		printf("%s\n", node -> value);
//		node = node -> next;
//	}
}

