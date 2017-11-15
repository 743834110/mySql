#ifndef LEX_H_
#define LEX_H_
//extern FILE *file;
//typedef enum{
//	NON, LP, RP, LC, RC, COMMA, SEMI, ASSIGN, ID, NUM, INT,  // 空，左圆括号，右圆括号，左花括号，右花括号,逗号，分号，等于号，变量名，整形常量， 整型
//	IF, ELSE, WHILE, PRINT, RETURN, READ, NOT,  // READ 同scanf
//	ADD, SUB, MUL, DIV, LT, LE, GT, GE, EQ, NE,  // +, -, *, /, >, >=, <, <=, ==, != 
//	END 
//}SYMBOL;

 typedef struct token{
	char *ID;
	int sym;
	double integer;
} *Token;

//Token getToken();
int yylex();
void errExit(char *msg);
extern Token token;

#endif

