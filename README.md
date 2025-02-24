# JSON Parser Library in C

This library provides a simple API for parsing, constructing, and manipulating JSON data in C. It uses an opaque data type (`json_value`) for JSON values and exposes functions to parse a JSON string into an internal representation, create new JSON objects or arrays programmatically, access their elements, and free the allocated memory.

## Features

- **Parsing:** Convert a JSON-formatted string into a tree of JSON values.
- **Construction:** Create JSON values such as objects, arrays, strings, numbers, booleans, and null.
- **Accessors:** Retrieve values from objects and arrays using simple APIs.
- **Error Handling:** Retrieve human-readable error messages when operations fail.
- **Memory Management:** Automatically allocate and free all memory associated with JSON values.

## Importing the Library

To use this library in your project, simply include the header file in your source code. For example, add the following line at the top of your C file:

```c
#include "jsonparser.h"
```
Then, compile your project along with all of the library’s source files. 
For example, if you are using GCC and the library files (`jsonparser.c` and `jsontokenizer.c`) are in the same directory as your source file, 
compile with:

```bash
gcc -o your_app your_app.c jsonparser.c jsontokenizer.c -I.
```
Make sure to adjust the include path (-I) if your header files are in a different directory.

## Installation

The library uses a simple Makefile for building the project. To compile the library and tests, run:

```bash
make
```

This command builds an executable named `tests` that runs a comprehensive suite of tests against the parser.

## API Overview

### Parsing
- `json_value *json_parse(const char *json_text);` <br />
  Parses a JSON string and returns a pointer to a JSON value tree. Returns `NULL` if parsing fails. <br /> <br />
  **Example** 
  ```c
  const char *json_text = "{\"name\": \"Alice\", \"age\": 30}";
  json_value *root = json_parse(json_text);
  if (!root) {
    fprintf(stderr, "Error: %s\n", json_get_last_error());
  }
  ```

### Serialization
- `char *json_serialize(const json_value *value);` <br />
  Serializes a JSON value into a string. The caller is responsible for freeing the returned string.

### Memory Management
- `void json_free(json_value *value);` <br />
Frees a JSON value and all its children.

### Creating JSON Values
- `json_value *json_new_null(void);` <br />
  Creates a JSON null value.
- `json_value *json_new_boolean(int boolean);` <br />
  Creates a JSON boolean value.
- `json_value *json_new_number(double number);` <br />
  Creates a JSON number value.
- `json_value *json_new_string(const char *string);` <br />
  Creates a JSON string value. Note: Passing NULL returns NULL.
- `json_value *json_new_array(void);` <br />
  Creates an empty JSON array.
- `json_value *json_new_object(void);` <br />
  Creates an empty JSON object.

### Object and array manipulation
- `int json_object_set(json_value *object, const char *key, json_value *value);` <br />
Adds or updates a key-value pair in a JSON object. <br />
**Example**:
  ```c
  json_value *obj = json_new_object();
  json_object_set(obj, "name", json_new_string("Bob"));
  ```
- `json_value *json_object_get(const json_value *object, const char *key);` <br />
Retrieves the JSON value associated with a key in an object.
- `int json_array_append(json_value *array, json_value *value);` <br />
Appends a value to a JSON array.
- `json_value *json_array_get(const json_value *array, size_t index);` <br />
Retrieves the element at the specified index in a JSON array.

### Accessors for JSON Value Types
- `int json_get_type(const json_value *v);` <br />
Returns the type of the JSON value (e.g., JSON_STRING, JSON_NUMBER).
- `char *json_get_string(const json_value *v);` <br />
Returns the C string if the JSON value is a string.
- `double json_get_number(const json_value *v);` <br />
Returns the number if the JSON value is a number.
- `uint8_t json_get_boolean(const json_value *v);` <br />
Returns the boolean value if the JSON value is a boolean.

### Error handling 
- `const char *json_get_last_error(void);`
Returns the last error message recorded by the library. This is useful for debugging when an API call fails.
```c
if (!json_parse(some_bad_json)) {
    fprintf(stderr, "Parsing error: %s\n", json_get_last_error());
}
```

### Example usage
```c
#include <stdio.h>
#include "jsonparser.h"

int main(void) {
    const char *json_text = "{\"name\": \"Alice\", \"age\": 30, \"hobbies\": [\"reading\", \"coding\"]}";
    json_value *root = json_parse(json_text);
    
    if (!root) {
        fprintf(stderr, "Failed to parse JSON: %s\n", json_get_last_error());
        return 1;
    }
    
    json_value *name = json_object_get(root, "name");
    if (name && json_get_type(name) == JSON_STRING) {
        printf("Name: %s\n", json_get_string(name));
    }
    
    // Clean up memory
    json_free(root);
    return 0;
}
```

## Running tests
To run the test suite, execute:
```bash
./tests
```
This will run both the basic tests and the extended tests (if present) that cover complex, deeply nested, and invalid JSON scenarios.

## Contributing
Feel free to fork the repository and submit pull requests. Improvements in error handling, serialization, or performance are welcome.
