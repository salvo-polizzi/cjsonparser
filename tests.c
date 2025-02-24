#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsonparser.h"

/* Test parsing a valid JSON string */
void test_json_parse_valid(void) {
    printf("Test: json_parse with valid JSON\n");
    const char *json_text = "{\"name\": \"Alice\", \"age\": 30, \"isStudent\": false}";
    json_value *root = json_parse(json_text);
    if (!root) {
        printf("  FAIL: json_parse returned NULL for valid JSON. Error: %s\n", json_get_last_error());
        return;
    }
    // Expecting an object. Check the key "name"
    json_value *name = json_object_get(root, "name");
    if (!name) {
        printf("  FAIL: json_object_get failed to retrieve key \"name\"");
    } else if (json_get_type(name) != JSON_STRING) {
        printf("  FAIL: Expected type JSON_STRING for key \"name\".\n");
    } else if (strcmp(json_get_string(name), "Alice") != 0) {
        printf("  FAIL: Value for \"name\" incorrect; expected \"Alice\", got \"%s\"\n", json_get_string(name));
    } else {
        printf("  PASS: Valid JSON parsed correctly.\n");
    }
    json_free(root);
}

/* Test parsing an invalid JSON string */
void test_json_parse_invalid(void) {
    printf("Test: json_parse with invalid JSON\n");
    const char *json_text = "{\"name\": \"Alice\", \"age\": }";  // invalid JSON
    json_value *root = json_parse(json_text);
    if (root) {
        printf("  FAIL: json_parse should return NULL for invalid JSON.\n");
        json_free(root);
    } else {
        printf("  PASS: json_parse returned NULL for invalid JSON as expected. Error: %s\n", json_get_last_error());
    }
}

/* Test creating a string JSON value with a NULL argument */
void test_json_new_string_null(void) {
    printf("Test: json_new_string with NULL argument\n");
    json_value *v = json_new_string(NULL);
    if (v) {
        printf("  FAIL: json_new_string(NULL) should return NULL.\n");
        json_free(v);
    } else {
        printf("  PASS: json_new_string(NULL) returned NULL as expected. Error: %s\n", json_get_last_error());
    }
}

/* Test setting and getting values in a JSON object */
void test_json_object_set_get(void) {
    printf("Test: json_object_set and json_object_get\n");
    json_value *obj = json_new_object();
    if (!obj) {
        printf("  FAIL: json_new_object returned NULL. Error: %s\n", json_get_last_error());
        return;
    }
    json_value *name_val = json_new_string("Bob");
    if (!name_val) {
        printf("  FAIL: json_new_string for \"Bob\" returned NULL. Error: %s\n", json_get_last_error());
        json_free(obj);
        return;
    }
    int ret = json_object_set(obj, "name", name_val);
    if (ret == 0) {
        printf("  FAIL: json_object_set failed. Error: %s\n", json_get_last_error());
        json_free(name_val);
        json_free(obj);
        return;
    }
    json_value *retrieved = json_object_get(obj, "name");
    if (!retrieved) {
        printf("  FAIL: json_object_get returned NULL for key \"name\".\n");
    } else if (json_get_type(retrieved) != JSON_STRING ||
               strcmp(json_get_string(retrieved), "Bob") != 0) {
        printf("  FAIL: json_object_get returned incorrect value for key \"name\".\n");
        printf("        type : %d\n", json_get_type(retrieved));
	printf("     	value : %s\n", json_get_string(retrieved));
        printf("        last errror: %s\n", json_get_last_error());
    } else {
        printf("  PASS: json_object_set and json_object_get work as expected.\n");
    }
    //json_print_value(obj);
    json_free(obj);
}

/* Test appending to and getting from a JSON array */
void test_json_array_append_get(void) {
    printf("Test: json_array_append and json_array_get\n");
    json_value *arr = json_new_array();
    if (!arr) {
        printf("  FAIL: json_new_array returned NULL. Error: %s\n", json_get_last_error());
        return;
    }
    json_value *num1 = json_new_number(42.0);
    json_value *num2 = json_new_number(3.14);
    if (!num1 || !num2) {
        printf("  FAIL: json_new_number returned NULL. Error: %s\n", json_get_last_error());
        json_free(arr);
        return;
    }
    if (json_array_append(arr, num1) == 0) {
        printf("  FAIL: json_array_append failed for first element. Error: %s\n", json_get_last_error());
    }
    if (json_array_append(arr, num2) == 0) {
        printf("  FAIL: json_array_append failed for second element. Error: %s\n", json_get_last_error());
    }
    json_value *elem0 = json_array_get(arr, 0);
    json_value *elem1 = json_array_get(arr, 1);
    json_value *elem_invalid = json_array_get(arr, 2);  // Out-of-bound index
    if (!elem0 || json_get_type(elem0) != JSON_NUMBER || json_get_number(elem0) != 42.0) {
        printf("  FAIL: json_array_get returned incorrect first element.\n");
    } else if (!elem1 || json_get_type(elem1) != JSON_NUMBER || json_get_number(elem1) != 3.14) {
        printf("  FAIL: json_array_get returned incorrect second element.\n");
    } else if (elem_invalid != NULL) {
        printf("  FAIL: json_array_get should return NULL for out-of-bound index.\n");
    } else {
        printf("  PASS: json_array_append and json_array_get work as expected.\n");
    }
    json_free(arr);
}

int main(void) {
    printf("Running JSON Library Tests...\n\n");
    test_json_parse_valid();
    printf("\n");
    test_json_parse_invalid();
    printf("\n");
    test_json_new_string_null();
    printf("\n");
    test_json_object_set_get();
    printf("\n");
    test_json_array_append_get();
    printf("\nAll tests completed.\n");
    return 0;
}

