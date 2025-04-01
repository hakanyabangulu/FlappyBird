# Compiler to use
CC = g++

# Compiler flags
CFLAGS = -Wall -g

# Linker flags for SDL2 and SDL2_ttf
LDFLAGS = -lSDL2 -lSDL2_ttf

# Target executable name
TARGET = flappy

# Source file
SOURCE = flappy.cpp

# Object file
OBJECT = $(SOURCE:.cpp=.o)

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
