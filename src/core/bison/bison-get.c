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

#include <ark-js/carbon/bison/bison-get.h>
#include <ark-js/carbon/bison/bison-find.h>

#define get_or_default(doc, path, type_name, default_val, test_fn, get_fn)                                             \
({                                                                                                                     \
        struct bison_find find;                                                                                        \
        enum bison_field_type field_type;                                                                              \
        type_name result = default_val;                                                                                \
                                                                                                                       \
        if (bison_find_open(&find, path, doc)) {                                                                       \
                bison_find_result_type(&field_type, &find);                                                            \
                if (test_fn(field_type)) {                                                                             \
                        get_fn(&result, &find);                                                                        \
                }                                                                                                      \
        }                                                                                                              \
                                                                                                                       \
        bison_find_close(&find);                                                                                       \
        result;                                                                                                        \
})


NG5_EXPORT(u64) bison_get_or_default_unsigned(struct bison *doc, const char *path, u64 default_val)
{
        return get_or_default(doc, path, u64, default_val, bison_field_type_is_unsigned_integer, bison_find_result_unsigned);
}

NG5_EXPORT(i64) bison_get_or_default_signed(struct bison *doc, const char *path, i64 default_val)
{
        return get_or_default(doc, path, i64, default_val, bison_field_type_is_signed_integer, bison_find_result_signed);
}

NG5_EXPORT(float) bison_get_or_default_float(struct bison *doc, const char *path, float default_val)
{
        return get_or_default(doc, path, float, default_val, bison_field_type_is_floating_number, bison_find_result_float);
}

NG5_EXPORT(bool) bison_get_or_default_boolean(struct bison *doc, const char *path, bool default_val)
{
        return get_or_default(doc, path, bool, default_val, bison_field_type_is_boolean, bison_find_result_boolean);
}

NG5_EXPORT(const char *) bison_get_or_default_string(u64 *len_out, struct bison *doc, const char *path, const char *default_val)
{
        struct bison_find find;
        enum bison_field_type field_type;
        const char *result = default_val;
        *len_out = result ? strlen(default_val) : 0;

        if (bison_find_open(&find, path, doc)) {
                bison_find_result_type(&field_type, &find);
                if (bison_field_type_is_string(field_type)) {
                        result = bison_find_result_string(len_out, &find);
                }
        }

        bison_find_close(&find);
        return result;
}

NG5_EXPORT(struct bison_binary *) bison_get_or_default_binary(struct bison *doc, const char *path, struct bison_binary *default_val)
{
        struct bison_find find;
        enum bison_field_type field_type;
        struct bison_binary *result = default_val;

        if (bison_find_open(&find, path, doc)) {
                bison_find_result_type(&field_type, &find);
                if (bison_field_type_is_binary(field_type)) {
                        result = bison_find_result_binary(&find);
                }
        }

        bison_find_close(&find);
        return result;
}

NG5_EXPORT(struct bison_array_it *) bison_get_array_or_null(struct bison *doc, const char *path)
{
        struct bison_find find;
        enum bison_field_type field_type;
        struct bison_array_it *result = NULL;

        if (bison_find_open(&find, path, doc)) {
                bison_find_result_type(&field_type, &find);
                if (bison_field_type_is_array(field_type)) {
                        result = bison_find_result_array(&find);
                }
        }

        bison_find_close(&find);
        return result;
}

NG5_EXPORT(struct bison_column_it *) bison_get_column_or_null(struct bison *doc, const char *path)
{
        struct bison_find find;
        enum bison_field_type field_type;
        struct bison_column_it *result = NULL;

        if (bison_find_open(&find, path, doc)) {
                bison_find_result_type(&field_type, &find);
                if (bison_field_type_is_column(field_type)) {
                        result = bison_find_result_column(&find);
                }
        }

        bison_find_close(&find);
        return result;
}