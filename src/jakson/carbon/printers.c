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

#include <jakson/carbon/printers.h>
#include <jakson/carbon/printers/compact.h>
#include <jakson/carbon/printers/extended.h>
#include <jakson/carbon/object_it.h>
#include <jakson/carbon/array_it.h>
#include <jakson/carbon/column_it.h>

bool carbon_printer_by_type(carbon_printer *printer, int impl)
{
        ERROR_IF_NULL(printer)

        switch (impl) {
                case JSON_EXTENDED:
                        json_extended_printer_create(printer);
                        break;
                case JSON_COMPACT:
                        json_compact_printer_create(printer);
                        break;
                default: ERROR_PRINT(ERR_NOTFOUND)
                        return false;
        }
        return true;
}

bool carbon_printer_drop(carbon_printer *printer)
{
        ERROR_IF_NULL(printer->drop);
        printer->drop(printer);
        return true;
}

bool carbon_printer_begin(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->record_begin);
        printer->record_begin(printer, str);
        return true;
}

bool carbon_printer_end(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->record_end);
        printer->record_end(printer, str);
        return true;
}

bool carbon_printer_header_begin(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->meta_begin);
        printer->meta_begin(printer, str);
        return true;
}

bool carbon_printer_header_contents(carbon_printer *printer, string_buffer *str,
                                    int key_type, const void *key, u64 key_length,
                                    u64 rev)
{
        ERROR_IF_NULL(printer->drop);
        printer->meta_data(printer, str, key_type, key, key_length, rev);
        return true;
}

bool carbon_printer_header_end(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->meta_end);
        printer->meta_end(printer, str);
        return true;
}

bool carbon_printer_payload_begin(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->doc_begin);
        printer->doc_begin(printer, str);
        return true;
}

bool carbon_printer_payload_end(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->doc_end);
        printer->doc_end(printer, str);
        return true;
}

bool carbon_printer_empty_record(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->empty_record);
        printer->empty_record(printer, str);
        return true;
}

bool carbon_printer_array_begin(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->array_begin);
        printer->array_begin(printer, str);
        return true;
}

bool carbon_printer_array_end(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->array_end);
        printer->array_end(printer, str);
        return true;
}

bool carbon_printer_unit_array_begin(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->unit_array_begin);
        printer->unit_array_begin(printer, str);
        return true;
}

bool carbon_printer_unit_array_end(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->unit_array_end);
        printer->unit_array_end(printer, str);
        return true;
}

bool carbon_printer_object_begin(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->obj_begin);
        printer->obj_begin(printer, str);
        return true;
}

bool carbon_printer_object_end(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->obj_end);
        printer->obj_end(printer, str);
        return true;
}

bool carbon_printer_null(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->const_null);
        printer->const_null(printer, str);
        return true;
}

bool carbon_printer_true(carbon_printer *printer, bool is_null, string_buffer *str)
{
        ERROR_IF_NULL(printer->const_true);
        printer->const_true(printer, is_null, str);
        return true;
}

bool carbon_printer_false(carbon_printer *printer, bool is_null, string_buffer *str)
{
        ERROR_IF_NULL(printer->const_false);
        printer->const_false(printer, is_null, str);
        return true;
}

bool carbon_printer_comma(carbon_printer *printer, string_buffer *str)
{
        ERROR_IF_NULL(printer->comma);
        printer->comma(printer, str);
        return true;
}

bool carbon_printer_signed_nonull(carbon_printer *printer, string_buffer *str, const i64 *value)
{
        ERROR_IF_NULL(printer->val_signed);
        printer->val_signed(printer, str, value);
        return true;
}

bool carbon_printer_unsigned_nonull(carbon_printer *printer, string_buffer *str, const u64 *value)
{
        ERROR_IF_NULL(printer->val_unsigned);
        printer->val_unsigned(printer, str, value);
        return true;
}

#define delegate_print_call(printer, str, value, null_test_func, print_func, ctype)                                    \
({                                                                                                                     \
        ERROR_IF_NULL(printer)                                                                                         \
        ERROR_IF_NULL(str)                                                                                             \
        bool status = false;                                                                                           \
        if (null_test_func(value)) {                                                                                       \
                status = carbon_printer_null(printer, str);                                                            \
        } else {                                                                                                       \
                ctype val = value;                                                                                     \
                status = print_func(printer, str, &val);                                                               \
        }                                                                                                              \
        status;                                                                                                        \
})

bool carbon_printer_u8_or_null(carbon_printer *printer, string_buffer *str, u8 value)
{
        return delegate_print_call(printer, str, value, IS_NULL_U8, carbon_printer_unsigned_nonull, u64);
}

bool carbon_printer_u16_or_null(carbon_printer *printer, string_buffer *str, u16 value)
{
        return delegate_print_call(printer, str, value, IS_NULL_U16, carbon_printer_unsigned_nonull, u64);
}

bool carbon_printer_u32_or_null(carbon_printer *printer, string_buffer *str, u32 value)
{
        return delegate_print_call(printer, str, value, IS_NULL_U32, carbon_printer_unsigned_nonull, u64);
}

bool carbon_printer_u64_or_null(carbon_printer *printer, string_buffer *str, u64 value)
{
        return delegate_print_call(printer, str, value, IS_NULL_U64, carbon_printer_unsigned_nonull, u64);
}

bool carbon_printer_i8_or_null(carbon_printer *printer, string_buffer *str, i8 value)
{
        return delegate_print_call(printer, str, value, IS_NULL_I8, carbon_printer_signed_nonull, i64);
}

bool carbon_printer_i16_or_null(carbon_printer *printer, string_buffer *str, i16 value)
{
        return delegate_print_call(printer, str, value, IS_NULL_I16, carbon_printer_signed_nonull, i64);
}

bool carbon_printer_i32_or_null(carbon_printer *printer, string_buffer *str, i32 value)
{
        return delegate_print_call(printer, str, value, IS_NULL_I32, carbon_printer_signed_nonull, i64);
}

bool carbon_printer_i64_or_null(carbon_printer *printer, string_buffer *str, i64 value)
{
        return delegate_print_call(printer, str, value, IS_NULL_I64, carbon_printer_signed_nonull, i64);
}

bool carbon_printer_float(carbon_printer *printer, string_buffer *str, const float *value)
{
        ERROR_IF_NULL(printer->val_float);
        printer->val_float(printer, str, value);
        return true;
}

bool carbon_printer_string(carbon_printer *printer, string_buffer *str, const char *value, u64 strlen)
{
        ERROR_IF_NULL(printer->val_string);
        printer->val_string(printer, str, value, strlen);
        return true;
}

bool carbon_printer_binary(carbon_printer *printer, string_buffer *str, const carbon_binary *binary)
{
        ERROR_IF_NULL(printer->val_binary);
        printer->val_binary(printer, str, binary);
        return true;
}

bool carbon_printer_prop_null(carbon_printer *printer, string_buffer *str,
                              const char *key_name, u64 key_len)
{
        ERROR_IF_NULL(printer->prop_null);
        printer->prop_null(printer, str, key_name, key_len);
        return true;
}

bool carbon_printer_prop_true(carbon_printer *printer, string_buffer *str,
                              const char *key_name, u64 key_len)
{
        ERROR_IF_NULL(printer->prop_true);
        printer->prop_true(printer, str, key_name, key_len);
        return true;
}

bool carbon_printer_prop_false(carbon_printer *printer, string_buffer *str,
                               const char *key_name, u64 key_len)
{
        ERROR_IF_NULL(printer->prop_false);
        printer->prop_false(printer, str, key_name, key_len);
        return true;
}

bool carbon_printer_prop_signed(carbon_printer *printer, string_buffer *str,
                                const char *key_name, u64 key_len, const i64 *value)
{
        ERROR_IF_NULL(printer->prop_signed);
        printer->prop_signed(printer, str, key_name, key_len, value);
        return true;
}

bool carbon_printer_prop_unsigned(carbon_printer *printer, string_buffer *str,
                                  const char *key_name, u64 key_len, const u64 *value)
{
        ERROR_IF_NULL(printer->prop_unsigned);
        printer->prop_unsigned(printer, str, key_name, key_len, value);
        return true;
}

bool carbon_printer_prop_float(carbon_printer *printer, string_buffer *str,
                               const char *key_name, u64 key_len, const float *value)
{
        ERROR_IF_NULL(printer->prop_float);
        printer->prop_float(printer, str, key_name, key_len, value);
        return true;
}

bool carbon_printer_prop_string(carbon_printer *printer, string_buffer *str,
                                const char *key_name, u64 key_len, const char *value, u64 strlen)
{
        ERROR_IF_NULL(printer->prop_string);
        printer->prop_string(printer, str, key_name, key_len, value, strlen);
        return true;
}

bool carbon_printer_prop_binary(carbon_printer *printer, string_buffer *str,
                                const char *key_name, u64 key_len, const carbon_binary *binary)
{
        ERROR_IF_NULL(printer->prop_binary);
        printer->prop_binary(printer, str, key_name, key_len, binary);
        return true;
}

bool carbon_printer_array_prop_name(carbon_printer *printer, string_buffer *str,
                                    const char *key_name, u64 key_len)
{
        ERROR_IF_NULL(printer->array_prop_name);
        printer->array_prop_name(printer, str, key_name, key_len);
        return true;
}

bool carbon_printer_column_prop_name(carbon_printer *printer, string_buffer *str,
                                     const char *key_name, u64 key_len)
{
        ERROR_IF_NULL(printer->column_prop_name);
        printer->column_prop_name(printer, str, key_name, key_len);
        return true;
}

bool carbon_printer_object_prop_name(carbon_printer *printer, string_buffer *str,
                                     const char *key_name, u64 key_len)
{
        ERROR_IF_NULL(printer->obj_prop_name);
        printer->obj_prop_name(printer, str, key_name, key_len);
        return true;
}

bool carbon_printer_print_object(carbon_object_it *it, carbon_printer *printer, string_buffer *builder)
{
        JAK_ASSERT(it);
        JAK_ASSERT(printer);
        JAK_ASSERT(builder);
        bool is_null_value = false;
        bool first_entry = true;
        carbon_printer_object_begin(printer, builder);

        while (carbon_object_it_next(it)) {
                if (LIKELY(!first_entry)) {
                        carbon_printer_comma(printer, builder);
                }
                DECLARE_AND_INIT(carbon_field_type_e, type)
                DECLARE_AND_INIT(u64, key_len)
                const char *key_name = carbon_object_it_prop_name(&key_len, it);

                carbon_object_it_prop_type(&type, it);
                switch (type) {
                        case CARBON_FIELD_NULL:
                                carbon_printer_prop_null(printer, builder, key_name, key_len);
                                break;
                        case CARBON_FIELD_TRUE:
                                /** in an array, there is no TRUE constant that is set to NULL because it will be replaced with
                                 * a constant NULL. In columns, there might be a NULL-encoded value */
                                carbon_printer_prop_true(printer, builder, key_name, key_len);
                                break;
                        case CARBON_FIELD_FALSE:
                                /** in an array, there is no FALSE constant that is set to NULL because it will be replaced with
                                 * a constant NULL. In columns, there might be a NULL-encoded value */
                                carbon_printer_prop_false(printer, builder, key_name, key_len);
                                break;
                        case CARBON_FIELD_NUMBER_U8:
                        case CARBON_FIELD_NUMBER_U16:
                        case CARBON_FIELD_NUMBER_U32:
                        case CARBON_FIELD_NUMBER_U64: {
                                u64 value;
                                carbon_object_it_unsigned_value(&is_null_value, &value, it);
                                carbon_printer_prop_unsigned(printer, builder, key_name, key_len,
                                                             is_null_value ? NULL : &value);
                        }
                                break;
                        case CARBON_FIELD_NUMBER_I8:
                        case CARBON_FIELD_NUMBER_I16:
                        case CARBON_FIELD_NUMBER_I32:
                        case CARBON_FIELD_NUMBER_I64: {
                                i64 value;
                                carbon_object_it_signed_value(&is_null_value, &value, it);
                                carbon_printer_prop_signed(printer, builder, key_name, key_len,
                                                           is_null_value ? NULL : &value);
                        }
                                break;
                        case CARBON_FIELD_NUMBER_FLOAT: {
                                float value;
                                carbon_object_it_float_value(&is_null_value, &value, it);
                                carbon_printer_prop_float(printer, builder, key_name, key_len,
                                                          is_null_value ? NULL : &value);
                        }
                                break;
                        case CARBON_FIELD_STRING: {
                                u64 strlen;
                                const char *value = carbon_object_it_string_value(&strlen, it);
                                carbon_printer_prop_string(printer, builder, key_name, key_len, value, strlen);
                        }
                                break;
                        case CARBON_FIELD_BINARY:
                        case CARBON_FIELD_BINARY_CUSTOM: {
                                carbon_binary binary;
                                carbon_object_it_binary_value(&binary, it);
                                carbon_printer_prop_binary(printer, builder, key_name, key_len, &binary);
                        }
                                break;
                        case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET: {
                                carbon_array_it *array = carbon_object_it_array_value(it);
                                carbon_printer_array_prop_name(printer, builder, key_name, key_len);
                                carbon_printer_print_array(array, printer, builder, false);
                                carbon_array_it_drop(array);
                        }
                                break;
                        case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                        case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                        case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                        case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                        case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                        case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET: {
                                carbon_column_it *column = carbon_object_it_column_value(it);
                                carbon_printer_column_prop_name(printer, builder, key_name, key_len);
                                carbon_printer_print_column(column, printer, builder);
                        }
                                break;
                        case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                        case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                        case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                        case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP: {
                                carbon_object_it *object = carbon_object_it_object_value(it);
                                carbon_printer_object_prop_name(printer, builder, key_name, key_len);
                                carbon_printer_print_object(object, printer, builder);
                                carbon_object_it_drop(object);
                        }
                                break;
                        default:
                                carbon_printer_object_end(printer, builder);
                                ERROR(&it->err, ERR_CORRUPTED);
                                return false;
                }
                first_entry = false;
        }

        carbon_printer_object_end(printer, builder);
        return true;
}

bool carbon_printer_print_array(carbon_array_it *it, carbon_printer *printer, string_buffer *builder,
                                bool is_record_container)
{
        JAK_ASSERT(it);
        JAK_ASSERT(printer);
        JAK_ASSERT(builder);

        bool first_entry = true;
        bool has_entries = false;
        bool is_single_entry_array = carbon_array_it_is_unit(it);

        while (carbon_array_it_next(it)) {
                bool is_null_value;

                if (LIKELY(!first_entry)) {
                        carbon_printer_comma(printer, builder);
                } else {
                        if (is_single_entry_array && is_record_container) {
                                carbon_printer_unit_array_begin(printer, builder);
                        } else {
                                carbon_printer_array_begin(printer, builder);
                        }
                        has_entries = true;
                }
                carbon_field_type_e type;
                carbon_array_it_field_type(&type, it);
                switch (type) {
                        case CARBON_FIELD_NULL:
                                carbon_printer_null(printer, builder);
                                break;
                        case CARBON_FIELD_TRUE:
                                /** in an array, there is no TRUE constant that is set to NULL because it will be replaced with
                                 * a constant NULL. In columns, there might be a NULL-encoded value */
                                carbon_printer_true(printer, false, builder);
                                break;
                        case CARBON_FIELD_FALSE:
                                /** in an array, there is no FALSE constant that is set to NULL because it will be replaced with
                                 * a constant NULL. In columns, there might be a NULL-encoded value */
                                carbon_printer_false(printer, false, builder);
                                break;
                        case CARBON_FIELD_NUMBER_U8:
                        case CARBON_FIELD_NUMBER_U16:
                        case CARBON_FIELD_NUMBER_U32:
                        case CARBON_FIELD_NUMBER_U64: {
                                u64 value;
                                carbon_array_it_unsigned_value(&is_null_value, &value, it);
                                carbon_printer_unsigned_nonull(printer, builder, is_null_value ? NULL : &value);
                        }
                                break;
                        case CARBON_FIELD_NUMBER_I8:
                        case CARBON_FIELD_NUMBER_I16:
                        case CARBON_FIELD_NUMBER_I32:
                        case CARBON_FIELD_NUMBER_I64: {
                                i64 value;
                                carbon_array_it_signed_value(&is_null_value, &value, it);
                                carbon_printer_signed_nonull(printer, builder, is_null_value ? NULL : &value);
                        }
                                break;
                        case CARBON_FIELD_NUMBER_FLOAT: {
                                float value;
                                carbon_array_it_float_value(&is_null_value, &value, it);
                                carbon_printer_float(printer, builder, is_null_value ? NULL : &value);
                        }
                                break;
                        case CARBON_FIELD_STRING: {
                                u64 strlen;
                                const char *value = carbon_array_it_string_value(&strlen, it);
                                carbon_printer_string(printer, builder, value, strlen);
                        }
                                break;
                        case CARBON_FIELD_BINARY:
                        case CARBON_FIELD_BINARY_CUSTOM: {
                                carbon_binary binary;
                                carbon_array_it_binary_value(&binary, it);
                                carbon_printer_binary(printer, builder, &binary);
                        }
                                break;
                        case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET: {
                                carbon_array_it *array = carbon_array_it_array_value(it);
                                carbon_printer_print_array(array, printer, builder, false);
                                carbon_array_it_drop(array);
                        }
                                break;
                        case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                        case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                        case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                        case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                        case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                        case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET: {
                                carbon_column_it *column = carbon_array_it_column_value(it);
                                carbon_printer_print_column(column, printer, builder);
                        }
                                break;
                        case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                        case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                        case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                        case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP: {
                                carbon_object_it *object = carbon_array_it_object_value(it);
                                carbon_printer_print_object(object, printer, builder);
                                carbon_object_it_drop(object);
                        }
                                break;
                        default:
                                carbon_printer_array_end(printer, builder);
                                ERROR(&it->err, ERR_CORRUPTED);
                                return false;
                }
                first_entry = false;
        }

        if (has_entries) {
                if (is_single_entry_array && is_record_container) {
                        carbon_printer_unit_array_end(printer, builder);
                } else {
                        carbon_printer_array_end(printer, builder);
                }
        } else {
                if (is_record_container) {
                        carbon_printer_empty_record(printer, builder);
                } else {
                        carbon_printer_array_begin(printer, builder);
                        carbon_printer_array_end(printer, builder);
                }
        }

        return true;
}

bool carbon_printer_print_column(carbon_column_it *it, carbon_printer *printer, string_buffer *builder)
{
        ERROR_IF_NULL(it)
        ERROR_IF_NULL(printer)
        ERROR_IF_NULL(builder)

        carbon_field_type_e type;
        u32 nvalues;
        const void *values = carbon_column_it_values(&type, &nvalues, it);

        carbon_printer_array_begin(printer, builder);
        for (u32 i = 0; i < nvalues; i++) {
                switch (type) {
                        case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET: {
                                u8 value = ((u8 *) values)[i];
                                if (IS_NULL_BOOLEAN(value)) {
                                        carbon_printer_null(printer, builder);
                                } else if (value == CARBON_BOOLEAN_COLUMN_TRUE) {
                                        carbon_printer_true(printer, false, builder);
                                } else {
                                        carbon_printer_false(printer, false, builder);
                                }
                        }
                                break;
                        case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET: {
                                u64 number = ((u8 *) values)[i];
                                carbon_printer_unsigned_nonull(printer, builder, IS_NULL_U8(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET: {
                                u64 number = ((u16 *) values)[i];
                                carbon_printer_unsigned_nonull(printer, builder, IS_NULL_U16(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET: {
                                u64 number = ((u32 *) values)[i];
                                carbon_printer_unsigned_nonull(printer, builder, IS_NULL_U32(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET: {
                                u64 number = ((u64 *) values)[i];
                                carbon_printer_unsigned_nonull(printer, builder, IS_NULL_U64(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET: {
                                i64 number = ((i8 *) values)[i];
                                carbon_printer_signed_nonull(printer, builder, IS_NULL_I8(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET: {
                                i64 number = ((i16 *) values)[i];
                                carbon_printer_signed_nonull(printer, builder, IS_NULL_I16(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET: {
                                i64 number = ((i32 *) values)[i];
                                carbon_printer_signed_nonull(printer, builder, IS_NULL_I32(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET: {
                                i64 number = ((i64 *) values)[i];
                                carbon_printer_signed_nonull(printer, builder, IS_NULL_I64(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET: {
                                float number = ((float *) values)[i];
                                carbon_printer_float(printer, builder, IS_NULL_FLOAT(number) ? NULL : &number);
                        }
                                break;
                        default:
                                carbon_printer_array_end(printer, builder);
                                ERROR(&it->err, ERR_CORRUPTED);
                                return false;
                }
                if (i + 1 < nvalues) {
                        carbon_printer_comma(printer, builder);
                }
        }
        carbon_printer_array_end(printer, builder);

        return true;
}