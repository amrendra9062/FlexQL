# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# Source files (based on your tree)
SRC = main.cpp \
      src/parser/tokenizer.cpp \
      src/parser/parser.cpp

# Output executable
TARGET = flexql_test

# Default target
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

# Run
run: $(TARGET)
	./$(TARGET)

# Clean
clean:
	rm -f $(TARGET)