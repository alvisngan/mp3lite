#include "../../mp3lite.c"
#include "../test_exit_code.h"

#include <stdio.h>

///TODO: si.bit has frames with different scfsi 


/*
* TEST_0
*
* Testing mono, main_data_begin
*/
static bool s_test_decode_side_info_t0(void)
{
    bool test_0 = false;

    /* si.bit frame 1: mono, 17B, main_data_begin = 0 */
    uint32_t si_bit_f1_header = 0xfffb50c0;
    si_bit_f1_header = s_swap_endian_u32(si_bit_f1_header);
    const uint8_t si_bit_f1[17] = {
        0x00, 0x00, 0x0b, 0xa8, 0x2a,
        0xc8, 0x0b, 0xde, 0xe4, 0x81,
        0x75, 0x05, 0x59, 0x01, 0x7b,
        0xdc, 0x90
    };

    side_info_t side_info;
    header_info_t header_info;
    (void) s_decode_frame_header(si_bit_f1_header, &header_info);
    uint8_t f1 = s_decode_side_info(si_bit_f1, &side_info, &header_info);
    bool f1_b = false;
    if (!f1)
    {
        f1_b = (side_info.main_data_begin == 0) ? true : false;
    }
    
    /* si.bit frame 21: mono, 17B, main_data_begin = 511 */
    uint32_t si_bit_f21_header = 0xfffb52c0;
    si_bit_f1_header = s_swap_endian_u32(si_bit_f21_header);
    const uint8_t si_bit_f21[17] = {
        0xff, 0x80, 0x2b, 0xac, 0x9c, 
        0xc8, 0x0b, 0xde, 0xeb, 0x05, 
        0x75, 0x93, 0x99, 0x01, 0x7b, 
        0xdd, 0x60
    };
    (void) s_decode_frame_header(si_bit_f21_header, &header_info);
    uint8_t f21 = s_decode_side_info(si_bit_f21, &side_info, &header_info);
    bool f21_b = false;
    if (!f21)
    {
        f21_b = (side_info.main_data_begin == 511) ? true : false;
    }


    test_0 = f1_b && f21_b;

    return test_0;
}


/* si.bit frame 43: mono, 17B, global_gain = 200, scalefactor = 0 */


/* si.bit frame 64: mono, 17B */
/* GR0: scalefactors: slen1 = 0 slen2 = 1 */
/* GR1: scalefactors: slen1 = 0 slen2 = 2 */


/* si.bit frame 72: mono, 17B */
/* GR0: scalefactors: slen1 = 0 slen2 = 1 scfsi = 0001 */
/* GR1: scalefactors: slen1 = 0 slen2 = 2 */


int main(void)
{
    int exit_code = 0;

    if (!s_test_decode_side_info_t0())
    {
        exit_code |= TEST_0_FAILED;
    }

    // if (!s_test_decode_frame_header_ver_t1())
    // {
    //     exit_code |= TEST_1_FAILED;
    // }

    if (exit_code)
    {
        printf("    EXIT_CODE: %d\n", exit_code);
    }


    return exit_code;
}