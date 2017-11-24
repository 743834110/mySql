
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
%nonassoc LE LT GT GE NE
%%

program:		{printf("mysql> ");}	stmts
	;

stmts:			
	|			stmts  stmt
	;
	

stmt:			'\n' 				{printf("mysql> ");}
	|			stmt_select ';'		{find_data();}
	|			error '\n'			{yyclearin;printf("mysql> ");}
	;


stmt_select:	SELECT sp columns sp FROM sp tables{col_tab_check();} sp where_part 
	;

sp://分隔符seperator	
	|			sp '\n'	{printf("    -> ");};
	;
	
columns:		'*'		{field_push("*", 1, COL);}
	|			_columns
	;

_columns:		column col_alias_part
	|			_columns ',' column	 col_alias_part
	;

column:			ID	{field_push($1, 1, COL);}
	|			ID 	{field_push($1, 1, TAB);} '.' ID {field_push(token -> ID, 0, NON);}
	;

col_alias_part:		
	|			alias		{field_push(token -> ID, 0, ALIAS);}
	|			AS alias	{field_push(token -> ID, 0, ALIAS);}
	;
	
alias:
				ID
	|			STR
	;


tables:			table				
	|			tables ',' table	
	;
	
table:			ID {table_push($1, TAB);}
	|			ID {table_push($1, TAB);} alias {table_push(token -> ID, ALIAS);}
	;

where_part:		
	|			WHERE expr
	;

expr:			column '=' value 	{cond_push_token('=',token);}	
	|			column '=' column	{cond_push_cond('=');}			
	|			column LIKE value ESCAPE '"' ID '"'
	|			expr AND expr		{code_binary(AND);}
	|			expr OR expr		{code_binary(OR);}
	|			column LT value		{cond_push_token(LT, token);}
	|			column LE value		{cond_push_token(LE, token);}
	|			column GT value		{cond_push_token(GT, token);}
	|			column GE value		{cond_push_token(GE, token);}
	|			column NE value		{cond_push_token(NE, token);}
	|			'(' expr ')'
	;

value:			STR		
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
	
		
