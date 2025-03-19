The Huffman code (`hcod`) is sorted by their length (`hlen`) first, then sorted by the magnitude of the hcod represented as integers. In this way binary search can be used to speed up decoding.

`huffman_tables_csv_to_c.py` converts Huffman tables from ISO/IEC 11172-3:1993 Table B.7 to C code. The C code generated are located at `huffman_tables.c`, the code is meant to be copied to another file hence `huffman_tables.c` does not compile.

At this moment all the data in the CSV files are manually entered, and tediously checked by eyes. There may be mistakes lurking somewhere!