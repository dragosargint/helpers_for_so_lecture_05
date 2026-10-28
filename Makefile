CC = gcc
CFLAGS = -Wall -Wextra
TARGET = shared_mmap
SRC = shared_mmap.c

# Default release build
all: CFLAGS += -O2
all: $(TARGET)

# Debug build with gdb symbols and optimizations disabled
debug: CFLAGS += -g -O0
debug: clean $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all debug clean
