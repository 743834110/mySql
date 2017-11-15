#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "lex.h"
#include "mysql.tab.h"
#define BUFLEN 20

Token token;

typedef struct{
	char *ID;
	int sym; 
}Keyword; 

extern int LINENUM; 
extern int COLNUM;

// 用于检查是否为关键字的关键字到sym映射库，减少多重判断 
Keyword keywords[] = {
	{"select", SELECT},
	{"from", FROM},
	{"where", WHERE},
	{"as", AS},
	{"like", LIKE},
	{"escape", ESCAPE},
	{"or", OR},
	{"and", AND},
	{"NULL", 0} 	
};


int keyword_lookup(char *ID){
	Keyword *p = keywords;
	while(strcmp(p -> ID,ID) != 0 && strcmp(p -> ID,"NULL") != 0) // NON，NULL待统一 
		p ++;
	return p -> sym; 	
}

Token newToken(char *ID,int sym){
	Token token = (Token)malloc(sizeof(*token));
	token->ID = strdup(ID);
	token->sym = sym;
	return token;
}


int yylex(){

//	Token token = NULL;	//用的是全局变量token 
	int ch, ch2;  // ch2用于双字节运算符的判断 
	ch = getchar();
	while(ch == '\t' || ch == '\x20'){
		// 暂时不统计行号和列号 
		ch = getchar();
	}
	// 暂时不考虑下划线开头的标识符 ：以及字符数越界等问题 
	char buf[BUFLEN];
	char *p = buf;
	if (isalpha(ch)){  // 开头为字母的情况
		do{
			*p++ = ch; 
		
		} while(isalnum((ch = getchar())) && ch != EOF);
		*p = '\0';
		ungetc(ch, stdin);  // 回退字符串到输入流中 
		int sym = keyword_lookup(buf); 
		token = sym == 0?newToken(buf,ID):newToken(buf,sym); 

	}
	else if (isdigit(ch)){
		ungetc(ch, stdin); 
		double integer;
		// 为了兼容性，应该使用fsanf :return 读入参数的个数 
		//***已经不兼容   fscanf(file,"%d",&integer);
		scanf("%lf", &integer);
		//格式化数据，并写入到字符串当中 
		sprintf(buf,"%G",integer);
		token = newToken(buf,NUM); 
		//给token->integer赋值 
		token->integer = integer;	
	}
	else	
		switch(ch){
			case '<':{
				ch2 = getchar();
				if (ch2 == '=') 
					token = newToken("<=",LE);
				else{
					ungetc(ch2, stdin);
					token = newToken("<",LT);
				}
				break;
			}
			case '>':{
				ch2 = getchar();
				if (ch2 == '=') 
					token = newToken(">=",GE);  // 大于等于 
				else{
					ungetc(ch2, stdin);
					token = newToken(">",GT);  // 大于 
				}
				break;
			}
			case '!':{
				ch2 = getchar();
				if (ch2 == '=')
					token = newToken("!=", NE);
				else{
					ungetc(ch2, stdin);
					token = newToken("!",'!'); 
				} 
				break;
			}
			case EOF:
				token = newToken("",0);
				break;
			default:{
				char strs[2] = {ch, 0};
				token = newToken(strs, ch);
			}
		}

	return token->sym;
}

