# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread -g

# Source files and target
# SRC = encrypt-driver.c encrypt-module-reproducible-fixed.c
SRC = encrypt-driver.c encrypt-module.c
TARGET = encrypt

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

# Clean up build files
clean:
	rm -f $(TARGET)
