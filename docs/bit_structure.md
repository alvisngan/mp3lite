## Frame
The header, ironically, does not necessarily reside ahead of the main data. It along with the side information can split the main data into two pieces, in the ISO/IEC specification they are called granules.
```

      | main_data[0] | header | crc | side_info | main_data[1] |

```

## Header
```
Total Length = 4 bytes

      0           1           2           3
| AAAA AAAA | AAAB BCCD | EEEE FFGH | IIJJ KLMM | 
      
        Length (bits)   Discription
    A   11              Syncword
    B   2               Version ID
    C   2               Layer Description
    D   1               Protection bit
    E   4               Bitrate index
    F   2               Sammpling rate frequency index
    G   1               Padding bit
    H   1               Private bit
    I   2               Channel mode index
    J   2               Mode extension index
    K   1               Copyright
    L   1               Original
    M   2               Emphasis index
```

### Useful Links 
http://www.mp3-tech.org/programmer/frame_header.html
http://www.mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm

## Side Information
The bit structure for the side information is a bit more complicated than the header. The size of the side information depends on the number of channels, where single channel frames have 17 bytes and dual channel frames have 32 bytes. To further the complication, the bit struction also depends on whether a flag (1 bit) is set inside the side information.

### Single Channel (Mono)
```
Total Length = 17 bytes

          0           1           2           3           4           5
    | AAAA AAAA | ABBB BBCC | CCDD DDDD | DDDD DDEE | EEEE EEEF | FFFF FFFG |

          6           7           8           9           10          11
    | GGGH ---- | ---- ---- | ---- ---- | --IJ KLLL | LLLL LLLL | LMMM MMMM |

          12          13          14          15          16     
    | MMNN NNNN | NNOO OOP- | ---- ---- | ---- ---- | ---- -QRS |


        Length (bits)   Discription
    A   9               main_data_begin
    B   5               private_bit
    C   4               scfsi[scfsi_band]  (scfsi_band ∈ [0, 3])
            gr = 0
    D   12              part2_3_length[0]
    E   9               big_values[0]
    F   8               global_gain[0]
    G   4               scalefac_compress[0]
    H   1               window_switching_flag
    -   22              if (window_switching_flag) block
    I   1               preflag[0]
    J   1               scalefac_scale[0]
    K   1               count1table_select[0]
            gr = 1
    L   12              part2_3_length[1]
    M   9               big_values[1]
    N   8               global_gain[1]
    O   4               scalefac_compress[1]
    P   1               window_switching_flag
    -   22              if (window_switching_flag) block
    Q   1               preflag[1]
    R   1               scalefac_scale[1]
    S   1               count1table_select[1]


if (window_switching_flag) block

          6           7           8           9 
    | ---- ZZYX | XXXX WWWW | WVVV UUUT | TT-- ---- | 

          13          14          15          16     
    | ---- ---S | SRQQ QQQP | PPPP OOON | NNMM M--- |


        Length (bits)   Discription
            gr = 0
    Z   2               block_type[0]
    Y   1               mixed_block_flag[0]
    X   5               table_select[0][region=0]
    W   5               table_select[0][region=1]
    V   3               subblock_gain[0][window=0]
    U   3               subblock_gain[0][window=1]
    T   3               subblock_gain[0][window=2]


if (!window_switching_flag) block

          6           7           8           9 
    | ---- ZZZZ | ZYYY YYXX | XXXW WWWV | VV-- ---- | 

          13          14          15          16     
    | ---- ---U | UUUU TTTT | TSSS SSRR | RRQQ Q--- |


        Length (bits)   Discription
            gr = 0
    Z   5               table_select[0][region=0]
    Y   5               table_select[0][region=1]
    X   5               table_select[0][region=2]
    W   4               region0_count[0]
    V   3               region1_count[0]
```

### Dual Channel
```
Total Length = 32 bytes

          0           1           2           3           4           5
    | AAAA AAAA | ABBB CCCC | CCCC DDDD | DDDD DDDD | EEEE EEEE | EFFF FFFF |

          6           7           8           9           10          11
    | FGGG GH-- | ---- ---- | ---- ---- | ---- IJKL | LLLL LLLL | LLLM MMMM |

          12          13          14          15          16          17
    | MMMM NNNN | NNNN OOOO | P--- ---- | ---- ---- | ---- ---Q | RSTT TTTT |

          18          19          20          21          22          23
    | TTTT TTUU | UUUU UUUV | VVVV VVVW | WWWX ---- | ---- ---- | ---- ---- |

          24          25          26          27          28          29
    | --YZ abbb | bbbb bbbb | bccc cccc | ccdd dddd | ddee eef- | ---- ---- |

          30          31     
    | ---- ---- | ---- -ghi |


        Length (bits)   Discription
    A   9               main_data_begin
    B   3               private_bit
    C   8               scfsi[ch][scfsi_band] (ch ∈ [0, 1]; scfsi_band ∈ [0, 3])
            gr = 0  ch = 0
    D   12              part2_3_length[0][0]
    E   9               big_values[0][0]
    F   8               global_gain[0][0]
    G   4               scalefac_compress[0][0]
    H   1               window_switching_flag
    -   22              if (window_switching_flag) block
    I   1               preflag[0][0]
    J   1               scalefac_scale[0][0]
    K   1               count1table_select[0][0]
            gr = 0  ch = 1
    L   12              part2_3_length[0][1]
    M   9               big_values[0][1]
    N   8               global_gain[0][1]
    O   4               scalefac_compress[0][1]
    P   1               window_switching_flag
    -   22              if (window_switching_flag) block
    Q   1               preflag[0][1]
    R   1               scalefac_scale[0][1]
    S   1               count1table_select[0][1]
            gr = 1  ch = 0
    T   12              part2_3_length[1][0]
    U   9               big_values[1][0]
    V   8               global_gain[1][0]
    W   4               scalefac_compress[1][0]
    X   1               window_switching_flag
    -   22              if (window_switching_flag) block
    Y   1               preflag[1][0]
    Z   1               scalefac_scale[1][0]
    a   1               count1table_select[1][0]
            gr = 1  ch = 1
    b   12              part2_3_length[1][1]
    c   9               big_values[1][1]
    d   8               global_gain[1][1]
    e   4               scalefac_compress[1][1]
    f   1               window_switching_flag
    -   22              if (window_switching_flag) block
    g   1               preflag[1][1]
    h   1               scalefac_scale[1][1]
    i   1               count1table_select[1][1]


if (window_switching_flag) block

          6           7           8           9      
    | ---- --zz | yxxx xxww | wwwv vvuu | uttt ---- | 

          14          15          16        
    | -ssr qqqq | qppp ppoo | onnn mmm- |

          21          22          23          24
    | ---- llkj | jjjj iiii | ihhh gggf | ff-- ---- |

          28          29          30          31     
    | ---- ---e | edcc cccb | bbbb aaaZ | ZZYY Y--- |


        Length (bits)   Discription
            gr = 0  ch = 0
    z   2               block_type[0][0]
    y   1               mixed_block_flag[0][0]
    x   5               table_select[0][0][region=0]
    w   5               table_select[0][0][region=1]
    v   3               subblock_gain[0][0][window=0]
    u   3               subblock_gain[0][0][window=1]
    t   3               subblock_gain[0][0][window=2]


if (!window_switching_flag) block

          6           7           8           9      
    | ---- --zz | zzzy yyyy | xxxx xwww | wvvv ---- | 

          14          15          16        
    | -uuu uutt | ttts ssss | rrrr qqq- |

          21          22          23          24
    | ---- pppp | pooo oonn | nnnm mmml | ll-- ---- |

          28          29          30          31     
    | ---- ---h | hhhh gggg | gfff ffee | eedd d--- |


        Length (bits)   Discription
            gr = 0  ch = 0
    z   5               table_select[0][0][region=0]
    y   5               table_select[0][0][region=1]
    x   5               table_select[0][0][region=2]
    w   4               region0_count[0][0]
    v   3               region1_count[0][0]
```

### Default values for window_switching_flag 

```
/* Default values only, refer to previous sections for the other variables */
If (window_switching_flag):
|     If (block_type == 1 or block_type == 3 or (block_type == 2 and mixed_block_flag)):
|     |     region0_count = 7
|     End If
|
|     If (block_type == 2 and not(mixed_block_flag)):
|     |     region0_count = 8
|     End If
|
|     region1_count = 36
Else:
|     block_type = 0
End If
```