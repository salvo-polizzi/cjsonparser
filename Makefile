# Compiler and flags
CC      = gcc
CFLAGS  = -Wall -Wextra -g

# Get all .c source files (including tests.c)
SRCS    = $(wildcard *.c)
# Generate object files names from the sources
OBJS    = $(SRCS:.c=.o)

# Name of the test executable
TARGET  = tests

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

