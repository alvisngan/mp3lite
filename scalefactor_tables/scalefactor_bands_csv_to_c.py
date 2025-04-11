import pandas as pd

# Settings
indent = "    "                     # 4 spaces as indent
static_var = True                   # Apply static keyword to variables
static_prefix = True                # Apply s_ prefix on static variable names
const_var = True                    # Applt const keyword to variables
file_name = "scalefactor_tables"    # Output filename
file_extension = ".c"               # Output file extension

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


block_types = ["long", "short"]
freq_list = ["32000hz", "44100hz", "48000hz"]
file_text_str = ''
table_str = ''

for freq in freq_list:
    for block in block_types:
        filename = "scalefactor_bands_" + block + "_block_" + freq + ".csv"

        df = pd.read_csv(filename, dtype = int)

        bitsize_str = ("uint8_t " + s_prefix_str  + "scalefac_" + freq + "_" +
                          block + "_bitsize[] = {")
        for num in df.loc[:, "bitsize"].astype(str).values:
            bitsize_str += num + ", "
        bitsize_str = static_str + const_str + bitsize_str[:-2] + "};"

        idx_start_str = ("uint16_t " + s_prefix_str  + "scalefac_" + freq +
                         "_" +block + "_idx_start[] = {")
        for num in df.loc[:, "idx_start"].astype(str).values:
            idx_start_str += num + ", "
        idx_start_str = static_str + const_str + idx_start_str[:-2] + "};"


        idx_end_str = ("uint16_t " + s_prefix_str  + "scalefac_" + freq + "_" +
                          block + "_idx_end[] = {")
        for num in df.loc[:, "idx_end"].astype(str).values:
            idx_end_str += num + ", "
        idx_end_str = static_str + const_str + idx_end_str[:-2] + "};"

        table_str += bitsize_str + '\n' + idx_start_str + '\n' + idx_end_str + "\n\n"

long_struct_str = ("typedef struct {\n" +
                   indent + "uint8_t bitsize[LONG_BLOCK_LEN];\n" +
                   indent + "uint16_t idx_start[LONG_BLOCK_LEN];\n" +
                   indent + "uint16_t idx_end[LONG_BLOCK_LEN];\n" +
                   "} scalefac_long_block_table_t;")

short_struct_str = ("typedef struct {\n" +
                    indent + "uint8_t bitsize[SHORT_BLOCK_LEN];\n" +
                    indent + "uint16_t idx_start[SHORT_BLOCK_LEN];\n" +
                    indent + "uint16_t idx_end[SHORT_BLOCK_LEN];\n" +
                    "} scalefac_short_block_table_t;")

file_text_str = table_str
with open(file_name + file_extension, 'w') as text_file:
    text_file.write(file_text_str)