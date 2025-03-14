#include "mp3lite.h"

#include <assert.h>
#include <stdbool.h>

/*
 * protection   If 1, CRC protected
 *              If 0, no redundancy
 *              (Note: ISO/IEC 11172-3 has '1' if no redundancy)
 *
 * mode         0: stereo
 *              1: joint stereo 
 *              2: dual channel
 *              3: singe channel
 *
 * mode_ext     Indicate which type of joint stereo coding method
 *                  intensity_stereo    ms_stereo
 *              0   off                 off
 *              1   on                  off
 *              2   off                 on
 *              3   on                  on
 *
 * emphasis     Indicate which type of de-emphasis
 *              (Note: not commonly used)
 *              0: none
 *              1: 50/15 ms
 *              2: reserved
 *              3: CCITT J.17
 *
 * bitrate      The bitrate in kbits/s
 *              If 0, the bitrate is free
 *
 * freq         The sampling frequency in Hz
 *
 * padding      The number of bytes of padding added to the frame
 */
typedef struct {
    uint8_t id;
    uint8_t layer;
    uint8_t protection;
    uint8_t mode;
    uint8_t mode_ext;
    uint8_t emphasis;
    uint16_t bitrate;
    uint16_t freq;
    uint16_t padding;
} frame_header_info_t;

static uint8_t s_decode_frame_header(uint32_t *frame_header);