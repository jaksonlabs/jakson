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

#ifndef CARBON_H
#define CARBON_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/error.h>
#include <ark-js/shared/mem/block.h>
#include <ark-js/shared/mem/file.h>
#include <ark-js/carbon/oid/oid.h>
#include <ark-js/shared/stdx/string.h>
#include <ark-js/shared/async/spin.h>
#include <ark-js/shared/stdx/vec.h>
#include <ark-js/shared/common.h>
#include <ark-js/carbon/alloc/alloc.h>
#include <ark-js/shared/stdx/bitmap.h>
#include <ark-js/shared/stdx/bloom.h>
#include <ark-js/carbon/archive/archive.h>
#include <ark-js/carbon/archive/archive-iter.h>
#include <ark-js/carbon/archive/archive-visitor.h>
#include <ark-js/carbon/archive/archive-converter.h>
#include <ark-js/shared/json/encoded_doc.h>
#include <ark-js/shared/common.h>
#include <ark-js/shared/utils/convert.h>
#include <ark-js/shared/json/columndoc.h>
#include <ark-js/shared/json/doc.h>
#include <ark-js/shared/error.h>
#include <ark-js/shared/stdx/hash_table.h>
#include <ark-js/shared/hash/hash.h>
#include <ark-js/carbon/coding/coding_huffman.h>
#include <ark-js/shared/json/json.h>
#include <ark-js/shared/mem/block.h>
#include <ark-js/shared/mem/file.h>
#include <ark-js/carbon/oid/oid.h>
#include <ark-js/shared/stdx/sort.h>
#include <ark-js/shared/async/parallel.h>
#include <ark-js/shared/stdx/slicelist.h>
#include <ark-js/shared/async/spin.h>
#include <ark-js/shared/stdx/strdic.h>
#include <ark-js/shared/stdx/strhash.h>
#include <ark-js/carbon/archive/archive-strid-iter.h>
#include <ark-js/carbon/archive/archive-cache.h>
#include <ark-js/shared/utils/time.h>
#include <ark-js/shared/types.h>
#include <ark-js/carbon/archive/archive-query.h>
#include <ark-js/shared/stdx/vec.h>
#include <ark-js/carbon/alloc/trace.h>
#include <ark-js/carbon/encode/encode_async.h>
#include <ark-js/carbon/encode/encode_sync.h>
#include <ark-js/carbon/strhash/strhash-mem.h>
#include <ark-js/carbon/string-pred/pred-contains.h>
#include <ark-js/carbon/string-pred/pred-equals.h>
#include <ark-js/carbon/carbon-printers.h>
#include <ark-js/shared/stdx/varuint.h>

ARK_BEGIN_DECL

struct carbon; /* forwarded */
struct carbon_array_it; /* forwarded from carbon-array-it.h */
struct carbon_find; /* forward from carbon-find.h */

typedef u32 listener_handle_t;

struct carbon {
    struct memblock *memblock;
    struct memfile memfile;

    struct {
        struct spinlock write_lock;
        bool commit_lock;
        bool is_latest;
    } versioning;

    struct err err;
};

struct carbon_revise {
    struct carbon *original;
    struct carbon *revised_doc;
    struct err err;
};

struct carbon_binary {
    const char *mime_type;
    u64 mime_type_strlen;

    const void *blob;
    u64 blob_len;
};

struct carbon_new {
    struct err err;
    struct carbon original;
    struct carbon_revise revision_context;
    struct carbon_array_it *content_it;
    struct carbon_insert *inserter;

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

enum carbon_key_type {
    /* no key, no revision number */
            CARBON_KEY_NOKEY,
    /* auto-generated 64bit integer key */
            CARBON_KEY_AUTOKEY,
    /* user-defined 64bit unsigned integer key */
            CARBON_KEY_UKEY,
    /* user-defined 64bit signed integer key */
            CARBON_KEY_IKEY,
    /* user-defined n-char string key */
            CARBON_KEY_SKEY
};

#define CARBON_NIL_STR "_nil"

#define CARBON_MARKER_NULL 'n'
#define CARBON_MARKER_TRUE 't'
#define CARBON_MARKER_FALSE 'f'
#define CARBON_MARKER_STRING 's'
#define CARBON_MARKER_U8 'c'
#define CARBON_MARKER_U16 'd'
#define CARBON_MARKER_U32 'i'
#define CARBON_MARKER_U64 'l'
#define CARBON_MARKER_I8 'C'
#define CARBON_MARKER_I16 'D'
#define CARBON_MARKER_I32 'I'
#define CARBON_MARKER_I64 'L'
#define CARBON_MARKER_FLOAT 'r'
#define CARBON_MARKER_BINARY 'b'
#define CARBON_MARKER_CUSTOM_BINARY 'x'

#define CARBON_MARKER_OBJECT_BEGIN '{'
#define CARBON_MARKER_OBJECT_END '}'

#define CARBON_MARKER_ARRAY_BEGIN '['
#define CARBON_MARKER_ARRAY_END ']'

#define CARBON_MARKER_COLUMN_U8 '1'
#define CARBON_MARKER_COLUMN_U16 '2'
#define CARBON_MARKER_COLUMN_U32 '3'
#define CARBON_MARKER_COLUMN_U64 '4'
#define CARBON_MARKER_COLUMN_I8 '5'
#define CARBON_MARKER_COLUMN_I16 '6'
#define CARBON_MARKER_COLUMN_I32 '7'
#define CARBON_MARKER_COLUMN_I64 '8'
#define CARBON_MARKER_COLUMN_FLOAT 'R'
#define CARBON_MARKER_COLUMN_BOOLEAN 'B'

#define CARBON_MARKER_KEY_NOKEY '?'
#define CARBON_MARKER_KEY_AUTOKEY '*'
#define CARBON_MARKER_KEY_UKEY '+'
#define CARBON_MARKER_KEY_IKEY '-'
#define CARBON_MARKER_KEY_SKEY '!'

ARK_DEFINE_ERROR_GETTER(carbon);

ARK_DEFINE_ERROR_GETTER(carbon_new);

#define CARBON_KEEP              0x0
#define CARBON_SHRINK            0x1
#define CARBON_COMPACT           0x2
#define CARBON_OPTIMIZE          (CARBON_SHRINK | CARBON_COMPACT)

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
struct carbon_insert *carbon_create_begin(struct carbon_new *context, struct carbon *doc,
                                          enum carbon_key_type key_type, int mode);
bool carbon_create_end(struct carbon_new *context);
bool carbon_create_empty(struct carbon *doc, enum carbon_key_type key_type);
bool carbon_create_empty_ex(struct carbon *doc, enum carbon_key_type key_type, u64 doc_cap_byte,
                            u64 array_cap_byte);
bool carbon_from_json(struct carbon *doc, const char *json, enum carbon_key_type key_type,
                      const void *key, struct err *err);
bool carbon_drop(struct carbon *doc);
const void *carbon_raw_data(u64 *len, struct carbon *doc);
bool carbon_is_up_to_date(struct carbon *doc);
bool carbon_key_get_type(enum carbon_key_type *out, struct carbon *doc);
const void *carbon_key_raw_value(u64 *key_len, enum carbon_key_type *type, struct carbon *doc);
bool carbon_key_signed_value(i64 *key, struct carbon *doc);
bool carbon_key_unsigned_value(u64 *key, struct carbon *doc);
const char *carbon_key_string_value(u64 *str_len, struct carbon *doc);
bool carbon_has_key(enum carbon_key_type type);
bool carbon_key_is_unsigned_type(enum carbon_key_type type);
bool carbon_key_is_signed_type(enum carbon_key_type type);
bool carbon_key_is_string_type(enum carbon_key_type type);
bool carbon_clone(struct carbon *clone, struct carbon *doc);
bool carbon_commit_hash(u64 *commit_hash, struct carbon *doc);
bool carbon_to_str(struct string *dst, enum carbon_printer_impl printer, struct carbon *doc);
const char *carbon_to_json_extended(struct string *dst, struct carbon *doc);
const char *carbon_to_json_compact(struct string *dst, struct carbon *doc);
char *carbon_to_json_extended_dup(struct carbon *doc);
char *carbon_to_json_compact_dup(struct carbon *doc);
bool carbon_iterator_open(struct carbon_array_it *it, struct carbon *doc);
bool carbon_iterator_close(struct carbon_array_it *it);
bool carbon_print(FILE *file, struct carbon *doc);
bool carbon_hexdump_print(FILE *file, struct carbon *doc);

ARK_END_DECL

#endif
