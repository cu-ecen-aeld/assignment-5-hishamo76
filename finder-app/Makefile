# Compiler and flags
CC ?= gcc
CROSS_COMPILE ?=
CFLAGS = -Wall -Werror

# Source and object files
SRC = writer.c
OBJ = $(SRC:.c=.o)
TARGET = writer

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJ)
	$(CROSS_COMPILE)$(CC) -o $@ $^

# Compile source files into object files
%.o: %.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(TARGET) $(OBJ)

# Phony targets
.PHONY: all clean
