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

#ifndef JAK_CARBON_H
#define JAK_CARBON_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_memblock.h>
#include <jak_memfile.h>
#include <jak_unique_id.h>
#include <jak_string.h>
#include <jak_spinlock.h>
#include <jak_vector.h>
#include <jak_stdinc.h>
#include <jak_alloc.h>
#include <jak_bitmap.h>
#include <jak_bloom.h>
#include <jak_archive.h>
#include <jak_archive_it.h>
#include <jak_archive_visitor.h>
#include <jak_archive_converter.h>
#include <jak_encoded_doc.h>
#include <jak_stdinc.h>
#include <jak_utils_convert.h>
#include <jak_column_doc.h>
#include <jak_doc.h>
#include <jak_error.h>
#include <jak_hash_table.h>
#include <jak_hash.h>
#include <jak_huffman.h>
#include <jak_json.h>
#include <jak_memblock.h>
#include <jak_memfile.h>
#include <jak_unique_id.h>
#include <jak_utils_sort.h>
#include <jak_async.h>
#include <jak_slicelist.h>
#include <jak_spinlock.h>
#include <jak_string_dict.h>
#include <jak_str_hash.h>
#include <jak_archive_strid_it.h>
#include <jak_archive_cache.h>
#include <jak_time.h>
#include <jak_types.h>
#include <jak_archive_query.h>
#include <jak_vector.h>
#include <jak_alloc_trace.h>
#include <jak_encode_async.h>
#include <jak_encode_sync.h>
#include <jak_str_hash_mem.h>
#include <jak_pred_contains.h>
#include <jak_pred_equals.h>
#include <jak_carbon_printers.h>
#include <jak_uintvar_stream.h>

JAK_BEGIN_DECL

typedef struct jak_carbon {
        jak_memblock *memblock;
        jak_memfile memfile;

        struct {
                jak_spinlock write_lock;
                bool commit_lock;
                bool is_latest;
        } versioning;

        jak_error err;
} jak_carbon;

typedef struct jak_carbon_revise {
        jak_carbon *original;
        jak_carbon *revised_doc;
        jak_error err;
} jak_carbon_revise;

typedef struct jak_carbon_binary {
        const char *mime_type;
        jak_u64 mime_type_strlen;
        const void *blob;
        jak_u64 blob_len;
} jak_carbon_binary;

typedef struct jak_carbon_new {
        jak_error err;
        jak_carbon original;
        jak_carbon_revise revision_context;
        jak_carbon_array_it *content_it;
        jak_carbon_insert *inserter;
        /* options shrink or compact (or both) documents, see
         * CARBON_KEEP, CARBON_SHRINK, CARBON_COMPACT, and CARBON_OPTIMIZE  */
        int mode;
} jak_carbon_new;

typedef enum jak_carbon_container_type {
        JAK_CARBON_OBJECT, JAK_CARBON_ARRAY, JAK_CARBON_COLUMN
} jak_carbon_container_e;

typedef enum jak_carbon_printer_impl {
        JAK_JSON_EXTENDED, JAK_JSON_COMPACT
} jak_carbon_printer_impl_e;

#define JAK_CARBON_NIL_STR "_nil"

// ---------------------------------------------------------------------------------------------------------------------
//  format markers, see carbonspec.org/format-specs/format-overview/marker-format.html
// ---------------------------------------------------------------------------------------------------------------------

/* data type marker */
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

/* container marker */
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

/* record identifier marker */
#define CARBON_MNOKEY                       '?'
#define CARBON_MAUTOKEY                     '*'
#define CARBON_MUKEY                        '+'
#define CARBON_MIKEY                        '-'
#define CARBON_MSKEY                        '!'

/* abstract types for object containers */
#define CARBON_MUNSORTED_MULTIMAP           CARBON_MOBJECT_BEGIN
#define CARBON_MSORTED_MULTIMAP             '~'
#define CARBON_MUNSORTED_MAP                ':'
#define CARBON_MSORTED_MAP                  '#'

/* abstract types for array containers */
#define CARBON_MUNSORTED_MULTISET_ARR       CARBON_MARRAY_BEGIN
#define CARBON_MSORTED_MULTISET_ARR         '<'
#define CARBON_MUNSORTED_SET_ARR            '/'
#define CARBON_MSORTED_SET_ARR              '='

/* abstract types for column-u8 containers */
#define CARBON_MUNSORTED_MULTISET_U8        CARBON_MCOLUMN_U8
#define CARBON_MSORTED_MULTISET_U8          0x01 /* SOH */
#define CARBON_MUNSORTED_SET_U8             0x02 /* STX */
#define CARBON_MSORTED_SET_U8               0x03 /* ETX */

/* abstract types for column-u16 containers */
#define CARBON_MUNSORTED_MULTISET_U16       CARBON_MCOLUMN_U16
#define CARBON_MSORTED_MULTISET_U16         0x05 /* ENQ */
#define CARBON_MUNSORTED_SET_U16            0x06 /* ACK */
#define CARBON_MSORTED_SET_U16              0x07 /* BEL */

/* abstract types for column-u32 containers */
#define CARBON_MUNSORTED_MULTISET_U32       CARBON_MCOLUMN_U32
#define CARBON_MSORTED_MULTISET_U32         0x09 /* TAB */
#define CARBON_MUNSORTED_SET_U32            0x0A /* LF */
#define CARBON_MSORTED_SET_U32              0x0B /* VT */

/* abstract types for column-u64 containers */
#define CARBON_MUNSORTED_MULTISET_U64       CARBON_MCOLUMN_U64
#define CARBON_MSORTED_MULTISET_U64         0x0D /* CR */
#define CARBON_MUNSORTED_SET_U64            0x0E /* SO */
#define CARBON_MSORTED_SET_U64              0x0F /* SI */

/* abstract types for column-i8 containers */
#define CARBON_MUNSORTED_MULTISET_I8        CARBON_MCOLUMN_I8
#define CARBON_MSORTED_MULTISET_I8          0x11 /* DC1 */
#define CARBON_MUNSORTED_SET_I8             0x12 /* DC2 */
#define CARBON_MSORTED_SET_I8               0x13 /* DC3 */

/* abstract types for column-i16 containers */
#define CARBON_MUNSORTED_MULTISET_I16       CARBON_MCOLUMN_I16
#define CARBON_MSORTED_MULTISET_I16         0x15 /* NAK */
#define CARBON_MUNSORTED_SET_I16            0x16 /* SYN */
#define CARBON_MSORTED_SET_I16              0x17 /* ETB */

/* abstract types for column-i32 containers */
#define CARBON_MUNSORTED_MULTISET_I32       CARBON_MCOLUMN_I32
#define CARBON_MSORTED_MULTISET_I32         0x19 /* EM */
#define CARBON_MUNSORTED_SET_I32            0x1A /* SUB */
#define CARBON_MSORTED_SET_I32              0x1B /* ESC */

/* abstract types for column-i64 containers */
#define CARBON_MUNSORTED_MULTISET_I64       CARBON_MCOLUMN_I64
#define CARBON_MSORTED_MULTISET_I64         0x1D /* GS */
#define CARBON_MUNSORTED_SET_I64            0x1E /* RS */
#define CARBON_MSORTED_SET_I64              0x1F /* US */

/* abstract types for column-float containers */
#define CARBON_MUNSORTED_MULTISET_FLOAT    CARBON_MCOLUMN_FLOAT
#define CARBON_MSORTED_MULTISET_FLOAT      '"'
#define CARBON_MUNSORTED_SET_FLOAT         '$'
#define CARBON_MSORTED_SET_FLOAT           '.'

/* abstract types for column-boolean containers */
#define CARBON_MUNSORTED_MULTISET_BOOLEAN  CARBON_MCOLUMN_BOOLEAN
#define CARBON_MSORTED_MULTISET_BOOLEAN    '_'
#define CARBON_MUNSORTED_SET_BOOLEAN       '\''
#define CARBON_MSORTED_SET_BOOLEAN         0x7F /* DEL */

typedef enum jak_carbon_key_type {
        /* no key, no revision number */
        JAK_CARBON_KEY_NOKEY = CARBON_MNOKEY,
        /* auto-generated 64bit unsigned integer key */
        JAK_CARBON_KEY_AUTOKEY = CARBON_MAUTOKEY,
        /* user-defined 64bit unsigned integer key */
        JAK_CARBON_KEY_UKEY = CARBON_MUKEY,
        /* user-defined 64bit signed integer key */
        JAK_CARBON_KEY_IKEY = CARBON_MIKEY,
        /* user-defined n-char string key */
        JAK_CARBON_KEY_SKEY = CARBON_MSKEY
} jak_carbon_key_e;

typedef enum carbon_abstract {
        /* Does not need further treatment to guarantee properties (unsorted, and not duplicate-free) */
        CARBON_ABSTRACT_BASE,
        /* particular abstract type with further properties (such as uniqueness of contained elements), enabling the
         * application to check certain promises and guarantees */
        CARBON_ABSTRACT_DERIVED,
} carbon_abstract_e;

typedef enum carbon_abstract_type {
        /* abstract base types */
        CARBON_TYPE_UNSORTED_MULTISET,     /* element type: values, distinct elements: no, sorted: no */
        CARBON_TYPE_UNSORTED_MULTIMAP,     /* element type: pairs, distinct elements: no, sorted: no */

        /* derived abstract types */
        CARBON_TYPE_SORTED_MULTISET,       /* element type: values, distinct elements: no, sorted: yes */
        CARBON_TYPE_UNSORTED_SET,          /* element type: values, distinct elements: yes, sorted: no */
        CARBON_TYPE_SORTED_MAP,            /* element type: pairs, distinct elements: yes, sorted: yes */
        CARBON_TYPE_SORTED_MULTIMAP,       /* element type: pairs, distinct elements: no, sorted: yes */
        CARBON_TYPE_UNSORTED_MAP          /* element type: pairs, distinct elements: yes, sorted: no */
} carbon_abstract_type_e;

typedef enum carbon_derived {
        /* abstract types for object containers */
        CARBON_UNSORTED_MULTIMAP = CARBON_MUNSORTED_MULTIMAP,
        CARBON_SORTED_MULTIMAP = CARBON_MSORTED_MULTIMAP,
        CARBON_UNSORTED_MAP = CARBON_MUNSORTED_MAP,
        CARBON_SORTED_MAP = CARBON_MSORTED_MAP,

        /* abstract types for array containers */
        CARBON_UNSORTED_MULTISET_ARRAY = CARBON_MUNSORTED_MULTISET_ARR,
        CARBON_SORTED_MULTISET_ARRAY = CARBON_MSORTED_MULTISET_ARR,
        CARBON_UNSORTED_SET_ARRAY = CARBON_MUNSORTED_SET_ARR,
        CARBON_SORTED_SET_ARRAY = CARBON_MSORTED_SET_ARR,

        /* abstract types for column-u8 containers */
        CARBON_UNSORTED_MULTISET_COL_U8 = CARBON_MUNSORTED_MULTISET_U8,
        CARBON_SORTED_MULTISET_COL_U8 = CARBON_MSORTED_MULTISET_U8,
        CARBON_UNSORTED_SET_COL_U8 = CARBON_MUNSORTED_SET_U8,
        CARBON_SORTED_SET_COL_U8 = CARBON_MSORTED_SET_U8,

        /* abstract types for column-u16 containers */
        CARBON_UNSORTED_MULTISET_COL_U16 = CARBON_MUNSORTED_MULTISET_U16,
        CARBON_SORTED_MULTISET_COL_U16 = CARBON_MSORTED_MULTISET_U16,
        CARBON_UNSORTED_SET_COL_U16 = CARBON_MUNSORTED_SET_U16,
        CARBON_SORTED_SET_COL_U16 = CARBON_MSORTED_SET_U16,

        /* abstract types for column-u32 containers */
        CARBON_UNSORTED_MULTISET_COL_U32 = CARBON_MUNSORTED_MULTISET_U32,
        CARBON_SORTED_MULTISET_COL_U32 = CARBON_MSORTED_MULTISET_U32,
        CARBON_UNSORTED_SET_COL_U32 = CARBON_MUNSORTED_SET_U32,
        CARBON_SORTED_SET_COL_U32 = CARBON_MSORTED_SET_U32,

        /* abstract types for column-u64 containers */
        CARBON_UNSORTED_MULTISET_COL_U64 = CARBON_MUNSORTED_MULTISET_U64,
        CARBON_SORTED_MULTISET_COL_U64 = CARBON_MSORTED_MULTISET_U64,
        CARBON_UNSORTED_SET_COL_U64 = CARBON_MUNSORTED_SET_U64,
        CARBON_SORTED_SET_COL_U64 = CARBON_MSORTED_SET_U64,

        /* abstract types for column-i8 containers */
        CARBON_UNSORTED_MULTISET_COL_I8 = CARBON_MUNSORTED_MULTISET_I8,
        CARBON_SORTED_MULTISET_COL_I8 = CARBON_MSORTED_MULTISET_I8,
        CARBON_UNSORTED_SET_COL_I8 = CARBON_MUNSORTED_SET_I8,
        CARBON_SORTED_SET_COL_I8 = CARBON_MSORTED_SET_I8,

        /* abstract types for column-i16 containers */
        CARBON_UNSORTED_MULTISET_COL_I16 = CARBON_MUNSORTED_MULTISET_I16,
        CARBON_SORTED_MULTISET_COL_I16 = CARBON_MSORTED_MULTISET_I16,
        CARBON_UNSORTED_SET_COL_I16 = CARBON_MUNSORTED_SET_I16,
        CARBON_SORTED_SET_COL_I16 = CARBON_MSORTED_SET_I16,

        /* abstract types for column-i32 containers */
        CARBON_UNSORTED_MULTISET_COL_I32 = CARBON_MUNSORTED_MULTISET_I32,
        CARBON_SORTED_MULTISET_COL_I32 = CARBON_MSORTED_MULTISET_I32,
        CARBON_UNSORTED_SET_COL_I32 = CARBON_MUNSORTED_SET_I32,
        CARBON_SORTED_SET_COL_I32 = CARBON_MSORTED_SET_I32,

        /* abstract types for column-i64 containers */
        CARBON_UNSORTED_MULTISET_COL_I64 = CARBON_MUNSORTED_MULTISET_I64,
        CARBON_SORTED_MULTISET_COL_I64 = CARBON_MSORTED_MULTISET_I64,
        CARBON_UNSORTED_SET_COL_I64 = CARBON_MUNSORTED_SET_I64,
        CARBON_SORTED_SET_COL_I64 = CARBON_MSORTED_SET_I64,

        /* abstract types for column-float containers */
        CARBON_UNSORTED_MULTISET_COL_FLOAT = CARBON_MUNSORTED_MULTISET_FLOAT,
        CARBON_SORTED_MULTISET_COL_FLOAT = CARBON_MSORTED_MULTISET_FLOAT,
        CARBON_UNSORTED_SET_COL_FLOAT = CARBON_MUNSORTED_SET_FLOAT,
        CARBON_SORTED_SET_COL_FLOAT = CARBON_MSORTED_SET_FLOAT,

        /* abstract types for column-boolean containers */
        CARBON_UNSORTED_MULTISET_COL_BOOLEAN = CARBON_MUNSORTED_MULTISET_BOOLEAN,
        CARBON_SORTED_MULTISET_COL_BOOLEAN = CARBON_MSORTED_MULTISET_BOOLEAN,
        CARBON_UNSORTED_SET_COL_BOOLEAN = CARBON_MUNSORTED_SET_BOOLEAN,
        CARBON_SORTED_SET_COL_BOOLEAN = CARBON_MSORTED_SET_BOOLEAN
} carbon_derived_e;

JAK_DEFINE_ERROR_GETTER(jak_carbon);
JAK_DEFINE_ERROR_GETTER(jak_carbon_new);

#define JAK_CARBON_KEEP              0x0
#define JAK_CARBON_SHRINK            0x1
#define JAK_CARBON_COMPACT           0x2
#define JAK_CARBON_OPTIMIZE          (JAK_CARBON_SHRINK | JAK_CARBON_COMPACT)

/**
 * Constructs a new context in which a new document can be created. The parameter <b>options</b> controls
 * how reserved spaces should be handled after document creation is done. Set <code>options</code> to
 * <code>CARBON_KEEP</code> for no optimization. With this option, all capacities (i.e., additional ununsed but free
 * space) in containers (objects, arrays, and columns) are kept and tailing free space after the document is
 * kept, too. Use this option to optimize for "insertion-heavy" documents since keeping all capacities lowerst the
 * probability of reallocations and memory movements. Set <b>options</b> to <code>CARBON_COMPACT</code> if capacities in
 * containers should be removed after creation, and <code>CARBON_COMPACT</code> to remove tailing free space. Use
 * <code>CARBON_OPTIMIZE</code> to use both <code>CARBON_SHRINK</code> and <code>CARBON_COMPACT</code>.
 *
 * As a rule of thumb for <b>options</b>. The resulting document...
 * <ul>
 *  <li>...will be updated heavily where updates may change the type-width of fields, will be target of many inserts
 *  containers, use <code>CARBON_KEEP</code>. The document will have a notable portion of reserved memory contained;
 *  insertions or updates will, however, not require immediately reallocation or memory movements.</li>
 *  <li>...will <i>not</i> be target of insertion of strings or blob fields in the near future, use
 *      <code>CARBON_SHRINK</code>. The document will not have padding reserved memory at the end, which means that
 *      a realloction will be required once the document grows (e.g., a container must be englarged). Typically,
 *      document growth is handled with container capacities (see <code>CARBON_COMPACT</code>). However, insertions
 *      of variable-length data (i.e., strings and blobs) may require container enlargement. In this case, having
 *      padding reserved memory at the end of the document lowers the risk of a reallocation.</li>
 *  <li>...will <i>not</i> not be target of insertion operations or update operations that changes a fields type-width
 *      in the near future. In simpler words, if a document is updated and each such update keeps the (byte) size
 *      of the field, use <code>CARBON_COMPACT</code>. This option will remove all capacities in containers.</li>
 *  <li>...is read-mostly, or updates will not change the type or type-width of fields, use <code>CARBON_OPTIMIZE</code>.
 *      The document will have the smallest memory footprint possible.</li>
 * </ul>
 */
jak_carbon_insert *jak_carbon_create_begin(jak_carbon_new *context, jak_carbon *doc, jak_carbon_key_e type, int options);
bool jak_carbon_create_end(jak_carbon_new *context);
bool jak_carbon_create_empty(jak_carbon *doc, jak_carbon_key_e type);
bool jak_carbon_create_empty_ex(jak_carbon *doc, jak_carbon_key_e type, jak_u64 doc_cap, jak_u64 array_cap);

bool jak_carbon_from_json(jak_carbon *doc, const char *json, jak_carbon_key_e type, const void *key, jak_error *err);
bool jak_carbon_from_raw_data(jak_carbon *doc, jak_error *err, const void *data, jak_u64 len);

bool jak_carbon_drop(jak_carbon *doc);

const void *jak_carbon_raw_data(jak_u64 *len, jak_carbon *doc);

bool jak_carbon_is_up_to_date(jak_carbon *doc);
bool jak_carbon_key_type(jak_carbon_key_e *out, jak_carbon *doc);
const void *jak_carbon_key_raw_value(jak_u64 *len, jak_carbon_key_e *type, jak_carbon *doc);
bool jak_carbon_key_signed_value(jak_i64 *key, jak_carbon *doc);
bool jak_carbon_key_unsigned_value(jak_u64 *key, jak_carbon *doc);
const char *jak_carbon_key_jak_string_value(jak_u64 *len, jak_carbon *doc);
bool jak_carbon_has_key(jak_carbon_key_e type);
bool jak_carbon_key_is_unsigned(jak_carbon_key_e type);
bool jak_carbon_key_is_signed(jak_carbon_key_e type);
bool jak_carbon_key_is_string(jak_carbon_key_e type);
bool jak_carbon_clone(jak_carbon *clone, jak_carbon *doc);
bool jak_carbon_commit_hash(jak_u64 *hash, jak_carbon *doc);

bool jak_carbon_to_str(jak_string *dst, jak_carbon_printer_impl_e printer, jak_carbon *doc);
const char *jak_carbon_to_json_extended(jak_string *dst, jak_carbon *doc);
const char *jak_carbon_to_json_compact(jak_string *dst, jak_carbon *doc);
char *jak_carbon_to_json_extended_dup(jak_carbon *doc);
char *jak_carbon_to_json_compact_dup(jak_carbon *doc);
bool jak_carbon_iterator_open(jak_carbon_array_it *it, jak_carbon *doc);
bool jak_carbon_iterator_close(jak_carbon_array_it *it);
bool jak_carbon_print(FILE *file, jak_carbon_printer_impl_e printer, jak_carbon *doc);
bool jak_carbon_hexdump_print(FILE *file, jak_carbon *doc);

JAK_END_DECL

#endif
