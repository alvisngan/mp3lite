
## Test Return Code
The test returns an integer code indicating success (0) or failure (non-0). The non-zero integer return code is a bitfield indicating which test(s) within a test program has/have failed, where the least significant bit (LSB) represent `TEST_0`. The following table shows an example of integer codes for a test program with four tests.  

|ERR_CODE   |0  |1  |2  |3  |4  |5  |6  |7  |8  |9  |10 |11 |12 |13 |14 |15 |
|-----------|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|TEST_0     |   |F  |   |F  |   |F  |   |F  |   |F  |   |F  |   |F  |   |F  |
|TEST_1     |   |   |F  |F  |   |   |F  |F  |   |   |F  |F  |   |   |F  |F  |
|TEST_2     |   |   |   |   |F  |F  |F  |F  |   |   |   |   |F  |F  |F  |F  |
|TEST_3     |   |   |   |   |   |   |   |   |F  |F  |F  |F  |F  |F  |F  |F  |

In general, convert the return code to its binary representation and check which bit is set. If the N-th bit from LSB (the right-most bit) is set, it means the TEST_N has failed. (The test order is zero-based, the LSB represents TEST_0)