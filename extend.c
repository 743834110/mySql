#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include "extend.h"
#include "dataTab.h"
#include "dataSear.h"
#include "lex.h"
#include "IOUtil.h"

extern Table *tables[100];
extern int _index;
//设置终端输入源
extern FILE* input_file;

void desc(char* tab_name){
	if (!isExistsTable(tab_name)) 
		printf("ERROR : Table '%s' doesn't exist\n", tab_name);
	else {
		Table* table = getTable(tab_name);
		int i = 0;
		printf("    +-------------------\n");
		printf("    |   Field\n");
		printf("    +-------------------\n");
		for (i = 0;  i < table -> col_num; i++) {
			Field field = table -> fields[i];
			printf("    |  %s\n", field -> col_name);
		}
		printf("    +-------------------\n");
	}
}
//显示
void show_tables(){
	int i = 0;
	printf("    +--------------------------------------------------+\n");
	printf("    | %15s\t%15s\t%15s|\n", "table_name","col_num", "row_num");
	printf("    +--------------------------------------------------+\n");
	for (i = 0; i < _index; i++){
		Table* table = tables[i];
		printf("    |  %15s\t%15d\t%15d|\n", table -> table_name, table -> col_num, table -> row_num);
		printf("    +--------------------------------------------------+\n");
	}
}

void edit() {
	system("gedit config/Edit.txt");
}

void bind_var() {
	input_file = fopen("config/Edit.txt", "r");
	printf("Edit.txt中代码执行情况如下：\n");
}

