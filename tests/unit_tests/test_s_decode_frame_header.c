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

#include <stdio.h>

static bool s_test_decode_frame_header_layer(void);
static bool s_test_decode_frame_header_bitrate(void);
static bool s_test_decode_frame_header_freq(void);
static bool s_test_decode_frame_header(void);

static bool s_test_decode_frame_header_layer(void)
{
    /*
     *  CC  Description
     *  --------------- 
     *  00  reserved
     *  01  Layer 3
     *  10  Layer 2
     *  11  Layer 1
     */
     const char *TEST_GROUP_NAME = "DECODE FRAME HEADER LAYER";

    /* =============================== TEST 0 =============================== */
    bool test_0 = false;

    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0110 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00060000;
    frame_header_info_t header_info_0;
    
    test_0 = s_decode_frame_header_layer(frame_header, &header_info_0);

    test_0 = !test_0; // The func returns false for non-layer 3 layers
    if (test_0)
    {    
        test_0 = (header_info_0.layer == 1) ? true : false;
    }

    if (!test_0)
    {
        printf("%s - TEST 0 FAILED\n", TEST_GROUP_NAME);
    }

    /* =============================== TEST 1 =============================== */
    bool test_1 = false;

    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0100 0000 0000 0000 0000 */
    frame_header = 0x00040000;
    frame_header_info_t header_info_1;
    
    test_1 = s_decode_frame_header_layer(frame_header, &header_info_1);

    test_1 = !test_1; // The func returns false for non-layer 3 layers
    if (test_1)
    {
        test_1 = (header_info_1.layer == 2) ? true : false;
    }

    if (!test_1)
    {
        printf("%s - TEST 1 FAILED\n", TEST_GROUP_NAME);
    }

    /* =============================== TEST 2 =============================== */
    bool test_2 = false;

    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0010 0000 0000 0000 0000 */
    frame_header = 0x00020000;
    frame_header_info_t header_info_2;
    
    test_2 = s_decode_frame_header_layer(frame_header, &header_info_2);

    if (test_2)
    {
        test_2 = (header_info_2.layer == 3) ? true : false;
    }

    if (!test_2)
    {
        printf("%s - TEST 2 FAILED\n", TEST_GROUP_NAME);
    }

    /* =============================== TEST 3 =============================== */
    bool test_3 = false;

    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0000 0000 0000 0000 0000 */
    frame_header = 0x00000000;
    frame_header_info_t header_info_3;
    
    test_3 = s_decode_frame_header_layer(frame_header, &header_info_3);

    test_3 = !test_3; // The func returns false for non-layer 3 layers
    if (test_3)
    {
        /* layer number can be anything other than 1, 2, 3*/
        bool test_3_bool = (header_info_3.layer != 3 ||
                            header_info_3.layer != 2 ||
                            header_info_3.layer != 1); 
        test_3 = (test_3_bool) ? true : false;
    }

    if (!test_3)
    {
        printf("%s - TEST 3 FAILED\n", TEST_GROUP_NAME);
    }


    return (test_0 && test_1 && test_2 && test_3);
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

int main(void)
{
    s_test_decode_frame_header_layer();
}
