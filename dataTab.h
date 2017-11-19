#ifndef DATATAB_H_
#define DATATAB_H_

enum Type{
	INPUT_INTEGER, INPUT_STRING
};

typedef struct _Value{
	char* value;
	struct _Value *next;
}Value;

typedef struct _field{

	char* col_name;
	char* alias;
	enum Type type;
	Value *R;//数据的跟跑指针
	Value *data;
}*Field;

typedef struct _table{
	char* table_name;
	char* alias;
	int col_num;//列的数目
	int row_num;//行的数目
	Field fields[100];//列首指针
}Table;

void create_table();
int isExistsTable(char* table_name);
int isExistsCol(char* table_name,char* col_name);

#endif
