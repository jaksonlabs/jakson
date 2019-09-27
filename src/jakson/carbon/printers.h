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

#ifndef CARBON_PRINTERS_H
#define CARBON_PRINTERS_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/types.h>
#include <jakson/std/string.h>
#include <jakson/stdx/unique_id.h>

BEGIN_DECL

typedef struct carbon_printer
{
        void *extra;

        void (*drop)(carbon_printer *self);
        void (*record_begin)(carbon_printer *self, string_buffer *builder);
        void (*record_end)(carbon_printer *self, string_buffer *builder);
        void (*meta_begin)(carbon_printer *self, string_buffer *builder);
        /** type is of carbon_key_e */
        void (*meta_data)(carbon_printer *self, string_buffer *builder, int key_type, const void *key, u64 key_length, u64 commit_hash);
        void (*meta_end)(carbon_printer *self, string_buffer *builder);
        void (*doc_begin)(carbon_printer *self, string_buffer *builder);
        void (*doc_end)(carbon_printer *self, string_buffer *builder);
        void (*empty_record)(carbon_printer *self, string_buffer *builder);
        void (*unit_array_begin)(carbon_printer *self, string_buffer *builder);
        void (*unit_array_end)(carbon_printer *self, string_buffer *builder);
        void (*array_begin)(carbon_printer *self, string_buffer *builder);
        void (*array_end)(carbon_printer *self, string_buffer *builder);
        void (*const_null)(carbon_printer *self, string_buffer *builder);
        void (*const_true)(carbon_printer *self, bool is_null, string_buffer *builder);
        void (*const_false)(carbon_printer *self, bool is_null, string_buffer *builder);
        /** if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*val_signed)(carbon_printer *self, string_buffer *builder, const i64 *value);
        /** if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*val_unsigned)(carbon_printer *self, string_buffer *builder, const u64 *value);
        /** if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*val_float)(carbon_printer *self, string_buffer *builder, const float *value);
        void (*val_string)(carbon_printer *self, string_buffer *builder, const char *value, u64 strlen);
        void (*val_binary)(carbon_printer *self, string_buffer *builder, const carbon_binary *binary);
        void (*comma)(carbon_printer *self, string_buffer *builder);
        void (*obj_begin)(carbon_printer *self, string_buffer *builder);
        void (*obj_end)(carbon_printer *self, string_buffer *builder);
        void (*prop_null)(carbon_printer *self, string_buffer *builder, const char *key_name, u64 key_len);
        void (*prop_true)(carbon_printer *self, string_buffer *builder, const char *key_name, u64 key_len);
        void (*prop_false)(carbon_printer *self, string_buffer *builder, const char *key_name, u64 key_len);
        void (*prop_signed)(carbon_printer *self, string_buffer *builder, const char *key_name, u64 key_len, const i64 *value);
        void (*prop_unsigned)(carbon_printer *self, string_buffer *builder, const char *key_name, u64 key_len, const u64 *value);
        void (*prop_float)(carbon_printer *self, string_buffer *builder, const char *key_name, u64 key_len, const float *value);
        void (*prop_string)(carbon_printer *self, string_buffer *builder, const char *key_name, u64 key_len, const char *value, u64 strlen);
        void (*prop_binary)(carbon_printer *self, string_buffer *builder, const char *key_name, u64 key_len, const carbon_binary *binary);
        void (*array_prop_name)(carbon_printer *self, string_buffer *builder, const char *key_name, u64 key_len);
        void (*column_prop_name)(carbon_printer *self, string_buffer *builder, const char *key_name, u64 key_len);
        void (*obj_prop_name)(carbon_printer *self, string_buffer *builder, const char *key_name, u64 key_len);
} carbon_printer;

/** 'impl' is of carbon_printer_impl_e */
bool carbon_printer_drop(carbon_printer *printer);
bool carbon_printer_by_type(carbon_printer *printer, int impl);

bool carbon_printer_begin(carbon_printer *printer, string_buffer *str);
bool carbon_printer_end(carbon_printer *printer, string_buffer *str);
bool carbon_printer_header_begin(carbon_printer *printer, string_buffer *str);
/** 'key_type' is of carbon_key_e */
bool carbon_printer_header_contents(carbon_printer *printer, string_buffer *str, int key_type, const void *key, u64 key_length, u64 rev);
bool carbon_printer_header_end(carbon_printer *printer, string_buffer *str);
bool carbon_printer_payload_begin(carbon_printer *printer, string_buffer *str);
bool carbon_printer_payload_end(carbon_printer *printer, string_buffer *str);
bool carbon_printer_empty_record(carbon_printer *printer, string_buffer *str);
bool carbon_printer_array_begin(carbon_printer *printer, string_buffer *str);
bool carbon_printer_array_end(carbon_printer *printer, string_buffer *str);
bool carbon_printer_unit_array_begin(carbon_printer *printer, string_buffer *str);
bool carbon_printer_unit_array_end(carbon_printer *printer, string_buffer *str);
bool carbon_printer_object_begin(carbon_printer *printer, string_buffer *str);
bool carbon_printer_object_end(carbon_printer *printer, string_buffer *str);
bool carbon_printer_null(carbon_printer *printer, string_buffer *str);
bool carbon_printer_true(carbon_printer *printer, bool is_null, string_buffer *str);
bool carbon_printer_false(carbon_printer *printer, bool is_null, string_buffer *str);
bool carbon_printer_comma(carbon_printer *printer, string_buffer *str);
bool carbon_printer_signed_nonull(carbon_printer *printer, string_buffer *str, const i64 *value);
bool carbon_printer_unsigned_nonull(carbon_printer *printer, string_buffer *str, const u64 *value);
bool carbon_printer_u8_or_null(carbon_printer *printer, string_buffer *str, u8 value);
bool carbon_printer_u16_or_null(carbon_printer *printer, string_buffer *str, u16 value);
bool carbon_printer_u32_or_null(carbon_printer *printer, string_buffer *str, u32 value);
bool carbon_printer_u64_or_null(carbon_printer *printer, string_buffer *str, u64 value);
bool carbon_printer_i8_or_null(carbon_printer *printer, string_buffer *str, i8 value);
bool carbon_printer_i16_or_null(carbon_printer *printer, string_buffer *str, i16 value);
bool carbon_printer_i32_or_null(carbon_printer *printer, string_buffer *str, i32 value);
bool carbon_printer_i64_or_null(carbon_printer *printer, string_buffer *str, i64 value);
bool carbon_printer_float(carbon_printer *printer, string_buffer *str, const float *value);
bool carbon_printer_string(carbon_printer *printer, string_buffer *str, const char *value, u64 strlen);
bool carbon_printer_binary(carbon_printer *printer, string_buffer *str, const carbon_binary *binary);
bool carbon_printer_prop_null(carbon_printer *printer, string_buffer *str, const char *key_name, u64 key_len);
bool carbon_printer_prop_true(carbon_printer *printer, string_buffer *str, const char *key_name, u64 key_len);
bool carbon_printer_prop_false(carbon_printer *printer, string_buffer *str, const char *key_name, u64 key_len);
bool carbon_printer_prop_signed(carbon_printer *printer, string_buffer *str, const char *key_name, u64 key_len, const i64 *value);
bool carbon_printer_prop_unsigned(carbon_printer *printer, string_buffer *str, const char *key_name, u64 key_len, const u64 *value);
bool carbon_printer_prop_float(carbon_printer *printer, string_buffer *str, const char *key_name, u64 key_len, const float *value);
bool carbon_printer_prop_string(carbon_printer *printer, string_buffer *str, const char *key_name, u64 key_len, const char *value, u64 strlen);
bool carbon_printer_prop_binary(carbon_printer *printer, string_buffer *str, const char *key_name, u64 key_len, const carbon_binary *binary);
bool carbon_printer_array_prop_name(carbon_printer *printer, string_buffer *str, const char *key_name, u64 key_len);
bool carbon_printer_column_prop_name(carbon_printer *printer, string_buffer *str, const char *key_name, u64 key_len);
bool carbon_printer_object_prop_name(carbon_printer *printer, string_buffer *str, const char *key_name, u64 key_len);
bool carbon_printer_print_object(carbon_object_it *it, carbon_printer *printer, string_buffer *builder);
bool carbon_printer_print_array(carbon_array_it *it, carbon_printer *printer, string_buffer *builder, bool is_record_container);
bool carbon_printer_print_column(carbon_column_it *it, carbon_printer *printer, string_buffer *builder);

END_DECL

#endif
