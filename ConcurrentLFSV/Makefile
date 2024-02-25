# Define compiler
CXX=g++

# Define compiler flags
CXXFLAGS=-std=c++11 -Wall -lpthread

TARGET=lfsv_test

# Define source files
SRC=driver.cpp

# Default rule to build the program
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

run:
	./$(TARGET)

help:
	@echo "Available commands:"
	@echo "make       - Compiles the source code into an executable."
	@echo "make clean - Removes compiled files."
	@echo "make run   - Runs the compiled executable."
