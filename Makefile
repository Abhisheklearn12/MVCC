# Usage:
#   make        - Build the program
#   make clean  - Remove build files
#   make run    - Build and run

# Compiler and flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -g

# Files
TARGET = mvcc_demo
SRCS = mvcc_main.c
HEADERS = mvcc_types.h mvcc_transaction_manager.h mvcc_visibility.h \
          mvcc_table.h mvcc_tests.h

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)
	@echo "✓ Build complete! Run with: ./$(TARGET)"

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean up
clean:
	rm -f $(TARGET)
	@echo "✓ Cleaned"

# Mark these as not real files
.PHONY: all run clean
