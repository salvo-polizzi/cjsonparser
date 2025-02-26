#include "jsontokenizer.h"

/* Global error message buffer */
static char error_buffer[256] = {0};

/* Sets the current error message and returns 0 (error code) */
static int set_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(error_buffer, sizeof(error_buffer) - 1, format, args);
    va_end(args);
    return 0;
}

/* Gets the last error message */
const char* get_tokenizer_error() {
    return error_buffer;
}

/* Safe memory allocation with error handling */
static void* safe_malloc(size_t size, const char *error_context) {
    void *ptr = malloc(size);
    if (!ptr) {
        set_error("Memory allocation failed for %s", error_context);
    }
    return ptr;
}

/* Creates a new token node representing the end of file */
static struct JSONTokenNode* createEofNode() {
    struct JSONTokenNode *n = safe_malloc(sizeof(struct JSONTokenNode), "EOF token node");
    if (!n) return NULL;
    
    n->token.type = END;
    n->token.value = NULL; /* No value needed for EOF token */
    n->next = NULL;
    return n;
}

/* Creates a new token node with the specified value and type */
struct JSONTokenNode* createNode(char *value, enum JSONTokenType t) {
    struct JSONTokenNode *n = safe_malloc(sizeof(struct JSONTokenNode), "token node");
    if (!n) return NULL;
    
    if (value) {
        n->token.value = safe_malloc(strlen(value) + 1, "token value");
        if (!n->token.value) {
            free(n);
            return NULL;
        }
        strcpy(n->token.value, value);
    } else {
        n->token.value = NULL;
    }
    
    n->token.type = t;
    n->next = NULL;
    return n;
}

/* Initializes a new, empty token list */
struct JSONTokenList* initTokenList() {
    return safe_malloc(sizeof(struct JSONTokenList), "token list");
}

struct JSONTokenNode* nextToken(struct JSONTokenNode *n) {
    if (!n || !n->next) return NULL;
    return n->next;
}

/* Parses a JSON string token from the input */
int readTokenString(struct JSONTokenList *l, char *jsonString, int *curPos, size_t len) {
    if (*curPos >= len) {
        return set_error("Unexpected end of input while reading string");
    }
    
    int start = *curPos + 1;  /* Skip opening quotation mark */
    (*curPos)++;
    
    /* Find end of string */
    while (*curPos < len && jsonString[*curPos] != '"') {
        /* Handle escape sequences */
        if (jsonString[*curPos] == '\\') {
            (*curPos)++;
            if (*curPos >= len) {
                return set_error("Unterminated string: unexpected end after escape character");
            }
        }
        (*curPos)++;
    }
    
    if (*curPos >= len) {
        return set_error("Unterminated string: reached end of input");
    }
    
    int end = *curPos - 1;
    (*curPos)++;  /* Skip closing quotation mark */

    /* Create string token */
    int length = end - start + 1;
    char *valueString = NULL;
    
    if (length > 0) {
        valueString = safe_malloc(length + 1, "string token value");
        if (!valueString) return 0;
        
        strncpy(valueString, jsonString + start, length);
        valueString[length] = '\0';
    } else {
        /* Empty string */
        valueString = safe_malloc(1, "empty string token");
        if (!valueString) return 0;
        
        valueString[0] = '\0';
    }
    
    struct JSONTokenNode *node = createNode(valueString, STRING);
    free(valueString); /* createNode makes a copy */
    
    if (!node) return 0;
    if (!appendTokenToList(l, node)) {
        free(node->token.value);
        free(node);
        return 0;
    }
    
    return 1;
}

/* Parses a JSON number token from the input */
int readTokenNumber(struct JSONTokenList *l, char *jsonString, int *curPos, size_t len) {
    if (*curPos >= len) {
        return set_error("Unexpected end of input while reading number");
    }
    
    int start = *curPos;
    int hasDecimal = 0;
    int hasExponent = 0;
    int hasDigits = 0;
    
    /* Handle negative sign */
    if (jsonString[*curPos] == '-') {
        (*curPos)++;
        if (*curPos >= len || !isdigit(jsonString[*curPos])) {
            return set_error("Invalid number format: - must be followed by digits");
        }
    }
    
    /* Read integer part */
    if (*curPos < len && jsonString[*curPos] == '0') {
        hasDigits = 1;
        (*curPos)++;
        /* Leading zero must not be followed by another digit */
        if (*curPos < len && isdigit(jsonString[*curPos])) {
            return set_error("Invalid number format: leading zero must not be followed by another digit");
        }
    } else {
        /* Read 1-9 followed by optional digits */
        while (*curPos < len && isdigit(jsonString[*curPos])) {
            hasDigits = 1;
            (*curPos)++;
        }
    }
    
    if (!hasDigits) {
        return set_error("Invalid number format: no digits found");
    }
    
    /* Read decimal part */
    if (*curPos < len && jsonString[*curPos] == '.') {
        hasDecimal = 1;
        (*curPos)++;
        
        /* Must have at least one digit after decimal */
        hasDigits = 0;
        while (*curPos < len && isdigit(jsonString[*curPos])) {
            hasDigits = 1;
            (*curPos)++;
        }
        
        if (!hasDigits) {
            return set_error("Invalid number format: no digits after decimal point");
        }
    }
    
    /* Read exponent part */
    if (*curPos < len && (jsonString[*curPos] == 'e' || jsonString[*curPos] == 'E')) {
        hasExponent = 1;
        (*curPos)++;
        
        /* Handle optional sign in exponent */
        if (*curPos < len && (jsonString[*curPos] == '+' || jsonString[*curPos] == '-')) {
            (*curPos)++;
        }
        
        /* Must have at least one digit in exponent */
        hasDigits = 0;
        while (*curPos < len && isdigit(jsonString[*curPos])) {
            hasDigits = 1;
            (*curPos)++;
        }
        
        if (!hasDigits) {
            return set_error("Invalid number format: no digits in exponent");
        }
    }
    
    /* Extract and create the token */
    int length = *curPos - start;
    char *valueNumber = safe_malloc(length + 1, "number token value");
    if (!valueNumber) return 0;
    
    strncpy(valueNumber, jsonString + start, length);
    valueNumber[length] = '\0';
    
    struct JSONTokenNode *node = createNode(valueNumber, NUMBER);
    free(valueNumber); /* createNode makes a copy */
    
    if (!node) return 0;
    if (!appendTokenToList(l, node)) {
        free(node->token.value);
        free(node);
        return 0;
    }
    
    return 1;
}

int readTokenKeyword(struct JSONTokenList *l, char *jsonString, int *curPos, size_t len) {
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

/* Adds a token to the end of the list */
int appendTokenToList(struct JSONTokenList *l, struct JSONTokenNode *t) {
    if (!l) {
        return set_error("NULL token list provided");
    }
    
    if (!t) {
        return set_error("NULL token node provided");
    }
    
    if (!l->head) {
        l->head = t;
        return 1;
    }
    
    struct JSONTokenNode *tmp = l->head;
    while (tmp->next) {
        tmp = tmp->next;
    }
    tmp->next = t;
    return 1;
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
    if (!f) {
        set_error("NULL input string provided");
        return NULL;
    }
    
    struct JSONTokenList *l = initTokenList();
    if (!l) return NULL;
    
    int current = 0;
    while (current < len) {
        char c = f[current];

        switch(c) {
            case ' ':
            case '\n':
            case '\t':
            case '\r':
                current++;
                break;
                
            case '{':
            case '}':
            case '[':
            case ']':
            case ',':
            case ':':
            case '.': {
                enum JSONTokenType type;
                char token_str[2] = {c, '\0'};
                
                switch (c) {
                    case '{': type = OPEN_CURLY_BRACKET; break;
                    case '}': type = CLOSE_CURLY_BRACKET; break;
                    case '[': type = OPEN_SQUARE_BRACKET; break;
                    case ']': type = CLOSE_SQUARE_BRACKET; break;
                    case ',': type = COMMA; break;
                    case ':': type = COLON; break;
                    case '.': type = PERIOD; break;
                }
                
                struct JSONTokenNode *node = createNode(token_str, type);
                if (!node || !appendTokenToList(l, node)) {
                    set_error("Failed to create token for '%c'", c);
                    freeTokenList(l);
                    return NULL;
                }
                current++;
                break;
            }
                
            case '"':
                if (!readTokenString(l, f, &current, len)) {
                    freeTokenList(l);
                    return NULL;
                }
                break;
                
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (!readTokenNumber(l, f, &current, len)) {
                    freeTokenList(l);
                    return NULL;
                }
                break;
                
            default:
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                    if (!readTokenKeyword(l, f, &current, len)) {
                        freeTokenList(l);
                        return NULL;
                    }
                } else {
                    set_error("Unexpected character '%c' at position %d", c, current);
                    freeTokenList(l);
                    return NULL;
                }
                break;
        }
    }
    
    struct JSONTokenNode *eofNode = createEofNode();
    if (!eofNode || !appendTokenToList(l, eofNode)) {
        set_error("Failed to create EOF token");
        freeTokenList(l);
        return NULL;
    }
    
    return l;
}

/* Free all resources used by a token list */
void freeTokenList(struct JSONTokenList *l) {
    if (!l) return;
    
    struct JSONTokenNode *current = l->head;
    struct JSONTokenNode *next;
    
    while (current) {
        next = current->next;
        if (current->token.type != END && current->token.value) {
            free(current->token.value);
        }
        free(current);
        current = next;
    }
    
    free(l);
}
