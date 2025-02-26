#ifndef JSONTOKENIZER_H
#define JSONTOKENIZER_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<stdarg.h>

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

/* Error handling function */
const char* get_tokenizer_error();

struct JSONTokenNode* createNode(char *value, enum JSONTokenType t);
struct JSONTokenList* initTokenList();
struct JSONTokenNode* nextToken(struct JSONTokenNode *n);
int readTokenString(struct JSONTokenList *l, char *jsonString, int *curPos, size_t len);
int readTokenNumber(struct JSONTokenList *l, char *jsonString, int *curPos, size_t len);
int readTokenKeyword(struct JSONTokenList *l, char *jsonString, int *curPos, size_t len);
int appendTokenToList(struct JSONTokenList *l, struct JSONTokenNode *t);
void freeTokenList(struct JSONTokenList *l);
void printTokenList(struct JSONTokenList *l);
struct JSONTokenList* buildTokenList(const char *f, size_t len);

#endif
