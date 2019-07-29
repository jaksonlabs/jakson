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

#ifndef BISON_PRINTERS_H
#define BISON_PRINTERS_H

#include "shared/common.h"
#include "shared/types.h"
#include "std/string_builder.h"
#include "core/oid/oid.h"
#include "bison.h"

NG5_BEGIN_DECL

struct bison_binary; /* forwarded from bison.h */

struct bison_printer
{
        void *extra;

        void (*drop)(struct bison_printer *self);

        void (*print_bison_begin)(struct bison_printer *self, struct string_builder *builder);
        void (*print_bison_end)(struct bison_printer *self, struct string_builder *builder);

        void (*print_bison_header_begin)(struct bison_printer *self, struct string_builder *builder);
        void (*print_bison_header_contents)(struct bison_printer *self, struct string_builder *builder,
                enum bison_primary_key_type key_type, const void *key, u64 key_length, u64 rev);
        void (*print_bison_header_end)(struct bison_printer *self, struct string_builder *builder);

        void (*print_bison_payload_begin)(struct bison_printer *self, struct string_builder *builder);
        void (*print_bison_payload_end)(struct bison_printer *self, struct string_builder *builder);

        void (*print_bison_array_begin)(struct bison_printer *self, struct string_builder *builder);
        void (*print_bison_array_end)(struct bison_printer *self, struct string_builder *builder);

        void (*print_bison_null)(struct bison_printer *self, struct string_builder *builder);
        void (*print_bison_true)(struct bison_printer *self, bool is_null, struct string_builder *builder);
        void (*print_bison_false)(struct bison_printer *self, bool is_null, struct string_builder *builder);

        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*print_bison_signed)(struct bison_printer *self, struct string_builder *builder, const i64 *value);
        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*print_bison_unsigned)(struct bison_printer *self, struct string_builder *builder, const u64 *value);
        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*print_bison_float)(struct bison_printer *self, struct string_builder *builder, const float *value);

        void (*print_bison_string)(struct bison_printer *self, struct string_builder *builder, const char *value, u64 strlen);
        void (*print_bison_binary)(struct bison_printer *self, struct string_builder *builder, const struct bison_binary *binary);

        void (*print_bison_comma)(struct bison_printer *self, struct string_builder *builder);

        void (*print_bison_object_begin)(struct bison_printer *self, struct string_builder *builder);
        void (*print_bison_object_end)(struct bison_printer *self, struct string_builder *builder);

        void (*print_bison_prop_null)(struct bison_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len);
        void (*print_bison_prop_true)(struct bison_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len);
        void (*print_bison_prop_false)(struct bison_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len);
        void (*print_bison_prop_signed)(struct bison_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len, const i64 *value);
        void (*print_bison_prop_unsigned)(struct bison_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len, const u64 *value);
        void (*print_bison_prop_float)(struct bison_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len, const float *value);
        void (*print_bison_prop_string)(struct bison_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len, const char *value, u64 strlen);
        void (*print_bison_prop_binary)(struct bison_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len, const struct bison_binary *binary);
        void (*print_bison_array_prop_name)(struct bison_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len);
        void (*print_bison_column_prop_name)(struct bison_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len);
        void (*print_bison_object_prop_name)(struct bison_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len);
};

NG5_EXPORT(bool) bison_json_formatter_create(struct bison_printer *printer);
NG5_EXPORT(bool) bison_json_formatter_set_intent(struct bison_printer *printer, bool enable);
NG5_EXPORT(bool) bison_json_formatter_set_strict(struct bison_printer *printer, bool enable);

NG5_END_DECL

#endif
