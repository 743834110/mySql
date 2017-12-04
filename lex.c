#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "lex.h"
#include "sql.tab.h"
#include "IOUtil.h"
#define BUFLEN 256

Token token;

typedef struct{
	char *ID;
	int sym; 
}Keyword; 

extern int LINENUM; 
extern int COLNUM;

FILE* input_file = NULL;

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
	{"show",SHOW},
	{"tables", TABLES},
	{"desc", DESC},
	{"ed", ED},
	{"clear", CLEAR},
	{"logout", LOGOUT},
	{"update", UPDATE},
	{"set", SET},
	{"insert", INSERT},
	{"into",INTO},
	{"values", VALUES},
	{"NULL", 0} 	
};

static int getCh(){
	if (input_file == NULL)
		input_file = stdin;
	return getc(input_file);
}

static void ungetCh(int ch){
	ungetc(ch, input_file);
}

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
	ch = getCh();
	while(ch == '\t' || ch == '\x20' ){
		// 暂时不统计行号和列号 
		
		ch = getCh();
	}
	if (ch == -1){
		if (input_file != stdin){
			input_file = stdin;
			return '\r';
		}
		else{
			
			return 0;
		}
	}
	// 暂时不考虑下划线开头的标识符 ：以及字符数越界等问题 
	char buf[BUFLEN];
	char *p = buf;
	//支持中文输入:当前字符于0x80的与运算和下一个字符与0x80的运算的结果大于１
	if (isalpha(ch) || ch&0x80){  // 开头为字母的情况
		do{
			*p++ = ch; 
		
		} while((isalnum((ch = getCh())) || ch == '_' || ch&0x80 ) && ch != EOF);
		*p = '\0';
		ungetCh(ch);  // 回退字符串到输入流中 
		int sym = keyword_lookup(buf); 
		token = sym == 0?newToken(buf,ID):newToken(buf,sym); 
		//新加的面对ID被跳过的解决方案之一
		if (sym == 0){
			yylval.VALUE_STRING = strdup(buf);
		}
	}
	//检测返回带“‘号内的内容
	else if (ch == '"'){
		while ((ch = getCh()) != '"' && ch != EOF && ch != ';' && ch != '\n')
			*p++ = ch;
		//正常结束时
		if (ch == '"'){
			*p = '\0';
			token = newToken(buf, STR);
		}
		//非正常结束时:有下方的switch语句处理EOF(会产生错误)
		else{
			
		}
	}
	else if (isdigit(ch)){
		ungetCh(ch); 
		double integer;
		// 为了兼容性，应该使用fsanf :return 读入参数的个数 
		fscanf(input_file,"%lf",&integer);
		//scanf("%lf", &integer);
		//格式化数据，并写入到字符串当中 
		sprintf(buf,"%G",integer);
		token = newToken(buf,NUM); 
		//给token->integer赋值 
		token->integer = integer;	
	}

	else	
		switch(ch){
			case '<':{
				ch2 = getCh();
				if (ch2 == '=') 
					token = newToken("<=",LE);
				else{
					ungetCh(ch2);
					token = newToken("<",LT);
				}
				break;
			}
			case '>':{
				ch2 = getCh();
				if (ch2 == '=') 
					token = newToken(">=",GE);  // 大于等于 
				else{
					ungetCh(ch2);
					token = newToken(">",GT);  // 大于 
				}
				break;
			}
			case '!':{
				ch2 = getCh();
				if (ch2 == '=')
					token = newToken("!=", NE);
				else{
					ungetCh(ch2);
					token = newToken("!",'!'); 
				} 
				break;
			}
			case EOF: {
				token = newToken("",0);
				break;
			}
			default:{
				//if (ch == ';') {
					//input_file = stdin;
				//}
				char strs[2] = {ch, 0};
				token = newToken(strs, ch);
			}
		}
	//printf("%d\n", token -> sym);
	
	return token->sym;
}

