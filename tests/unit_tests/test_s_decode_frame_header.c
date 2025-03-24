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

///TODO: padding
///TODO: copy header bitstream from 11172-4

/*
 * Bitrate, frequency, version, and layer are tested in seperate files
 *
 * Here we will test these in isolation:
 *      Syncword
 *      Protection
 *      Padding
 *      Channel mode
 *      Mode extension
 *      Emphasis
 *
 * Then we will test actual headers from MP3 files
 */

/*
 * TEST_0 
 *
 * Testing syncword
 */
static bool s_test_decode_frame_header_t0(void)
{
    bool test_0;

    /* =========== 11bits syncword =========== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 1111 1111 1110 0000 0000 0000 0000 0000 */
    uint32_t frame_header = 0xFFE00000;
    frame_header_info_t header_info;

    /* s_decode_frame_header will swap endianness */
    frame_header = s_swap_endian_u32(frame_header);
    uint8_t test_11b = s_decode_frame_header(frame_header, &header_info);
    test_11b = (test_11b & DECODE_HEADER_ERR_SYNCWORD) ? false : true; //result should be zero

    /* =========== 12bits syncword =========== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 1111 1111 1111 0000 0000 0000 0000 0000 */
    frame_header = 0xFFF00000;

    /* s_decode_frame_header will swap endianness */
    frame_header = s_swap_endian_u32(frame_header);
    uint8_t test_12b = s_decode_frame_header(frame_header, &header_info);
    test_12b = (test_12b & DECODE_HEADER_ERR_SYNCWORD) ? false : true; //result should be zero


    /* =========== invalid syncword ========== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0000 0000 0000 0000 0000 */
    frame_header = 0x00000000;
    uint8_t test_inv = s_decode_frame_header(frame_header, &header_info);
    test_inv = (test_inv & DECODE_HEADER_ERR_SYNCWORD) ? true : false;

    test_0 = test_11b && test_12b && test_inv;

    return test_0;
}


/*
 * TEST_1
 *
 * Testing protection
 */
static bool s_test_decode_frame_header_t1(void)
{
    bool test_1 = false;

    /* ============ CRC Protected ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0000 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00000000;
    frame_header_info_t header_info;

    /* s_decode_frame_header will swap endianness */
    frame_header = s_swap_endian_u32(frame_header);
    s_decode_frame_header(frame_header, &header_info);
    bool test_yes_crc = (header_info.protection == 1) ? true : false;

    /* ============ Not Protected ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0001 0000 0000 0000 0000 */
    frame_header = 0x00010000;

    /* s_decode_frame_header will swap endianness */
    frame_header = s_swap_endian_u32(frame_header);
    s_decode_frame_header(frame_header, &header_info);
    bool test_no_crc = (header_info.protection == 0) ? true : false;

    test_1 = test_yes_crc && test_no_crc;

    return test_1;
}


/*
 * TEST_2
 *
 * Testing channel mode
 */
static bool s_test_decode_frame_header_t2(void)
{
    bool test_2 = false;

    /* ================ stereo =============== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0000 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00000000;
    frame_header_info_t header_info;

    /* s_decode_frame_header will swap endianness */
    frame_header = s_swap_endian_u32(frame_header);
    s_decode_frame_header(frame_header, &header_info);
    bool test_stereo = (header_info.mode == 0) ? true : false;

    /* ============= joint-stereo ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0000 0000 0000 0100 0000 */
    frame_header = 0x00000040;

    /* s_decode_frame_header will swap endianness */
    frame_header = s_swap_endian_u32(frame_header);
    s_decode_frame_header(frame_header, &header_info);
    bool test_j_stereo = (header_info.mode == 1) ? true : false;

    /* =============== dual ch =============== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0000 0000 0000 1000 0000 */
    frame_header = 0x00000080;

    /* s_decode_frame_header will swap endianness */
    frame_header = s_swap_endian_u32(frame_header);
    s_decode_frame_header(frame_header, &header_info);
    bool test_dch = (header_info.mode == 2) ? true : false;

    /* ============== single ch ============== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0000 0000 0000 1100 0000 */
    frame_header = 0x000000C0;

    /* s_decode_frame_header will swap endianness */
    frame_header = s_swap_endian_u32(frame_header);
    s_decode_frame_header(frame_header, &header_info);
    bool test_mono = (header_info.mode == 3) ? true : false;

    test_2 = test_stereo && test_j_stereo && test_dch && test_mono;

    return test_2;
}


/*
 * TEST_3
 *
 * Testing channel mode extension
 * No need to test everything since the function simply bit-shift
 */
static bool s_test_decode_frame_header_t3(void)
{
    bool test_3 = false;
    
    /* ================ JJ=10 ================ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0000 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00000020;
    frame_header_info_t header_info;
    
    /* s_decode_frame_header will swap endianness */
    frame_header = s_swap_endian_u32(frame_header);
    s_decode_frame_header(frame_header, &header_info);
    bool test_jj_10 = (header_info.mode_ext == 2) ? true : false;
    
    test_3 = test_jj_10;
    
    return test_3;
}


/*
 * TEST_3
 *
 * Testing emphasis
 * No need to test everything since the function simply bit-shift
 */
static bool s_test_decode_frame_header_t4(void)
{
    bool test_4 = false;
    
    /* ================ MM=10 ================ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0000 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00000002;
    frame_header_info_t header_info;
    
    /* s_decode_frame_header will swap endianness */
    frame_header = s_swap_endian_u32(frame_header);
    s_decode_frame_header(frame_header, &header_info);
    bool test_mm_10 = (header_info.emphasis == 2) ? true : false;
    
    test_4 = test_mm_10;
    
    return test_4;
}


int main(void)
{
    int exit_code = 0;

    if (!s_test_decode_frame_header_t0())
    {
        exit_code |= TEST_0_FAILED;
    }

    if (!s_test_decode_frame_header_t1())
    {
        exit_code |= TEST_1_FAILED;
    }

    if (!s_test_decode_frame_header_t2())
    {
        exit_code |= TEST_2_FAILED;
    }

    if (!s_test_decode_frame_header_t3())
    {
        exit_code |= TEST_3_FAILED;
    }

    if (!s_test_decode_frame_header_t4())
    {
        exit_code |= TEST_4_FAILED;
    }

    if (exit_code)
    {
        printf("    EXIT_CODE: %d\n", exit_code);
    }

    return exit_code;
}