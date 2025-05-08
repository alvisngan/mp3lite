## ISO/IEC 11172-3 Notable (Confussing) Naming Convention
### Scalefactor
`scalefac` scalefactor
`scfsi`    scalefactor side information
`scalefac_band != scfsi_band` (ISO/IEC 11172-3: 1993 (E) page 25)
`scalefac_band`, or `sfb` is the scalefactor for each frequency subband block
`scfsi_band` is the scalefactor selection group number, it categorize a set of `scalefac_band` that share some common properties such as `slen`
| `scfsi_band`  | `scalefac_band`   |
| ------------- | ----------------- |
| 0             | [0, 5]            |
| 1             | [6, 10]           |
| 2             | [11, 15]          |
| 3             | [16, 20]          |
(ISO/IEC 11172-3: 1993 (E) P.25)


`frame_length` size (in bytes?) of the compressed (Huffman coded) data (may include CRC?)
`frame_size` size (in bits?) of the decompressed data

## Variable Naming Convention
These are the variable naming convention I try to stick to, unless the variable is already defined by ISO/IEC 11172-3, please refer to the code documentation or comment for the actual variable meaning.

`len` Number of elements in an array, used interchangeably with `size` if the array elements have size of 1 byte

`size` Number of bytes allocated in an array, used interchangeably with `len` if the array elements have size of 1 byte

`bitsize` Number of bits of the data


## Comment Style
`/* Comment here */`
Capitalized
No period after a sentence

```
/*****************************************************************************
 *                                                                           *
 * Source code for decoding side information                                 *
 *                                                                           *
 *****************************************************************************/
```
Linking in C is annoying, pseudo-files

### Range
`[A, B]` from `A` to `B` inclusively 
`(A, B)` excluding `A` and `B`, i.e. from `(A+1)` to `(B-1)`

## Line Width
80 characters, unless it is a long constant array

two lines margin between functions
one line between function prototypes since there are comment documentations

## Function Parameters Order
Not sure lol