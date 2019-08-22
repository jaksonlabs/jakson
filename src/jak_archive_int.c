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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_archive_int.h>

void jak_int_read_prop_offsets(jak_archive_prop_offs *prop_offsets, jak_memfile *memfile,
                               const jak_object_flags_u *flags)
{
        JAK_ZERO_MEMORY(prop_offsets, sizeof(jak_archive_prop_offs));
        if (flags->bits.has_null_props) {
                prop_offsets->nulls = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_bool_props) {
                prop_offsets->bools = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_int8_props) {
                prop_offsets->int8s = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_int16_props) {
                prop_offsets->int16s = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_int32_props) {
                prop_offsets->int32s = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_int64_props) {
                prop_offsets->int64s = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_uint8_props) {
                prop_offsets->uint8s = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_uint16_props) {
                prop_offsets->uint16s = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_uint32_props) {
                prop_offsets->uint32s = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_uint64_props) {
                prop_offsets->uint64s = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_float_props) {
                prop_offsets->floats = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_jak_string_props) {
                prop_offsets->strings = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_object_props) {
                prop_offsets->objects = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_null_array_props) {
                prop_offsets->null_arrays = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_bool_array_props) {
                prop_offsets->bool_arrays = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_int8_array_props) {
                prop_offsets->int8_arrays = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_int16_array_props) {
                prop_offsets->int16_arrays = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_int32_array_props) {
                prop_offsets->int32_arrays = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_int64_array_props) {
                prop_offsets->int64_arrays = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_uint8_array_props) {
                prop_offsets->uint8_arrays = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_uint16_array_props) {
                prop_offsets->uint16_arrays = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_uint32_array_props) {
                prop_offsets->uint32_arrays = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_uint64_array_props) {
                prop_offsets->uint64_arrays = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_float_array_props) {
                prop_offsets->float_arrays = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_jak_string_array_props) {
                prop_offsets->jak_string_arrays = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
        if (flags->bits.has_object_array_props) {
                prop_offsets->object_arrays = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        }
}

void jak_int_embedded_fixed_props_read(jak_fixed_prop *prop, jak_memfile *memfile)
{
        prop->header = JAK_MEMFILE_READ_TYPE(memfile, jak_prop_header);
        prop->keys = (jak_archive_field_sid_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries *
                                                                           sizeof(jak_archive_field_sid_t));
        prop->values = jak_memfile_peek(memfile, 1);
}

void jak_int_embedded_var_props_read(jak_var_prop *prop, jak_memfile *memfile)
{
        prop->header = JAK_MEMFILE_READ_TYPE(memfile, jak_prop_header);
        prop->keys = (jak_archive_field_sid_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries *
                                                                           sizeof(jak_archive_field_sid_t));
        prop->offsets = (jak_offset_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(jak_offset_t));
        prop->values = jak_memfile_peek(memfile, 1);
}

void jak_int_embedded_null_props_read(jak_null_prop *prop, jak_memfile *memfile)
{
        prop->header = JAK_MEMFILE_READ_TYPE(memfile, jak_prop_header);
        prop->keys = (jak_archive_field_sid_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries *
                                                                           sizeof(jak_archive_field_sid_t));
}

void jak_int_embedded_array_props_read(jak_array_prop *prop, jak_memfile *memfile)
{
        prop->header = JAK_MEMFILE_READ_TYPE(memfile, jak_prop_header);
        prop->keys = (jak_archive_field_sid_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries *
                                                                           sizeof(jak_archive_field_sid_t));
        prop->lengths = (jak_u32 *) JAK_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(jak_u32));
        prop->values_begin = jak_memfile_tell(memfile);
}

void jak_int_embedded_table_props_read(jak_table_prop *prop, jak_memfile *memfile)
{
        prop->header->marker = *JAK_MEMFILE_READ_TYPE(memfile, char);
        prop->header->num_entries = *JAK_MEMFILE_READ_TYPE(memfile, jak_u8);
        prop->keys = (jak_archive_field_sid_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries *
                                                                           sizeof(jak_archive_field_sid_t));
        prop->group_offs = (jak_offset_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(jak_offset_t));
}

jak_archive_field_e jak_int_get_value_type_of_char(char c)
{
        size_t len = sizeof(jak_global_value_array_marker_mapping) / sizeof(jak_global_value_array_marker_mapping[0]);
        for (size_t i = 0; i < len; i++) {
                if (jak_global_marker_symbols[jak_global_value_array_marker_mapping[i].marker].symbol == c) {
                        return jak_global_value_array_marker_mapping[i].value_type;
                }
        }
        return JAK_FIELD_NULL;
}

jak_archive_field_e // TODO: check whether 'jak_archive_field_e' can be replaced by 'enum jak_archive_field_type'
jak_int_marker_to_field_type(char symbol)
{
        switch (symbol) {
                case JAK_MARKER_SYMBOL_PROP_NULL:
                case JAK_MARKER_SYMBOL_PROP_NULL_ARRAY:
                        return JAK_FIELD_NULL;
                case JAK_MARKER_SYMBOL_PROP_BOOLEAN:
                case JAK_MARKER_SYMBOL_PROP_BOOLEAN_ARRAY:
                        return JAK_FIELD_BOOLEAN;
                case JAK_MARKER_SYMBOL_PROP_INT8:
                case JAK_MARKER_SYMBOL_PROP_INT8_ARRAY:
                        return JAK_FIELD_INT8;
                case JAK_MARKER_SYMBOL_PROP_INT16:
                case JAK_MARKER_SYMBOL_PROP_INT16_ARRAY:
                        return JAK_FIELD_INT16;
                case JAK_MARKER_SYMBOL_PROP_INT32:
                case JAK_MARKER_SYMBOL_PROP_INT32_ARRAY:
                        return JAK_FIELD_INT32;
                case JAK_MARKER_SYMBOL_PROP_INT64:
                case JAK_MARKER_SYMBOL_PROP_INT64_ARRAY:
                        return JAK_FIELD_INT64;
                case JAK_MARKER_SYMBOL_PROP_UINT8:
                case JAK_MARKER_SYMBOL_PROP_UINT8_ARRAY:
                        return JAK_FIELD_UINT8;
                case JAK_MARKER_SYMBOL_PROP_UINT16:
                case JAK_MARKER_SYMBOL_PROP_UINT16_ARRAY:
                        return JAK_FIELD_UINT16;
                case JAK_MARKER_SYMBOL_PROP_UINT32:
                case JAK_MARKER_SYMBOL_PROP_UINT32_ARRAY:
                        return JAK_FIELD_UINT32;
                case JAK_MARKER_SYMBOL_PROP_UINT64:
                case JAK_MARKER_SYMBOL_PROP_UINT64_ARRAY:
                        return JAK_FIELD_UINT64;
                case JAK_MARKER_SYMBOL_PROP_REAL:
                case JAK_MARKER_SYMBOL_PROP_REAL_ARRAY:
                        return JAK_FIELD_FLOAT;
                case JAK_MARKER_SYMBOL_PROP_TEXT:
                case JAK_MARKER_SYMBOL_PROP_TEXT_ARRAY:
                        return JAK_FIELD_STRING;
                case JAK_MARKER_SYMBOL_PROP_OBJECT:
                case JAK_MARKER_SYMBOL_PROP_OBJECT_ARRAY:
                        return JAK_FIELD_OBJECT;
                default: {
                        JAK_ERROR_PRINT_AND_DIE(JAK_ERR_MARKERMAPPING);
                }
        }
}