`scalefactor_bands_csv_to_c.py` converts Huffman tables from ISO/IEC 11172-3:1993 Table B.8 to C code. The C code generated are located at `scalefactor_tables.c`, the code is meant to be copied to another file hence `scalefactor_tables.c` does not compile.

At this moment all the data in the CSV files are manually entered, and tediously checked by eyes. There may be mistakes lurking somewhere!