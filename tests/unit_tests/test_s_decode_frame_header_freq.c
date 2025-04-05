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
 * Testing s_decode_frame_header_bitrate return and header_info.bitrate
 *
 * MPEG Audio Version 1
 */
static bool s_test_decode_frame_header_freq_t0(void)
{
    bool test_0 = false;

    /* ============ MPEG1 44100Hz ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00180000;
    header_info_t header_info;
    
    bool test_v1_44khz = s_decode_frame_header_freq(frame_header, &header_info);

    if (test_v1_44khz)
    {    
        test_v1_44khz = (header_info.freq == 44100) ? true : false;
    }

    /* ============ MPEG1 48000Hz ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 0000 0100 0000 0000 */
    frame_header = 0x00180400;
    
    bool test_v1_48khz = s_decode_frame_header_freq(frame_header, &header_info);

    if (test_v1_48khz)
    {    
        test_v1_48khz = (header_info.freq == 48000) ? true : false;
    }

    /* ============ MPEG1 32000Hz ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 0000 1000 0000 0000 */
    frame_header = 0x00180800;
    
    bool test_v1_32khz = s_decode_frame_header_freq(frame_header, &header_info);

    if (test_v1_32khz)
    {    
        test_v1_32khz = (header_info.freq == 32000) ? true : false;
    }

    test_0 = test_v1_44khz && test_v1_48khz && test_v1_32khz;

    return test_0;
}


 /*
 * TEST_01
 *
 * Testing s_decode_frame_header_bitrate return and header_info.bitrate
 *
 * MPEG Audio Version 1
 * reserved frequency
 */
static bool s_test_decode_frame_header_freq_t1(void)
{
    bool test_1 = false;

    /* =========== MPEG-1 reserved =========== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 0000 1100 0000 0000 */
    uint32_t frame_header = 0x00180C00;
    header_info_t header_info;
    
    bool test_v1_reserv = s_decode_frame_header_freq(frame_header, &header_info);

    /* reserved ffrequency returns failure */
    test_v1_reserv = !test_v1_reserv;
    if (test_v1_reserv)
    {    
        test_v1_reserv = (header_info.freq == 0) ? true : false;
    }

    test_1 = test_v1_reserv;

    return test_1;
}


/*
 * TEST_2
 *
 * Test if s_decode_frame_header returns the correct bitfield
 */
static bool s_test_decode_frame_header_freq_t2(void)
{
    bool test_2;

    /* ============ MPEG1 48000Hz ============ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 0000 0100 0000 0000 */
    uint32_t frame_header = 0x00180400;
    header_info_t header_info;

    /* s_decode_frame_header will swap endianness */
    frame_header = s_swap_endian_u32(frame_header);
    uint8_t f48khz_return = s_decode_frame_header(frame_header, &header_info);
    bool f48khz = (f48khz_return & DECODE_HEADER_ERR_FREQ) ? false : true;

    /* =========== MPEG-1 reserved =========== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 0000 1100 0000 0000 */
    frame_header = 0x00180C00;
    frame_header = s_swap_endian_u32(frame_header);
    uint8_t reserv_return = s_decode_frame_header(frame_header, &header_info);
    bool reserv = (reserv_return & DECODE_HEADER_ERR_FREQ) ? true : false;

    test_2 = f48khz && reserv;

    return test_2;
}


int main(void)
{
    int exit_code = 0;

    if (!s_test_decode_frame_header_freq_t0())
    {
        exit_code |= TEST_0_FAILED;
    }

    if (!s_test_decode_frame_header_freq_t1())
    {
        exit_code |= TEST_1_FAILED;
    }

    if (!s_test_decode_frame_header_freq_t2())
    {
        exit_code |= TEST_2_FAILED;
    }

    if (exit_code)
    {
        printf("    EXIT_CODE: %d\n", exit_code);
    }

    return exit_code;
}