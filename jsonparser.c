#include "jsonparser.h"
#include <stdlib.h>
#include <string.h>

static char last_error[256] = {0};

/* keeping a reference to the current node of the list */
static struct JSONTokenNode *curNode;

/* forward declaration of parse_value */
static int parse_value(json_value *v);

/* Full definition of the structure */
struct json_value {
    int type;
    union {
        int boolean;
        double number;
        char *string;
        struct {
            json_value **items;
            size_t count;
            size_t capacity;
        } array;
        struct {
            char **keys;
            json_value **values;
            size_t count;
            size_t capacity;
        } object; /* TODO IMPLEMENT OBJECT WITH HASH MAPS */
    } u;
};

/* Utility function to set the error message */
static void json_set_last_error(const char *msg) {
    strncpy(last_error, msg, sizeof(last_error) - 1);
    last_error[sizeof(last_error) - 1] = '\0';
}

/* Utility function to safe allocate the json_value */
static json_value* safeJsonMalloc() {
    json_value *val = malloc(sizeof(json_value));
    if (!val) {
        fprintf(stderr, "Failed to allocate json_value\n");
        exit(EXIT_FAILURE);
    }
    return val;
}

/*
static int copyString(char *dest, char *source) {
    dest = malloc(strlen(source)+1);
    if (!dest) {
        fprintf(stderr, "failed to allocate string to copy\n");
        return 0;
    }
    strcpy(dest, source);
    return 1;
}

static int copyJsonArray(json_value *dest, json_value *source) {
    dest->u.array.items = calloc(source->u.array.count, sizeof(json_value *));
    dest->u.array.count = source->u.array.count;

    for (int i = 0; i < dest->u.array.count; i++) {
        dest->u.array.items[i] = safeJsonMalloc();
        *dest->u.array.items[i] = *source->u.array.items[i];
    }
    return 1; 
}

static void copyJsonValue(json_value *dest, json_value *source) {
    dest = malloc(sizeof(json_value));
    if (!dest) {
        fprintf(stderr, "failed to allocate json_value\n");
        return 0;
    }

    switch (source->type) {
        case JSON_BOOLEAN:
            dest->u.boolean = source->u.boolean;
            break;
        case JSON_NUMBER:
            dest->u.number = source->u.number;
            break;
        case JSON_STRING:
            copyString(dest->u.string, source->u.string);
            break;
        case JSON_ARRAY:
            copyJsonArray(dest, source);
            break;
        case JSON_OBJECT:
            copyJsonObject();
            break;
        default:
            break;
    }
}
*/

/** Consume a token of a specific type
 * return 1 if it is of the specified type
 * return 0 otherwise
 *
 * When in the right of a non-terminal there are more than one possible
 * productions like:
 * 
 * value := object | array | number | string 
 *
 * This way you have the possibility to consume one of those
 */
static int consumeToken(enum JSONTokenType t) {
    if (curNode->token.type == t) {
        curNode = nextToken(curNode);
        return 1;
    }
    return 0;
}

static int expectToken(enum JSONTokenType t) {
    if(!consumeToken(t)) {
        fprintf(stderr, "expectTokenError: token = \"%s\", expectedType:%d\n", curNode->token.value, t);
        json_set_last_error("Unexpected token error\n");
        return 0;
    }
    return 1;
}

static int parse_keyword(json_value *v) {
    if (curNode->token.type != KEYWORD) return 0;

    if (strcmp(curNode->token.value, "true") == 0 || 
        strcmp(curNode->token.value, "false") == 0) {
        v->type = JSON_BOOLEAN;
        v->u.boolean = strcmp(curNode->token.value, "true") == 0 ? 1 : 0;
    } else if (strcmp(curNode->token.value, "null") == 0) {
        v->type = JSON_NULL;
    }

    curNode = nextToken(curNode);
    return 1;
}

static int parse_string(json_value *v) {
    if (curNode->token.type != STRING) return 0;

    if (strcmp(curNode->token.value, "true") == 0 || 
        strcmp(curNode->token.value, "false") == 0) {
        v->type = JSON_BOOLEAN;
        v->u.boolean = strcmp(curNode->token.value, "true") == 0 ? 1 : 0;
    } else if (strcmp(curNode->token.value, "null") == 0) {
        v->type = JSON_NULL;
    } else {
        v->type = JSON_STRING;
        v->u.string = malloc(strlen(curNode->token.value)+1);
        strcpy(v->u.string, curNode->token.value);
    }

    curNode = nextToken(curNode);
    return 1;
}

static int parse_number(json_value *v) {
    if (curNode->token.type != NUMBER) return 0;
    v->type = JSON_NUMBER;
    v->u.number = atof(curNode->token.value);

    curNode = nextToken(curNode);
    return 1;
}

/* Parsing following the rules for JSON objects
 *
 * object -> '{' members '}'
 * members -> member | member ',' members
 * member -> string ':' value */
static int parse_object(json_value *v) {
    /* expect '{' at the start of an object */
    if (!consumeToken(OPEN_CURLY_BRACKET)) return 0;
    
    v->type = JSON_OBJECT;

    /* iterate at least one time for a single object */
    do {
        /* parse the key */
        char *key;
        if (curNode->token.type != STRING) return 0; 

        /* the key exists */
        key = malloc(strlen(curNode->token.value)+1);
        if (!key) {
            fprintf(stderr, "Failed to allocate key object\n");
            exit(EXIT_FAILURE);
        }
        strcpy(key, curNode->token.value);
        curNode = nextToken(curNode);
        
        /* expect ':' */
        if(!expectToken(COLON)) {
            free(key);
            return 0;
        }

        /* parse the object value */
        json_value *obj_val = safeJsonMalloc(); 
        if (!parse_value(obj_val)) {
            fprintf(stderr, "failed to parse value for key \"%s\"\n", key);
            return 0;
        }
        if (!json_object_set(v, key, obj_val)) {
            fprintf(stderr, "failed to allocate object value for key \"%s\"\n", key);
            return 0;
        }
    } while (consumeToken(COMMA));

    /* expect '}' at the end of the object */
    if (!expectToken(CLOSE_CURLY_BRACKET)) return 0;
    return 1;
}

/* Parsing following JSON array structure
 *
 * array -> '[' elements ']'
 * elements -> value | value ',' elements */
static int parse_array(json_value *v) {
    /* array starts with '[' */
    if(!consumeToken(OPEN_SQUARE_BRACKET)) return 0;

    v->type = JSON_ARRAY;

    /* iterate for at least one element */
    do {
       json_value *item = safeJsonMalloc();
       if(!parse_value(item)) return 0;
       if(!json_array_append(v, item)) return 0;
    } while (consumeToken(COMMA));

    if (!expectToken(CLOSE_SQUARE_BRACKET)) return 0;

    return 1;
}

static int parse_value(json_value *v) {
    if (parse_object(v) || parse_array(v) || parse_string(v) || parse_number(v) || parse_keyword(v)) 
        return 1;
    json_set_last_error("parse_value: not defined object\n");
    return 0;
}


/* Parses a JSON-formatted string and returns a pointer to a json_value tree.
 * Returns NULL if parsing fails.
 */
json_value *json_parse(const char *json_text) {
    struct JSONTokenList *l = buildTokenList(json_text, strlen(json_text)); 
    curNode = l->head;

    json_value *v = safeJsonMalloc();
    if(!parse_value(v)) {
        json_free(v);
        return NULL;
    }

    return v; 
}

/**
 * Serializes a json_value into a JSON string.
 * The caller is responsible for freeing the returned string.
 */
char *json_serialize(const json_value *value) {
}

/**
 * Frees a json_value and all its children.
 */
void json_free(json_value *value) {
    if(!value) return;

    switch (value->type) {
        case JSON_STRING:
            free(value->u.string);
            break;
        case JSON_ARRAY:
            for (int i = 0; i < value->u.array.count; i++) {
                json_free(value->u.array.items[i]);
            }
            free(value->u.array.items);
            break;
        case JSON_OBJECT:
            for (int i = 0; i < value->u.object.count; i++) {
                free(value->u.object.keys[i]);
                json_free(value->u.object.values[i]);
            }
            free(value->u.object.keys);
            free(value->u.object.values);
            break;
    }
    free(value);
}

json_value *json_new_null() {
    json_value *val = safeJsonMalloc();
    val->type = JSON_NULL;
    return val;
}

json_value *json_new_boolean(int boolean) {
    json_value *val = safeJsonMalloc();
    val->type = JSON_BOOLEAN;
    val->u.boolean = boolean;
    return val;
}

json_value *json_new_number(double number) {
    json_value *v = safeJsonMalloc();
    v->type = JSON_NUMBER;
    v->u.number = number;
    return v;
}

json_value *json_new_string(const char *string) {
    if(!string) return NULL;
    json_value *v = safeJsonMalloc();
    v->u.string = malloc(strlen(string)+1);
    v->type = JSON_STRING;
    strcpy(v->u.string, string);
    return v;
}

json_value *json_new_array(void) {
    json_value *v = malloc(sizeof(json_value));
    if (!v) return NULL;
    v->type = JSON_ARRAY;
    v->u.array.items = NULL;
    v->u.array.count = 0;
    return v;
}

json_value *json_new_object(void) {
    json_value *v = malloc(sizeof(json_value));
    if (!v) return NULL;
    v->type = JSON_OBJECT;
    v->u.object.keys = NULL;
    v->u.object.values = NULL;
    v->u.object.count = 0;
    return v;
}

char *json_get_string(const json_value *value) {
    if(value->type != JSON_STRING) {
        json_set_last_error("value is not of type JSON_STRING\n");
        return NULL;
    }
    return value->u.string;
}

double json_get_number(const json_value *value) {
    if (value->type != JSON_NUMBER) {
        json_set_last_error("value is not of type JSON_NUMBER\n");
        return 0;
    }
    return value->u.number;
}

uint8_t json_get_boolean(const json_value *value) {
    if (value->type != JSON_BOOLEAN) {
        json_set_last_error("value is not of type JSON_BOOLEAN\n");
        return 10;
    }
    return value->u.boolean;
}

/* Adds or updates a key-value pair in a JSON object. */
int json_object_set(json_value *object, const char *key, json_value *value) {
    /* allocate space for 10 members */
    if (object->u.object.keys == NULL) {
        object->u.object.capacity = 10;
        object->u.object.keys = calloc(object->u.object.capacity, sizeof(char *));
        object->u.object.values = calloc(object->u.object.capacity, sizeof(json_value *));
        if(!object->u.object.keys || !object->u.object.values) {
            fprintf(stderr, "Failed to allocate space for object members\n");
            exit(EXIT_FAILURE);
        }
    }
     
    /* resize the arrays if object count reach capacity */
    if(object->u.object.count == object->u.object.capacity) {
        object->u.object.capacity *= 2; /* double capacity every time count == capacity */

        char **temp_keys = realloc(object->u.object.keys, object->u.object.capacity*sizeof(char *));
        json_value **temp_values = realloc(object->u.object.values, object->u.object.capacity*sizeof(json_value *));
        if(!temp_keys || !temp_values) {
            fprintf(stderr, "resize of array failed because of realloc\n");
            free(object->u.object.keys);
            free(object->u.object.values);
            return 0; 
        }
        object->u.object.keys = temp_keys;
        object->u.object.values = temp_values;
    }

    uint32_t ind = object->u.object.count++;
    /* TODO make a deep copy of the objects */
    //object->u.object.keys[ind] = key; 
    object->u.object.keys[ind] = calloc(strlen(key)+1,sizeof(char));
    strcpy(object->u.object.keys[ind], key);
    object->u.object.values[ind] = value;
    return 1;
}

/**
 * Returns the json_value associated with a key in a JSON object.
 * Returns NULL if the key is not found or if value is not an object.
 */
json_value *json_object_get(const json_value *object, const char *key) {
    if(object->type != JSON_OBJECT) return NULL;
    for(int i = 0; i < object->u.object.count; i++) {
        if (strcmp(key, object->u.object.keys[i]) == 0)
            return object->u.object.values[i];
    }
    json_set_last_error("json_object_get: key not found in object\n");
    return NULL;
}

/**
 * Appends a value to a JSON array.
 */
int json_array_append(json_value *array, json_value *value) {
    /* allocate space for 10 members */
    if (array->u.array.items == NULL) {
        array->u.array.capacity = 10;
        array->u.array.items = calloc(array->u.array.capacity, sizeof(json_value *));
        if(!array->u.array.items) {
            fprintf(stderr, "Failed to allocate space for array members\n");
            exit(EXIT_FAILURE);
        }
    }
     
    /* resize the arrays if array count reach capacity */
    if(array->u.array.count == array->u.array.capacity) {
        array->u.array.capacity *= 2; /* double capacity every time count == capacity */

        json_value **temp_items = realloc(array->u.array.items, array->u.array.capacity*sizeof(json_value *));
        if(!temp_items) {
            fprintf(stderr, "resize of array failed because of realloc\n");
            free(array->u.array.items);
            return 0; 
        }
        array->u.array.items = temp_items;
    }

    uint32_t ind = array->u.array.count++;
    /* TODO make a deep copy of the arrays */
    array->u.array.items[ind] = value;
    return 1;
}

/**
 * Returns the json_value at the specified index in a JSON array.
 * Returns NULL if the index is out of bounds or if value is not an array.
 */
json_value *json_array_get(const json_value *array, size_t index) {
    if (array->type != JSON_ARRAY || index >= array->u.array.count) {
        if (array->type != JSON_ARRAY) json_set_last_error("object is not of type JSON_ARRAY\n");
        if (index >= array->u.array.count) json_set_last_error("index of array out of bounds\n");
        return NULL;
    }
    return array->u.array.items[index];
}

int json_get_type(const json_value *v) {
    return v->type;
}

/* Exposed error retrieval function */
const char *json_get_last_error(void) {
    return last_error;
}

void json_print_value(const json_value *v) {
    if(!v) {
        printf("value: NULL\n");
        return;
    }

    //printf("value:\n\t");
    switch (v->type) {
        case JSON_NULL:
            printf("null");
            break;
        case JSON_BOOLEAN:
            v->u.boolean == 1 ? printf("true") : printf("false");
            break;
        case JSON_NUMBER:
            printf("%lf", v->u.number);
            break;
        case JSON_STRING:
            printf("\"%s\"", v->u.string);
            break;
        case JSON_OBJECT:
            printf("{\n");
            for (int i = 0; i < v->u.object.count; i++) {
                printf("  \"%s\": ", v->u.object.keys[i]);
                json_print_value(v->u.object.values[i]);
                printf(",\n");
            }
            printf("}");
            break;
        case JSON_ARRAY:
            printf("[\n");
            for (int i = 0; i < v->u.array.count; i++) {
                printf("  ");
                json_print_value(v->u.array.items[i]);
                printf(",\n");
            }
            printf("]");
            break;
    }

    //printf("\n");
}
