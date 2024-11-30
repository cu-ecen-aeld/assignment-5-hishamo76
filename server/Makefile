# Compiler and flags
CC ?= gcc
CROSS_COMPILE ?=
CFLAGS = -Wall -Werror

# Source and object files
SRC = aesdsocket.c
OBJ = $(SRC:.c=.o)
TARGET = aesdsocket

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
# all:	aesdsocket
# aesdsocket : aesdsocket.o
# 	$(CROSS_COMPILE)gcc -o aesdsocket aesdsocket.o 
# #	$(GCC) -o aesdsocket aesdsocket.o 
# aesdsocket.o : aesdsocket.c 
# 	$(CROSS_COMPILE)gcc -c aesdsocket.c
# #	$(GCC) -c aesdsocket.c

# clean :
# 	rm -f aesdsocket aesdsocket.o
	