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
    frame_header = 0x00182000;
    
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


int main(void)
{
    int exit_code = 0;

    if (!s_test_decode_frame_header_bitrate_t0())
    {
        exit_code |= TEST_0_FAILED;
    }

    return exit_code;
}