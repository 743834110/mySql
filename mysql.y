
%{
	#include <stdio.h>
	#include "lex.h"
	#include "dataSear.h"
	#include "dataTab.h"
	
	int yyerror(char* msg);
%}

%union{
	double 	VALUE_NUM;
	char* 	VALUE_STRING;
}

%token <VALUE_NUM>NUM	<VALUE_STRING>ID <VALUE_STRING>STR
%token SELECT FROM WHERE AS LIKE ESCAPE
%nonassoc OR
%nonassoc AND
%left LE LT GT GE NE
%%

program:		{printf("mysql> ");}	stmts
	;

stmts:			
	|			stmts  stmt
	;
	

stmt:			'\n' 				{printf("mysql> ");}
	|			stmt_select ';'		{find_all_data("");}
	|			error '\n'			{yyclearin;printf("mysql> ");}
	;


stmt_select:	SELECT sp columns sp FROM sp tables sp where_part 
	;

sp://分隔符seperator	
	|			sp '\n'	{printf("    -> ");};
	;
	
columns:		'*'
	|			_columns
	;

_columns:		column col_alias_part
	|			_columns ',' column	 col_alias_part
	;

column:			ID	{$1 = token -> ID;}
	|			ID 		'.' ID	{$1 = token -> ID;}
	;

col_alias_part:		
	|			ID
	|			AS ID
	;

tables:			table
	|			tables ',' table
	;
	
table:			ID
	|			ID ID
	;

where_part:		
	|			WHERE expr
	;

expr:			ID '=' value
	|			ID LIKE value ESCAPE '"' ID '"'
	|			expr AND expr
	|			expr OR expr
	|			ID LT value
	|			ID LE value
	|			ID GT value
	|			ID GE value
	|			ID NE value
	|			'(' expr ')'
	;
		

value:			'"' ID '"'
	|			NUM
	;

%%

int yyerror(char *msg){
	printf("%s\n", msg);
	return 0;
}

int main(){
	create_table();
	return yyparse();
}
	
		
