#ifndef JSONTOKENIZER_H
#define JSONTOKENIZER_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

enum JSONTokenType{
	//separators
	OPEN_CURLY_BRACKET, CLOSE_CURLY_BRACKET,
	OPEN_SQUARE_BRACKET, CLOSE_SQUARE_BRACKET,       
	COMMA, COLON, PERIOD,
	//number
	NUMBER,
	//string
	STRING,
	//keywords
	KEYWORD,

        //END OF TOKENS
        END
};

struct JSONToken{
	char *value;
	enum JSONTokenType type;
};

struct JSONTokenNode{
	struct JSONToken token;
	struct JSONTokenNode *next;	
};

struct JSONTokenList{
	struct JSONTokenNode *head;
};

struct JSONTokenNode* createNode(char *value, enum JSONTokenType t);
struct JSONTokenList* initTokenList();
struct JSONTokenNode* nextToken(struct JSONTokenNode *n);
void readTokenString(struct JSONTokenList *l, char *jsonString, int *curPos);
void readTokenNumber(struct JSONTokenList *l, char *jsonString, int *curPos);
void readTokenKeyword(struct JSONTokenList *l, char *jsonString, int *curPos);
void appendTokenToList(struct JSONTokenList *l, struct JSONTokenNode *t);
void printTokenList(struct JSONTokenList *l);
struct JSONTokenList* buildTokenList(const char *f, size_t len);

#endif
