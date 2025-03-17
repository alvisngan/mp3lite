/*
 * useful link: http://www.mp3-tech.org/programmer/frame_header.html
 *
 *       AAAAAAAA AAABBCCD EEEEFFGH IIJJKLMM 
 *       
 *          Length (bits)   Discription
 *      A   11              Syncword
 *      B   2               Version ID
 *      C   2               Layer Description
 *      D   1               Protection bit
 *      E   4               Bitrate index
 *      F   2               Sammpling rate frequency index
 *      G   1               Padding bit
 *      H   1               Private bit
 *      I   2               Channel mode index
 *      J   2               Mode extension index
 *      K   1               Copyright
 *      L   1               Original
 *      M   2               Emphasis index
 *
 */

#include "../../mp3lite.c"

#include <printf.h>

static bool s_test_decode_frame_header_layer(void);
static bool s_test_decode_frame_header_bitrate(void);
static bool s_test_decode_frame_header_freq(void);
static bool s_test_decode_frame_header(void);

static bool s_test_decode_frame_header_layer(void)
{
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0110 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00060000;

    return false;
}


static bool s_test_decode_frame_header_bitrate(void)
{
    return false;
}


static bool s_test_decode_frame_header_freq(void)
{
    return false;
}


static bool s_test_decode_frame_header(void)
{
    return false;
}



// /* Error log for s_decode_frame_header() */
// #define DECODE_HEADER_ERR_SYNCWORD  0x01
// #define DECODE_HEADER_ERR_LAYER     0x02
// #define DECODE_HEADER_ERR_BITRATE   0x04
// #define DECODE_HEADER_ERR_FREQ      0x08

