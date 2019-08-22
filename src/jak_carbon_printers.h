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

#ifndef JAK_CARBON_PRINTERS_H
#define JAK_CARBON_PRINTERS_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_types.h>
#include <jak_string.h>
#include <jak_global_id.h>

JAK_BEGIN_DECL

struct jak_carbon_binary; /* forwarded from carbon.h */

struct printer {

        void *extra;

        void (*drop)(struct printer *self);

        void (*record_begin)(struct printer *self, struct jak_string *builder);

        void (*record_end)(struct printer *self, struct jak_string *builder);

        void (*meta_begin)(struct printer *self, struct jak_string *builder);

        /* type is of jak_carbon_key_e */
        void (*meta_data)(struct printer *self, struct jak_string *builder,
                          int key_type, const void *key, jak_u64 key_length,
                          jak_u64 commit_hash);

        void (*meta_end)(struct printer *self, struct jak_string *builder);

        void (*doc_begin)(struct printer *self, struct jak_string *builder);

        void (*doc_end)(struct printer *self, struct jak_string *builder);

        void (*empty_record)(struct printer *self, struct jak_string *builder);

        void (*unit_array_begin)(struct printer *self, struct jak_string *builder);

        void (*unit_array_end)(struct printer *self, struct jak_string *builder);

        void (*array_begin)(struct printer *self, struct jak_string *builder);

        void (*array_end)(struct printer *self, struct jak_string *builder);

        void (*const_null)(struct printer *self, struct jak_string *builder);

        void (*const_true)(struct printer *self, bool is_null, struct jak_string *builder);

        void (*const_false)(struct printer *self, bool is_null, struct jak_string *builder);

        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*val_signed)(struct printer *self, struct jak_string *builder, const jak_i64 *value);

        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*val_unsigned)(struct printer *self, struct jak_string *builder, const jak_u64 *value);

        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*val_float)(struct printer *self, struct jak_string *builder, const float *value);

        void (*val_string)(struct printer *self, struct jak_string *builder, const char *value, jak_u64 strlen);

        void (*val_binary)(struct printer *self, struct jak_string *builder, const struct jak_carbon_binary *binary);

        void (*comma)(struct printer *self, struct jak_string *builder);

        void (*obj_begin)(struct printer *self, struct jak_string *builder);

        void (*obj_end)(struct printer *self, struct jak_string *builder);

        void (*prop_null)(struct printer *self, struct jak_string *builder,
                          const char *key_name, jak_u64 key_len);

        void (*prop_true)(struct printer *self, struct jak_string *builder,
                          const char *key_name, jak_u64 key_len);

        void (*prop_false)(struct printer *self, struct jak_string *builder,
                           const char *key_name, jak_u64 key_len);

        void (*prop_signed)(struct printer *self, struct jak_string *builder,
                            const char *key_name, jak_u64 key_len, const jak_i64 *value);

        void (*prop_unsigned)(struct printer *self, struct jak_string *builder,
                              const char *key_name, jak_u64 key_len, const jak_u64 *value);

        void (*prop_float)(struct printer *self, struct jak_string *builder,
                           const char *key_name, jak_u64 key_len, const float *value);

        void (*prop_string)(struct printer *self, struct jak_string *builder,
                            const char *key_name, jak_u64 key_len, const char *value, jak_u64 strlen);

        void (*prop_binary)(struct printer *self, struct jak_string *builder,
                            const char *key_name, jak_u64 key_len, const struct jak_carbon_binary *binary);

        void (*array_prop_name)(struct printer *self, struct jak_string *builder,
                                const char *key_name, jak_u64 key_len);

        void (*column_prop_name)(struct printer *self, struct jak_string *builder,
                                 const char *key_name, jak_u64 key_len);

        void (*obj_prop_name)(struct printer *self, struct jak_string *builder,
                              const char *key_name, jak_u64 key_len);
};

/* 'impl' is of jak_carbon_printer_impl_e */
bool carbon_printer_by_type(struct printer *printer, int impl);

bool carbon_printer_drop(struct printer *printer);

bool carbon_printer_begin(struct printer *printer, struct jak_string *str);

bool carbon_printer_end(struct printer *printer, struct jak_string *str);

bool carbon_printer_header_begin(struct printer *printer, struct jak_string *str);

/* 'key_type' is of jak_carbon_key_e */
bool carbon_printer_header_contents(struct printer *printer, struct jak_string *str,
                                    int key_type, const void *key, jak_u64 key_length,
                                    jak_u64 rev);

bool carbon_printer_header_end(struct printer *printer, struct jak_string *str);

bool carbon_printer_payload_begin(struct printer *printer, struct jak_string *str);

bool carbon_printer_payload_end(struct printer *printer, struct jak_string *str);

bool carbon_printer_empty_record(struct printer *printer, struct jak_string *str);

bool carbon_printer_array_begin(struct printer *printer, struct jak_string *str);

bool carbon_printer_array_end(struct printer *printer, struct jak_string *str);

bool carbon_printer_unit_array_begin(struct printer *printer, struct jak_string *str);

bool carbon_printer_unit_array_end(struct printer *printer, struct jak_string *str);

bool carbon_printer_object_begin(struct printer *printer, struct jak_string *str);

bool carbon_printer_object_end(struct printer *printer, struct jak_string *str);

bool carbon_printer_null(struct printer *printer, struct jak_string *str);

bool carbon_printer_true(struct printer *printer, bool is_null, struct jak_string *str);

bool carbon_printer_false(struct printer *printer, bool is_null, struct jak_string *str);

bool carbon_printer_comma(struct printer *printer, struct jak_string *str);

bool carbon_printer_signed_nonull(struct printer *printer, struct jak_string *str, const jak_i64 *value);

bool carbon_printer_unsigned_nonull(struct printer *printer, struct jak_string *str, const jak_u64 *value);

bool carbon_printer_u8_or_null(struct printer *printer, struct jak_string *str, jak_u8 value);

bool carbon_printer_u16_or_null(struct printer *printer, struct jak_string *str, jak_u16 value);

bool carbon_printer_u32_or_null(struct printer *printer, struct jak_string *str, jak_u32 value);

bool carbon_printer_u64_or_null(struct printer *printer, struct jak_string *str, jak_u64 value);

bool carbon_printer_i8_or_null(struct printer *printer, struct jak_string *str, jak_i8 value);

bool carbon_printer_i16_or_null(struct printer *printer, struct jak_string *str, jak_i16 value);

bool carbon_printer_i32_or_null(struct printer *printer, struct jak_string *str, jak_i32 value);

bool carbon_printer_i64_or_null(struct printer *printer, struct jak_string *str, jak_i64 value);

bool carbon_printer_float(struct printer *printer, struct jak_string *str, const float *value);

bool carbon_printer_string(struct printer *printer, struct jak_string *str, const char *value, jak_u64 strlen);

bool carbon_printer_binary(struct printer *printer, struct jak_string *str, const struct jak_carbon_binary *binary);

bool carbon_printer_prop_null(struct printer *printer, struct jak_string *str,
                              const char *key_name, jak_u64 key_len);

bool carbon_printer_prop_true(struct printer *printer, struct jak_string *str,
                              const char *key_name, jak_u64 key_len);

bool carbon_printer_prop_false(struct printer *printer, struct jak_string *str,
                               const char *key_name, jak_u64 key_len);

bool carbon_printer_prop_signed(struct printer *printer, struct jak_string *str,
                                const char *key_name, jak_u64 key_len, const jak_i64 *value);

bool carbon_printer_prop_unsigned(struct printer *printer, struct jak_string *str,
                                  const char *key_name, jak_u64 key_len, const jak_u64 *value);

bool carbon_printer_prop_float(struct printer *printer, struct jak_string *str,
                               const char *key_name, jak_u64 key_len, const float *value);

bool carbon_printer_prop_string(struct printer *printer, struct jak_string *str,
                                const char *key_name, jak_u64 key_len, const char *value, jak_u64 strlen);

bool carbon_printer_prop_binary(struct printer *printer, struct jak_string *str,
                                const char *key_name, jak_u64 key_len, const struct jak_carbon_binary *binary);

bool carbon_printer_array_prop_name(struct printer *printer, struct jak_string *str,
                                    const char *key_name, jak_u64 key_len);

bool carbon_printer_column_prop_name(struct printer *printer, struct jak_string *str,
                                     const char *key_name, jak_u64 key_len);

bool carbon_printer_object_prop_name(struct printer *printer, struct jak_string *str,
                                     const char *key_name, jak_u64 key_len);

bool carbon_printer_print_object(jak_carbon_object_it *it, struct printer *printer, struct jak_string *builder);

bool carbon_printer_print_array(jak_carbon_array_it *it, struct printer *printer, struct jak_string *builder,
                                bool is_record_container);

bool carbon_printer_print_column(jak_carbon_column_it *it, struct printer *printer, struct jak_string *builder);

JAK_END_DECL

#endif
