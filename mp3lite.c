#include "mp3lite.h"

#include <assert.h>
#include <stdbool.h>

/* Maximum number of channels (2 for MPEG-1 11172-3) */
#define NCH_MAX 2u

/* Maximum number of scfsi_band (NOT scalefactor bands) */
/* (4 for MPEG-1 11172-3)                               */
#define NUM_SCFSI_BAND_MAX 4u

/* Uncompressed frame size, in bytes, for MPEG-1 Audio Layer 3 */
#define FRAME_SIZE (1152u / 8u)

/* Scalefactor table array lengths */
#define LONG_BLOCK_LEN  21
#define SHORT_BLOCK_LEN 12

/*****************************************************************************
 *                                                                           *
 * Function prototypes for generic helper functions                          *
 *                                                                           *
 *****************************************************************************/

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

/*
 * Example:
 *      bitshift = 2 (bits)
 *      len = 2 (bytes)
 *      dest:
 *          | AAAA AAAA | BBBB BBBB |
 *      src:
 *          | --AA AAAA | AABB BBBB | BB-- ---- |
 *
 * \param dest      Pointer to the destination array, the array should
 *                  have `len` bytes allocated
 *                  dest and src MUST not alias
 *
 * \param src       Pointer to the source array, the array should have 
 *                  `len` + 1 bytes allocated
 *                  dest and src MUST not alias
 *
 * \param bitshift  Number of bits to be shifted to the left, 
 *                  less than 8, in bits
 *
 * \param len       Number of bytes of data to be shifted
 */
static void s_align_array(uint8_t *dest, const uint8_t *src, 
                          const uint32_t bitshift, const uint32_t len);

/*****************************************************************************
 *                                                                           *
 * Source code for generic helper functions                                  *
 *                                                                           *
 *****************************************************************************/

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

    return ((uint16_t)((uint16_t) val_ptr[0] << 8)|
            (uint16_t) val_ptr[1]);
}


static uint16_t s_copy_bitstream_u16(const uint8_t *bitstream_ptr)
{
    uint16_t val = 0;

#if !defined (MP3LITE_BIG_ENDIAN)
    val = ((uint16_t)((uint16_t) bitstream_ptr[0] << 8)|
           (uint16_t) bitstream_ptr[1]);
#else
    val = ((uint16_t)((uint16_t) bitstream_ptr[1] << 8)|
           (uint16_t) bitstream_ptr[0]);
#endif

    return val;
}


static void s_align_array(uint8_t *dest, const uint8_t *src, 
                          const uint32_t bitshift, const uint32_t len)
{
    assert(dest && src);
    assert(dest != src);
    assert(bitshift < 8u);

    for (uint8_t i = 0; i < len; ++i)
    {   
        /* Each gr_ch_ptr element are scatter into two adjacent bytes */
        /* e.g. 0bXXXXXABC|0bDEFGHXXX ==> 0bABCDEFGH   (bitshift = 5) */
        dest[i] = (uint8_t) ((src[i] << bitshift) | 
                             ((src[i + 1u]) >> (8u - bitshift)));
    }
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
} header_info_t;



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
 * \return              0: success
 *                      
 *                      The error log is stored in a one byte bitfield
 *                      err_log = 0b0000DCBA
 *                      A: invalid syncword
 *                      B: invalid/unsupported layer type
 *                      C: invalid bitrate
 *                      D: invalid sampling frequency
 */
static uint8_t s_decode_frame_header(const uint32_t frame_header, 
                                     header_info_t *header_info);

/*
 * \param frame_header  frame header in system endianness
 *
 * \return  true:   MPEG version 1
 *          false:  reserved (header_info->ver = 0) 
                    2 & 2.5 (for 2.5, header_info->ver = 25)
 */
static bool s_decode_frame_header_ver(uint32_t frame_header, 
                                      header_info_t *header_info);

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
                                        header_info_t *header_info);

/*
 * \param frame_header  frame header in system endianness
 *
 * \return  if bitrate index is 0000, this function returns success
 *          if bitrate index is 1111, this function returns failure
 */
static bool s_decode_frame_header_bitrate(const uint32_t frame_header,
                                          header_info_t *header_info);
/*
 * \param frame_header  frame header in system endianness
 *
 * \return  if the frequency index is 11 (i.e. reserved), 
 *          this function will return failure
 */
static bool s_decode_frame_header_freq(const uint32_t frame_header,
                                       header_info_t *header_info);

/*
 * \param frame_header  frame header in system endianness
 *
 * \return  Huffman coded frame length in bytes,
 *          excludes the header, padding, and side information
 */
static uint32_t s_frame_compressed_len(header_info_t *header_info);

/*****************************************************************************
 *                                                                           *
 * Source code for decoding frame header                                     *
 *                                                                           *
 *****************************************************************************/

static uint8_t s_decode_frame_header(uint32_t frame_header, 
                                     header_info_t *header_info)
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

    /* padding is 1 byte for layer 2/3, and 4 bytes for layer 1 (unsupported)*/
    header_info->padding = (uint16_t) ((frame_header_e & 0x00000200u) >> 9);
    header_info->mode = (uint8_t) ((frame_header_e & 0x000000C0u) >> 6);
    header_info->mode_ext = (uint8_t) ((frame_header_e & 0x00000030u) >> 4);
    header_info->emphasis = (uint8_t) (frame_header_e & 0x00000003u);

    return result;
}


static bool s_decode_frame_header_ver(const uint32_t frame_header, 
                                      header_info_t *header_info)
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
                                        header_info_t *header_info)
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
                                          header_info_t *header_info)
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
        static const uint16_t s_bitrate_layer3[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320};
        header_info->bitrate = s_bitrate_layer3[bitrate_idx];
    }

    return success;
}             


static bool s_decode_frame_header_freq(const uint32_t frame_header,
                                       header_info_t *header_info)
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


static uint32_t s_frame_compressed_len(header_info_t *header_info)
{
    return ((FRAME_SIZE * header_info->bitrate / header_info->freq) +
            header_info->padding);
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
 * Unused array element are assigned as all bits set
 *
 * Reference: ISO/IEC 1172-3: 1993 2.4.1.7 
 *
 * Abbreviation
 * ------------
 * uimsbf:  unsigned integer, most significant bit first
 * bslbf:   bit string, left bit first
 *
 * Members
 * -------
 * part2_3_length   Number of BITS used for scalefactors and Huffman code data
 *
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
 * gr_ch[idx]               Information unique to each granule and channel
 *                          idx = gr * NCH_MAX + ch
 */
typedef struct {
    uint16_t main_data_begin;
    uint8_t scfsi[NCH_MAX * 4u];
    side_info_gr_ch_t gr_ch[2u * NCH_MAX];
} side_info_t;

/* Error log for s_decode_side_info() */
#define DECODE_SIDEINFO_ERR_SCFSI       0x01
#define DECODE_SIDEINFO_ERR_GR_CH       0x02

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
static uint8_t s_decode_side_info(const uint8_t *side_info_ptr, 
                                  side_info_t *side_info,
                                  const header_info_t *header_info);

/*
 * Helper function for finding the current index of the scfsi array in 
 * side_info_t struct
 *
 * \param scfsi_band    current scalefactor side_info band, starts at 0
 *
 * \param ch            current channel, starts at 0
 */
static uint8_t s_scfsi_idx(const uint8_t scfsi_band, const uint8_t ch);

/*
 * Helper function for finding the current index of the scfsi array in 
 * side_info_t struct
 *
 * \param grc   Current granule, starts at 0
 *
 * \param ch    Current channel, starts at 0
 */
static uint8_t s_gr_ch_idx(const uint8_t gr, const uint8_t ch);

static bool s_decode_side_info_scfsi(const uint8_t *side_info_ptr, 
                                     side_info_t *side_info,
                                     const header_info_t *header_info);

static bool s_decode_side_info_gr_ch(const uint8_t *side_info_ptr, 
                                     side_info_t *side_info,
                                     const header_info_t *header_info);

/*
 * Decoding side information for EACH granule and channel, This function is a
 * helper function for s_decode_side_info_gr_ch
 *
 * \param gr_ch_ptr     Pointer to the start of side_info for the [gr][ch]
 *                      must be byte-aligned
 *
 * \param grc           Current granule
 *      
 * \param ch            Current channel
 */
static bool s_decode_side_info_gr_ch_loop(const uint8_t *gr_ch_ptr, 
                                          const uint8_t gr,
                                          const uint8_t ch,
                                          side_info_t *side_info,
                                          const header_info_t *header_info);

/*
 * Helper function for s_decode_side_info_gr_ch_loop, decodes information inside
 * the if (window_switching_flag) statement
 *
 * \param cur_gr_ch     Pointer to current [gr][ch] side_info_gr_ch_t struct
 */
static void s_decode_side_info_gr_ch_win_sw_flag(const uint8_t *gr_ch_ptr,
                                                 const uint8_t win_sw_flag,
                                                 side_info_gr_ch_t *cur_gr_ch);

/*
 * Calculate the byte offset of the second granule from main_data_begin
 */
static uint32_t s_next_granule_pos(const side_info_t *side_info,
                                   const header_info_t *header_info);

/*****************************************************************************
 *                                                                           *
 * Source code for decoding side information                                 *
 *                                                                           *
 *****************************************************************************/

static uint8_t s_decode_side_info(const uint8_t *side_info_ptr, 
                                  side_info_t *side_info,
                                  const header_info_t *header_info)
{
    assert(side_info_ptr);
    assert(side_info);
    assert(header_info);

    uint8_t result = 0;

    uint16_t main_data_begin = s_copy_bitstream_u16(side_info_ptr);
    side_info->main_data_begin = main_data_begin >> 7;

    bool scfsi_b = s_decode_side_info_scfsi(side_info_ptr, 
                                            side_info, 
                                            header_info);
    result |= (scfsi_b) ? 0 : DECODE_SIDEINFO_ERR_SCFSI;   
    
    bool gr_ch_b = s_decode_side_info_gr_ch(side_info_ptr, 
                                            side_info, 
                                            header_info);
    result |= (gr_ch_b) ? 0 : DECODE_SIDEINFO_ERR_GR_CH;

    return result;
}


static uint8_t s_scfsi_idx(const uint8_t scfsi_band, const uint8_t ch)
{
    assert(scfsi_band < NUM_SCFSI_BAND_MAX);
    assert(ch < NCH_MAX);
    
    /* type casting to suppress complier warning */
    return (uint8_t) ((scfsi_band * NCH_MAX) + ch); 
}


static uint8_t s_gr_ch_idx(const uint8_t gr, const uint8_t ch)
{
    assert(gr < 2u);
    assert(ch < NCH_MAX);
    
    /* type casting to suppress complier warning */
    return (uint8_t) ((gr * NCH_MAX) + ch);
}


static bool s_decode_side_info_scfsi(const uint8_t *side_info_ptr, 
                                     side_info_t *side_info,
                                     const header_info_t *header_info)
{
    assert(side_info_ptr && side_info && header_info);
    
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
                side_info->scfsi[s_scfsi_idx(scfsi_band, 0)] = foo; 
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


static bool s_decode_side_info_gr_ch(const uint8_t *side_info_ptr, 
                                     side_info_t *side_info,
                                     const header_info_t *header_info)
{
    assert(side_info_ptr && side_info && header_info);

    bool success = false;
    bool success_arr[2u * NCH_MAX];

    /* gr_ch_ptr to be aligned the byte boundary */
    uint8_t gr_ch_ptr[8u];// 59 bits for each [gr][ch]; gr_ch_len = 8

    /* data precede [gr][ch]: 18 bits for mono, 20 bits for dual channels */
    /* for MPEG2/2.5, nch & pre_gr_ch_bits needs to change                */
    const uint8_t nch = (header_info->mode == 3u) ? 1u : 2u;
    const uint8_t pre_gr_ch_bits = (header_info->mode == 3u) ? 18u : 20u;
    const uint8_t gr_ch_bitsize = 59u;
    const uint8_t gr_ch_len = 8u; /* in bytes */

    /* Initiate variables outside the loop */
    uint32_t preceding_bits = 0;
    uint32_t idx = 0;
    uint8_t bitshift = 0;
    uint8_t i = 0;

    for (uint8_t gr = 0; gr < 2u; ++gr)
    {
        for (uint8_t ch = 0; ch < nch; ++ch)
        {
            /* Number of bits precede the current [gr][ch] from side_info_ptr */
            /* preceding_bits = pre_gr_ch_bits + (gr + ch) * gr_ch_bitsize*/
            preceding_bits = ((uint32_t) pre_gr_ch_bits + 
                              (((uint32_t) gr + (uint32_t) ch) * 
                              (uint32_t) gr_ch_bitsize));

            /* Index of the current [gr][ch] in side_info_ptr */
            idx = preceding_bits / 8u;
            bitshift = (uint8_t) preceding_bits % 8u;

            s_align_array(gr_ch_ptr, &side_info_ptr[idx], bitshift, gr_ch_len);

            /* Decoding [gr][ch] */
            success_arr[i] = s_decode_side_info_gr_ch_loop(gr_ch_ptr, gr, ch, 
                                                           side_info, 
                                                           header_info);
            i++;
        }
    }

    success = true;
    for (uint8_t j = 0; j < (2u * nch); ++j)
    {
        success = success && success_arr[j];
    }

    return success;
}


static bool s_decode_side_info_gr_ch_loop(const uint8_t *gr_ch_ptr, 
                                          const uint8_t gr,
                                          const uint8_t ch,
                                          side_info_t *side_info,
                                          const header_info_t *header_info)
{
    assert(gr_ch_ptr && side_info && header_info);
    assert(gr < 2u);
    assert(ch < NCH_MAX);
    
    bool success = false;
    side_info_gr_ch_t *cur_gr_ch = &(side_info->gr_ch[s_gr_ch_idx(gr, ch)]);

    /* Bit structure before the if (window_switching_flag) statement */
    /* |     0     |     1     |     2     |     3     |     4     | */
    /* | DDDD DDDD | DDDD EEEE | EEEE EFFF | FFFF FGGG | GH-- ---- | */

    uint16_t foo = s_copy_bitstream_u16(gr_ch_ptr);
    cur_gr_ch->part2_3_length = foo >> 4;

    foo = s_copy_bitstream_u16(&gr_ch_ptr[1]);
    cur_gr_ch->big_values = (foo & 0x0FF8u) >> 3;

    foo = s_copy_bitstream_u16(&gr_ch_ptr[2]);
    cur_gr_ch->global_gain = (uint8_t) ((foo & 0x07F8u) >> 3);

    foo = s_copy_bitstream_u16(&gr_ch_ptr[3]);
    cur_gr_ch->scalefac_compress = (uint8_t) ((foo & 0x0780u) >> 7);

    uint8_t win_flag = (gr_ch_ptr[4] & 0x40u) >> 6;
    cur_gr_ch->window_switching_flag = win_flag;

    s_decode_side_info_gr_ch_win_sw_flag(gr_ch_ptr, win_flag, cur_gr_ch);

    /* |     7     | */
    /* | IJK- ---- | */
    cur_gr_ch->preflag = (gr_ch_ptr[7] & 0x80u) >> 7;
    cur_gr_ch->scalefac_scale = (gr_ch_ptr[7] & 0x40u) >> 6;
    cur_gr_ch->count1table_select = (gr_ch_ptr[7] & 0x20u) >> 5;

    /// TODO: currently there is no error detection
    success = true;
    return success;
}


static void s_decode_side_info_gr_ch_win_sw_flag(const uint8_t *gr_ch_ptr,
                                                 const uint8_t win_sw_flag,
                                                 side_info_gr_ch_t *cur_gr_ch)
{
    uint16_t foo = 0;
    
    /* Assign unused array element as all bits set */
    if (win_sw_flag == 1u)
    {
        /* |     4     |     5     |     6     | */
        /* | --ZZ YXXX | XXWW WWWV | VVUU UTTT | */

        uint8_t block_type = (gr_ch_ptr[4] & 0x30u) >> 4; 
        uint8_t mixed_block_flag = (gr_ch_ptr[4] & 0x08u) >> 3;
        cur_gr_ch->block_type = block_type;
        cur_gr_ch->mixed_block_flag = mixed_block_flag;

        foo = s_copy_bitstream_u16(&gr_ch_ptr[4]);
        cur_gr_ch->table_select[0] = (uint8_t) ((foo & 0x07C0u) >> 6);
        cur_gr_ch->table_select[1] = (gr_ch_ptr[5] & 0x3Eu) >> 1;
        cur_gr_ch->table_select[2] = 0xFF; // Unused

        foo = s_copy_bitstream_u16(&gr_ch_ptr[5]);
        cur_gr_ch->subblock_gain[0] = (uint8_t) ((foo & 0x01C0u) >> 6);
        cur_gr_ch->subblock_gain[1] = (uint8_t) ((gr_ch_ptr[6] & 0x38u) >> 3);
        cur_gr_ch->subblock_gain[2] = (uint8_t) (gr_ch_ptr[6] & 0x07u);

        /* Default region_count if window_switching_flag is set */
        bool region0_b = ((block_type == 1u) || (block_type == 3u) ||
                          ((block_type == 2u) && (mixed_block_flag == 1u)));
        cur_gr_ch->region_count[0] = (region0_b) ? 7 : 8;
        cur_gr_ch->region_count[1] = 36;
    }
    else
    {
        /* |     4     |     5     |     6     | */
        /* | --ZZ ZZZY | YYYY XXXX | XWWW WVVV | */ 
        
        /* Default */
        cur_gr_ch->block_type = 0;

        /* Unused */
        cur_gr_ch->mixed_block_flag = 0xFFu;
        cur_gr_ch->subblock_gain[1] = 0xFFu;
        cur_gr_ch->subblock_gain[0] = 0xFFu;
        cur_gr_ch->subblock_gain[2] = 0xFFu;

        cur_gr_ch->table_select[0] = (gr_ch_ptr[4] & 0x3Eu) >> 1;
        foo = s_copy_bitstream_u16(&gr_ch_ptr[4]);
        cur_gr_ch->table_select[1] = (uint8_t) ((foo & 0x01F0u) >> 4);
        foo = s_copy_bitstream_u16(&gr_ch_ptr[5]);
        cur_gr_ch->table_select[2] = (uint8_t) ((foo & 0x0F80u) >> 7);

        cur_gr_ch->region_count[0] = (gr_ch_ptr[6] & 0x78u) >> 3;
        cur_gr_ch->region_count[1] = gr_ch_ptr[6] & 0x07u;
    }
}


static uint32_t s_next_granule_pos(const side_info_t *side_info,
                                   const header_info_t *header_info)
{
    ///TODO: Verify (Why do we need part2_3_length accroding to page 25???)
    assert(side_info && header_info);
    
    /* for MPEG2/2.5 side_info_len needs to change */
    const uint32_t header_len = 4u;
    const uint32_t crc_len = (header_info->protection) ? 2 : 0;
    const uint8_t side_info_len = (header_info->mode == 3u) ? 17u : 32u;
    
    return ((uint32_t) (side_info->main_data_begin) + 
            header_len + crc_len + side_info_len);
}


/*****************************************************************************
*                                                                           *
* Typedef's and function prototypes for decoding scale factors (scalefac)   *
*                                                                           *
*****************************************************************************/

/* 
 * Obtaining the scfsi_band from scalefac_band, yes they are different, I swear
 * (for more detail see ./docs/naming_convention.md)
 * 
 * scalefac_band    scalefactor for each frequency subband block
 *
 * scfsi_band       scalefactor selection (scfsi) group number,
 *                  where a set of scalefac_band have the same scfsi property
 *                  (ISO/IEC 11172-3: 1993 (E) 2.4.2.7 P.25)
 */
static uint8_t s_decode_scalefac_scfsi_band(uint8_t scalefac_band);

/*
 * Obtainning the granule number where scalefactor information is stored
 *
 * When scfsi is set to 1, the scalefactor of the first granule are also 
 * used for the second granule, therefore they are not transmitted for the 
 * second granule (ISO/IEC 11172-3: 1993 (E) 2.4.3.4.5 P.34)
 *
 * \param gr            current granule, starts at 0
 *
 * \param ch            current channel, starts at 0
 *
 * \param scfsi_band    scalefactor selection group number,
 *                      see ./docs/naming_convention.md for more detail
 *
 * \param side_info     side information for the current frame
 *
 * \return              granule number where scalefactor information is stored,
 *                          0: first granule
 *                          1: second granule
 */
static uint8_t s_decode_scalefac_location(const uint8_t gr,         
                                          const uint8_t ch,
                                          const uint8_t scfsi_band,
                                          const side_info_t *side_info);

/*
 * Getting slen1 and slen2, the BITSIZE of scalefactor for a range of
 * scalefactor bands
 *
 * For the bitsize of a specific scalefactor band, use the
 * s_decode_scalefac_band_bitsize function after finding slen1 and slen2
 * 
 * If scfsi == 1 && gr == 1, the scalefactor of the first granule will be used
 * instead (ISO/IEC 11172-3: 1993 (E) 2.4.3.4.5 P.34)
 * 
 * \param slen1     Address of slen1, will be modified by the function
 *
 * \param slen2     Address of slen2, will be modified by the function
 *      
 * \param gr        if scfsi == 1 && gr == 1, the scalefactor of the first 
 *                  granule are also used for the second granule, this function
 *                  will automatically choose the correct granule based on gr
 */
static void s_decode_scalefac_slen(uint8_t *slen1,
                                   uint8_t *slen2,
                                   const uint8_t gr,         
                                   const uint8_t ch,
                                   const uint8_t scfsi_band);

/*
 * part2_length is defined in ISO/IEC 11172-3 as the number of BITS used to 
 * encode scalefactors (ISO/IEC 11172-3: 1993 (E) 2.4.3.4.5 P.34)
 *
 * \return  The number of BITS used to encode scalefactors
 *          If unsuccessful, return 0
 */
static uint32_t s_decode_scalefac_part2_length(const uint8_t gr,
                                               const uint8_t ch,
                                               const uint8_t scfsi_band,
                                               const side_info_t *side_info);
 
/* 
 * Obtaining the number of bits used for the transmission of the scalefactor
 * for a specific granule, channel, scalefac_band, and window
 * (window is only applicable for short blocks)
 *
 * \params scalefac_band    Current scalefactor band
 *
 * \param slen1             See s_decode_scalefac_slen
 *
 * \param slen2             See s_decode_scalefac_slen
 *
 * \param get_long          Only used when the following condition is met:
 *                          (block_type == 2 && mixed_block_flag == 1)
 *                          When this parameter is set to TRUE, this function 
 *                          will obtain the bitsize of scalefac_l
 *                          When this parameter is set to FALSE this function 
 *                          will obtain the bitsize of scalefac_s
 *                          Otherwise, see return for more detail
 *                    
 * \param side_info_gr_ch   Side information where scalefactors are stored
 *                          Note that scalefactor is not necessarily stored in
 *                          the same granule, see s_decode_scalefac_location for 
 *                          more detail
 *
 * \return                  If success, it returns the bitsize of either 
 *                          scalefac_l or scalefac_s, depending on the 
 *                          block_type and mixed_block_flag
 *                          If failure, return 0
 *
 *                          block_type  mixed_block_flag    return    
 *                          ----------  ----------------    -------------------
 *                          0, 1, 3     N/A                 scalefac_l   
 *                          2           0                   scalefac_s   
 *                          2           1                   *see param get_long 
 */
static uint8_t s_decode_scalefac_band_bitsize(const uint8_t scalefac_band,
                                              const uint8_t slen1,
                                              const uint8_t slen2,
                                              const bool get_long,
                                              const side_info_gr_ch_t *side_info_gr_ch,
                                              const side_info_t *side_info);

static bool s_decode_scalefac(const uint8_t *main_data_ptr,
                              const side_info_t *side_info,
                              const header_info_t header_info);

/*
 * Decoding scalefactor for EACH granule and channel, 
 */                              
static bool s_decode_scalefac_gr_ch_loop(const uint8_t *gr_ch_ptr, 
                                         const uint8_t gr,
                                         const uint8_t ch,
                                         const side_info_t *side_info,
                                         const header_info_t header_info);

/*****************************************************************************
 *                                                                           *
 * Source code for decoding scale factors (scalefac)                         *
 *                                                                           *
 *****************************************************************************/


static uint8_t s_decode_scalefac_scfsi_band(const uint8_t scalefac_band)
{
    assert(scalefac_band < 20);

    /* ISO/IEC 11172-3: 1993(E) P.25 */
    uint8_t scfsi_band = 0;

    if (scalefac_band <= 5)
    {
        scfsi_band = 0;
    }
    else if (scalefac_band <= 10)
    {
        scfsi_band = 1;
    }
    else if (scalefac_band <= 15)
    {
        scfsi_band = 16;
    }
    else
    {
        scfsi_band = 20;
    }

    return scfsi_band;
}


static uint8_t s_decode_scalefac_location(const uint8_t gr,         
                                          const uint8_t ch,
                                          const uint8_t scfsi_band,
                                          const side_info_t *side_info)
{
    uint8_t gr_t = gr; /* temporary granule number */

    if ((side_info->scfsi[s_scfsi_idx(scfsi_band, ch)] == 1u) && (gr == 1u))
    {
        gr_t = 0; /* scalefactors for gr==0 is valid for gr==1 */
    }

    return gr_t;
}                                          


static void s_decode_scalefac_slen(uint8_t *slen1,
                                   uint8_t *slen2,
                                   const uint8_t gr,         
                                   const uint8_t ch,
                                   const uint8_t scfsi_band,
                                   const side_info_t *side_info)
{
    /* The index for slen1 and slen2 is scalefac_compress[gr][ch] */
    static const uint8_t s_slen1_arr[16] = {0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4};
    static const uint8_t s_slen2_arr[16] = {0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3};

    /* temporary granule number where scalefac are stored */
    uint8_t gr_t = s_decode_scalefac_location(gr, ch, scfsi_band, side_info);

    const side_info_gr_ch_t *gr_ch = &(side_info->gr_ch[s_gr_ch_idx(gr_t, ch)]);
    *slen1 = s_slen1_arr[gr_ch->scalefac_compress];
    *slen2 = s_slen2_arr[gr_ch->scalefac_compress];
}


static uint8_t s_decode_scalefac_band_bitsize(const uint8_t scalefac_band,
                                              const uint8_t slen1,
                                              const uint8_t slen2,
                                              const bool get_long,
                                              const side_info_gr_ch_t *side_info_gr_ch)
{
    assert(scalefac_band <= 20);

    uint8_t bitsize = 0;

    switch (side_info_gr_ch->block_type) 
    {
        case 0:
        case 1:
        case 3:
            if (scalefac_band <= 10)
            {
                bitsize = slen1;
            }
            else /* [11, 20] */ 
            {
                bitsize = slen2;
            }
            break;
        case 2:
            switch (side_info_gr_ch->mixed_block_flag)
            {
                case 0:
                    if (scalefac_band <= 5)
                    {
                        bitsize = slen1;
                    }
                    else if (scalefac_band <= 11)
                    {
                        bitsize = slen2;
                    }
                    else /* scalefac_band = [11, 20] */ 
                    {
                        /* Invalid sccalefac_band */
                        bitsize = 0;
                    }
                    break;
                case 1:
                    /* The ISO/IEC 11172-3 spec is worded terribly */
                    if (get_long)
                    {
                        if (scalefac_band <= 7)
                        {
                            bitsize = slen1;
                        }
                        else 
                        {
                            /* Other scalefac_band are either undef or slen2 */
                            bitsize = 0;
                        }
                    }
                    else 
                    {
                        if (scalefac_band <= 2)
                        {
                            /* First two bands are block_type == 0 (reserved) */
                            bitsize = 0;
                        }
                        else if (scalefac_band <= 5)
                        {
                            bitsize = slen1;
                        }
                        else if (scalefac_band <= 11)
                        {
                            bitsize = slen2;
                        }
                        else /* scalefac_band = [11, 20] */ 
                        {
                            /* Invalid scalefac_band */
                            bitsize = 0;
                        }
                    }
                    break;
                default:
                    /* Invalid mixed_flog_flag */
                    bitsize = 0;
                    break;
            }
            break;
        default:
            /* Invalid block type */
            bitsize = 0;
            break;
    }

    return bitsize;
}


static uint32_t s_decode_scalefac_part2_length(const uint8_t gr,
                                               const uint8_t ch,
                                               const uint8_t scfsi_band,
                                               const side_info_t *side_info)
{
    assert(side_info);
    assert(gr < 2u);
    assert(ch < NCH_MAX);
    assert(scfsi_band < NUM_SCFSI_BAND_MAX);
    
    uint32_t slen1_const = 0;
    uint32_t slen2_const = 0;

    uint8_t slen1_u8 = 0;
    uint8_t slen2_u8 = 0;

    s_decode_scalefac_slen(&slen1_u8, &slen2_u8, gr, 
                           ch, scfsi_band, side_info);
   
    const uint32_t slen1 = (uint32_t) slen1_u8;
    const uint32_t slen2 = (uint32_t) slen2_u8;

    /* temporary granule number where scalefac are stored */
    uint8_t gr_t = s_decode_scalefac_location(gr, ch, scfsi_band, side_info);
    
    const side_info_gr_ch_t *gr_ch = &(side_info->gr_ch[s_gr_ch_idx(gr_t, ch)]);
    
    switch (gr_ch->block_type) 
    {
        /* Long block */
        case 0:
        case 1:
        case 3:
            slen1_const = 11;
            slen2_const = 10;
            break;
        /* Short block */
        case 2:
            switch (gr_ch->mixed_block_flag) 
            {
                case 0:
                    slen1_const = 18;
                    slen2_const = 18;
                    break;
                case 1:
                    slen1_const = 17;
                    slen2_const = 18;
                    break;
                default:
                    slen1_const = 0;
                    slen2_const = 0;
                    break;
            }
            break;
        default:
            slen1_const = 0;
            slen2_const = 0;
            break;
    }

    return (slen1_const * slen1) + (slen2_const * slen2);
}


static bool s_decode_scalefac(const uint8_t *main_data_ptr,
                              const side_info_t *side_info,
                              const header_info_t header_info)
{
    bool success = false;

    /* gr_ch_ptr to be aligned the byte boundary */
    uint8_t gr_ch_ptr[18u]; /* Max 144 bits for each [gr][ch] */

    return success;
}