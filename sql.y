
%{
	#include <stdio.h>
	#include <stdlib.h>
	#include "lex.h"
	#include "symtab.h"
	#include "dataTab.h"
	#include "extend.h"
	#include "result.h"
	#include "dataUpd.h"
	#include "dataIns.h"
	
	int yyerror(char* msg);
%}

%union{
	double 	VALUE_NUM;
	char* 	VALUE_STRING;
	Token TOKEN;
}

%type <TOKEN> value
%token <VALUE_NUM>NUM	<VALUE_STRING>ID <VALUE_STRING>STR
%token SELECT FROM WHERE AS LIKE ESCAPE DESC TABLES SHOW ED CLEAR LOGOUT UPDATE SET INSERT INTO VALUES
%nonassoc OR
%nonassoc AND
%nonassoc LE LT GT GE NE
%%

program:		{printf("mysql> ");}	stmts
	;

stmts:			
	|			stmts  stmt
	;
	

stmt:			'\r'
	|			'\n' 					{printf("mysql> ");}
	|			stmt_select ';'	'\n'	{find_data();}
	|			stmt_update ';'	'\n'	{}
	|			stmt_insert ';'	'\n'	{clear_of_dataIns();}
	|			SHOW TABLES	'\n'		{show_tables();}
	|			DESC ID '\n'			{desc($2);}
	|			ED	'\n'				{edit();}
	|			'@'						{bind_var();}
	|			CLEAR 					{system("clear");}
	|			LOGOUT '\n'				{printf("退出成功\n");exit(0);}
	|			error '\n'				{yyerrok;printf("mysql> ");}
	;

stmt_insert:	INSERT INTO insert_part values_part
	;
	
values_part:	VALUES '('values ')'					{insert();}
	|			values_part ',' VALUES '('values ')'	{insert();}
	;

values:			value					{add_token($1);}
	|			values ',' value		{add_token($3);}
	;

insert_part:	table
	|			table '(' _columns ')'	{col_tab_check();}
	;
	
stmt_update:	UPDATE table SET ID '=' value where_part	{update($4, $6);}
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
	|			column LIKE value ESCAPE '"' ID '"'
	|			expr AND expr		{code_binary(AND);}
	|			expr OR expr		{code_binary(OR);}
	|			column LT value		{cond_push_token(LT, token);}
	|			column LE value		{cond_push_token(LE, token);}
	|			column GT value		{cond_push_token(GT, token);}
	|			column GE value		{cond_push_token(GE, token);}
	|			column NE value		{cond_push_token(NE, token);}
	|			'(' expr ')'		{generateMediumResult();}
	;

value:			STR		{$$ = token;}
	|			NUM		{$$ = token;}
	|			ID		{$$ = token;}
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
	
		
