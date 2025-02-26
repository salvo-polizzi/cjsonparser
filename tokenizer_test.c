#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "jsontokenizer.h"

/* Utility function to check if tokenization was successful */
void assert_tokenize_success(const char* json, const char* test_name) {
    printf("Running test: %s\n", test_name);
    
    struct JSONTokenList* list = buildTokenList(json, strlen(json));
    if (!list) {
        printf("ERROR: Failed to tokenize: %s\n", get_tokenizer_error());
        printf("TEST FAILED: %s\n\n", test_name);
        exit(EXIT_FAILURE);
    }
    
    printf("TEST PASSED: %s\n\n", test_name);
    freeTokenList(list);
}

/* Function to check if tokenization fails as expected */
void assert_tokenize_failure(const char* json, const char* test_name) {
    printf("Running test: %s\n", test_name);
    
    struct JSONTokenList* list = buildTokenList(json, strlen(json));
    if (list) {
        printf("ERROR: Expected tokenization to fail but it succeeded\n");
        printf("TEST FAILED: %s\n\n", test_name);
        freeTokenList(list);
        exit(EXIT_FAILURE);
    }
    
    printf("TEST PASSED: %s (Expected failure: %s)\n\n", test_name, get_tokenizer_error());
}

/* Function to check if token sequence matches expected types */
void assert_token_types(const char* json, enum JSONTokenType expected_types[], int num_types, const char* test_name) {
    printf("Running test: %s\n", test_name);
    
    struct JSONTokenList* list = buildTokenList(json, strlen(json));
    if (!list) {
        printf("ERROR: Failed to tokenize: %s\n", get_tokenizer_error());
        printf("TEST FAILED: %s\n\n", test_name);
        exit(EXIT_FAILURE);
    }
    
    struct JSONTokenNode* node = list->head;
    int i = 0;
    
    while (node && i < num_types) {
        if (node->token.type != expected_types[i]) {
            printf("ERROR: Token at position %d has type %d, expected %d\n", 
                   i, node->token.type, expected_types[i]);
            printf("TEST FAILED: %s\n\n", test_name);
            freeTokenList(list);
            exit(EXIT_FAILURE);
        }
        node = node->next;
        i++;
    }
    
    if (i < num_types) {
        printf("ERROR: Too few tokens. Expected %d, got %d\n", num_types, i);
        printf("TEST FAILED: %s\n\n", test_name);
        freeTokenList(list);
        exit(EXIT_FAILURE);
    }
    
    if (node && node->token.type != END) {
        printf("ERROR: Too many tokens. Extra token of type %d found after expected tokens\n", 
               node->token.type);
        printf("TEST FAILED: %s\n\n", test_name);
        freeTokenList(list);
        exit(EXIT_FAILURE);
    }
    
    printf("TEST PASSED: %s\n\n", test_name);
    freeTokenList(list);
}

/* Function to check if token values match expected values */
void assert_token_values(const char* json, const char* expected_values[], int num_values, const char* test_name) {
    printf("Running test: %s\n", test_name);
    
    struct JSONTokenList* list = buildTokenList(json, strlen(json));
    if (!list) {
        printf("ERROR: Failed to tokenize: %s\n", get_tokenizer_error());
        printf("TEST FAILED: %s\n\n", test_name);
        exit(EXIT_FAILURE);
    }
    
    struct JSONTokenNode* node = list->head;
    int i = 0;
    
    while (node && i < num_values) {
        if (node->token.type == END) {
            break;  // Skip comparing END token
        }
        
        if (strcmp(node->token.value, expected_values[i]) != 0) {
            printf("ERROR: Token at position %d has value '%s', expected '%s'\n", 
                   i, node->token.value, expected_values[i]);
            printf("TEST FAILED: %s\n\n", test_name);
            freeTokenList(list);
            exit(EXIT_FAILURE);
        }
        node = node->next;
        i++;
    }
    
    if (i < num_values) {
        printf("ERROR: Too few tokens. Expected %d, got %d\n", num_values, i);
        printf("TEST FAILED: %s\n\n", test_name);
        freeTokenList(list);
        exit(EXIT_FAILURE);
    }
    
    printf("TEST PASSED: %s\n\n", test_name);
    freeTokenList(list);
}

int main() {
    printf("=== JSON Tokenizer Tests ===\n\n");
    
    /* Test 1: Simple Object */
    assert_tokenize_success("{\"name\":\"John\",\"age\":30}", "Simple Object");
    
    /* Test 2: Nested Objects */
    assert_tokenize_success(
        "{\"person\":{\"name\":\"Alice\",\"address\":{\"city\":\"New York\"}}}",
        "Nested Objects"
    );
    
    /* Test 3: Array */
    assert_tokenize_success("[1,2,3,4,5]", "Simple Array");
    
    /* Test 4: Object with Array */
    assert_tokenize_success(
        "{\"name\":\"Bob\",\"scores\":[95,87,92]}",
        "Object with Array"
    );
    
    /* Test 5: Empty Object */
    assert_tokenize_success("{}", "Empty Object");
    
    /* Test 6: Empty Array */
    assert_tokenize_success("[]", "Empty Array");
    
    /* Test 7: Null, True, False */
    assert_tokenize_success(
        "{\"active\":true,\"verified\":false,\"meta\":null}",
        "Null, True, False Keywords"
    );
    
    /* Test 8: String with Escape Sequences */
    assert_tokenize_success(
        "{\"message\":\"Hello\\nWorld\\\"Quote\\\"\"}",
        "String with Escape Sequences"
    );
    
    /* Test 9: Numbers */
    assert_tokenize_success(
        "[0,123,-456,7.89,1e10,-2.5e-5]",
        "Various Number Formats"
    );
    
    /* Test 10: Token Type Sequence */
    enum JSONTokenType expected_types[] = {
        OPEN_CURLY_BRACKET,
        STRING,           // "name"
        COLON,
        STRING,           // "value"
        CLOSE_CURLY_BRACKET,
        END               // Last token is always END
    };
    assert_token_types(
        "{\"name\":\"value\"}",
        expected_types,
        5,  // Don't count END token
        "Token Type Sequence"
    );
    
    /* Test 11: Token Value Check */
    const char* expected_values[] = {
        "{", "name", ":", "value", "}"
    };
    assert_token_values(
        "{\"name\":\"value\"}",
        expected_values,
        5,
        "Token Value Check"
    );
    
    /* ======= EDGE CASES ======= */
    
    /* Test 12: Empty String */
    assert_tokenize_success("{\"key\":\"\"}", "Empty String Value");
    
    /* Test 13: Very Long String */
    char* long_string = malloc(10002); // 10000 chars + quotes + null terminator
    if (!long_string) {
        printf("Failed to allocate memory for long string test\n");
        exit(EXIT_FAILURE);
    }
    
    strcpy(long_string, "{\"long\":\"");
    memset(long_string + 9, 'a', 10000);
    strcpy(long_string + 10009, "\"}");
    
    assert_tokenize_success(long_string, "Very Long String (10000 chars)");
    free(long_string);
    
    /* Test 14: Deeply Nested Structure */
    assert_tokenize_success(
        "[[[[[[[[[[\"deep\"]]]]]]]]]]",
        "Deeply Nested Array (10 levels)"
    );
    
    /* Test 15: Leading/Trailing Whitespace */
    assert_tokenize_success(
        "   \n\t{\"key\":123}   \r\n",
        "Leading/Trailing Whitespace"
    );
    
    /* Test 16: Unicode characters */
    assert_tokenize_success(
        "{\"unicode\":\"héllo wörld\"}",
        "Unicode Characters"
    );
    
    /* Test 17: Number Edge Cases */
    assert_tokenize_success(
        "[0,0.0,-0,1e0,1e+0,1e-0]",
        "Number Edge Cases"
    );
    
    /* Test 18: All JSON Token Types */
    assert_tokenize_success(
        "{\"arr\":[1,true,null,\"str\"],\"obj\":{}}",
        "All JSON Token Types"
    );
    
    /* ======= ERROR CASES ======= */
    
    /* Test 19: Unterminated String */
    assert_tokenize_failure(
        "{\"key\":\"value",
        "Unterminated String"
    );
    
    /* Test 20: Invalid Number Format */
    assert_tokenize_failure(
        "[01.5]",  // Leading zeros not allowed
        "Invalid Number Format (leading zero)"
    );
    
    /* Test 21: Invalid Number Format 2 */
    assert_tokenize_failure(
        "[1.]",  // Decimal point must be followed by digits
        "Invalid Number Format (decimal with no digits)"
    );
    
    /* Test 22: Invalid Keyword */
    assert_tokenize_failure(
        "{\"valid\":truee}",  // Invalid keyword
        "Invalid Keyword"
    );
    
    /* Test 23: Invalid Character */
    assert_tokenize_failure(
        "{key:\"value\"}",  // Missing quotes around key
        "Invalid Character (unquoted key)"
    );
    
    /* Test 24: Invalid JSON Structure */
    assert_tokenize_failure(
        "{{}}",  // Double opening brace
        "Invalid JSON Structure"
    );
    
    /* Test 25: Missing Colon */
    assert_tokenize_failure(
        "{\"key\" \"value\"}",  // Missing colon
        "Missing Colon"
    );
    
    /* Test 26: Trailing Comma */
    assert_tokenize_failure(
        "[1,2,3,]",  // Trailing comma
        "Trailing Comma"
    );
    
    /* Test 27: Missing Comma */
    assert_tokenize_failure(
        "[1 2 3]",  // Missing commas
        "Missing Comma"
    );
    
    /* Test 28: Invalid Escape Sequence */
    assert_tokenize_failure(
        "{\"key\":\"\\z\"}",  // Invalid escape sequence
        "Invalid Escape Sequence"
    );
    
    printf("All tests completed successfully!\n");
    return 0;
}
