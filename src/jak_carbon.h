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
#include <jak_global_id.h>
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
#include <jak_global_id.h>
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

struct jak_carbon; /* forwarded */
struct jak_carbon_array_it; /* forwarded from carbon-array-it.h */
struct jak_carbon_find; /* forward from carbon-find.h */

typedef u32 listener_handle_t;

struct jak_carbon {
    struct memblock *memblock;
    struct memfile memfile;

    struct {
        struct spinlock write_lock;
        bool commit_lock;
        bool is_latest;
    } versioning;

    struct err err;
};

struct jak_carbon_revise {
    struct jak_carbon *original;
    struct jak_carbon *revised_doc;
    struct err err;
};

struct jak_carbon_binary {
    const char *mime_type;
    u64 mime_type_strlen;

    const void *blob;
    u64 blob_len;
};

struct jak_carbon_new {
    struct err err;
    struct jak_carbon original;
    struct jak_carbon_revise revision_context;
    struct jak_carbon_array_it *content_it;
    struct jak_carbon_insert *inserter;

    /* options shrink or compact (or both) documents, see
     * CARBON_KEEP, CARBON_SHRINK, CARBON_COMPACT, and CARBON_OPTIMIZE  */
    int mode;
};

enum carbon_container_type {
    CARBON_OBJECT, CARBON_ARRAY, CARBON_COLUMN
};

enum carbon_printer_impl {
    JSON_EXTENDED,
    JSON_COMPACT
};


#define JAK_CARBON_MARKER_KEY_NOKEY '?'
#define JAK_CARBON_MARKER_KEY_AUTOKEY '*'
#define JAK_CARBON_MARKER_KEY_UKEY '+'
#define JAK_CARBON_MARKER_KEY_IKEY '-'
#define JAK_CARBON_MARKER_KEY_SKEY '!'

enum carbon_key_type {
        /* no key, no revision number */
        CARBON_KEY_NOKEY = JAK_CARBON_MARKER_KEY_NOKEY,
        /* auto-generated 64bit unsigned integer key */
        CARBON_KEY_AUTOKEY = JAK_CARBON_MARKER_KEY_AUTOKEY,
        /* user-defined 64bit unsigned integer key */
        CARBON_KEY_UKEY = JAK_CARBON_MARKER_KEY_UKEY,
        /* user-defined 64bit signed integer key */
        CARBON_KEY_IKEY = JAK_CARBON_MARKER_KEY_IKEY,
        /* user-defined n-char string key */
        CARBON_KEY_SKEY = JAK_CARBON_MARKER_KEY_SKEY
};

#define JAK_CARBON_NIL_STR "_nil"

#define JAK_CARBON_MARKER_NULL 'n'
#define JAK_CARBON_MARKER_TRUE 't'
#define JAK_CARBON_MARKER_FALSE 'f'
#define JAK_CARBON_MARKER_STRING 's'
#define JAK_CARBON_MARKER_U8 'c'
#define JAK_CARBON_MARKER_U16 'd'
#define JAK_CARBON_MARKER_U32 'i'
#define JAK_CARBON_MARKER_U64 'l'
#define JAK_CARBON_MARKER_I8 'C'
#define JAK_CARBON_MARKER_I16 'D'
#define JAK_CARBON_MARKER_I32 'I'
#define JAK_CARBON_MARKER_I64 'L'
#define JAK_CARBON_MARKER_FLOAT 'r'
#define JAK_CARBON_MARKER_BINARY 'b'
#define JAK_CARBON_MARKER_CUSTOM_BINARY 'x'

#define JAK_CARBON_MARKER_OBJECT_BEGIN '{'
#define JAK_CARBON_MARKER_OBJECT_END '}'

#define JAK_CARBON_MARKER_ARRAY_BEGIN '['
#define JAK_CARBON_MARKER_ARRAY_END ']'

#define JAK_CARBON_MARKER_COLUMN_U8 '1'
#define JAK_CARBON_MARKER_COLUMN_U16 '2'
#define JAK_CARBON_MARKER_COLUMN_U32 '3'
#define JAK_CARBON_MARKER_COLUMN_U64 '4'
#define JAK_CARBON_MARKER_COLUMN_I8 '5'
#define JAK_CARBON_MARKER_COLUMN_I16 '6'
#define JAK_CARBON_MARKER_COLUMN_I32 '7'
#define JAK_CARBON_MARKER_COLUMN_I64 '8'
#define JAK_CARBON_MARKER_COLUMN_FLOAT 'R'
#define JAK_CARBON_MARKER_COLUMN_BOOLEAN 'B'

JAK_DEFINE_ERROR_GETTER(jak_carbon);

JAK_DEFINE_ERROR_GETTER(jak_carbon_new);

#define JAK_CARBON_KEEP              0x0
#define JAK_CARBON_SHRINK            0x1
#define JAK_CARBON_COMPACT           0x2
#define JAK_CARBON_OPTIMIZE          (JAK_CARBON_SHRINK | JAK_CARBON_COMPACT)

/**
 * Constructs a new context in which a new document can be created. The parameter <b>mode</b> controls
 * how reserved spaces should be handled after document creation is done. Set <code>mode</code> to
 * <code>CARBON_KEEP</code> for no optimization. With this mode, all capacities (i.e., additional ununsed but free
 * space) in containers (objects, arrays, and columns) are kept and tailing free space after the document is
 * kept, too. Use this mode to optimize for "insertion-heavy" documents since keeping all capacities lowerst the
 * probability of reallocations and memory movements. Set <b>mode</b> to <code>CARBON_COMPACT</code> if capacities in
 * containers should be removed after creation, and <code>CARBON_COMPACT</code> to remove tailing free space. Use
 * <code>CARBON_OPTIMIZE</code> to use both <code>CARBON_SHRINK</code> and <code>CARBON_COMPACT</code>.
 *
 * As a rule of thumb for <b>mode</b>. The resulting document...
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
 *      of the field, use <code>CARBON_COMPACT</code>. This mode will remove all capacities in containers.</li>
 *  <li>...is read-mostly, or updates will not change the type or type-width of fields, use <code>CARBON_OPTIMIZE</code>.
 *      The document will have the smallest memory footprint possible.</li>
 * </ul>
 */
struct jak_carbon_insert *carbon_create_begin(struct jak_carbon_new *context, struct jak_carbon *doc,
                                          enum carbon_key_type key_type, int mode);
bool carbon_create_end(struct jak_carbon_new *context);
bool carbon_create_empty(struct jak_carbon *doc, enum carbon_key_type key_type);
bool carbon_create_empty_ex(struct jak_carbon *doc, enum carbon_key_type key_type, u64 doc_cap_byte,
                            u64 array_cap_byte);
bool carbon_from_json(struct jak_carbon *doc, const char *json, enum carbon_key_type key_type,
                      const void *key, struct err *err);
bool carbon_drop(struct jak_carbon *doc);
const void *carbon_raw_data(u64 *len, struct jak_carbon *doc);
bool carbon_is_up_to_date(struct jak_carbon *doc);
bool carbon_key_type(enum carbon_key_type *out, struct jak_carbon *doc);
const void *carbon_key_raw_value(u64 *key_len, enum carbon_key_type *type, struct jak_carbon *doc);
bool carbon_key_signed_value(i64 *key, struct jak_carbon *doc);
bool carbon_key_unsigned_value(u64 *key, struct jak_carbon *doc);
const char *carbon_key_string_value(u64 *str_len, struct jak_carbon *doc);
bool carbon_has_key(enum carbon_key_type type);
bool carbon_key_is_unsigned_type(enum carbon_key_type type);
bool carbon_key_is_signed_type(enum carbon_key_type type);
bool carbon_key_is_string_type(enum carbon_key_type type);
bool carbon_clone(struct jak_carbon *clone, struct jak_carbon *doc);
bool carbon_commit_hash(u64 *commit_hash, struct jak_carbon *doc);
bool carbon_to_str(struct jak_string *dst, enum carbon_printer_impl printer, struct jak_carbon *doc);
const char *carbon_to_json_extended(struct jak_string *dst, struct jak_carbon *doc);
const char *carbon_to_json_compact(struct jak_string *dst, struct jak_carbon *doc);
char *carbon_to_json_extended_dup(struct jak_carbon *doc);
char *carbon_to_json_compact_dup(struct jak_carbon *doc);
bool carbon_iterator_open(struct jak_carbon_array_it *it, struct jak_carbon *doc);
bool carbon_iterator_close(struct jak_carbon_array_it *it);
bool carbon_print(FILE *file, enum carbon_printer_impl printer, struct jak_carbon *doc);
bool carbon_hexdump_print(FILE *file, struct jak_carbon *doc);

JAK_END_DECL

#endif
