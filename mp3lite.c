#include "mp3lite.h"

#include <assert.h>
#include <stdbool.h>

/* Maximum number of channels (2 for MPEG-1 11172-3) */
#define NCH_MAX 2u

static uint32_t s_swap_endian_u32(const uint32_t val);
static uint16_t s_swap_endian_u16(const uint16_t val);

/*
 * This function exist to circumvent MISRA C:2012 3rd ed, rule 21.15
 * which restricts the use of memcpy of different types,
 * in this case uint8_t and uint16_t
 *
 * \return  Bitstream stored as uint16_t in system endianness
 *          i.e. converted to little endian unless MP3LITE_BIG_ENDIAN is defined
 */
static uint16_t s_copy_bitstream_u16(const uint8_t *bitstream_ptr);

static uint32_t s_swap_endian_u32(uint32_t val)
{
    uint8_t *val_ptr = (uint8_t *) &val;

    return (((uint32_t) val_ptr[0] << 24) |
            ((uint32_t) val_ptr[1] << 16) |
            ((uint32_t) val_ptr[2] << 8)|
            (uint32_t) val_ptr[3]);
}


static uint16_t s_swap_endian_u16(uint16_t val)
{
    uint8_t *val_ptr = (uint8_t *) &val;

    return (((uint16_t) val_ptr[0] << 8)|
            (uint16_t) val_ptr[1]);
}


static uint16_t s_copy_bitstream_u16(const uint8_t *bitstream_ptr)
{
    uint16_t val = 0;

#if !defined (MP3LITE_BIG_ENDIAN)
    val = (((uint16_t) bitstream_ptr[0] << 8)|
           (uint16_t) bitstream_ptr[1]);
#else
    val = (((uint16_t) bitstream_ptr[1] << 8)|
           (uint16_t) bitstream_ptr[0]);
#endif

    return val;
}

/*****************************************************************************
 *                                                                           *
 * Typedef's and function prototypes for decoding frame header               *
 *                                                                           *
 *****************************************************************************/

/*
 * Members
 * -------
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
static uint8_t s_decode_frame_header(const uint32_t frame_header, 
                                     frame_header_info_t *header_info);

/*
 * \param frame_header  frame header in system endianness
 *
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
 * \param frame_header  frame header in system endianness
 *
 * \return  true:   layer 3
 *          false:  all other layers
 */
static bool s_decode_frame_header_layer(const uint32_t frame_header, 
                                        frame_header_info_t *header_info);

/*
 * \param frame_header  frame header in system endianness
 *
 * \return  if bitrate index is 0000, this function returns success
 *          if bitrate index is 1111, this function returns failure
 */
static bool s_decode_frame_header_bitrate(const uint32_t frame_header,
                                          frame_header_info_t *header_info);
/*
 * \param frame_header  frame header in system endianness
 *
 * \return  if the frequency index is 11 (i.e. reserved), 
 *          this function will return failure
 */
static bool s_decode_frame_header_freq(const uint32_t frame_header,
                                       frame_header_info_t *header_info);


/*****************************************************************************
 *                                                                           *
 * Source code for decoding frame header                                     *
 *                                                                           *
 *****************************************************************************/

static uint8_t s_decode_frame_header(uint32_t frame_header, 
                                     frame_header_info_t *header_info)
{
    assert(header_info);

    uint8_t result = 0;
    uint32_t frame_header_e = frame_header;

#if !defined (MP3LITE_BIG_ENDIAN)
    /* Swaping byte order for little-endian systems (a.k.a. most systems) */
    frame_header_e = s_swap_endian_u32(frame_header_e);
#endif

    /* Ensure syncword is valid (first 11 bits) */
    if ((frame_header_e & 0xFFE00000u) != 0xFFE00000u)
    {
        result |= DECODE_HEADER_ERR_SYNCWORD;
    }

    bool ver_b = s_decode_frame_header_ver(frame_header_e, header_info);
    result |= (ver_b) ? 0 : DECODE_HEADER_ERR_VERSION;
    
    bool layer_b = s_decode_frame_header_layer(frame_header_e, header_info);
    result |= (layer_b) ? 0 : DECODE_HEADER_ERR_LAYER;
    
    /* Protection bit is inverted from the official specs */
    header_info->protection = (frame_header_e & 0x00010000u) ? 0 : 1;

    bool bitrate_b = s_decode_frame_header_bitrate(frame_header_e, header_info);
    result |= (bitrate_b) ? 0 : DECODE_HEADER_ERR_BITRATE;
    

    bool freq_b = s_decode_frame_header_freq(frame_header_e, header_info);
    result |= (freq_b) ? 0 : DECODE_HEADER_ERR_FREQ;

    ///TODO: skip padding for now, need more reading

    header_info->mode = (uint8_t) ((frame_header_e & 0x000000C0u) >> 6);
    header_info->mode_ext = (uint8_t) ((frame_header_e & 0x00000030u) >> 4);
    header_info->emphasis = (uint8_t) (frame_header_e & 0x00000003u);

    return result;
}


static bool s_decode_frame_header_ver(const uint32_t frame_header, 
                                      frame_header_info_t *header_info)
{
    assert(header_info);
    bool success = true;

    uint32_t ver_id = (frame_header & 0x00180000u);

    switch (ver_id)
    {
        case 0x00180000u:
            header_info->ver = 1;
            break;
        case 0x00100000u:
            header_info->ver = 2;
            success = false;
            break;
        case 0x00000000u:
            header_info->ver = 25;
            success = false;
            break;
        default:
            header_info->ver = 0;
            success = false;
            break;
    }

    return success;
}


static bool s_decode_frame_header_layer(const uint32_t frame_header, 
                                        frame_header_info_t *header_info)
{
    assert(header_info);
    bool success = true;

    uint8_t layer_idx = (uint8_t) ((frame_header & 0x00060000u) >> 17);
    uint8_t layer_num = ((~layer_idx) & 0x03) + 1;

    /* only supporting layer 3 (MP3) */
    if (layer_num != 3u)
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

    uint32_t bitrate_idx = (frame_header & 0x0000F000u) >> 12;
    if (bitrate_idx >= 15u)
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

    uint32_t freq_idx = (frame_header & 0x00000C00u) >> 10;
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
            break;
    }

    return success;
}


/*****************************************************************************
 *                                                                           *
 * Typedef's and function prototypes for decoding side information           *
 *                                                                           *
 *****************************************************************************/

/*
 * The side information for each granule (gr) and channel (ch);
 * This is NOT the complete side information!
 *
 * All values will be converted to decimals with system endianness
 *
 * Reference: ISO/IEC 1172-3: 1993 2.4.1.7 
 *
 * Abbreviation
 * ------------
 * uimsbf:  unsigned integer, most significant bit first
 * bslbf:   bit string, left bit first
 */
typedef struct {
    uint16_t part2_3_length;
    uint16_t big_values;
    uint8_t global_gain;
    uint8_t scalefac_compress;
    uint8_t window_switching_flag;
    uint8_t table_select[3];

    /* if (window_switching_flag) */
    uint8_t block_type;
    uint8_t mixed_block_flag;
    uint8_t subblock_gain[3];
    /* else */
    uint8_t region_count[2];
    /* end if */

    uint8_t preflag;
    uint8_t scalefac_scale;
    uint8_t count1table_select;
} side_info_gr_ch_t;

/*
 * The side information for the frame
 *
 * All values will be converted to decimals with system endianness
 *
 * Reference: ISO/IEC 1172-3: 1993 2.4.1.7 
 *
 * Abbreviation
 * ------------
 * scfsi:   scalefactor selection information
 * NCH_MAX: maximum number of channels (defined in this file)
 * ch:      channel number, starts at 0
 * gr:      granule number, starts at 0
 *
 * Members
 * -------
 * scfsi[idx]               idx = scfsi_band * NCH_MAX + ch
 *
 * side_info_gr_ch[idx]     idx = gr * NCH_MAX + ch
 */
typedef struct {
    uint16_t main_data_begin;
    uint8_t scfsi[NCH_MAX * 4u];
    side_info_gr_ch_t side_info_gr_ch[2u * NCH_MAX];
} side_info_t;

/* Error log for s_decode_side_info() */
#define DECODE_SIDEINFO_ERR_SCFSI       0x01
#define DECODE_SIDEINFO_ERR_VERSION     0x02
#define DECODE_SIDEINFO_ERR_LAYER       0x04
#define DECODE_SIDEINFO_ERR_BITRATE     0x08
#define DECODE_SIDEINFO_ERR_FREQ        0x10

/*
 * Decoding side information, where the side_info_ptr is the pointer of array
 * containing the side_info bitstream. Since the array is stored is 8 bits
 * integer array, there is no need to swap the endian.
 *
 * \param side_info_ptr     The first bit of the side_info MUST be aligned on
 *                          the byte boundary
 *
 * \return
 */
static uint8_t s_decode_side_info(uint8_t *side_info_ptr, 
                                  side_info_t *side_info,
                                  const frame_header_info_t *header_info);

/*
 * Helper function for finding the current index of the scfsi array in 
 * side_info_t struct
 *
 * \param scfsi_band    current scalefactor side_info band
 *
 * \param ch            current channel
 */
static uint8_t s_scfsi_idx(uint8_t scfsi_band, uint8_t ch);

/*
 * Helper function for finding the current index of the scfsi array in 
 * side_info_t struct
 *
 * \param grc   current granule
 *
 * \param ch    current channel
 */
static uint8_t s_gr_ch_idx(uint8_t gr, uint8_t ch);

static bool s_decode_side_info_scfsi(uint8_t *side_info_ptr, 
                                     side_info_t *side_info,
                                     const frame_header_info_t *header_info);

/*****************************************************************************
 *                                                                           *
 * Source code for decoding side information                                 *
 *                                                                           *
 *****************************************************************************/

static uint8_t s_decode_side_info(uint8_t *side_info_ptr, 
                                  side_info_t *side_info,
                                  const frame_header_info_t *header_info)
{
    uint8_t result = 0;

    /* main_data_begin */
    uint16_t main_data_begin = s_copy_bitstream_u16(side_info_ptr);
    side_info->main_data_begin = main_data_begin >> 7;

    bool scfsi_b = s_decode_side_info_scfsi(side_info_ptr, 
                                            side_info, 
                                            header_info);
    result |= (scfsi_b) ? 0 : DECODE_SIDEINFO_ERR_SCFSI;                                      
    

    return result;
}


static uint8_t s_scfsi_idx(uint8_t scfsi_band, uint8_t ch)
{
    return (scfsi_band * NCH_MAX) + ch;
}


static uint8_t s_gr_ch_idx(uint8_t gr, uint8_t ch)
{
    return (gr * NCH_MAX) + ch;
}


static bool s_decode_side_info_scfsi(uint8_t *side_info_ptr, 
                                     side_info_t *side_info,
                                     const frame_header_info_t *header_info)
{
    bool success = false;

    uint16_t scfsi_temp = s_copy_bitstream_u16(&side_info_ptr[1]);
    uint8_t bitshift = 0;
    uint8_t foo = 0;
    switch (header_info->mode) 
    {
        /* stereo, joint stereo, and dual channel */
        case 0:
        case 1:
        case 2:
            /* data from bit 12 to 19 (0-based ordering) */
            scfsi_temp = (scfsi_temp & 0x0FF0u) >> 4;
            bitshift = 7; // 8 - 1
            for (uint8_t ch = 0; ch < 2u; ++ch)
            {
                for (uint8_t scfsi_band = 0; scfsi_band < 4u; ++scfsi_band)
                {
                    foo = (uint8_t) (((scfsi_temp) >> bitshift) & 0x01u);
                    side_info->scfsi[s_scfsi_idx(scfsi_band, ch)] = foo;
                    bitshift--;
                }
            }
            success = false;
            break;
        
        /* single channel */
        case 3:
            /* data from bit 14 to 17 (0-based ordering) */
            scfsi_temp = (scfsi_temp & 0x03C0u) >> 6;
            bitshift = 3; // 4 - 1
            for (uint8_t scfsi_band = 0; scfsi_band < 4u; ++scfsi_band)
            {
                foo = (uint8_t) (((scfsi_temp) >> bitshift) & 0x01u);
                side_info->scfsi[s_scfsi_idx(scfsi_band, 1)] = foo; 
                bitshift--;
            }
            success = true;
            break;
        
        default:
            success = false;
            break;
    }

    return success;
}