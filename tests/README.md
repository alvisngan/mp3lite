## Running the Tests
Navigate to the `tests` directory and use the following commands in the terminal:
```bash
$ cd build
$ cmake ..
$ cmake --build .
$ ctest --output-on-failure
```
### Choosing a Compiler
Different compilers provide different warning and error messages. To specify a compiler, use the following commands instead (in the `tests` directrory):
```bash
# Example using clang
# Change the complier executable directory depending on your system

$ cmake -DCMAKE_C_COMPILER=/usr/bin/clang ..
$ cmake --build .
$ ctest --output-on-failure
```

## Test Exit Code
The test returns an integer exit code indicating success (0) or failure (non-0). The non-zero integer exit code is a bitfield indicating which test(s) within a test program has/have failed, where the least significant bit (LSB) represent `TEST_0`. The following table shows an example of exit codes for a test program with four tests.  

|EXIT_CODE  |0  |1  |2  |3  |4  |5  |6  |7  |8  |9  |10 |11 |12 |13 |14 |15 |
|-----------|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|TEST_0     |   |F  |   |F  |   |F  |   |F  |   |F  |   |F  |   |F  |   |F  |
|TEST_1     |   |   |F  |F  |   |   |F  |F  |   |   |F  |F  |   |   |F  |F  |
|TEST_2     |   |   |   |   |F  |F  |F  |F  |   |   |   |   |F  |F  |F  |F  |
|TEST_3     |   |   |   |   |   |   |   |   |F  |F  |F  |F  |F  |F  |F  |F  |

In general, convert the exit code to its binary representation and check which bit is set. If the N-th bit from LSB (the right-most bit) is set, it means the TEST_N has failed. (The test order is zero-based, the LSB represents TEST_0)