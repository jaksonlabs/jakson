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

#include <jak_carbon_get.h>
#include <jak_carbon_find.h>

#define get_or_default(doc, path, type_name, default_val, test_fn, get_fn)                                             \
({                                                                                                                     \
        jak_carbon_find find;                                                                                        \
        jak_carbon_field_type_e field_type;                                                                              \
        type_name result = default_val;                                                                                \
                                                                                                                       \
        if (jak_carbon_find_open(&find, path, doc)) {                                                                       \
                jak_carbon_find_result_type(&field_type, &find);                                                            \
                if (test_fn(field_type)) {                                                                             \
                        get_fn(&result, &find);                                                                        \
                }                                                                                                      \
        }                                                                                                              \
                                                                                                                       \
        jak_carbon_find_close(&find);                                                                                       \
        default_val;                                                                                                        \
})


jak_u64 jak_carbon_get_or_default_unsigned(jak_carbon *doc, const char *path, jak_u64 default_val)
{
        return get_or_default(doc, path, jak_u64, default_val, jak_carbon_field_type_is_unsigned,
                              jak_carbon_find_result_unsigned);
}

jak_i64 jak_carbon_get_or_default_signed(jak_carbon *doc, const char *path, jak_i64 default_val)
{
        return get_or_default(doc, path, jak_i64, default_val, jak_carbon_field_type_is_signed,
                              jak_carbon_find_result_signed);
}

float jak_carbon_get_or_default_float(jak_carbon *doc, const char *path, float default_val)
{
        return get_or_default(doc, path, float, default_val, jak_carbon_field_type_is_floating,
                              jak_carbon_find_result_float);
}

bool jak_carbon_get_or_default_boolean(jak_carbon *doc, const char *path, bool default_val)
{
        return get_or_default(doc, path, bool, default_val, jak_carbon_field_type_is_boolean, jak_carbon_find_result_boolean);
}

const char *
jak_carbon_get_or_default_string(jak_u64 *len_out, jak_carbon *doc, const char *path, const char *default_val)
{
        jak_carbon_find find;
        jak_carbon_field_type_e field_type;
        const char *result = default_val;
        *len_out = result ? strlen(default_val) : 0;

        if (jak_carbon_find_open(&find, path, doc)) {
                jak_carbon_find_result_type(&field_type, &find);
                if (jak_carbon_field_type_is_string(field_type)) {
                        result = jak_carbon_find_result_string(len_out, &find);
                        jak_carbon_find_close(&find);
                        return result;
                }
        }

        jak_carbon_find_close(&find);
        return default_val;
}

jak_carbon_binary *
jak_carbon_get_or_default_binary(jak_carbon *doc, const char *path, jak_carbon_binary *default_val)
{
        jak_carbon_find find;
        jak_carbon_field_type_e field_type;
        jak_carbon_binary *result = NULL;

        if (jak_carbon_find_open(&find, path, doc)) {
                jak_carbon_find_result_type(&field_type, &find);
                if (jak_carbon_field_type_is_binary(field_type)) {
                        result = jak_carbon_find_result_binary(&find);
                        jak_carbon_find_close(&find);
                        return result;
                }
        }

        jak_carbon_find_close(&find);
        return default_val;
}

jak_carbon_array_it *carbon_get_array_or_null(jak_carbon *doc, const char *path)
{
        jak_carbon_find find;
        jak_carbon_field_type_e field_type;
        jak_carbon_array_it *result = NULL;

        if (jak_carbon_find_open(&find, path, doc)) {
                jak_carbon_find_result_type(&field_type, &find);
                if (jak_carbon_field_type_is_array(field_type)) {
                        result = jak_carbon_find_result_array(&find);
                }
        }

        jak_carbon_find_close(&find);
        return result;
}

jak_carbon_column_it *carbon_get_column_or_null(jak_carbon *doc, const char *path)
{
        jak_carbon_find find;
        jak_carbon_field_type_e field_type;
        jak_carbon_column_it *result = NULL;

        if (jak_carbon_find_open(&find, path, doc)) {
                jak_carbon_find_result_type(&field_type, &find);
                if (jak_carbon_field_type_is_column(field_type)) {
                        result = jak_carbon_find_result_column(&find);
                }
        }

        jak_carbon_find_close(&find);
        return result;
}
