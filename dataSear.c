#include <stdio.h>
#include "dataSear.h"
#include "dataTab.h"

extern Table *tables[100];

void find_all_data(char* table_name){
	
	int k;
	Table *table = tables[0];
	Value* node = table -> fields[2] -> data;
	for (k = 0; k < table -> row_num; k++){
		printf("%s\n", node -> value);
		node = node -> next;
	}
	printf("\n");
}
