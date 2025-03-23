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
 * Testing every combination of bitrate would be tedious and error-prone,
 * therefore I will select some coomon and some random bitrates to test
 *
 * Bitrate MUST be tested with Version ID as different MPEG Audio versions 
 * have different bitrate tables
 */

/*
 * TEST_0 
 *
 * Testing s_decode_frame_header_bitrate return and header_info.bitrate
 *
 * MPEG Audio Version 1
 * Bitrate tested (kbits/s): 320, 256, 192, 160, 128, 80, 48, 32
 */
static bool s_test_decode_frame_header_bitrate_t0(void)
{
    bool test_0 = false;

    /* ============ MPEG1 320kbps ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 1101 0000 0000 0000 */
    uint32_t frame_header = 0x0018E000;
    frame_header_info_t header_info;
    
    bool test_v1_320kbps = s_decode_frame_header_bitrate(frame_header, &header_info);

    if (test_v1_320kbps)
    {    
        test_v1_320kbps = (header_info.bitrate == 320) ? true : false;
    }

    /* ============ MPEG1 256kbps ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 1101 0000 0000 0000 */
    frame_header = 0x0018D000;
    
    bool test_v1_256kbps = s_decode_frame_header_bitrate(frame_header, &header_info);

    if (test_v1_256kbps)
    {    
        test_v1_256kbps = (header_info.bitrate == 256) ? true : false;
    }

    /* ============ MPEG1 192kbps ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 1011 0000 0000 0000 */
    frame_header = 0x0018B000;
    
    bool test_v1_192kbps = s_decode_frame_header_bitrate(frame_header, &header_info);

    if (test_v1_192kbps)
    {    
        test_v1_192kbps = (header_info.bitrate == 192) ? true : false;
    }

    /* ============ MPEG1 160kbps ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 1010 0000 0000 0000 */
    frame_header = 0x0018A000;
    
    bool test_v1_160kbps = s_decode_frame_header_bitrate(frame_header, &header_info);

    if (test_v1_160kbps)
    {    
        test_v1_160kbps = (header_info.bitrate == 160) ? true : false;
    }

    /* ============ MPEG1 128kbps ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 1001 0000 0000 0000 */
    frame_header = 0x00189000;
    
    bool test_v1_128kbps = s_decode_frame_header_bitrate(frame_header, &header_info);

    if (test_v1_128kbps)
    {    
        test_v1_128kbps = (header_info.bitrate == 128) ? true : false;
    }

    /* ============ MPEG1  80kbps ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 0110 0000 0000 0000 */
    frame_header = 0x00186000;
    
    bool test_v1_80kbps = s_decode_frame_header_bitrate(frame_header, &header_info);

    if (test_v1_80kbps)
    {    
        test_v1_80kbps = (header_info.bitrate == 80) ? true : false;
    }

    /* ============ MPEG1  48kbps ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 0010 0000 0000 0000 */
    frame_header = 0x00183000;
    
    bool test_v1_48kbps = s_decode_frame_header_bitrate(frame_header, &header_info);

    if (test_v1_48kbps)
    {    
        test_v1_48kbps = (header_info.bitrate == 48) ? true : false;
    }

    /* ============ MPEG1  32kbps ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 0001 0000 0000 0000 */
    frame_header = 0x00181000;
    
    bool test_v1_32kbps = s_decode_frame_header_bitrate(frame_header, &header_info);

    if (test_v1_32kbps)
    {    
        test_v1_32kbps = (header_info.bitrate == 32) ? true : false;
    }

    test_0 = (test_v1_320kbps && test_v1_256kbps && test_v1_192kbps &&
              test_v1_160kbps && test_v1_128kbps && test_v1_80kbps  &&
              test_v1_48kbps  && test_v1_32kbps);

    return test_0;
}


/*
 * TEST_1
 *
 * Testing s_decode_frame_header_bitrate return and header_info.bitrate
 *
 * MPEG Audio Version 1
 * Invalid bitrates
 */
static bool s_test_decode_frame_header_bitrate_t1(void)
{
    bool test_1 = false;

    /* ============= MPEG-1 free ============= */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00180000;
    frame_header_info_t header_info;

    bool test_v1_free = s_decode_frame_header_bitrate(frame_header, &header_info);

    /* free bitrate returns success */
    if (test_v1_free)
    {    
        test_v1_free = (header_info.bitrate == 0) ? true : false;
    }

    /* =========== MPEG1 forbidden =========== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 1111 0000 0000 0000 */
    frame_header = 0x0018F000;

    bool test_v1_forb = s_decode_frame_header_bitrate(frame_header, &header_info);

    /* forbidden bitrate returns failure */
    test_v1_forb = !test_v1_forb;
    if (test_v1_forb)
    {    
        test_v1_forb = (header_info.bitrate == 0) ? true : false;
    }
    printf("%d, %d\n",test_v1_free ,test_v1_forb);

    test_1 = test_v1_free && test_v1_forb;

    return test_1;
}


/*
 * TEST_2
 *
 * Test if s_decode_frame_header returns the correct bitfield
 */
static bool s_test_decode_frame_header_bitrate_t2(void)
{
    bool test_2 = false;

    /* =========== MPEG1 forbidden =========== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 1111 0000 0000 0000 */
    uint32_t frame_header = 0x0018F000;
    frame_header_info_t header_info;

    /* s_decode_frame_header will swap endianness */
    frame_header = s_swap_endian_u32(frame_header);
    uint8_t forb_return = s_decode_frame_header(frame_header, &header_info);
    bool forb = (forb_return & DECODE_HEADER_ERR_BITRATE) ? true : false;

    /* ============= MPEG-1 free ============= */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 0000 0000 0000 0000 */
    frame_header = 0x00180000;
    frame_header = s_swap_endian_u32(frame_header);
    uint8_t free_return = s_decode_frame_header(frame_header, &header_info);
    bool free = (free_return & DECODE_HEADER_ERR_BITRATE) ? false : true; //result should be zero

    test_2 = forb && free;

    return test_2;
}


int main(void)
{
    int exit_code = 0;

    if (!s_test_decode_frame_header_bitrate_t0())
    {
        exit_code |= TEST_0_FAILED;
    }

    if (!s_test_decode_frame_header_bitrate_t1())
    {
        exit_code |= TEST_1_FAILED;
    }

    if (!s_test_decode_frame_header_bitrate_t2())
    {
        exit_code |= TEST_2_FAILED;
    }

    if (exit_code)
    {
        printf("    EXIT_CODE: %d\n", exit_code);
    }

    return exit_code;
}