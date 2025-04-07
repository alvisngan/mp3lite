#include "../../mp3lite.c"
#include "../test_exit_code.h"

#include <stdio.h>


/*
 * TEST_0
 *
 * test
 */
static bool s_test_align_array(void)
{
    bool test_0 = false;

    uint8_t src[] = {0x3F, 0xFF, 0xC0};
    uint8_t dest[] = {0, 0, 0};

    s_align_array(dest, src, 2, 2);

    bool byte_1 = (dest[0] == 0xFFu) ? true : false;
    bool byte_2 = (dest[1] == 0xFFu) ? true : false;
    bool byte_3 = (dest[2] == 0x00u) ? true : false;

    test_0 = byte_1 && byte_2 && byte_3;

    return test_0;
}

int main(void)
{
    int exit_code = 0;

    if (!s_test_align_array())
    {
        exit_code |= TEST_0_FAILED;
    }

    // if (!s_test_decode_side_info_t1())
    // {
    //     exit_code |= TEST_1_FAILED;
    // }

    if (exit_code)
    {
        printf("    EXIT_CODE: %d\n", exit_code);
    }


    return exit_code;
}