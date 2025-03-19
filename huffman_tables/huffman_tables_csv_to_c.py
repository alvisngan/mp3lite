import numpy as np
import pandas as pd

# Settings
indent = "    "                 # 4 spaces as indent
static_var = True               # Apply static keyword to variables
static_prefix = True            # Apply s_ prefix on static variable names
const_var = True                # Applt const keyword to variables
file_name = "huffman_tables"    # Output filename
file_extension = ".c"           # Output file extension

if static_var == True:
    static_str = "static "
else:
    static_str = ''

if static_prefix == True:
    s_prefix_str = "s_"
else:
    s_prefix_str = '' 

if const_var == True:
    const_str = "const "
else:
    const_str = ''


# unused_tables = [4, 14]

# # linbits for table 16 to 23, otherwise it is 0 or table is unused
# linbits_tb16_to_tb23 = [1, 2, 3, 4, 6, 8, 10, 13]
# linbits_tb24_to_tb31 = [4, 5, 6, 7, 8, 9, 11, 13]


# Creating the strings for each of the available table
table_numbers = ['1', '2', '3', '5', '6', '7', '8', '9', '10', '11', '12', '13', '15', '16', '24', 'a', 'b']
all_table_str = ''
all_arr_to_struct_str = ''

for table_number in table_numbers:
    # Reading the Huffman tables
    filename = "huffman_tables_" + table_number + ".csv"

    df = pd.read_csv(filename, dtype = int)

    # Converting hcod from binary numbers to decimals 
    hcod_dec = np.vectorize(lambda x: int(str(x), 2))(df.loc[:, "hcod"].astype(str).values)
    df["hcod_dec"] = hcod_dec

    # Sort by hlen as shorter length bitstreams are more likely to occur
    # Then sort by hcod_dec for binary search
    df = df.sort_values(by = ["hlen", "hcod_dec"])

    # Creating a the strings
    if df.max(axis=0)["hcod_dec"] < 256:
        uint_type = "uint8_t"
    else:
        uint_type = "uint16_t"
    hcod_str = uint_type + ' ' + s_prefix_str + "htb" + table_number + "_hcod[] = {"
    for num in df.loc[:, "hcod_dec"].astype(str).values:
        hcod_str += num + ", "
    hcod_str = static_str + const_str + hcod_str[:-2] + "};"

    assert (np.max((df.loc[:, "idx"])) <= 255), "idx must be smaller than 256 to fit into uint8_t"
    idx_str = "uint8_t" + ' ' + s_prefix_str + "htb" + table_number + "_idx[] = {"
    for num in df.loc[:, "idx"].astype(str).values:
        idx_str += num + ", "
    idx_str = static_str + const_str + idx_str[:-2] + "};"

    # Extract hlen into unique elements
    hlen_str = "uint8_t" + ' ' + s_prefix_str + "htb" + table_number + "_hlen[] = {"
    unique_hlen = np.sort(df["hlen"].unique())
    for num in unique_hlen:
        hlen_str += str(num) + ", "
    hlen_str = static_str + const_str + hlen_str[:-2] + "};"

    # Count the number of each hlen
    hlen_cnt_str = "uint8_t" + ' ' + s_prefix_str + "htb" + table_number + "_hlen_cnt[] = {"
    hlen_cnt_dict = df["hlen"].value_counts().to_dict()
    for hlen in unique_hlen:
        hlen_cnt_str += str(hlen_cnt_dict[hlen]) + ", "
    hlen_cnt_str = static_str + const_str + hlen_cnt_str[:-2] + "};"

    # Useful constants
    hlen_arrlen_str = static_str + const_str + "uint8_t" + ' ' + s_prefix_str + "htb" + table_number + "_hlen_arrlen = " + str(np.size(unique_hlen)) + ';'

    xy_max_str = ''
    xy_max = 0
    if (table_number != 'a' and table_number != 'b'):
        xy_max = np.max((df.loc[:, "x"]))
        xy_max_str = static_str + const_str + "uint8_t" + ' ' + s_prefix_str + "htb" + table_number + "_xy_max = " + str(xy_max) + ';'

    # Assigning the array addresses to the table struct
    # Casting 'a' and 'b' to uint8_t as init_huffman_table takes integer parameter
    table_number_cast = table_number
    if (table_number == 'a' or table_number == 'b'):
        table_number_cast = "(uint_8_t) '" + table_number + "'"

    arr_to_struct_str = ("s_init_huffman_table(&" + s_prefix_str + "htb_" + table_number + ", " +
                         table_number_cast + ", " +
                         str(xy_max) + ", " +
                         str(np.size(unique_hlen)) + ", " +
                         s_prefix_str + "htb" + table_number + "_hlen" + ", " +
                         s_prefix_str + "htb" + table_number + "_hlen_cnt" + ", " +
                         "(void *) " + s_prefix_str + "htb" + table_number + "_hcod" + ", " +
                         s_prefix_str + "htb" + table_number + "_idx" + 
                         ");")

    table_str = (xy_max_str + '\n' + hlen_arrlen_str + '\n' + hlen_str + '\n' +
                hlen_cnt_str + '\n' + hcod_str + '\n' + idx_str + "\n\n\n")

    all_table_str += table_str
    all_arr_to_struct_str += arr_to_struct_str + '\n'


heading_str = ("/*\n" +
               " * The Huffman code (hcod) are represented as unsigned integer arrays with\n" +
               " * table 1 to 15 represented as uint8_t and table 16 to 31 as uint16_t \n" +
               " *\n" +
               " * For table A and table B, ASCII values of 'a' and 'b' are casted to uint8_t\n" +
               " * and used in the num field, and xy_max field is unused\n" +
               " *\n" +
               " * For big _values, the x and y values can be found with idx and max_xy \n" + 
               " * with the following equations:\n" +
               " *" + indent + "x = (idx - y) / (max_xy + 1)\n" +
               " *" + indent + "y = idx - x * (max_xy + 1)\n" +
               " *\n" +
               " * For count1 quadruples , the v, w, x and y values can be found \n" +
               " * with idx with the following equations:\n" +
               " *" + indent + "v = (idx & 0x8)? 1 : 0\n" +
               " *" + indent + "u = (idx & 0x4)? 1 : 0\n" +
               " *" + indent + "x = (idx & 0x2)? 1 : 0\n" +
               " *" + indent + "y = (idx & 0x1)? 1 : 0\n" +
               " * (where & is bitwise the AND operation)\n" +
               " *\n" +
               " * hcod arrays are casted to void* because table 1 to 15 are uint8_t*\n" +
               " * and 16 to 31 are uint16_t*\n" +
               " *\n" +
               " * Reference: ISO/IEC 11172-3:1993 Table B.7.\n" +
               " */")

struct_str = ("typedef struct {\n" +
              indent + "uint8_t num;\n" +
              indent + "uint8_t xy_max;\n" +
              indent + "uint8_t hlen_arrlen;\n" +
              indent + "uint8_t *hlen;\n" +
              indent + "uint8_t *hlen_cnt;\n" +
              indent + "void *hcod;\n" +
              indent + "uint8_t *idx;\n" +
              "} huffman_table_t;")

# add matching indent if the static is used in a function
static_indent_str = ''
for i in range(len(static_str)):
    static_indent_str += ' '
for i in range(len(s_prefix_str)):
    static_indent_str += ' '


init_huffman_table_str = (static_str + "void " + s_prefix_str + "init_huffman_table(huffman_table_t *htb,\n" +
                          static_indent_str + "                        uint8_t htb_num,\n" +
                          static_indent_str + "                        uint8_t htb_xy_max,\n" +
                          static_indent_str + "                        uint8_t htb_hlen_arrlen,\n" +
                          static_indent_str + "                        uint8_t *htb_hlen,\n" +
                          static_indent_str + "                        uint8_t *htb_hlen_cnt,\n" +
                          static_indent_str + "                        void *htb_hcod,\n" +
                          static_indent_str + "                        uint8_t *htb_idx)\n" +
                          "{\n" +
                          indent + "assert(htb);\n" +
                          indent + "assert(htb_hlen_arrlen > 0);\n" +
                          indent + "assert(htb_hlen);\n" +
                          indent + "assert(htb_hlen_cnt);\n" +
                          indent + "assert(htb_hcod);\n" +
                          indent + "assert(htb_idx);\n" +
                          '\n' +
                          indent + "htb->num = htb_num;\n" +
                          indent + "htb->xy_max = htb_xy_max;\n" +
                          indent + "htb->hlen_arrlen = htb_hlen_arrlen;\n" +
                          indent + "htb->hlen = htb_hlen;\n" +
                          indent + "htb->hlen_cnt = htb_hlen_cnt;\n" +
                          indent + "htb->hcod = htb_hcod;\n" +
                          indent + "htb->idx = htb_idx;\n"
                          "}")

# Initializing the structs
struct_init_str = static_str + "huffman_table_t"
for table_number in table_numbers:
    struct_init_str += ' ' + s_prefix_str + "htb_" + table_number + ','
struct_init_str = struct_init_str[:-1] + ';'

file_text_str = ("#include <stdint.h>\n#include <assert.h>\n\n\n" + 
                 heading_str + '\n' + struct_str + "\n\n" +
                 init_huffman_table_str +
                 "\n\n\n" + all_table_str + "\n\n" + struct_init_str + "\n\n" +
                 all_arr_to_struct_str + '\n')

with open(file_name + file_extension, 'w') as text_file:
    text_file.write(file_text_str)