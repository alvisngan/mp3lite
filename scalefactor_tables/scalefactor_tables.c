typedef struct {
    uint16_t idx_start[LONG_BLOCK_LEN];
    uint16_t idx_end[LONG_BLOCK_LEN];
} scalefac_long_table_t;

typedef struct {
    uint16_t idx_start[SHORT_BLOCK_LEN];
    uint16_t idx_end[SHORT_BLOCK_LEN];
} scalefac_short_table_t;

static const scalefac_long_table_t s_scalefac_long_32000hz = {
    .idx_start = {0, 4, 8, 12, 16, 20, 24, 30, 36, 44, 54, 66, 82, 102, 126, 156, 194, 240, 296, 364, 448},
    .idx_end = {3, 7, 11, 15, 19, 23, 29, 35, 43, 53, 65, 81, 101, 125, 155, 193, 239, 295, 363, 447, 549}
};
static const scalefac_short_table_t s_scalefac_short_32000hz = {
    .idx_start = {0, 4, 8, 12, 16, 22, 30, 42, 58, 78, 104, 138},
    .idx_end = {3, 7, 11, 15, 21, 29, 41, 57, 77, 103, 137, 179}
};
static const scalefac_long_table_t s_scalefac_long_44100hz = {
    .idx_start = {0, 4, 8, 12, 16, 20, 24, 30, 36, 44, 52, 62, 74, 90, 110, 134, 162, 196, 238, 288, 342},
    .idx_end = {3, 7, 11, 15, 19, 23, 29, 35, 43, 51, 61, 73, 89, 109, 133, 161, 195, 237, 287, 341, 417}
};
static const scalefac_short_table_t s_scalefac_short_44100hz = {
    .idx_start = {0, 4, 8, 12, 16, 22, 30, 40, 52, 66, 84, 106},
    .idx_end = {3, 7, 11, 15, 21, 29, 39, 51, 65, 83, 105, 135}
};
static const scalefac_long_table_t s_scalefac_long_48000hz = {
    .idx_start = {0, 4, 8, 12, 16, 20, 24, 30, 36, 42, 50, 60, 72, 88, 106, 128, 156, 190, 230, 276, 330},
    .idx_end = {3, 7, 11, 15, 19, 23, 29, 35, 41, 49, 59, 71, 87, 105, 127, 155, 189, 229, 275, 329, 383}
};
static const scalefac_short_table_t s_scalefac_short_48000hz = {
    .idx_start = {0, 4, 8, 12, 16, 22, 28, 38, 50, 64, 80, 100},
    .idx_end = {3, 7, 11, 15, 21, 27, 37, 49, 63, 79, 99, 125}
};
