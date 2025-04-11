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

for freq in freq_list:
    for block in block_types:
        print(freq)
        filename = "scalefactor_bands_" + block + "_block_" + freq + ".csv"

        df = pd.read_csv(filename, dtype = int)

        bitsize_str = ("uint8_t " + s_prefix_str  + "scf_" + freq + "_" +
                          block + "_ibitsize = {")
        for num in df.loc[:, "bitsize"].astype(str).values:
            bitsize_str += num + ", "
        bitsize_str = static_str + const_str + bitsize_str[:-2] + "};"

        idx_start_str = ("uint16_t " + s_prefix_str  + "scf_" + freq + "_" +
                          block + "_idx_start = {")
        for num in df.loc[:, "idx_start"].astype(str).values:
            idx_start_str += num + ", "
        idx_start_str = static_str + const_str + idx_start_str[:-2] + "};"


        idx_end_str = ("uint16_t " + s_prefix_str  + "scf_" + freq + "_" +
                          block + "_idx_end = {")
        for num in df.loc[:, "idx_end"].astype(str).values:
            idx_end_str += num + ", "
        idx_end_str = static_str + const_str + idx_end_str[:-2] + "};"

        file_text_str += bitsize_str + '\n' + idx_start_str + '\n' + idx_end_str + "\n\n"

with open(file_name + file_extension, 'w') as text_file:
    text_file.write(file_text_str)