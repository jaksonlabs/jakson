/**
 * Copyright 2018 Marcus Pinnecke
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

#include <assert.h>
#include "core/carbon/archive_int.h"

void
carbon_int_read_prop_offsets(carbon_archive_prop_offs_t *prop_offsets,
                             struct memfile *memfile,
                             const carbon_archive_object_flags_t *flags)
{
    NG5_ZERO_MEMORY(prop_offsets, sizeof(carbon_archive_prop_offs_t));
    if (flags->bits.has_null_props) {
        prop_offsets->nulls = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_bool_props) {
        prop_offsets->bools = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_int8_props) {
        prop_offsets->int8s = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_int16_props) {
        prop_offsets->int16s = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_int32_props) {
        prop_offsets->int32s = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_int64_props) {
        prop_offsets->int64s = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_uint8_props) {
        prop_offsets->uint8s = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_uint16_props) {
        prop_offsets->uint16s = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_uint32_props) {
        prop_offsets->uint32s = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_uint64_props) {
        prop_offsets->uint64s = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_float_props) {
        prop_offsets->floats = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_string_props) {
        prop_offsets->strings = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_object_props) {
        prop_offsets->objects = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_null_array_props) {
        prop_offsets->null_arrays = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_bool_array_props) {
        prop_offsets->bool_arrays = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_int8_array_props) {
        prop_offsets->int8_arrays = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_int16_array_props) {
        prop_offsets->int16_arrays = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_int32_array_props) {
        prop_offsets->int32_arrays = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_int64_array_props) {
        prop_offsets->int64_arrays = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_uint8_array_props) {
        prop_offsets->uint8_arrays = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_uint16_array_props) {
        prop_offsets->uint16_arrays = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_uint32_array_props) {
        prop_offsets->uint32_arrays = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_uint64_array_props) {
        prop_offsets->uint64_arrays = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_float_array_props) {
        prop_offsets->float_arrays = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_string_array_props) {
        prop_offsets->string_arrays = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
    if (flags->bits.has_object_array_props) {
        prop_offsets->object_arrays = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    }
}

void
carbon_int_embedded_fixed_props_read(carbon_fixed_prop_t *prop, struct memfile *memfile) {
    prop->header = NG5_MEMFILE_READ_TYPE(memfile, carbon_prop_header_t);
    prop->keys = (field_sid_t *) NG5_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(field_sid_t));
    prop->values = carbon_memfile_peek(memfile, 1);
}

void
carbon_int_embedded_var_props_read(carbon_var_prop_t *prop, struct memfile *memfile) {
    prop->header = NG5_MEMFILE_READ_TYPE(memfile, carbon_prop_header_t);
    prop->keys = (field_sid_t *) NG5_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(field_sid_t));
    prop->offsets = (offset_t *) NG5_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(offset_t));
    prop->values = carbon_memfile_peek(memfile, 1);
}

void
carbon_int_embedded_null_props_read(carbon_null_prop_t *prop, struct memfile *memfile) {
    prop->header = NG5_MEMFILE_READ_TYPE(memfile, carbon_prop_header_t);
    prop->keys = (field_sid_t *) NG5_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(field_sid_t));
}

void
carbon_int_embedded_array_props_read(carbon_array_prop_t *prop, struct memfile *memfile) {
    prop->header = NG5_MEMFILE_READ_TYPE(memfile, carbon_prop_header_t);
    prop->keys = (field_sid_t *) NG5_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(field_sid_t));
    prop->lengths = (u32 *) NG5_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(u32));
    prop->values_begin = memfile_tell(memfile);
}

void
carbon_int_embedded_table_props_read(carbon_table_prop_t *prop, struct memfile *memfile) {
    prop->header->marker = *NG5_MEMFILE_READ_TYPE(memfile, char);
    prop->header->num_entries = *NG5_MEMFILE_READ_TYPE(memfile, u8);
    prop->keys = (field_sid_t *) NG5_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(field_sid_t));
    prop->groupOffs = (offset_t *) NG5_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(offset_t));
}

field_e
carbon_int_get_value_type_of_char(char c)
{
    size_t len = sizeof(value_array_marker_mapping)/ sizeof(value_array_marker_mapping[0]);
    for (size_t i = 0; i < len; i++) {
        if (marker_symbols[value_array_marker_mapping[i].marker].symbol == c) {
            return value_array_marker_mapping[i].value_type;
        }
    }
    return field_null;
}

field_e // TODO: check whether 'field_e' can be replaced by 'enum field_type'
carbon_int_marker_to_field_type(char symbol)
{
    switch (symbol) {
    case MARKER_SYMBOL_PROP_NULL:
    case MARKER_SYMBOL_PROP_NULL_ARRAY:
        return field_null;
    case MARKER_SYMBOL_PROP_BOOLEAN:
    case MARKER_SYMBOL_PROP_BOOLEAN_ARRAY:
        return field_bool;
    case MARKER_SYMBOL_PROP_INT8:
    case MARKER_SYMBOL_PROP_INT8_ARRAY:
        return field_int8;
    case MARKER_SYMBOL_PROP_INT16:
    case MARKER_SYMBOL_PROP_INT16_ARRAY:
        return field_int16;
    case MARKER_SYMBOL_PROP_INT32:
    case MARKER_SYMBOL_PROP_INT32_ARRAY:
        return field_int32;
    case MARKER_SYMBOL_PROP_INT64:
    case MARKER_SYMBOL_PROP_INT64_ARRAY:
        return field_int64;
    case MARKER_SYMBOL_PROP_UINT8:
    case MARKER_SYMBOL_PROP_UINT8_ARRAY:
        return field_uint8;
    case MARKER_SYMBOL_PROP_UINT16:
    case MARKER_SYMBOL_PROP_UINT16_ARRAY:
        return field_uint16;
    case MARKER_SYMBOL_PROP_UINT32:
    case MARKER_SYMBOL_PROP_UINT32_ARRAY:
        return field_uint32;
    case MARKER_SYMBOL_PROP_UINT64:
    case MARKER_SYMBOL_PROP_UINT64_ARRAY:
        return field_uint64;
    case MARKER_SYMBOL_PROP_REAL:
    case MARKER_SYMBOL_PROP_REAL_ARRAY:
        return field_float;
    case MARKER_SYMBOL_PROP_TEXT:
    case MARKER_SYMBOL_PROP_TEXT_ARRAY:
        return field_string;
    case MARKER_SYMBOL_PROP_OBJECT:
    case MARKER_SYMBOL_PROP_OBJECT_ARRAY:
        return field_object;
    default: {
        carbon_print_error_and_die(NG5_ERR_MARKERMAPPING);
    }
    }
}

enum field_type
carbon_int_field_type_to_basic_type(field_e type)
{
    switch (type) {
    case field_null:
        return NG5_BASIC_TYPE_NULL;
    case field_bool:
        return NG5_BASIC_TYPE_BOOLEAN;
    case field_int8:
        return NG5_BASIC_TYPE_INT8;
    case field_int16:
        return NG5_BASIC_TYPE_INT16;
    case field_int32:
        return NG5_BASIC_TYPE_INT32;
    case field_int64:
        return NG5_BASIC_TYPE_INT64;
    case field_uint8:
        return NG5_BASIC_TYPE_UINT8;
    case field_uint16:
        return NG5_BASIC_TYPE_UINT16;
    case field_uint32:
        return NG5_BASIC_TYPE_UINT32;
    case field_uint64:
        return NG5_BASIC_TYPE_UINT64;
    case field_float:
        return NG5_BASIC_TYPE_NUMBER;
    case field_string:
        return NG5_BASIC_TYPE_STRING;
    case field_object:
        return NG5_BASIC_TYPE_OBJECT;
    default:
        carbon_print_error_and_die(NG5_ERR_INTERNALERR);
    }
}