# Lock-Free Sorted Vector (LFSV)

## Overview
This repository contains the implementation of a Lock-Free Sorted Vector (LFSV) optimized for high concurrency environments. The LFSV leverages atomic operations to ensure thread-safe updates and accesses to a dynamic array without traditional locking mechanisms. It integrates two main components for efficient memory management:
- `MemoryBank`: A pool that recycles vector instances to minimize dynamic allocation overhead.
- `GarbageRemover`: Delays the deletion of vectors to ensure safe concurrent access, preventing use-after-free errors.

The design aims to minimize performance bottlenecks associated with memory management in concurrent applications, making it an ideal solution for scenarios requiring high throughput and parallel data updates.

## How to Run

### Prerequisites
- C++ compiler with C++11 support (e.g., GCC, Clang)
- Make (optional for compilation automation)

### Compilation
To compile the test program using the provided Makefile, follow these steps:

1. Navigate to the directory containing the source files and the Makefile. If you're using a terminal, you can use the `cd` command to change the directory. For example, if your project is located in the 'ConcurrentLFSV' folder, you can navigate to it with:

    ```bash
    cd ConcurrentLFSV
    ```

2. Once you're in the correct directory, you can compile the program by simply running the `make` command. This will use the Makefile to compile your program according to the predefined rules and settings:

    ```bash
    make
    ```

    This command will automatically use the settings defined in the Makefile to compile your program, creating an executable named `lfsv_test`.

3. If you want to clean up the compiled files, you can run:

    ```bash
    make clean
    ```

4. To run the compiled program, you can either use:

    ```bash
    ./lfsv_test <test_number>
    ```

    Or, if you've added a `run` rule in your Makefile:

    ```bash
    make run
    ```

### Running Tests
The program accepts a single command-line argument that specifies the test number to run. Each test varies the number of threads and operations per thread to measure the performance under different concurrency levels.

To run a specific test, execute the compiled program with the test number as an argument. For example, to run test number 4:

```bash
./lfsv_test 4
