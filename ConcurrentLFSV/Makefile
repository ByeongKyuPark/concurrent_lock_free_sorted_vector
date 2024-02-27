# Define compiler
CXX=g++

# Define compiler flags
CXXFLAGS=-std=c++11 -Wall -lpthread

TARGET=lfsv_test

# Define source files
SRC=driver.cpp GarbageRemover.cpp lfsv.cpp

# Default rule to build the program
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

# Modified run target to execute tests from 0 to 7
run:
	for i in 0 1 2 3 4 5 6 7 ; do \
		./$(TARGET) $$i ; \
	done

help:
	@echo "Available commands:"
	@echo "make       - Compiles the source code into an executable."
	@echo "make clean - Removes compiled files."
	@echo "make run   - Runs the compiled executable with tests from 0 to 7."
