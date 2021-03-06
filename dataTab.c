#include "IOUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "dataTab.h"

static char* config_file_path = "config/config.ini";
static FILE* table_files[100];//表的所在文件路径指针。
static char* table_name[100];//表名表
int _index = 0;//表的索引(size)
extern int LINENUM;//用于构建表
Table *tables[100];//创建的表指针数组:将会被广泛使用
//临时文件，存放select后的文本信息文件
static char* temp_txt = "../temp.txt"; 
static Table* temp_table = NULL;
FILE* temp_file = NULL;


//检查文件名,查看是否有重复表名
int build(char* words){

	//取得真正的表名字
	int i = 0;
	char* temp = strchr(words, '.');
	if (temp != NULL){
		int pos = temp - words;
		temp = strdup(words);
		temp[pos] = '\0';
	}
	//当没有后缀时
	else
		temp = words;
	//查询是否有重复的表名(首先去除后缀)
	for (; i < _index; i++){
		if (strcmp(table_name[i], temp) == 0){
			//printf("config.ini 第%d行, 表名重复\n", _index + 1);
			//exit(1);
		}
	}
	//不重复时把表明加入到表名表中
	table_name[_index] = temp;
	return 1;
}

void init(){
	//字典树的根节点:打开配置文件,读取目标路径
	//root = (node) malloc(sizeof(*root));
	setSource(fopen(config_file_path, "r"));
	while(hasNextChar()){
		//取得目标文件名(去除前缀)
		char* temp = getString();
		char *pos = strrchr(temp, '/');
		pos = pos == NULL? temp: pos + 1;
		//验证是否可用。
		if (build(pos)){
			FILE* file = fopen(temp, "r");
			vadidate(file);
			table_files[_index++] = file;
		}
	}
}

static void init_column(Field field){
	field -> col_name = NULL;
	field -> alias = NULL;
	field -> data = NULL;
	field -> type = INPUT_STRING;
	field -> R = NULL;
}
//初始化表
static void init_table(Table *table, int index){

	table -> table_name = table_name[index];
	table -> alias = NULL;
	table -> col_num = 0;
	table -> row_num = 0;
}

static Value* new_node(char* _value){

	Value* node = (Value*)malloc(sizeof(*node));
	node -> value = _value;
	node -> next = NULL;
	return node;
}

//遍历表结构
static void ergodic_col (Table* table){
	int i = 0;
	for (; i < table -> col_num; i++) {
		Field field = table -> fields[i];
		printf("列%d:\t%s\n", i + 1 ,field -> col_name);
	}
}

//填充数据
static void fill_data(Table *table){
	
	while (hasNextChar()){
		int j = table -> col_num;
		int i = LINENUM,k = 0;
		//当行号相同时才进行数据同行数据的读取
		//printf("%d", LINENUM);
		while (LINENUM == i && hasNextChar()){
		
			Value* node = new_node(getString());
			//printf("%s", node -> value);
			Value* T = table -> fields[k] -> data;
			Value* R;
			if (T == NULL){
				table -> fields[k] -> data = node;
				table -> fields[k] -> R = node;
				T = node;
			}
			else{
				R = table -> fields[k] -> R;
				R -> next = node;
				table -> fields[k] -> R = node;
			}
			k++;
		}
		if (k != table -> col_num)
			errExit("列属性量不匹配");
		//调整行的数目
		table -> row_num++;
	}

}

//构建列
static int create_column(Table *table){
	
	skip();//跳过空格换行符和制表符
	while(LINENUM == 1 && hasNextChar()){
	
		Field field = (Field)malloc(sizeof(*field));
		init_column(field);
		field -> col_name = getString();
		table -> fields[table -> col_num ++] = field;
	}
	if(table -> col_num == 0)
		errExit("表结构期待在文本第一行出现");
}

void create_table(){
	
	init();	//载入config.ini,填充表名,打开文件,读取内容。
	printf("共有%d张表:\n", _index);
	int i;
	for (i = 0; i < _index; i++){
		
		tables[i] = (Table*)malloc(sizeof(*tables[i]));
		init_table(tables[i], i);//初始化表结构
		setSource(table_files[i]);//设置将要读取文件源
		printf("\t表名：%s\n", tables[i]->table_name);
		create_column(tables[i]);//根据文件源建立表结构
		printf("\t\t构建%s表结构成功.\n", tables[i] -> table_name);
		ergodic_col(tables[i]);//遍历表结构
		//开始填充数据表数据
		fill_data(tables[i]);//根据文件源对该数据结构进行数据填充
		printf("\t\t填充%s表数据成功\n", tables[i] -> table_name);
	}
}

//将字符串输出到文本文件中
void outputResultset(char* msg) {
	if (temp_file == NULL)
		temp_file = fopen(temp_txt, "w+");
	if (temp_file == NULL)
		printf("dfdfdf");
	fprintf(temp_file, "%s\t", msg);
}

void newLine(){
	if (temp_file == NULL)	
		printf("临时文件尚未打开");
	fprintf(temp_file,"\n");	
}

void close_file() {
	if (temp_file == NULL)
		printf("临时文件尚未打开");
	fclose(temp_file);
	temp_file = NULL;
}

//返回一张临时表
Table* new_table(){
	
	if (temp_file == NULL)	
		temp_file = fopen(temp_txt, "r");
	temp_table = (Table*)malloc(sizeof(*temp_table));
	init_table(temp_table, 0);
	temp_table -> table_name = "temp";
	setSource(temp_file);
	create_column(temp_table);
	//ergodic_col(temp_table);
	fill_data(temp_table);
	close_file();
	return temp_table;
}

//通过索引找到某表
Table* getTable(char* tab_name){
	int i = 0;
	for (; i < _index; i++) {
		Table* tmp = tables[i];
		if (strcmp(tab_name, tmp -> table_name) == 0)
			return tmp;
	}
	return NULL;
}
//通过某表查找某段
Field getField(Table* table, char* col_name){

	int i = 0;
	for (i = 0; i < table -> col_num; i++) {
		Field field = table -> fields[i];
		//printf("%s == %s\n", field -> col_name, col_name);
		if (strcmp(field -> col_name, col_name) == 0)
			return field;
	}
	return NULL;
}
//更新某段某行的信息:这里可能会越界
void update_field_by_row(int row, Field field,char* val) {

	int i = 0;
	Value* value = field -> data;
	for (; i < row; i++)
		value = value -> next;
	free(value -> value);
	value -> value = strdup(val);
}


//查找失败返回-1,查找成功返回争取的索引值
int getIndexByTabName(char* tab_name) {
	int i = 0;
	for (; i < _index; i++) {
		Table* tmp = tables[i];
		if (strcmp(tab_name, tmp -> table_name) == 0)
			return i;
	}
	return -1;
}

//根据表名查找目标表名是否存在
int isExistsTable(char* table_name){
	int i = 0;
	for (; i < _index; i++){
		if (strcmp(table_name, tables[i] -> table_name) == 0)
			return 1;
	}
	return 0;
}

//检查列名是否已经存在或者存在多个相同的列名
//0代表不存在相关列, 1代表只有一个相关列, 其余代表存在多个相关列
int isExistsCol(char* table_name, char* col_name){
	int index = 0, i;
	for (i = 0; i < _index; i++) {
		Table* table = tables[i];//得到其中一个表的指针
		
		if (strcmp(table_name, table -> table_name) != 0)
			continue;
		//printf("第%d张表的表名称为:%s\n", i, table -> table_name);
		int j = 0;
		// 遍历每一张表的
		for (; j < table -> col_num; j++) {
			Field field = table -> fields[j];
			if (strcmp(col_name, field -> col_name) == 0)
				index++;
		}
	}
	//printf("比较情况:index = %d\n", index);
	return index ;
}

void write_cols(char* tab_name) {
	//printf("将要打印的表名%s\n", tab_name);
	Table* table = getTable(tab_name);
	int i = 0;
	for (; i < table -> col_num; i++) {
		Field field = table -> fields[i];
		outputResultset(field -> col_name);
	}
}

void insert_value_in_field(Field field,char* col_value) {

	Value* p = (Value*)malloc(sizeof(*p));
	p -> next = NULL;
	p -> value = strdup(col_value);
	//后插法
	field -> R -> next = p;
	field -> R = p;
}

//void main(){
//	create_table();
//}
