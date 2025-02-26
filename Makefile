CC = gcc
CFLAGS = -Wall -Wextra -g
DEPS = jsontokenizer.h jsonparser.h
OBJ_TOKENIZER = jsontokenizer.o
OBJ_PARSER = jsonparser.o
OBJ_TOKENIZER_TEST = tokenizer_test.o
OBJ_PARSER_TEST = tests.o

.PHONY: all clean test test_tokenizer test_parser

# Default target: build all executables
all: tokenizer_test parser_test

# Rule for object files
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Tokenizer test executable
tokenizer_test: $(OBJ_TOKENIZER_TEST) $(OBJ_TOKENIZER)
	$(CC) -o $@ $^ $(CFLAGS)

# Parser test executable
parser_test: $(OBJ_PARSER_TEST) $(OBJ_PARSER) $(OBJ_TOKENIZER)
	$(CC) -o $@ $^ $(CFLAGS)

# Run tokenizer tests
test_tokenizer: tokenizer_test
	./tokenizer_test

# Run parser tests
test_parser: parser_test
	./parser_test

# Run all tests
test: test_tokenizer test_parser

# Clean up build artifacts
clean:
	rm -f *.o tokenizer_test parser_test

# Debug info
debug:
	@echo "DEPS: $(DEPS)"
	@echo "OBJ_TOKENIZER: $(OBJ_TOKENIZER)"
	@echo "OBJ_PARSER: $(OBJ_PARSER)"
	@echo "OBJ_TOKENIZER_TEST: $(OBJ_TOKENIZER_TEST)"
	@echo "OBJ_PARSER_TEST: $(OBJ_PARSER_TEST)"
