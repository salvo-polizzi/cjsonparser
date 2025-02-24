#include "jsontokenizer.h"

static struct JSONTokenNode* createEofNode() {
	struct JSONTokenNode *n = malloc(sizeof(struct JSONTokenNode));
        n->token.type = END;
        n->next = NULL;
        return n;
}

struct JSONTokenNode* createNode(char *value, enum JSONTokenType t) {
	struct JSONTokenNode *n = malloc(sizeof(struct JSONTokenNode));
	n->token.value = malloc(strlen(value)+1);
	strcpy(n->token.value, value);
	n->token.type = t;
	n->next = NULL;
	return n;
}

struct JSONTokenList* initTokenList() {
	struct JSONTokenList *l = malloc(sizeof(struct JSONTokenList));
	l->head = NULL;
	return l;
}

struct JSONTokenNode* nextToken(struct JSONTokenNode *n) {
    if (!n || !n->next) return NULL;
    return n->next;
}

void readTokenString(struct JSONTokenList *l, char *jsonString, int *curPos){
	int start = *curPos + 1;
	(*curPos)++;
	while(jsonString[*curPos] != '"'){
		(*curPos)++;
	}
	int end = *curPos - 1;
	(*curPos)++;

        if (end < start) { /* empty string */
            appendTokenToList(l, createNode("", STRING));
            return;
        }

	char valueString[end-start+2];
	strncpy(valueString,jsonString+start,end-start+1);
	valueString[end-start+1] = '\0';
	appendTokenToList(l,createNode(valueString,STRING));
}

void readTokenNumber(struct JSONTokenList *l, char *jsonString, int *curPos){
	int start = *curPos;
	(*curPos)++;
	while(
		isdigit(jsonString[*curPos]) || 
		jsonString[*curPos] == 'e' || 
		jsonString[*curPos] == 'E' ||
		jsonString[*curPos] == '.' ||
		jsonString[*curPos] == '+' ||
		jsonString[*curPos] == '-'
	) {
		(*curPos)++;
	}
	int end = *curPos - 1;
	char valueNumber[end-start+2];
	strncpy(valueNumber,jsonString+start,end-start+1);
	valueNumber[end-start+1] = '\0';
	appendTokenToList(l,createNode(valueNumber,NUMBER));
}


void readTokenKeyword(struct JSONTokenList *l, char *jsonString, int *curPos) {
	int start = *curPos;
	(*curPos)++;
	while(jsonString[*curPos] >= 'a' && jsonString[*curPos] <= 'z') {
		(*curPos)++;
	}
	int end = *curPos - 1;
	char valueKeyword[end-start+2];
	strncpy(valueKeyword,jsonString+start,end-start+1);
	appendTokenToList(l,createNode(valueKeyword,KEYWORD));
}

void appendTokenToList(struct JSONTokenList *l, struct JSONTokenNode *t) {
	if(!l->head) {
		l->head = t;
		return;
	}	
	struct JSONTokenNode *tmp = l->head;
	while(tmp->next) {
		tmp = tmp->next;
	}
	tmp->next = t;
}

void printTokenList(struct JSONTokenList *l) {
	struct JSONTokenNode *t = l->head;
	int numPrint = 0;
	while(t && numPrint < 20) {
		printf("(%s,%d), ", t->token.value, t->token.type);
		t = t->next;
		numPrint++;
	}
}

struct JSONTokenList* buildTokenList(const char *f, size_t len) {
	struct JSONTokenList *l = initTokenList(); 
	int current = 0;
	while (current < len) {
		char c =  f[current];

		//finite state machine
		switch(c) {
			case ' ':
			case '\n':
			case '\t':
			case '\r':
				current++;
				break;
			case '{':
				appendTokenToList(l,createNode("{", OPEN_CURLY_BRACKET));
				current++;
				break;
			case '}':
				appendTokenToList(l,createNode("}", CLOSE_CURLY_BRACKET));
				current++;
				break;
			case '[':
				appendTokenToList(l,createNode("[", OPEN_SQUARE_BRACKET));
				current++;
				break;
			case ']':
				appendTokenToList(l,createNode("]", CLOSE_SQUARE_BRACKET));
				current++;
				break;
			case ',':
				appendTokenToList(l,createNode(",", COMMA));
				current++;
				break;
			case ':':
				appendTokenToList(l,createNode(":", COLON));
				current++;
				break;
			case '.':
				appendTokenToList(l,createNode(".", PERIOD));
				current++;
				break;
			case '"':
				readTokenString(l,f,&current);
				break;
			case '0' ... '9':
				readTokenNumber(l,f,&current);
				break;
			default:
				readTokenKeyword(l,f,&current);
				break;
		}
	}
        appendTokenToList(l,createEofNode());
	//printTokenList(l);
        return l;
}
