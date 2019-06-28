/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements the document format itself
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

#ifndef BISON_H
#define BISON_H

#include "shared/common.h"
#include "shared/error.h"
#include "core/mem/block.h"
#include "core/mem/file.h"
#include "core/oid/oid.h"
#include "std/string_builder.h"
#include "core/async/spin.h"
#include "std/vec.h"

NG5_BEGIN_DECL

struct bison; /* forwarded */
struct bison_array_it; /* forwarded from bison-array-it.h */
struct bison_find; /* forward from bison-find.h */

struct bison_event_listener
{
        void *extra;
        void (*clone)(struct bison_event_listener *dst, struct bison_event_listener *self);
        void (*drop)(struct bison_event_listener *self);

        void (*on_revision_begin)(struct bison_event_listener *self, struct bison *doc);
        void (*on_revision_end)(struct bison_event_listener *self, struct bison *doc);
        void (*on_revision_abort)(struct bison_event_listener *self, struct bison *doc);
        void (*on_new_revision)(struct bison_event_listener *self, struct bison *revised, struct bison *original);
};

struct bison_handler
{
        bool in_use;
        struct bison_event_listener listener;
};

typedef u32 listener_handle_t;

struct bison
{
        struct memblock *memblock;
        struct memfile memfile;

        struct vector ofType(struct bison_handler) handler;

        struct
        {
                struct spinlock write_lock;
                bool revision_lock;
                bool is_latest;
        } versioning;

        struct err err;
};

struct bison_revise
{
        struct bison *original;
        struct bison *revised_doc;
};

enum bison_field_type
{
        /* constants */
        BISON_FIELD_TYPE_NULL = 'n', /* null */
        BISON_FIELD_TYPE_TRUE = 't', /* true */
        BISON_FIELD_TYPE_FALSE = 'f', /* false */

        /* JSON typing */
        BISON_FIELD_TYPE_OBJECT = 'o', /* object */
        BISON_FIELD_TYPE_ARRAY = '[', /* variable-type array of elements of varying type */
        BISON_FIELD_TYPE_COLUMN = '(', /* fixed-type array of elements of particular type */
        BISON_FIELD_TYPE_STRING = 's', /* UTF-8 string */

        /* JSON numbers */
        BISON_FIELD_TYPE_NUMBER_U8 = 'c', /* 8bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_U16 = 'd', /* 16bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_U32 = 'i', /* 32bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_U64 = 'l', /* 64bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_I8 = 'C', /* 8bit signed integer */
        BISON_FIELD_TYPE_NUMBER_I16 = 'D', /* 16bit signed integer */
        BISON_FIELD_TYPE_NUMBER_I32 = 'I', /* 32bit signed integer */
        BISON_FIELD_TYPE_NUMBER_I64 = 'L', /* 64bit signed integer */
        BISON_FIELD_TYPE_NUMBER_FLOAT = 'r', /* 32bit float */

        /* user-defined binary data */
        BISON_FIELD_TYPE_BINARY = 'b', /* arbitrary binary object with known mime type */
        BISON_FIELD_TYPE_BINARY_CUSTOM = 'x', /* arbitrary binary object with unknown mime type*/
};

struct bison_binary
{
        const char *mime_type;
        u64 mime_type_strlen;

        const void *blob;
        u64 blob_len;
};

struct bison_printer
{
        void *extra;

        void (*drop)(struct bison_printer *self);

        void (*print_bison_begin)(struct bison_printer *self, struct string_builder *builder);
        void (*print_bison_end)(struct bison_printer *self, struct string_builder *builder);

        void (*print_bison_header_begin)(struct bison_printer *self, struct string_builder *builder);
        void (*print_bison_header_contents)(struct bison_printer *self, struct string_builder *builder, object_id_t oid, u64 rev);
        void (*print_bison_header_end)(struct bison_printer *self, struct string_builder *builder);

        void (*print_bison_payload_begin)(struct bison_printer *self, struct string_builder *builder);
        void (*print_bison_payload_end)(struct bison_printer *self, struct string_builder *builder);

        void (*print_bison_array_begin)(struct bison_printer *self, struct string_builder *builder);
        void (*print_bison_array_end)(struct bison_printer *self, struct string_builder *builder);

        void (*print_bison_null)(struct bison_printer *self, struct string_builder *builder);
        void (*print_bison_true)(struct bison_printer *self, struct string_builder *builder);
        void (*print_bison_false)(struct bison_printer *self, struct string_builder *builder);

        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*print_bison_signed)(struct bison_printer *self, struct string_builder *builder, const i64 *value);
        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*print_bison_unsigned)(struct bison_printer *self, struct string_builder *builder, const u64 *value);
        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*print_bison_float)(struct bison_printer *self, struct string_builder *builder, const float *value);

        void (*print_bison_string)(struct bison_printer *self, struct string_builder *builder, const char *value, u64 strlen);
        void (*print_bison_binary)(struct bison_printer *self, struct string_builder *builder, const struct bison_binary *binary);

        void (*print_bison_comma)(struct bison_printer *self, struct string_builder *builder);
};

enum bison_printer_impl
{
        JSON_FORMATTER
};

struct bison_new
{
        struct err err;
        struct bison original;
        struct bison_revise revision_context;
        struct bison_array_it *content_it;
        struct bison_insert *inserter;

        /* options shrink or compact (or both) documents, see
         * BISON_KEEP, BISON_SHRINK, BISON_COMPACT, and BISON_OPTIMIZE  */
        int mode;
};

enum bison_container_type { BISON_ARRAY, BISON_COLUMN };

#define BISON_FIELD_TYPE_NULL_STR "null"
#define BISON_FIELD_TYPE_TRUE_STR "boolean (true)"
#define BISON_FIELD_TYPE_FALSE_STR "boolean (false)"
#define BISON_FIELD_TYPE_OBJECT_STR "object"
#define BISON_FIELD_TYPE_ARRAY_STR "array"
#define BISON_FIELD_TYPE_COLUMN_STR "column"
#define BISON_FIELD_TYPE_STRING_STR "string"
#define BISON_FIELD_TYPE_BINARY_STR "binary"
#define BISON_FIELD_TYPE_NUMBER_U8_STR "number (u8)"
#define BISON_FIELD_TYPE_NUMBER_U16_STR "number (u16)"
#define BISON_FIELD_TYPE_NUMBER_U32_STR "number (u32)"
#define BISON_FIELD_TYPE_NUMBER_U64_STR "number (u64)"
#define BISON_FIELD_TYPE_NUMBER_I8_STR "number (i8)"
#define BISON_FIELD_TYPE_NUMBER_I16_STR "number (i16)"
#define BISON_FIELD_TYPE_NUMBER_I32_STR "number (i32)"
#define BISON_FIELD_TYPE_NUMBER_I64_STR "number (i64)"
#define BISON_FIELD_TYPE_NUMBER_FLOAT_STR "number (float)"

#define BISON_MARKER_ARRAY_BEGIN '['
#define BISON_MARKER_ARRAY_END ']'

#define BISON_MARKER_COLUMN_BEGIN '('
#define BISON_MARKER_COLUMN_END ')'

NG5_DEFINE_ERROR_GETTER(bison);
NG5_DEFINE_ERROR_GETTER(bison_new);

#define BISON_KEEP              0x0
#define BISON_SHRINK            0x1
#define BISON_COMPACT           0x2
#define BISON_OPTIMIZE          (BISON_SHRINK | BISON_COMPACT)

/**
 * Constructs a new context in which a new document can be created. The parameter <b>mode</b> controls
 * how reserved spaces should be handled after document creation is done. Set <code>mode</code> to
 * <code>BISON_KEEP</code> for no optimization. With this mode, all capacities (i.e., additional ununsed but free
 * space) in containers (objects, arrays, and columns) are kept and tailing free space after the document is
 * kept, too. Use this mode to optimize for "insertion-heavy" documents since keeping all capacities lowerst the
 * probability of reallocations and memory movements. Set <b>mode</b> to <code>BISON_COMPACT</code> if capacities in
 * containers should be removed after creation, and <code>BISON_COMPACT</code> to remove tailing free space. Use
 * <code>BISON_OPTIMIZE</code> to use both <code>BISON_SHRINK</code> and <code>BISON_COMPACT</code>.
 *
 * As a rule of thumb for <b>mode</b>. The resulting document...
 * <ul>
 *  <li>...will be updated heavily where updates may change the type-width of fields, will be target of many inserts
 *  containers, use <code>BISON_KEEP</code>. The document will have a notable portion of reserved memory contained;
 *  insertions or updates will, however, not require immediately reallocation or memory movements.</li>
 *  <li>...will <i>not</i> be target of insertion of strings or blob fields in the near future, use
 *      <code>BISON_SHRINK</code>. The document will not have padding reserved memory at the end, which means that
 *      a realloction will be required once the document grows (e.g., a container must be englarged). Typically,
 *      document growth is handled with container capacities (see <code>BISON_COMPACT</code>). However, insertions
 *      of variable-length data (i.e., strings and blobs) may require container enlargement. In this case, having
 *      padding reserved memory at the end of the document lowers the risk of a reallocation.</li>
 *  <li>...will <i>not</i> not be target of insertion operations or update operations that changes a fields type-width
 *      in the near future. In simpler words, if a document is updated and each such update keeps the (byte) size
 *      of the field, use <code>BISON_COMPACT</code>. This mode will remove all capacities in containers.</li>
 *  <li>...is read-mostly, or updates will not change the type or type-width of fields, use <code>BISON_OPTIMIZE</code>.
 *      The document will have the smallest memory footprint possible.</li>
 * </ul>
 */
NG5_EXPORT(struct bison_insert *) bison_create_begin(struct bison_new *context, struct bison *doc, int mode);

NG5_EXPORT(bool) bison_create_end(struct bison_new *context);

NG5_EXPORT(bool) bison_create_empty(struct bison *doc);

NG5_EXPORT(bool) bison_create_empty_ex(struct bison *doc, u64 doc_cap_byte, u64 array_cap_byte);

NG5_EXPORT(bool) bison_drop(struct bison *doc);

NG5_EXPORT(bool) bison_is_up_to_date(struct bison *doc);

NG5_EXPORT(bool) bison_register_listener(listener_handle_t *handle, struct bison_event_listener *listener, struct bison *doc);

NG5_EXPORT(bool) bison_unregister_listener(struct bison *doc, listener_handle_t handle);

NG5_EXPORT(bool) bison_clone(struct bison *clone, struct bison *doc);

NG5_EXPORT(bool) bison_revision(u64 *rev, struct bison *doc);

NG5_EXPORT(bool) bison_object_id(object_id_t *oid, struct bison *doc);

NG5_EXPORT(bool) bison_to_str(struct string_builder *dst, enum bison_printer_impl printer, struct bison *doc);

NG5_EXPORT(const char *) bison_to_json(struct string_builder *dst, struct bison *doc);

NG5_EXPORT(bool) bison_find_open(struct bison_find *out, const char *dot_path, struct bison *doc);

NG5_EXPORT(bool) bison_find_close(struct bison_find *find);


NG5_EXPORT(u64) bison_get_or_default_unsigned(struct bison *doc, const char *path, u64 default_val);

NG5_EXPORT(i64) bison_get_or_default_signed(struct bison *doc, const char *path, i64 default_val);

NG5_EXPORT(float) bison_get_or_default_float(struct bison *doc, const char *path, float default_val);

NG5_EXPORT(bool) bison_get_or_default_boolean(struct bison *doc, const char *path, bool default_val);

NG5_EXPORT(const char *) bison_get_or_default_string(u64 *len_out, struct bison *doc, const char *path, const char *default_val);

NG5_EXPORT(struct bison_binary *) bison_get_or_default_binary(struct bison *doc, const char *path, struct bison_binary *default_val);

NG5_EXPORT(struct bison_array_it *) bison_get_array_or_null(struct bison *doc, const char *path);

NG5_EXPORT(struct bison_column_it *) bison_get_column_or_null(struct bison *doc, const char *path);

NG5_EXPORT(bool) bison_path_exists(struct bison *doc, const char *path);

NG5_EXPORT(bool) bison_path_is_array(struct bison *doc, const char *path);

NG5_EXPORT(bool) bison_path_is_column(struct bison *doc, const char *path);

NG5_EXPORT(bool) bison_path_is_object(struct bison *doc, const char *path);

NG5_EXPORT(bool) bison_path_is_container(struct bison *doc, const char *path);

NG5_EXPORT(bool) bison_path_is_null(struct bison *doc, const char *path);

NG5_EXPORT(bool) bison_path_is_number(struct bison *doc, const char *path);

NG5_EXPORT(bool) bison_path_is_boolean(struct bison *doc, const char *path);

NG5_EXPORT(bool) bison_path_is_string(struct bison *doc, const char *path);

/**
 * Acquires a new revision context for the bison document.
 *
 * In case of an already running revision, the function returns <code>false</code> without blocking.
 * Otherwise, <code>bison_revise_begin</code> is called internally.
 *
 * @param context non-null pointer to revision context
 * @param doc document that should be revised
 * @return <code>false</code> in case of an already running revision. Otherwise returns value of
 *                            <code>bison_revise_begin</code>
 */
NG5_EXPORT(bool) bison_revise_try_begin(struct bison_revise *context, struct bison *revised_doc, struct bison *doc);

NG5_EXPORT(bool) bison_revise_begin(struct bison_revise *context, struct bison *revised_doc, struct bison *original);

NG5_EXPORT(bool) bison_revise_gen_object_id(object_id_t *out, struct bison_revise *context);

NG5_EXPORT(bool) bison_revise_iterator_open(struct bison_array_it *it, struct bison_revise *context);

NG5_EXPORT(bool) bison_revise_iterator_close(struct bison_array_it *it);

NG5_EXPORT(bool) bison_revise_find_open(struct bison_find *out, const char *dot_path, struct bison_revise *context);

NG5_EXPORT(bool) bison_revise_find_close(struct bison_find *find);










NG5_EXPORT(bool) bison_revise_pack(struct bison_revise *context);

NG5_EXPORT(bool) bison_revise_shrink(struct bison_revise *context);

NG5_EXPORT(const struct bison *) bison_revise_end(struct bison_revise *context);

NG5_EXPORT(bool) bison_revise_abort(struct bison_revise *context);

NG5_EXPORT(bool) bison_iterator_open(struct bison_array_it *it, struct bison *doc);

NG5_EXPORT(bool) bison_iterator_close(struct bison_array_it *it);

NG5_EXPORT(const char *) bison_field_type_str(struct err *err, enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_traversable(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_signed_integer(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_unsigned_integer(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_floating_number(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_number(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_integer(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_binary(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_boolean(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_array(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_column(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_object(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_null(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_string(enum bison_field_type type);

NG5_EXPORT(bool) bison_print(FILE *file, struct bison *doc);

NG5_EXPORT(bool) bison_hexdump_print(FILE *file, struct bison *doc);

NG5_END_DECL

#endif
