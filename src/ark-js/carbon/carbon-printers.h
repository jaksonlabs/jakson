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

#include <ark-js/shared/common.h>
#include <ark-js/shared/types.h>
#include <ark-js/shared/stdx/string.h>
#include <ark-js/carbon/oid/oid.h>

ARK_BEGIN_DECL

struct carbon_binary; /* forwarded from carbon.h */
struct carbon_object_it; /* forwarded from carbon-object-it.h */
struct carbon_array_it; /* forwarded from carbon-array-it.h */
struct carbon_column_it; /* forwarded from carbon-column-it.h */

struct printer {

    void *extra;

    void (*drop)(struct printer *self);
    void (*record_begin)(struct printer *self, struct string *builder);
    void (*record_end)(struct printer *self, struct string *builder);
    void (*meta_begin)(struct printer *self, struct string *builder);

    /* type is of enum carbon_key_type */
    void (*meta_data)(struct printer *self, struct string *builder,
                      int key_type, const void *key, u64 key_length,
                      u64 rev);
    void (*meta_end)(struct printer *self, struct string *builder);
    void (*doc_begin)(struct printer *self, struct string *builder);
    void (*doc_end)(struct printer *self, struct string *builder);
    void (*empty_record)(struct printer *self, struct string *builder);
    void (*unit_array_begin)(struct printer *self, struct string *builder);
    void (*unit_array_end)(struct printer *self, struct string *builder);
    void (*array_begin)(struct printer *self, struct string *builder);
    void (*array_end)(struct printer *self, struct string *builder);
    void (*const_null)(struct printer *self, struct string *builder);
    void (*const_true)(struct printer *self, bool is_null, struct string *builder);
    void (*const_false)(struct printer *self, bool is_null, struct string *builder);
    /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
    void (*val_signed)(struct printer *self, struct string *builder, const i64 *value);
    /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
    void (*val_unsigned)(struct printer *self, struct string *builder, const u64 *value);
    /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
    void (*val_float)(struct printer *self, struct string *builder, const float *value);
    void (*val_string)(struct printer *self, struct string *builder, const char *value, u64 strlen);
    void (*val_binary)(struct printer *self, struct string *builder, const struct carbon_binary *binary);
    void (*comma)(struct printer *self, struct string *builder);
    void (*obj_begin)(struct printer *self, struct string *builder);
    void (*obj_end)(struct printer *self, struct string *builder);
    void (*prop_null)(struct printer *self, struct string *builder,
                      const char *key_name, u64 key_len);
    void (*prop_true)(struct printer *self, struct string *builder,
                      const char *key_name, u64 key_len);
    void (*prop_false)(struct printer *self, struct string *builder,
                       const char *key_name, u64 key_len);
    void (*prop_signed)(struct printer *self, struct string *builder,
                        const char *key_name, u64 key_len, const i64 *value);
    void (*prop_unsigned)(struct printer *self, struct string *builder,
                          const char *key_name, u64 key_len, const u64 *value);
    void (*prop_float)(struct printer *self, struct string *builder,
                       const char *key_name, u64 key_len, const float *value);
    void (*prop_string)(struct printer *self, struct string *builder,
                        const char *key_name, u64 key_len, const char *value, u64 strlen);
    void (*prop_binary)(struct printer *self, struct string *builder,
                        const char *key_name, u64 key_len, const struct carbon_binary *binary);
    void (*array_prop_name)(struct printer *self, struct string *builder,
                            const char *key_name, u64 key_len);
    void (*column_prop_name)(struct printer *self, struct string *builder,
                             const char *key_name, u64 key_len);
    void (*obj_prop_name)(struct printer *self, struct string *builder,
                          const char *key_name, u64 key_len);
};

/* 'impl' is of enum carbon_printer_impl */
bool carbon_printer_by_type(struct printer *printer, int impl);
bool carbon_printer_drop(struct printer *printer);
bool carbon_printer_begin(struct printer *printer, struct string *str);
bool carbon_printer_end(struct printer *printer, struct string *str);
bool carbon_printer_header_begin(struct printer *printer, struct string *str);

/* 'key_type' is of enum carbon_key_type */
bool carbon_printer_header_contents(struct printer *printer, struct string *str,
                                    int key_type, const void *key, u64 key_length,
                                    u64 rev);
bool carbon_printer_header_end(struct printer *printer, struct string *str);
bool carbon_printer_payload_begin(struct printer *printer, struct string *str);
bool carbon_printer_payload_end(struct printer *printer, struct string *str);
bool carbon_printer_empty_record(struct printer *printer, struct string *str);
bool carbon_printer_array_begin(struct printer *printer, struct string *str);
bool carbon_printer_array_end(struct printer *printer, struct string *str);
bool carbon_printer_unit_array_begin(struct printer *printer, struct string *str);
bool carbon_printer_unit_array_end(struct printer *printer, struct string *str);
bool carbon_printer_object_begin(struct printer *printer, struct string *str);
bool carbon_printer_object_end(struct printer *printer, struct string *str);
bool carbon_printer_null(struct printer *printer, struct string *str);
bool carbon_printer_true(struct printer *printer, bool is_null, struct string *str);
bool carbon_printer_false(struct printer *printer, bool is_null, struct string *str);
bool carbon_printer_comma(struct printer *printer, struct string *str);
bool carbon_printer_signed_nonull(struct printer *printer, struct string *str, const i64 *value);
bool carbon_printer_unsigned_nonull(struct printer *printer, struct string *str, const u64 *value);
bool carbon_printer_u8_or_null(struct printer *printer, struct string *str, u8 value);
bool carbon_printer_u16_or_null(struct printer *printer, struct string *str, u16 value);
bool carbon_printer_u32_or_null(struct printer *printer, struct string *str, u32 value);
bool carbon_printer_u64_or_null(struct printer *printer, struct string *str, u64 value);
bool carbon_printer_i8_or_null(struct printer *printer, struct string *str, i8 value);
bool carbon_printer_i16_or_null(struct printer *printer, struct string *str, i16 value);
bool carbon_printer_i32_or_null(struct printer *printer, struct string *str, i32 value);
bool carbon_printer_i64_or_null(struct printer *printer, struct string *str, i64 value);
bool carbon_printer_float(struct printer *printer, struct string *str, const float *value);
bool carbon_printer_string(struct printer *printer, struct string *str, const char *value, u64 strlen);
bool carbon_printer_binary(struct printer *printer, struct string *str, const struct carbon_binary *binary);
bool carbon_printer_prop_null(struct printer *printer, struct string *str,
                              const char *key_name, u64 key_len);
bool carbon_printer_prop_true(struct printer *printer, struct string *str,
                              const char *key_name, u64 key_len);
bool carbon_printer_prop_false(struct printer *printer, struct string *str,
                               const char *key_name, u64 key_len);
bool carbon_printer_prop_signed(struct printer *printer, struct string *str,
                                const char *key_name, u64 key_len, const i64 *value);
bool carbon_printer_prop_unsigned(struct printer *printer, struct string *str,
                                  const char *key_name, u64 key_len, const u64 *value);
bool carbon_printer_prop_float(struct printer *printer, struct string *str,
                               const char *key_name, u64 key_len, const float *value);
bool carbon_printer_prop_string(struct printer *printer, struct string *str,
                                const char *key_name, u64 key_len, const char *value, u64 strlen);
bool carbon_printer_prop_binary(struct printer *printer, struct string *str,
                                const char *key_name, u64 key_len, const struct carbon_binary *binary);
bool carbon_printer_array_prop_name(struct printer *printer, struct string *str,
                                    const char *key_name, u64 key_len);
bool carbon_printer_column_prop_name(struct printer *printer, struct string *str,
                                     const char *key_name, u64 key_len);
bool carbon_printer_object_prop_name(struct printer *printer, struct string *str,
                                     const char *key_name, u64 key_len);
bool carbon_printer_print_object(struct carbon_object_it *it, struct printer *printer, struct string *builder);
bool carbon_printer_print_array(struct carbon_array_it *it, struct printer *printer, struct string *builder,
                        bool is_record_container);
bool carbon_printer_print_column(struct carbon_column_it *it, struct printer *printer, struct string *builder);

ARK_END_DECL

#endif
