# Lock-Free Sorted Vector (LFSV) Implementation

## Overview

This repository contains an implementation of a Lock-Free Sorted Vector (LFSV), optimized for high-concurrency environments. Unlike traditional approaches that may rely on lock-based synchronization mechanisms, this LFSV leverages atomic operations to ensure thread safety and high performance.

A key component of our implementation is the **Garbage Remover**, which provides an efficient memory management strategy by deferring the deletion of vector elements. This approach ensures that memory is not freed prematurely, preventing use-after-free errors that can occur in highly concurrent scenarios. The decision to use Garbage Remover over a Memory Bank was made after careful consideration of the performance benefits, particularly in reducing dynamic allocation overhead and minimizing lock contention.

## Key Features

- **High Concurrency Support**: Designed to perform efficiently in multi-threaded environments, allowing for simultaneous updates and accesses by multiple threads without traditional locks.
- **Efficient Memory Management**: Utilizes the Garbage Remover component for deferred deletion, optimizing memory usage and preventing use-after-free vulnerabilities.
- **Lock-Free Design**: Employs atomic operations to maintain consistency and integrity of the vector, avoiding the performance bottlenecks associated with locks.

## Building and Running

This project uses a Makefile for easy building and running. The following commands are supported:

- `make`: Compiles the source code into an executable.
- `make clean`: Removes compiled files to clean the directory.
- `make run`: Runs the compiled executable. This command has been updated to execute tests for all scenarios from 0 to 7, showcasing the robustness and efficiency of the LFSV implementation in various use cases.

To run the test suite across all scenarios, simply use:

```bash
make run
```

This will sequentially execute tests for scenarios 0 through 7, demonstrating the LFSV's performance and correctness in a variety of conditions.

## Conclusion

The choice of Garbage Remover over a traditional Memory Bank represents a significant optimization in our LFSV implementation, offering enhanced performance and safety in concurrent applications. This project is a testament to the potential for lock-free data structures to improve scalability and efficiency in modern software development.
