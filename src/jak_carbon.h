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
#include <jak_carbon_markers.h>
#include <jak_carbon_abstract.h>

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

typedef enum jak_carbon_printer_impl {
        JAK_JSON_EXTENDED, JAK_JSON_COMPACT
} jak_carbon_printer_impl_e;

#define JAK_CARBON_NIL_STR "_nil"

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

JAK_DEFINE_ERROR_GETTER(jak_carbon);
JAK_DEFINE_ERROR_GETTER(jak_carbon_new);

#define JAK_CARBON_KEEP              0x00 /* do not shrink, do not compact, use UNSORTED_MULTISET (equiv. JSON array) */
#define JAK_CARBON_SHRINK            0x01 /* perform shrinking, i.e., remove tail-buffer from carbon file */
#define JAK_CARBON_COMPACT           0x02 /* perform compacting, i.e., remove reserved memory from containers */
#define JAK_CARBON_UNSORTED_MULTISET 0x04 /* annotate the record outer-most array as unsorted multi set */
#define JAK_CARBON_SORTED_MULTISET   0x08 /* annotate the record outer-most array as sorted multi set */
#define JAK_CARBON_UNSORTED_SET      0x10 /* annotate the record outer-most array as unsorted set */
#define JAK_CARBON_SORTED_SET        0x20 /* annotate the record outer-most array as sorted set */

#define JAK_CARBON_OPTIMIZE          (JAK_CARBON_SHRINK | JAK_CARBON_COMPACT | JAK_CARBON_UNSORTED_MULTISET)

/**
 * Constructs a new context in which a new document can be created. The parameter <b>options</b> controls
 * how reserved spaces should be handled after document creation is done, and which abstract type for the outer-most
 * record array should be used.
 *
 * Set <code>options</code> to <code>CARBON_KEEP</code> for no optimization. With this option, all capacities
 * (i.e., additional ununsed but free space) in containers (objects, arrays, and columns) are kept and tailing free
 * space after the document is kept, too. Use this option to optimize for "insertion-heavy" documents since keeping all
 * capacities lowest the probability of reallocations and memory movements. Set <b>options</b> to
 * <code>CARBON_COMPACT</code> if capacities in containers should be removed after creation, and
 * <code>CARBON_COMPACT</code> to remove tailing free space. Use <code>CARBON_OPTIMIZE</code> to use both
 * <code>CARBON_SHRINK</code> and <code>CARBON_COMPACT</code>.
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
 *
 * To annotate the outer-most record array as an array container with particular properties, set <code>options</code>
 * to...
 * <ul>
 *  <li>...<code>CARBON_UNSORTED_MULTISET</code> that allows duplicates and has no sorting</li>
 *  <li>...<code>CARBON_SORTED_MULTISET</code> that allows duplicates and has a particular sorting</li>
 *  <li>...<code>CARBON_UNSORTED_SET</code> that disallows duplicates and has no sorting</li>
 *  <li>...<code>CARBON_SORTED_SET</code> that disallows duplicates and has a particular sorting</li>
 *  </ul>
 *  If no annotation is given, the outer-most record array is annotated as <code>CARBON_UNSORTED_MULTISET</code>, which
 *  matches the semantics of a JSON array. Please note that this library does not provide any features to make
 *  deduplication and sorting work. Instead, the annotation stores the semantics of that particular container, which
 *  functionality must be effectively implemented at caller site.
 */
jak_carbon_insert *jak_carbon_create_begin(jak_carbon_new *context, jak_carbon *doc, jak_carbon_key_e type, int options);
bool jak_carbon_create_end(jak_carbon_new *context);
bool jak_carbon_create_empty(jak_carbon *doc, carbon_list_derivable_e derivation, jak_carbon_key_e type);
bool jak_carbon_create_empty_ex(jak_carbon *doc, carbon_list_derivable_e derivation, jak_carbon_key_e type, jak_u64 doc_cap, jak_u64 array_cap);

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
