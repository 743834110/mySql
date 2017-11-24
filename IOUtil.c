#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "IOUtil.h"
#define BUFLEN 2000

int LINENUM = 1;                        // 行号 
int COLNUM = 0;                         // 列号 
char buff[BUFLEN];                      // 从文件中读取的字符的数目
char *pos = buff + BUFLEN;          // 指向当前
int size = 0;                               //当前缓冲区读入字符的 
int lastChar;
FILE *file = NULL;

//用于获取下一个字符，并来统计当前所读字符所在的行号和列号 
char getChar(){
	if (pos == buff + BUFLEN ){  //表示缓冲区已经数据已经使用完毕了，需要重新读取文件到缓冲区中 
		pos = buff;
		size = fread(buff,1,BUFLEN,file); 	 
		if (size != BUFLEN){}
			//fclose(file);
	} 
	if (pos < buff + size  && pos < buff + BUFLEN){
		if (*pos == '\n'){//原来是lastChar
			LINENUM ++;
			COLNUM = 0;
		}
		COLNUM ++;
		lastChar = *pos ;
		return *pos++;	
	}
	 
	return EOF;  // 文件数据读完，将返回-1 
}
//将已读字符退回到缓冲区中 
void ungetChar(){
	pos--; // 指针回退，表示将字符协会 	
	
	lastChar = *(pos - 1);	   // 上一个字符也要跟着回退 
	//如果当前指针所指向的字符为‘\n’为保证行号的正确性，LINENUM应减去1
	COLNUM--;
	if (*pos == '\n')		//原来是lastChar
		LINENUM--;
}

void skip(){
	char tmp = getChar();
	while(tmp == '\x20' || tmp == '\n' || tmp == '\t' || tmp == 10 || tmp == 13){
		tmp = getChar();
	}
	if (tmp != EOF){
		ungetChar();
		
	}
}

int hasNextChar(){
	skip();
	char ch = getChar();
	if (ch != -1){
		ungetChar();
		return 1;	
	}
	else 
		return 0;
}

int getNumber(){
	int var = 0;
	char ch;
	while((ch = getChar()) != EOF && isdigit(ch) )
		var  = var * 10 + ch - '0';
	ungetChar();
	return var;
}

char* getString(){
	skip();
	char buf[BUFLEN];
	char *p = buf;
	char tmp = getChar();
	while (tmp != ' ' && tmp != '\n' && tmp != EOF && tmp != '\t'){
		*p++ = tmp;
		tmp = getChar();
	}
	//ungetChar();
	*p = '\0';
	return strdup(buf);
}

void setSource(FILE *_file){
	//重置缓冲区。
	LINENUM = 1;
	COLNUM = 0;
	pos = buff + BUFLEN;
	size = 0;
	file = _file;
	lastChar = -1;
}

void vadidate(FILE *_file){
	if (_file == NULL){
		printf("config.ini　第%d行:表路径配置有误\n", LINENUM);
		exit(1);
	}
}

// 检查注释，将注释当做空白字符来处理 
int check_comment(int ch){
	if (ch != '/')
	 	return 0;  //0代表为不用过滤
	char ch2 = getChar(); 
	if (ch2 == '/'){  // 得知为单行注释，对所在行出现的任何字符做跳过处理  
		 
		 while((ch2 = getChar()) != EOF && ch2 != '\n');
		  return 1;
	}
	else if (ch2 == '*'){  //检查得知为多行注释 
		for (ch = getChar(); ch != EOF; ch = getChar()){
			if (ch == '*'){  //检查到‘*’多行注释，进入检查结束状态 
				if (getChar() == '/')
					return 1; 
			}
		} 
		return 1;
	}
	else{  //词法分析器不检查语法的错误,但送到语法分析器中时保证送去的token的正确性（错误的token会被忽略掉） 
		ungetChar();
		return 0;
	}
} 

void errExit(char *msg){
	fprintf(stderr,"%s,在就近第%d行,第%d列\n",msg,LINENUM,COLNUM);
	exit(1);
}


//void main(){
//	FILE *file = fopen("dirlist.txt", "r");
//	setSource(file);
//	char* string;
//	int index;
//	while (hasNextChar()){
		
//		printf("%d:%s\n",LINENUM,getString());
//	}
//}
