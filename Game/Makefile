# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Linker flags for ncurses
LDFLAGS = -lncurses

# Target executable name
TARGET = flappy_bird

# Source file
SOURCE = flappy_bird.c

# Object file
OBJECT = $(SOURCE:.c=.o)

# Default target
all: $(TARGET)

# Link the object file to create the executable
$(TARGET): $(OBJECT)
	$(CC) $(OBJECT) -o $(TARGET) $(LDFLAGS)

# Compile the source file to an object file
$(OBJECT): $(SOURCE)
	$(CC) $(CFLAGS) -c $(SOURCE) -o $(OBJECT)

# Clean up
clean:
	rm -f $(OBJECT) $(TARGET)

# Phony targets
.PHONY: all clean