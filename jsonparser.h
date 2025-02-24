#ifndef JSON_H
#define JSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include<stdlib.h>
#include "jsontokenizer.h"

#define JSON_NULL 0
#define JSON_BOOLEAN 1
#define JSON_NUMBER 2
#define JSON_STRING 3
#define JSON_ARRAY 4
#define JSON_OBJECT 5

/* Forward declaration of the structure */
typedef struct json_value json_value;

/*====================PARSING FUNCTIONS===================*/ 

json_value *json_parse(const char *json_text);
// char *json_serialize(const json_value *value);
void json_free(json_value *value);


/*====================CREATE JSON VALUES==================*/

json_value *json_new_null(void);
json_value *json_new_boolean(int boolean);
json_value *json_new_number(double number);
json_value *json_new_string(const char *string);
json_value *json_new_array(void);
json_value *json_new_object(void);


/*====================NAVIGATORS==========================*/ 

int json_object_set(json_value *object, const char *key, json_value *value);
json_value *json_object_get(const json_value *object, const char *key);
int json_array_append(json_value *array, json_value *value);
json_value *json_array_get(const json_value *array, size_t index);

char *json_get_string(const json_value *value);
double json_get_number(const json_value *value);
uint8_t json_get_boolean(const json_value *value);

int json_get_type(const json_value *value);

/* Error retrieval function */
const char *json_get_last_error(void);

void json_print_value(const json_value *value);

#ifdef __cplusplus
}
#endif

#endif  /* JSON_H */

