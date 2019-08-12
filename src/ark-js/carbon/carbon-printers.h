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
#include <ark-js/carbon/carbon.h>

ARK_BEGIN_DECL

struct carbon_binary; /* forwarded from carbon.h */

struct printer {

    void *extra;

    void (*drop)(struct printer *self);
    void (*record_begin)(struct printer *self, struct string *builder);
    void (*record_end)(struct printer *self, struct string *builder);
    void (*meta_begin)(struct printer *self, struct string *builder);
    void (*meta_data)(struct printer *self, struct string *builder,
                      enum carbon_key_type key_type, const void *key, u64 key_length,
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

ARK_END_DECL

#endif
