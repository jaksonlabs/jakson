/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#include <ark-js/carbon/carbon-get.h>
#include <ark-js/carbon/carbon-find.h>

#define get_or_default(doc, path, type_name, default_val, test_fn, get_fn)                                             \
({                                                                                                                     \
        struct carbon_find find;                                                                                        \
        enum carbon_field_type field_type;                                                                              \
        type_name result = default_val;                                                                                \
                                                                                                                       \
        if (carbon_find_open(&find, path, doc)) {                                                                       \
                carbon_find_result_type(&field_type, &find);                                                            \
                if (test_fn(field_type)) {                                                                             \
                        get_fn(&result, &find);                                                                        \
                }                                                                                                      \
        }                                                                                                              \
                                                                                                                       \
        carbon_find_close(&find);                                                                                       \
        result;                                                                                                        \
})


ARK_EXPORT(u64) carbon_get_or_default_unsigned(struct carbon *doc, const char *path, u64 default_val)
{
        return get_or_default(doc, path, u64, default_val, carbon_field_type_is_unsigned_integer, carbon_find_result_unsigned);
}

ARK_EXPORT(i64) carbon_get_or_default_signed(struct carbon *doc, const char *path, i64 default_val)
{
        return get_or_default(doc, path, i64, default_val, carbon_field_type_is_signed_integer, carbon_find_result_signed);
}

ARK_EXPORT(float) carbon_get_or_default_float(struct carbon *doc, const char *path, float default_val)
{
        return get_or_default(doc, path, float, default_val, carbon_field_type_is_floating_number, carbon_find_result_float);
}

ARK_EXPORT(bool) carbon_get_or_default_boolean(struct carbon *doc, const char *path, bool default_val)
{
        return get_or_default(doc, path, bool, default_val, carbon_field_type_is_boolean, carbon_find_result_boolean);
}

ARK_EXPORT(const char *) carbon_get_or_default_string(u64 *len_out, struct carbon *doc, const char *path, const char *default_val)
{
        struct carbon_find find;
        enum carbon_field_type field_type;
        const char *result = default_val;
        *len_out = result ? strlen(default_val) : 0;

        if (carbon_find_open(&find, path, doc)) {
                carbon_find_result_type(&field_type, &find);
                if (carbon_field_type_is_string(field_type)) {
                        result = carbon_find_result_string(len_out, &find);
                }
        }

        carbon_find_close(&find);
        return result;
}

ARK_EXPORT(struct carbon_binary *) carbon_get_or_default_binary(struct carbon *doc, const char *path, struct carbon_binary *default_val)
{
        struct carbon_find find;
        enum carbon_field_type field_type;
        struct carbon_binary *result = default_val;

        if (carbon_find_open(&find, path, doc)) {
                carbon_find_result_type(&field_type, &find);
                if (carbon_field_type_is_binary(field_type)) {
                        result = carbon_find_result_binary(&find);
                }
        }

        carbon_find_close(&find);
        return result;
}

ARK_EXPORT(struct carbon_array_it *) carbon_get_array_or_null(struct carbon *doc, const char *path)
{
        struct carbon_find find;
        enum carbon_field_type field_type;
        struct carbon_array_it *result = NULL;

        if (carbon_find_open(&find, path, doc)) {
                carbon_find_result_type(&field_type, &find);
                if (carbon_field_type_is_array(field_type)) {
                        result = carbon_find_result_array(&find);
                }
        }

        carbon_find_close(&find);
        return result;
}

ARK_EXPORT(struct carbon_column_it *) carbon_get_column_or_null(struct carbon *doc, const char *path)
{
        struct carbon_find find;
        enum carbon_field_type field_type;
        struct carbon_column_it *result = NULL;

        if (carbon_find_open(&find, path, doc)) {
                carbon_find_result_type(&field_type, &find);
                if (carbon_field_type_is_column(field_type)) {
                        result = carbon_find_result_column(&find);
                }
        }

        carbon_find_close(&find);
        return result;
}