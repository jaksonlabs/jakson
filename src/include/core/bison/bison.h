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
struct bison_array_it; /* forwarded fron bison-array-it.h */

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
        BISON_FIELD_TYPE_ARRAY = 'a', /* variable-type array */
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

        /* fixed-type number arrays */
        BISON_FIELD_TYPE_NUMBER_U8_COLUMN = 15, /* 8bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_U16_COLUMN = 16, /* 16bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_U32_COLUMN = 17, /* 32bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_U64_COLUMN = 18, /* 64bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_I8_COLUMN = 19, /* 8bit signed integer */
        BISON_FIELD_TYPE_NUMBER_I16_COLUMN = 20, /* 16bit signed integer */
        BISON_FIELD_TYPE_NUMBER_I32_COLUMN = 21, /* 32bit signed integer */
        BISON_FIELD_TYPE_NUMBER_I64_COLUMN = 22, /* 64bit signed integer */
        BISON_FIELD_TYPE_NUMBER_FLOAT_COLUMN = 23, /* 32bit float */

        /* fixed-type string array */
        BISON_FIELD_TYPE_NCHAR_COLUMN = 24, /* maximum n characters per string */

        /* user-defined binary data */
        BISON_FIELD_TYPE_BINARY = 25, /* arbitrary binary object */

        /* user-defined binary data */
        BISON_FIELD_TYPE_NBINARY_COLUMN = 26 /* maximum n byte per datum */
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

        void (*print_bison_comma)(struct bison_printer *self, struct string_builder *builder);
};

enum bison_printer_impl
{
        JSON_FORMATTER
};

#define BISON_FIELD_TYPE_NULL_STR "null"
#define BISON_FIELD_TYPE_TRUE_STR "boolean (true)"
#define BISON_FIELD_TYPE_FALSE_STR "boolean (false)"
#define BISON_FIELD_TYPE_OBJECT_STR "object"
#define BISON_FIELD_TYPE_ARRAY_STR "array"
#define BISON_FIELD_TYPE_STRING_STR "string"
#define BISON_FIELD_TYPE_NUMBER_U8_STR "number (u8)"
#define BISON_FIELD_TYPE_NUMBER_U16_STR "number (u16)"
#define BISON_FIELD_TYPE_NUMBER_U32_STR "number (u32)"
#define BISON_FIELD_TYPE_NUMBER_U64_STR "number (u64)"
#define BISON_FIELD_TYPE_NUMBER_I8_STR "number (i8)"
#define BISON_FIELD_TYPE_NUMBER_I16_STR "number (i16)"
#define BISON_FIELD_TYPE_NUMBER_I32_STR "number (i32)"
#define BISON_FIELD_TYPE_NUMBER_I64_STR "number (i64)"
#define BISON_FIELD_TYPE_NUMBER_FLOAT_STR "number (float)"
#define BISON_FIELD_TYPE_NUMBER_U8_COLUMN_STR "number column (u8)"
#define BISON_FIELD_TYPE_NUMBER_U16_COLUMN_STR "number column (u16)"
#define BISON_FIELD_TYPE_NUMBER_U32_COLUMN_STR "number column (u32)"
#define BISON_FIELD_TYPE_NUMBER_U64_COLUMN_STR "number column (u64)"
#define BISON_FIELD_TYPE_NUMBER_I8_COLUMN_STR "number column (i8)"
#define BISON_FIELD_TYPE_NUMBER_I16_COLUMN_STR "number column (i16)"
#define BISON_FIELD_TYPE_NUMBER_I32_COLUMN_STR "number column (i32)"
#define BISON_FIELD_TYPE_NUMBER_I64_COLUMN_STR "number column (i64)"
#define BISON_FIELD_TYPE_NUMBER_FLOAT_COLUMN_STR "number column (float)"
#define BISON_FIELD_TYPE_NUMBER_NCHAR_COLUMN_STR "string column (nchar)"
#define BISON_FIELD_TYPE_NUMBER_BINARY_STR "binary"
#define BISON_FIELD_TYPE_NUMBER_NBINARY_COLUMN_STR "binary column (nbinary)"

#define BISON_MARKER_ARRAY_BEGIN '['
#define BISON_MARKER_ARRAY_END ']'

NG5_DEFINE_GET_ERROR_FUNCTION(bison, struct bison, doc);

NG5_EXPORT(bool) bison_create(struct bison *doc);

NG5_EXPORT(bool) bison_create_ex(struct bison *doc, u64 doc_cap_byte, u64 array_cap_byte);

NG5_EXPORT(bool) bison_drop(struct bison *doc);

NG5_EXPORT(bool) bison_is_up_to_date(struct bison *doc);

NG5_EXPORT(bool) bison_register_listener(listener_handle_t *handle, struct bison_event_listener *listener, struct bison *doc);

NG5_EXPORT(bool) bison_unregister_listener(struct bison *doc, listener_handle_t handle);

NG5_EXPORT(bool) bison_clone(struct bison *clone, struct bison *doc);

NG5_EXPORT(bool) bison_revision(u64 *rev, struct bison *doc);

NG5_EXPORT(bool) bison_object_id(object_id_t *oid, struct bison *doc);

NG5_EXPORT(bool) bison_to_str(struct string_builder *dst, enum bison_printer_impl printer, struct bison *doc);

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

NG5_EXPORT(bool) bison_revise_access(struct bison_array_it *it, struct bison_revise *context);

NG5_EXPORT(const struct bison *) bison_revise_end(struct bison_revise *context);

NG5_EXPORT(bool) bison_revise_abort(struct bison_revise *context);

NG5_EXPORT(bool) bison_access(struct bison_array_it *it, struct bison *doc);

NG5_EXPORT(const char *) bison_field_type_str(struct err *err, enum bison_field_type type);

NG5_EXPORT(bool) bison_print(FILE *file, struct bison *doc);

NG5_EXPORT(bool) bison_hexdump_print(FILE *file, struct bison *doc);

NG5_END_DECL

#endif
