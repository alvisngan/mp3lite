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
 * Testing Version ID, currently only supporting MPEG-1
 */
static bool s_test_decode_frame_header_ver_t0(void)
{
    bool test_0 = false;

    /* ================ MPEG1 ================ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00180000;
    header_info_t header_info;
    bool ver_1 = s_decode_frame_header_ver(frame_header, &header_info);
    if (ver_1)
    {
        ver_1 = (header_info.ver == 1) ? true : false;
    }

    /* ================ MPEG2 ================ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 0000 0000 0000 0000 0000 */
    frame_header = 0x00100000;
    bool ver_2 = s_decode_frame_header_ver(frame_header, &header_info);
    ver_2 = !ver_2; // return false for non-MPEG-1 versions
    if (ver_2)
    {
        ver_2 = (header_info.ver == 2) ? true : false;
    }

    /* =============== MPEG2.5 =============== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 0000 0000 0000 0000 0000 */
    frame_header = 0x00000000;
    bool ver_25 = s_decode_frame_header_ver(frame_header, &header_info);
    ver_25 = !ver_25; // return false for non-MPEG-1 versions
    if (ver_25)
    {
        ver_25 = (header_info.ver == 25) ? true : false;
    }

    /* =============== reserved ============== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 1000 0000 0000 0000 0000 */
    frame_header = 0x00080000;
    bool ver_reserv = s_decode_frame_header_ver(frame_header, &header_info);
    ver_reserv = !ver_reserv; // return false for non-MPEG-1 versions
    if (ver_reserv)
    {
        ver_reserv = (header_info.ver == 0) ? true : false;
    }

    test_0 = ver_1 && ver_2 && ver_25 && ver_reserv;

    return test_0;
}


/*
 * TEST_1
 *
 * Test if s_decode_frame_header returns the correct bitfield
 */
static bool s_test_decode_frame_header_ver_t1(void)
{
    bool test_1;

    /* ================ MPEG1 ================ */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0001 1000 0000 0000 0000 0000 */
    uint32_t frame_header = 0x00180000;
    header_info_t header_info;

    /* s_decode_frame_header will swap endianness */
    frame_header = s_swap_endian_u32(frame_header);
    uint8_t ver_1_return = s_decode_frame_header(frame_header, &header_info);
    bool ver_1 = (ver_1_return & DECODE_HEADER_ERR_VERSION) ? false : true;

    /* =============== reserved ============== */
    /* AAAA AAAA AAAB BCCD EEEE FFGH IIJJ KLMM */
    /* 0000 0000 0000 1000 0000 0000 0000 0000 */
    frame_header = 0x00080000;
    frame_header = s_swap_endian_u32(frame_header);
    uint8_t reserv_return = s_decode_frame_header(frame_header, &header_info);
    bool reserv = (reserv_return & DECODE_HEADER_ERR_VERSION) ? true : false;

    test_1 = ver_1 && reserv;

    return test_1;
}


int main(void)
{
    int exit_code = 0;

    if (!s_test_decode_frame_header_ver_t0())
    {
        exit_code |= TEST_0_FAILED;
    }

    if (!s_test_decode_frame_header_ver_t1())
    {
        exit_code |= TEST_1_FAILED;
    }

    if (exit_code)
    {
        printf("    EXIT_CODE: %d\n", exit_code);
    }

    return exit_code;
}