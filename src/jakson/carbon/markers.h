/**
 * Copyright 2019 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CARBON_MARKERS
#define CARBON_MARKERS

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>

BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//  format markers, see carbonspec.org/format-specs/format-overview/marker-format.html
// ---------------------------------------------------------------------------------------------------------------------

/** data type marker */
#define CARBON_MNULL                        'n'
#define CARBON_MTRUE                        't'
#define CARBON_MFALSE                       'f'
#define CARBON_MSTRING                      's'
#define CARBON_MU8                          'c'
#define CARBON_MU16                         'd'
#define CARBON_MU32                         'i'
#define CARBON_MU64                         'l'
#define CARBON_MI8                          'C'
#define CARBON_MI16                         'D'
#define CARBON_MI32                         'I'
#define CARBON_MI64                         'L'
#define CARBON_MFLOAT                       'r'
#define CARBON_MBINARY                      'b'
#define CARBON_MCUSTOM_BINARY               'x'

/** container marker */
#define CARBON_MOBJECT_BEGIN                '{'
#define CARBON_MOBJECT_END                  '}'
#define CARBON_MARRAY_BEGIN                 '['
#define CARBON_MARRAY_END                   ']'
#define CARBON_MCOLUMN_U8                   '1'
#define CARBON_MCOLUMN_U16                  '2'
#define CARBON_MCOLUMN_U32                  '3'
#define CARBON_MCOLUMN_U64                  '4'
#define CARBON_MCOLUMN_I8                   '5'
#define CARBON_MCOLUMN_I16                  '6'
#define CARBON_MCOLUMN_I32                  '7'
#define CARBON_MCOLUMN_I64                  '8'
#define CARBON_MCOLUMN_FLOAT                'R'
#define CARBON_MCOLUMN_BOOLEAN              'B'

/** record identifier marker */
#define CARBON_MNOKEY                       '?'
#define CARBON_MAUTOKEY                     '*'
#define CARBON_MUKEY                        '+'
#define CARBON_MIKEY                        '-'
#define CARBON_MSKEY                        '!'

/** abstract types for object containers */
#define CARBON_MUNSORTED_MULTIMAP           CARBON_MOBJECT_BEGIN
#define CARBON_MSORTED_MULTIMAP             '~'
#define CARBON_MUNSORTED_MAP                ':'
#define CARBON_MSORTED_MAP                  '#'

/** abstract types for array containers */
#define CARBON_MUNSORTED_MULTISET_ARR       CARBON_MARRAY_BEGIN
#define CARBON_MSORTED_MULTISET_ARR         '<'
#define CARBON_MUNSORTED_SET_ARR            '/'
#define CARBON_MSORTED_SET_ARR              '='

/** abstract types for column-u8 containers */
#define CARBON_MUNSORTED_MULTISET_U8        CARBON_MCOLUMN_U8
#define CARBON_MSORTED_MULTISET_U8          0x01 /** SOH */
#define CARBON_MUNSORTED_SET_U8             0x02 /** STX */
#define CARBON_MSORTED_SET_U8               0x03 /** ETX */

/** abstract types for column-u16 containers */
#define CARBON_MUNSORTED_MULTISET_U16       CARBON_MCOLUMN_U16
#define CARBON_MSORTED_MULTISET_U16         0x05 /** ENQ */
#define CARBON_MUNSORTED_SET_U16            0x06 /** ACK */
#define CARBON_MSORTED_SET_U16              0x07 /** BEL */

/** abstract types for column-u32 containers */
#define CARBON_MUNSORTED_MULTISET_U32       CARBON_MCOLUMN_U32
#define CARBON_MSORTED_MULTISET_U32         0x09 /** TAB */
#define CARBON_MUNSORTED_SET_U32            0x0A /** LF */
#define CARBON_MSORTED_SET_U32              0x0B /** VT */

/** abstract types for column-u64 containers */
#define CARBON_MUNSORTED_MULTISET_U64       CARBON_MCOLUMN_U64
#define CARBON_MSORTED_MULTISET_U64         0x0D /** CR */
#define CARBON_MUNSORTED_SET_U64            0x0E /** SO */
#define CARBON_MSORTED_SET_U64              0x0F /** SI */

/** abstract types for column-i8 containers */
#define CARBON_MUNSORTED_MULTISET_I8        CARBON_MCOLUMN_I8
#define CARBON_MSORTED_MULTISET_I8          0x11 /** DC1 */
#define CARBON_MUNSORTED_SET_I8             0x12 /** DC2 */
#define CARBON_MSORTED_SET_I8               0x13 /** DC3 */

/** abstract types for column-i16 containers */
#define CARBON_MUNSORTED_MULTISET_I16       CARBON_MCOLUMN_I16
#define CARBON_MSORTED_MULTISET_I16         0x15 /** NAK */
#define CARBON_MUNSORTED_SET_I16            0x16 /** SYN */
#define CARBON_MSORTED_SET_I16              0x17 /** ETB */

/** abstract types for column-i32 containers */
#define CARBON_MUNSORTED_MULTISET_I32       CARBON_MCOLUMN_I32
#define CARBON_MSORTED_MULTISET_I32         0x19 /** EM */
#define CARBON_MUNSORTED_SET_I32            0x1A /** SUB */
#define CARBON_MSORTED_SET_I32              0x1B /** ESC */

/** abstract types for column-i64 containers */
#define CARBON_MUNSORTED_MULTISET_I64       CARBON_MCOLUMN_I64
#define CARBON_MSORTED_MULTISET_I64         0x1D /** GS */
#define CARBON_MUNSORTED_SET_I64            0x1E /** RS */
#define CARBON_MSORTED_SET_I64              0x1F /** US */

/** abstract types for column-float containers */
#define CARBON_MUNSORTED_MULTISET_FLOAT    CARBON_MCOLUMN_FLOAT
#define CARBON_MSORTED_MULTISET_FLOAT      '"'
#define CARBON_MUNSORTED_SET_FLOAT         '$'
#define CARBON_MSORTED_SET_FLOAT           '.'

/** abstract types for column-boolean containers */
#define CARBON_MUNSORTED_MULTISET_BOOLEAN  CARBON_MCOLUMN_BOOLEAN
#define CARBON_MSORTED_MULTISET_BOOLEAN    '_'
#define CARBON_MUNSORTED_SET_BOOLEAN       '\''
#define CARBON_MSORTED_SET_BOOLEAN         0x7F /** DEL */

END_DECL

#endif
