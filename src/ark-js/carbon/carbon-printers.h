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

#ifndef carbon_PRINTERS_H
#define carbon_PRINTERS_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/types.h>
#include <ark-js/shared/stdx/string_builder.h>
#include <ark-js/carbon/oid/oid.h>
#include <ark-js/carbon/carbon.h>

NG5_BEGIN_DECL

struct carbon_binary; /* forwarded from carbon.h */

struct carbon_printer
{
        void *extra;

        void (*drop)(struct carbon_printer *self);

        void (*print_carbon_begin)(struct carbon_printer *self, struct string_builder *builder);
        void (*print_carbon_end)(struct carbon_printer *self, struct string_builder *builder);

        void (*print_carbon_header_begin)(struct carbon_printer *self, struct string_builder *builder);
        void (*print_carbon_header_contents)(struct carbon_printer *self, struct string_builder *builder,
                enum carbon_primary_key_type key_type, const void *key, u64 key_length, u64 rev);
        void (*print_carbon_header_end)(struct carbon_printer *self, struct string_builder *builder);

        void (*print_carbon_payload_begin)(struct carbon_printer *self, struct string_builder *builder);
        void (*print_carbon_payload_end)(struct carbon_printer *self, struct string_builder *builder);

        void (*print_carbon_array_begin)(struct carbon_printer *self, struct string_builder *builder);
        void (*print_carbon_array_end)(struct carbon_printer *self, struct string_builder *builder);

        void (*print_carbon_null)(struct carbon_printer *self, struct string_builder *builder);
        void (*print_carbon_true)(struct carbon_printer *self, bool is_null, struct string_builder *builder);
        void (*print_carbon_false)(struct carbon_printer *self, bool is_null, struct string_builder *builder);

        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*print_carbon_signed)(struct carbon_printer *self, struct string_builder *builder, const i64 *value);
        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*print_carbon_unsigned)(struct carbon_printer *self, struct string_builder *builder, const u64 *value);
        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*print_carbon_float)(struct carbon_printer *self, struct string_builder *builder, const float *value);

        void (*print_carbon_string)(struct carbon_printer *self, struct string_builder *builder, const char *value, u64 strlen);
        void (*print_carbon_binary)(struct carbon_printer *self, struct string_builder *builder, const struct carbon_binary *binary);

        void (*print_carbon_comma)(struct carbon_printer *self, struct string_builder *builder);

        void (*print_carbon_object_begin)(struct carbon_printer *self, struct string_builder *builder);
        void (*print_carbon_object_end)(struct carbon_printer *self, struct string_builder *builder);

        void (*print_carbon_prop_null)(struct carbon_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len);
        void (*print_carbon_prop_true)(struct carbon_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len);
        void (*print_carbon_prop_false)(struct carbon_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len);
        void (*print_carbon_prop_signed)(struct carbon_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len, const i64 *value);
        void (*print_carbon_prop_unsigned)(struct carbon_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len, const u64 *value);
        void (*print_carbon_prop_float)(struct carbon_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len, const float *value);
        void (*print_carbon_prop_string)(struct carbon_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len, const char *value, u64 strlen);
        void (*print_carbon_prop_binary)(struct carbon_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len, const struct carbon_binary *binary);
        void (*print_carbon_array_prop_name)(struct carbon_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len);
        void (*print_carbon_column_prop_name)(struct carbon_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len);
        void (*print_carbon_object_prop_name)(struct carbon_printer *self, struct string_builder *builder,
                const char *key_name, u64 key_len);
};

NG5_EXPORT(bool) carbon_json_formatter_create(struct carbon_printer *printer);
NG5_EXPORT(bool) carbon_json_formatter_set_intent(struct carbon_printer *printer, bool enable);
NG5_EXPORT(bool) carbon_json_formatter_set_strict(struct carbon_printer *printer, bool enable);

NG5_END_DECL

#endif
