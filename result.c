#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "result.h"
#include "dataTab.h"
#include "symtab.h"
#include "sql.tab.h"
#define RESULTROWSIZE 1000
#define RESULTCOLSIZE 1000

static void print();
static void make_condition();
static void output_result();
static void make_with_one_table();

//列栈
extern Meta col_metas[100];
extern int col_top ;

//表栈
extern Meta tab_metas[100];
extern int tab_top ;

//表达式栈
extern Cond conds[100];
extern int cond_top ;

//存放表信息的地方
extern Table *tables[100];
//表的索引
extern int _index;
//转存方便处理
static B_table b_tables[100];
//是否已经转存
static int isTrans = 0;
//结果索引数组,告诉系统结果的输出顺序
static OutputOrder outputOrder[1000];
//结果索引数组的长度
static int outputOrder_index = 0;
//输出中间结果
B_table medium;
static B_table media;//中间结果媒介 
static int isCopy = 0;
//中间结果的行数和列数
int medium_row;
int medium_col;

//两个中间结果的合并:and就是在合适中找，or就是在不合适中找
void merge(int op) {
	//if (op == AND)
		//printf("AND");
	int i, j;
	char* cmp_symbol = op == AND?"$":"$$";
	for (j = 1; j <= medium_row; j++) {
		if (strcmp(media.table[j][0].value,cmp_symbol) != 0)
			continue;
		//printf("%d\n", j);
		if (strcmp(medium.table[j][0].value, "$") == 0)
			media.table[j][0].value = "$";
		else
			media.table[j][0].value = "$$";
	}
	//保存中间结果
	memcpy(&medium, &media, sizeof(media));
	
}

//通过表的别名找到真名所在的索引
int getTab_nameByAlias(char* alias) {
	int i, count = 0;
	for (i = 0; i < tab_top; i++) {
		if (strcmp(alias, tab_metas[i].alias) == 0)
			return i;
	}
	return -1;
}

//将alias名字返回
char* getTableNameByCol(char* col_name) {
	int i = 0;
	for (; i < tab_top; i++) {
		
		Table* table = getTable(tab_metas[i].tab_name);
		int col = 0;
		for (; col < table -> col_num; col++) {
			Field field = table -> fields[col];
			if (strcmp(col_name, field -> col_name) == 0)
				return tab_metas[i].alias;
		}
	}
	return NULL;
}

//只是转换一次
static void transform() {
	if (isTrans == 1)
		return ;
	isTrans = 1;
	int i = 0;
	for (; i < _index; i++) {
		Table* table = tables[i];
		int col = 0;
		for (; col < table -> col_num; col++) {
			Field field = table -> fields[col];
			
			char buf[256];
			sprintf(buf, "%s.%s",table -> table_name,field -> col_name);
			b_tables[i].table[0][col + 1].value = strdup(buf);
			//printf("%-20s",buf);
			int row = 0;
			Value* value = NULL;
			for (; row < table -> row_num; row++) {	
				if (row == 0)
					value = field -> data;
				else 
					value = value -> next;
				b_tables[i].table[row + 1][col + 1].value = value -> value;
				
			}
		}
	}
	//int row = 0;
	//Table *table = tables[1];
	//B_table b_table = b_tables[1];
	//for (; row <= table -> row_num; row++) {
		//int col = 1;
		//for (; col <= table -> col_num; col++) {
			//printf("\t%s", b_table.table[row][col].value);
		//}
		//printf("\n");
	//}
	
}
//清空本次结果
void clear() {
	outputOrder_index = 0;
	isCopy = 0;
}

//整理列将要输出的顺序
static void make_column() {
	int i = 0;
	for (i = 0; i < col_top; i++) {
		char* tab_name = col_metas[i].tab_name;
		char* col_name = col_metas[i].col_name;
		char* alias = col_metas[i].alias ;
		char buf[256];
		if (*col_name != '*') {
			//处理key
			if (tab_name == NULL)  
				tab_name = getTableNameByCol(col_name);
			
			sprintf(buf, "%s.%s", tab_name, col_name);
			outputOrder[outputOrder_index].key = strdup(buf);
			//处理别名
			if (alias == NULL)
				alias = col_name;
			outputOrder[outputOrder_index++].alias = alias;	
		}
		//当为*号列为：
		else {
			if (tab_name == NULL) {
				int index = 0;
				for (index = 0; index < tab_top; index++)  {
					char* tab_name = tab_metas[index].tab_name;
					Table* table = getTable(tab_name);
					tab_name = tab_metas[index].alias;
					int col;
					for (col = 0; col < table -> col_num; col++) {
						Field field = table -> fields[col];
						char* col_name = field -> col_name;
						sprintf(buf, "%s.%s", tab_name, col_name);
						outputOrder[outputOrder_index].key = strdup(buf);
						//???????以前没有加
						//outputOrder[outputOrder_index].num = col + 1;
						//printf("%s\n", buf);
						//处理*号列的列名
						outputOrder[outputOrder_index++].alias = col_name;						
					}
				}
			}
			//列名为不为空时:
			else {
				Table* table = getTable(tab_name);
				int col = 0;
				for (col = 0; col < table -> col_num; col++) {
					Field field = table -> fields[col];
					char* col_name = field -> col_name;
					sprintf(buf, "%s.%s", tab_name, col_name);
					outputOrder[outputOrder_index].key = strdup(buf);
					//printf("%s\n", buf);
					//处理*号列的列名
					outputOrder[outputOrder_index++].alias = col_name;						
				}
				
			}
		}
	}
	//记录结果集合中的列的输出顺序:通过key进行分析
	for (i = 0; i < outputOrder_index; i++) {
		
		int col = 1;
		for (; col <= medium_col; col++) {
			if (strcmp(outputOrder[i].key, medium.table[0][col].value) == 0){
				outputOrder[i].num = col;
				continue;
			}
		}
		//printf("%s:%s\t", outputOrder[i].key, outputOrder[i].alias);
		//printf("出现的顺序为：%d\n", outputOrder[i].num);
	}
	
}

static void make_with_one_table() {
	
	char* tab_name = tab_metas[0].alias;
	int index = getIndexByTabName(tab_metas[0].tab_name);
	B_table b_table = b_tables[index];
	Table* table = tables[index];
	//printf("该表的索引值为：%d\n", index);
	int row, col;
	for (row = 0; row <= table -> row_num; row++) {
		medium.table[row][0].value = "$$";//$$表示不可以输出，$表示可输出
		for (col = 1; col <= table -> col_num; col++) {
			if (row == 0) {
				Field field = table -> fields[col - 1];
				char* col_name  = field -> col_name;
				char buf[256];
				sprintf(buf, "%s.%s", tab_name, col_name);
				medium.table[row][col].value = strdup(buf);
			}
			else {
				medium.table[row][col] = b_table.table[row][col];
			} 
		}
	}
	//为中间结果的显示赋予明显的界限
	medium_row = table -> row_num;
	medium_col = table -> col_num;
	//printf("%d:%d\n",medium_row, medium_col);
}

void generateMediumResult() {
	
	//printf("cond_top:%d\n", cond_top);
	Cond tmp[10];//最多存10个临时条件
	int change = 0;
	//数据转换
	if (!isCopy) {
		transform();
	}
	//根据tab_metas输出结果集
	//整理列:的输出顺序
	outputOrder_index = 0;
	make_with_one_table();
	make_column();
	//根据条件进行相关处理
	//printf("-----------n------------\n");
	//printf("cond_top = %d\n", cond_top);
	//printf("-----------n------------\n");
	//select* from income where (age>40 or age<30) and (job = "教师");
	//cond_top > 3的要进行元素的切换。
	if (cond_top > 3) {
		int i;
		change = cond_top - 3;//要切换的元素的位置
		for (i = 0; i < change; i++)
			tmp[i] = conds[i];
		for (i = change; i < cond_top; i++) 
			conds[i - change] = conds[i];
		cond_top = cond_top - change;
	}
	make_condition();	
	//output_result();
	//保存旧结果,为新的中间结果申请空间
	if (!isCopy){
		memcpy(&media, &medium, sizeof(medium));
		isCopy = 1;
	}
	//调整被运算条件的位置
	if (change == 0)
		cond_top = 0;
	else {
		int i ;
		for (i = 0;i < change; i++)
			conds[i] = tmp[i];
		cond_top = change;
	}
}

static void make_with_tables() {
	
}

//static void make_result() {
//	int i = 0;
	//大于2的采用另一种表的构造方式
//	if (tab_top >= 2)
//		make_with_tables();
//	else
//		make_with_one_table();
//}

static int getIndexInMedium(char* buf) {
	int col = 1;
	for (; col <= medium_col; col++) {
		if (strcmp(buf, medium.table[0][col].value) == 0)
			return col;
	}
	return -1;
}
//查看列值是否是数字
static int isInteger(char* source){
	int length = strlen(source), i;
	for (i = 0; i < length; i++) {
		 if (!isdigit(source[i]))
		 	return 0;
	}
	return 1;

}

//cmp_style比较类型有0字符串类型比较和1数值比较
//
static int compare(int op, char* sour, char* dest, int cmp_style) {
	
	//printf("sour:%s\n", sour);
	//printf("dest:%s\n", dest);
	//printf("cmp_style:%d\n", cmp_style);
	int sour_num;
	int dest_num;
	int result = 0;
	if (cmp_style) {
		sour_num = atoi(sour);
		dest_num = atoi(dest);
		//printf("sour_value:%d\n", sour_num);
		//printf("desc_value:%d\n", dest_num);
	}
	switch(op) {
		case '=':{
			if (cmp_style){
				if (sour_num == dest_num)
					result = 1;
				else
					result = 0;
			}
			else{
				if (strcmp(sour, dest) == 0)
					result = 1;
				else
					result = 0;
			}	
			break;
		}
		case NE:{
			//printf("不相等");
			if (cmp_style){
				if (sour_num != dest_num)
					result = 1;
				else
					result = 0;
			}
			else{
				if (strcmp(sour, dest) != 0)
					result = 1;
				else
					result = 0;
			}	
			break;			
		}
		case LT: {
			if (cmp_style){
				if (sour_num < dest_num)
					result = 1;
				else
					result = 0;
			}
			else{
				if (strcmp(sour, dest) < 0)
					result = 1;
				else
					result = 0;
			}	
			break;	
		}
		case LE: {
			if (cmp_style){
				if (sour_num <= dest_num)
					result = 1;
				else
					result = 0;
			}
			else{
				if (strcmp(sour, dest) <= 0)
					result = 1;
				else
					result = 0;
			}	
			break;				
		}
		case GT: {
			if (cmp_style){
				if (sour_num > dest_num)
					result = 1;
				else
					result = 0;
			}
			else{
				if (strcmp(sour, dest) > 0)
					result = 1;
				else
					result = 0;
			}	
			break;				
		}
		case GE: {
			if (cmp_style){
				if (sour_num >= dest_num)
					result = 1;
				else
					result = 0;
			}
			else{
				if (strcmp(sour, dest) >= 0)
					result = 1;
				else
					result = 0;
			}	
			break;			
		}
	}
	return result;
}

//目标列匹配：把不符合条件的跳出来
//or就是在不合适($$)中找合适的，and就是在合适($)中找合适的
static void target_match(int target_col, Cond cond, int range) {
	int i = 0;
	char* cmp_symbol = range == 0?"$$":"$";
	int cmp_style = 0 ;//比较类型有0字符串类型比较和1数值比较
	//1代表列的数据类型为数值
	int isDigit = isInteger(medium.table[1][target_col].value);
	//printf("fffffffff%d\n", isDigit);
	for (i = 1; i <= medium_row; i++){
		//先进行过滤
		if (strcmp(cmp_symbol, medium.table[i][0].value) != 0)
			continue;
		char* value = medium.table[i][target_col].value;
		//printf("%s\n",value);
		Token token = cond.value;
		if (isDigit){
			if (token -> sym == STR && isInteger(token -> ID))
				cmp_style = 1;
			else if (token -> sym == NUM)
				cmp_style = 1;
		}
		if (compare(cond.op, medium.table[i][target_col].value,token -> ID, cmp_style))
			//成功匹配时,将状态为变为$,否则$$
			medium.table[i][0].value = "$";
		else
			medium.table[i][0].value = "$$";
	}
}

//处理条件:过滤掉连接语句
static void make_condition() {
	int i = 0;
	int range = 0;//0为or操作, 1为and操作
	for (; i < cond_top; i++) {
		//暂时就这样处理:and or操作处理
		if (conds[i]._meta != NULL)
			continue;
		if (conds[i].op == AND){
			range = 1;
			continue;
		}
		else if (conds[i].op == OR){
			range = 0;
			continue;
		}
		//printf("cond.token:%s\n", conds[i].value -> ID);
		char buf[256];//实时表名的生成
		char* tab_name = conds[i].meta.tab_name;
		char* col_name = conds[i].meta.col_name;
		if (tab_name == NULL )
			tab_name = getTableNameByCol(col_name);
		sprintf(buf, "%s.%s", tab_name, col_name);
		int target_col = getIndexInMedium(buf);
		target_match(target_col, conds[i], range);	
	}
}
//结果输出:通过判断状态位进行相关输出
static void output_result() {
	int row = 1,i;
	//输出表头
	printf("\n");
	for (i = 0; i < outputOrder_index; i++) 
		printf("\t%-5s", outputOrder[i].alias);
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
		for (index = 0; index < outputOrder_index; index++) {
			int temp = outputOrder[index].num;
			printf("\t%-5s", medium.table[row][temp].value);
		}
		printf("\n");
	}
	
	printf("\n                                  共查询到%d条记录\n", count);
	printf("    -----------------------------------------\n");
}

void pre_do(){
	//printf("fff");
	if (isCopy == 0) {
		transform();
		make_with_one_table();	
		make_column();	
	}
	if (isCopy == 0)
		make_condition();
	else if (cond_top == 2) {
		memcpy(&medium, &media, sizeof (media));
		make_condition();
	}
	else //小bug
		cond_top = 1;
}

//结果输出调用函数:利用表达式栈，表栈，列栈
void common_search(){
	//结果预处理
	pre_do();
	//结果输出
	output_result();
	clear();
}

