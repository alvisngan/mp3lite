#include "mp3lite.h"

#include <assert.h>
#include <stdbool.h>

#define IS_BIG_ENDIAN (!*(unsigned char *)&(uint16_t){1})
static uint32_t s_swap_endian_u32(const uint32_t val);

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
    uint8_t ver;
    uint8_t layer;
    uint8_t protection;
    uint8_t mode;
    uint8_t mode_ext;
    uint8_t emphasis;
    uint16_t bitrate;
    uint16_t freq;
    uint16_t padding;
} frame_header_info_t;



/* Error log for s_decode_frame_header() */
#define DECODE_HEADER_ERR_SYNCWORD  0x01
#define DECODE_HEADER_ERR_VERSION   0x02
#define DECODE_HEADER_ERR_LAYER     0x04
#define DECODE_HEADER_ERR_BITRATE   0x08
#define DECODE_HEADER_ERR_FREQ      0x10

/* 
 * \param frame_header  The frame header bitstream stored as uint32_t,
 *                      including the 11/12 bits syncword
 *                      This function will swap frame_header endianness
 *
 * \param header_info   The address of the header information struct
 *
 * \return              0: sucess
 *                      
 *                      The error log is stored in a one byte bitfield
 *                      err_log = 0b0000DCBA
 *                      A: invalid syncword
 *                      B: invalid/unsupported layer type
 *                      C: invalid bitrate
 *                      D: invalid sampling frequency
 */
static uint8_t s_decode_frame_header(uint32_t frame_header, 
                                     frame_header_info_t *header_info);

/*
 * \return  true:   MPEG version 1
 *          false:  reserved (header_info->ver = 0) 
                    2 & 2.5 (for 2.5, header_info->ver = 25)
 */
static bool s_decode_frame_header_ver(uint32_t frame_header, 
                                      frame_header_info_t *header_info);

/*
 * This function will assign layer number to header_info, even for un-supported
 * layer types
 *
 * \return  true:   layer 3
 *          false:  all other layers
 */
static bool s_decode_frame_header_layer(const uint32_t frame_header, 
                                        frame_header_info_t *header_info);

/*
 * \return  if bitrate index is 0000, this function returns success
 *          if bitrate index is 1111, this function returns failure
 */
static bool s_decode_frame_header_bitrate(const uint32_t frame_header,
                                          frame_header_info_t *header_info);
/*
 * \return  if the frequency index is 11 (i.e. reserved), this function will
 *          return failure
 */
static bool s_decode_frame_header_freq(const uint32_t frame_header,
                                       frame_header_info_t *header_info);


static uint32_t s_swap_endian_u32(const uint32_t val)
{
    uint8_t *val_ptr = (uint8_t *) &val;

    return ((uint32_t) val_ptr[0] << 24 |
            (uint32_t) val_ptr[1] << 16 |
            (uint32_t) val_ptr[2] << 8 |
            (uint32_t) val_ptr[3]);
}


static uint8_t s_decode_frame_header(uint32_t frame_header, 
                                     frame_header_info_t *header_info)
{
    assert(header_info);

    uint8_t result = 0;

#if !defined (IS_BIG_ENDIAN) || !defined (MP3LITE_BIG_ENDIAN)
    /* Swaping byte order for little-endian systems (a.k.a. most systems) */
    frame_header = s_swap_endian_u32(frame_header);
#endif

    /* ensure syncword is valid (first 11 bits) */
    if ((frame_header & 0xFFE00000) != 0xFFE00000)
    {
        result |= DECODE_HEADER_ERR_SYNCWORD;
    }

    bool ver_b = s_decode_frame_header_ver(frame_header, header_info);
    result |= (ver_b) ? 0 : DECODE_HEADER_ERR_VERSION;
    
    bool layer_b = s_decode_frame_header_layer(frame_header, header_info);
    result |= (layer_b) ? 0 : DECODE_HEADER_ERR_LAYER;
    
    header_info->protection = (frame_header & 0x00010000) ? 0 : 1; // inverted

    bool bitrate_b = s_decode_frame_header_bitrate(frame_header, header_info);
    result |= (bitrate_b) ? 0 : DECODE_HEADER_ERR_BITRATE;
    

    bool freq_b = s_decode_frame_header_freq(frame_header, header_info);
    result |= (freq_b) ? 0 : DECODE_HEADER_ERR_FREQ;

    ///TODO: skip padding for now, need more reading

    header_info->mode = (uint8_t) ((frame_header & 0x000000C0) >> 6);
    header_info->mode_ext = (uint8_t) ((frame_header & 0x00000030) >> 4);
    header_info->emphasis = (uint8_t) (frame_header & 0x00000030);

    return result;
}


static bool s_decode_frame_header_ver(const uint32_t frame_header, 
                                      frame_header_info_t *header_info)
{
    assert(header_info);
    bool success = true;

    uint32_t ver_id = (frame_header & 0x00180000);

    switch (ver_id)
    {
        case 0x00180000:
            header_info->ver = 1;
            break;
        case 0x00100000:
            header_info->ver = 2;
            success = false;
            break;
        case 0x00000000:
            header_info->ver = 25;
            success = false;
            break;
        default:
            header_info->ver = 0;
            success = false;
    }

    return success;
}


static bool s_decode_frame_header_layer(const uint32_t frame_header, 
                                        frame_header_info_t *header_info)
{
    assert(header_info);
    bool success = true;

    uint8_t layer_idx = (uint8_t) ((frame_header & 0x00060000) >> 17);
    uint8_t layer_num = ((~layer_idx) & 0x03) + 1;

    /* only supporting layer 3 (MP3) */
    if (layer_num != 3)
    {
        success = false;
    }

    header_info->layer = layer_num;    

    return success;
}


static bool s_decode_frame_header_bitrate(const uint32_t frame_header,
                                          frame_header_info_t *header_info)
{
    assert(header_info);
    bool success = true;

    uint32_t bitrate_idx = (frame_header & 0x0000F000) >> 12;
    if (bitrate_idx >= 15)
    {
        success = false;
        header_info->bitrate = 0;
    }
    else
    {
        const uint16_t bitrate_layer3[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320};
        header_info->bitrate = bitrate_layer3[bitrate_idx];
    }

    return success;
}             


static bool s_decode_frame_header_freq(const uint32_t frame_header,
                                       frame_header_info_t *header_info)
{
    assert(header_info);
    bool success = true;

    uint32_t freq_idx = (frame_header & 0x00000C00) >> 10;
    switch (freq_idx) 
    {
        case 0:
            header_info->freq = 44100;
            break;
        case 1:
            header_info->freq = 48000;
            break;
        case 2:
            header_info->freq = 32000;
            break;
        default:
            header_info->freq = 0;
            success = false;
    }

    return success;
}