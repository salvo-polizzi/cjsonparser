#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsonparser.h"

/* Extended Test 1: Complex JSON Object */
void test_complex_object(void) {
    printf("Test: Parse a complex JSON object\n");
    const char *json_str = 
        "{"
          "\"name\": \"Complex\","
          "\"details\": {"
              "\"age\": 42, "
              "\"address\": {"
                  "\"street\": \"123 Main St\","
                  "\"city\": \"Testville\""
              "}"
          "},"
          "\"scores\": [100, 98, 95, [1,2,3], {\"sub\": 10}],"
          "\"emptyArray\": [],"
          "\"emptyObject\": {}"
        "}";
    json_value *v = json_parse(json_str);
    if (!v) {
        printf("  FAIL: Failed to parse complex object. Error: %s\n", json_get_last_error());
        return;
    }
    printf("  PASS: Parsed complex object successfully.\n");
    json_print_value(v);
    printf("\n");
    json_free(v);
}

/* Extended Test 2: Deeply Nested JSON (100 levels deep) */
void test_deeply_nested(void) {
    printf("Test: Parse deeply nested JSON structure\n");
    /* Build a deeply nested JSON array such as: [[[[ ... 100 times ... ]]]]
       We build a string with 100 nested '[' then a simple value "0" and then 100 closing ']' */
    size_t nest = 100;
    size_t bufSize = nest + nest + 10;  // extra room for the value and null terminator
    char *json_str = malloc(bufSize);
    if (!json_str) {
         printf("Memory allocation error for deeply nested test\n");
         return;
    }
    json_str[0] = '\0';
    for (size_t i = 0; i < nest; i++) {
         strcat(json_str, "[");
    }
    strcat(json_str, "0");
    for (size_t i = 0; i < nest; i++) {
         strcat(json_str, "]");
    }
    json_value *v = json_parse(json_str);
    if (!v) {
         printf("  FAIL: Failed to parse deeply nested JSON. Error: %s\n", json_get_last_error());
         free(json_str);
         return;
    }
    printf("  PASS: Parsed deeply nested JSON successfully.\n");
    json_print_value(v);
    printf("\n");
    json_free(v);
    free(json_str);
}

/* Extended Test 3: Invalid JSON Cases */
void test_invalid_json_cases(void) {
    printf("Test: Parse various invalid JSON strings\n");
    const char *invalid_cases[] = {
        "",                                   // Empty string
        "{",                                  // Missing closing brace
        "[",                                  // Missing closing bracket
        "{\"name\": \"Alice\", \"age\": }",   // Missing value for key "age"
        "{\"key\": \"value\",}",              // Trailing comma in object
        "[1, 2, 3,]",                        // Trailing comma in array
        "nulla",                             // Extra characters
        "{\"a\": [1, 2, \"three\"]",          // Missing closing brace
        NULL
    };
    for (int i = 0; invalid_cases[i] != NULL; i++) {
         json_value *v = json_parse(invalid_cases[i]);
         if (v) {
              printf("  FAIL: Expected failure for invalid JSON: %s\n", invalid_cases[i]);
              json_free(v);
         } else {
              printf("  PASS: Correctly failed to parse invalid JSON: %s; Error: %s\n",
                     invalid_cases[i], json_get_last_error());
         }
    }
}

/* Extended Test 4: Varied Valid JSON Strings (Simulated Random Testing) */
void test_varied_valid_json(void) {
    printf("Test: Parse a variety of valid JSON strings\n");
    const char *valid_cases[] = {
         "{\"a\":1,\"b\":2,\"c\":[true,false,null]}",
         "[{\"name\":\"Alice\"},{\"name\":\"Bob\"}]",
         "{\"nested\":{\"array\":[{\"deep\":123},{\"deep\":456}]}}",
         "{\"emptyArray\":[],\"emptyObject\":{}}",
         "[[[[[]]]]]",
         NULL
    };
    for (int i = 0; valid_cases[i] != NULL; i++) {
         json_value *v = json_parse(valid_cases[i]);
         if (!v) {
              printf("  FAIL: Failed to parse valid JSON: %s; Error: %s\n",
                     valid_cases[i], json_get_last_error());
         } else {
              printf("  PASS: Parsed valid JSON successfully: %s\n", valid_cases[i]);
              // Optionally print the structure:
              json_print_value(v);
              printf("\n");
              json_free(v);
         }
    }
}

/* Main: Run all extended tests */
int main(void) {
    printf("Running Extended JSON Library Tests...\n\n");
    
    test_complex_object();
    printf("\n-------------------------\n\n");
    
    test_deeply_nested();
    printf("\n-------------------------\n\n");
    
    test_invalid_json_cases();
    printf("\n-------------------------\n\n");
    
    test_varied_valid_json();
    printf("\nAll extended tests completed.\n");
    
    return 0;
}

