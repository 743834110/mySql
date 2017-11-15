#ifndef IOUTIL_H_
#define IOUTIL_H_

#include <stdio.h>
int getNumber();
char* getString();
void setSource(FILE *file);
int hasNextChar();
void vadidate(FILE* _file);
int check_comment(int ch);
void errExit(char *msg);
void skip();
char getChar();
void ungetChar();
#endif
