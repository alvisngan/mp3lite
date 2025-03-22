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
#include "../test_exit_code.h"

#include <stdio.h>

/*
 * TEST_0
 *
 * Test s_decode_frame_header_layer return and header_info.layer 
 * if header indicates Layer 1
 *
 */
static bool s_test_decode_frame_header_layer_t0(void)
{
    /*
     *  CC  Description
     *  --------------- 
     *  00  reserved
     *  01  Layer 3
     *  10  Layer 2
     *  11  Layer 1
     */

    bool test_0 = false;

    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0110 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00060000;
    frame_header_info_t header_info;
    
    test_0 = s_decode_frame_header_layer(frame_header, &header_info);

    test_0 = !test_0; // The func returns false for non-layer 3 layers
    if (test_0)
    {    
        test_0 = (header_info.layer == 1) ? true : false;
    }

    return test_0;
}


/*
 * TEST_1
 *
 * Test s_decode_frame_header_layer return and header_info.layer 
 * if header indicates Layer 2
 */
static bool s_test_decode_frame_header_layer_t1(void)
{
    /*
     *  CC  Description
     *  --------------- 
     *  00  reserved
     *  01  Layer 3
     *  10  Layer 2
     *  11  Layer 1
     */
     
    bool test_1 = false;

    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0100 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00040000;
    frame_header_info_t header_info;
    
    test_1 = s_decode_frame_header_layer(frame_header, &header_info);

    test_1 = !test_1; // The func returns false for non-layer 3 layers
    if (test_1)
    {
        test_1 = (header_info.layer == 2) ? true : false;
    }

    return test_1;
}


 /*
 * TEST_2
 *
 * Test s_decode_frame_header_layer return and header_info.layer 
 * if header indicates Layer 3
 *
 */
static bool s_test_decode_frame_header_layer_t2(void)
{
    /*
     *  CC  Description
     *  --------------- 
     *  00  reserved
     *  01  Layer 3
     *  10  Layer 2
     *  11  Layer 1
     */

    bool test_2 = false;

    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0010 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00020000;
    frame_header_info_t header_info;
    
    test_2 = s_decode_frame_header_layer(frame_header, &header_info);

    if (test_2)
    {
        test_2 = (header_info.layer == 3) ? true : false;
    }

    return test_2;
}


/*
 * TEST_3
 *
 * Test s_decode_frame_header_layer return and header_info.layer 
 * if header indicates Layer number that's not 1, 2, or 3
 *
 */
 static bool s_test_decode_frame_header_layer_t3(void)
 {
    /*
     *  CC  Description
     *  --------------- 
     *  00  reserved
     *  01  Layer 3
     *  10  Layer 2
     *  11  Layer 1
     */

    bool test_3 = false;

    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0000 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00000000;
    frame_header_info_t header_info;
    
    test_3 = s_decode_frame_header_layer(frame_header, &header_info);

    test_3 = !test_3; // The func returns false for non-layer 3 layers
    if (test_3)
    {
        /* layer number can be anything other than 1, 2, 3*/
        bool test_3_bool = (header_info.layer != 3 ||
                            header_info.layer != 2 ||
                            header_info.layer != 1); 
        test_3 = (test_3_bool) ? true : false;
    }

    return test_3;
}


/*
 * TEST_4
 *
 * Test if s_decode_frame_header returns the correct bitfield
 *
 */
static bool s_test_decode_frame_header_layer_t4(void)
{
    bool test_4 = false;

    /* =============== Layer 3 =============== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0010 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00020000;
    frame_header_info_t header_info;

    /* s_decode_frame_header will swap endianess */
    frame_header = s_swap_endian_u32(frame_header);
    uint8_t layer3_return = s_decode_frame_header(frame_header, &header_info);
    bool layer3 = (layer3_return & DECODE_HEADER_ERR_LAYER) ? false : true; //result should be zero

    /* Err flag will be set if layer is not 3 (if the above tests pass) */
    /* We don't need to check every possible layers                     */

    /* =============== Layer 2 =============== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0100 0000 0000 0000 0000 */
    frame_header = 0x00040000;
    uint8_t layer2_return = s_decode_frame_header(frame_header, &header_info);
    bool layer2 = (layer2_return & DECODE_HEADER_ERR_LAYER) ? true : false; //result should NOT be zero

    return (layer3 && layer2);
}


int main(void)
{
    int exit_code = 0;

    if (!s_test_decode_frame_header_layer_t0())
    {
        exit_code |= TEST_0_FAILED;
    }

    if (!s_test_decode_frame_header_layer_t1())
    {
        exit_code |= TEST_1_FAILED;
    }

    if (!s_test_decode_frame_header_layer_t2())
    {
        exit_code |= TEST_2_FAILED;
    }

    if (!s_test_decode_frame_header_layer_t3())
    {
        exit_code |= TEST_3_FAILED;
    }

    if (!s_test_decode_frame_header_layer_t4())
    {
        exit_code |= TEST_4_FAILED;
    }

    if (exit_code)
    {
        printf("    EXIT_CODE: %d\n", exit_code);
    }

    return exit_code;
}
