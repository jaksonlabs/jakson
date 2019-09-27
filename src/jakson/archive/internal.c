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

#include <jakson/archive/internal.h>

void int_read_prop_offsets(archive_prop_offs *prop_offsets, memfile *memfile,
                               const object_flags_u *flags)
{
        ZERO_MEMORY(prop_offsets, sizeof(archive_prop_offs));
        if (flags->bits.has_null_props) {
                prop_offsets->nulls = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_bool_props) {
                prop_offsets->bools = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int8_props) {
                prop_offsets->int8s = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int16_props) {
                prop_offsets->int16s = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int32_props) {
                prop_offsets->int32s = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int64_props) {
                prop_offsets->int64s = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint8_props) {
                prop_offsets->uint8s = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint16_props) {
                prop_offsets->uint16s = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint32_props) {
                prop_offsets->uint32s = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint64_props) {
                prop_offsets->uint64s = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_float_props) {
                prop_offsets->floats = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_string_props) {
                prop_offsets->strings = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_object_props) {
                prop_offsets->objects = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_null_array_props) {
                prop_offsets->null_arrays = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_bool_array_props) {
                prop_offsets->bool_arrays = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int8_array_props) {
                prop_offsets->int8_arrays = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int16_array_props) {
                prop_offsets->int16_arrays = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int32_array_props) {
                prop_offsets->int32_arrays = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int64_array_props) {
                prop_offsets->int64_arrays = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint8_array_props) {
                prop_offsets->uint8_arrays = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint16_array_props) {
                prop_offsets->uint16_arrays = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint32_array_props) {
                prop_offsets->uint32_arrays = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint64_array_props) {
                prop_offsets->uint64_arrays = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_float_array_props) {
                prop_offsets->float_arrays = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_string_array_props) {
                prop_offsets->string_arrays = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_object_array_props) {
                prop_offsets->object_arrays = *MEMFILE_READ_TYPE(memfile, offset_t);
        }
}

void int_embedded_fixed_props_read(fixed_prop *prop, memfile *memfile)
{
        prop->header = MEMFILE_READ_TYPE(memfile, prop_header);
        prop->keys = (archive_field_sid_t *) MEMFILE_READ(memfile, prop->header->num_entries *
                                                                           sizeof(archive_field_sid_t));
        prop->values = memfile_peek(memfile, 1);
}

void int_embedded_var_props_read(var_prop *prop, memfile *memfile)
{
        prop->header = MEMFILE_READ_TYPE(memfile, prop_header);
        prop->keys = (archive_field_sid_t *) MEMFILE_READ(memfile, prop->header->num_entries *
                                                                           sizeof(archive_field_sid_t));
        prop->offsets = (offset_t *) MEMFILE_READ(memfile, prop->header->num_entries * sizeof(offset_t));
        prop->values = memfile_peek(memfile, 1);
}

void int_embedded_null_props_read(null_prop *prop, memfile *memfile)
{
        prop->header = MEMFILE_READ_TYPE(memfile, prop_header);
        prop->keys = (archive_field_sid_t *) MEMFILE_READ(memfile, prop->header->num_entries *
                                                                           sizeof(archive_field_sid_t));
}

void int_embedded_array_props_read(array_prop *prop, memfile *memfile)
{
        prop->header = MEMFILE_READ_TYPE(memfile, prop_header);
        prop->keys = (archive_field_sid_t *) MEMFILE_READ(memfile, prop->header->num_entries *
                                                                           sizeof(archive_field_sid_t));
        prop->lengths = (u32 *) MEMFILE_READ(memfile, prop->header->num_entries * sizeof(u32));
        prop->values_begin = memfile_tell(memfile);
}

void int_embedded_table_props_read(table_prop *prop, memfile *memfile)
{
        prop->header->marker = *MEMFILE_READ_TYPE(memfile, char);
        prop->header->num_entries = *MEMFILE_READ_TYPE(memfile, u8);
        prop->keys = (archive_field_sid_t *) MEMFILE_READ(memfile, prop->header->num_entries *
                                                                           sizeof(archive_field_sid_t));
        prop->group_offs = (offset_t *) MEMFILE_READ(memfile, prop->header->num_entries * sizeof(offset_t));
}

archive_field_e int_get_value_type_of_char(char c)
{
        size_t len = sizeof(global_value_array_marker_mapping) / sizeof(global_value_array_marker_mapping[0]);
        for (size_t i = 0; i < len; i++) {
                if (global_marker_symbols[global_value_array_marker_mapping[i].marker].symbol == c) {
                        return global_value_array_marker_mapping[i].value_type;
                }
        }
        return FIELD_NULL;
}

archive_field_e // TODO: check whether 'archive_field_e' can be replaced by 'enum archive_field_type'
int_marker_to_field_type(char symbol)
{
        switch (symbol) {
                case MARKER_SYMBOL_PROP_NULL:
                case MARKER_SYMBOL_PROP_NULL_ARRAY:
                        return FIELD_NULL;
                case MARKER_SYMBOL_PROP_BOOLEAN:
                case MARKER_SYMBOL_PROP_BOOLEAN_ARRAY:
                        return FIELD_BOOLEAN;
                case MARKER_SYMBOL_PROP_INT8:
                case MARKER_SYMBOL_PROP_INT8_ARRAY:
                        return FIELD_INT8;
                case MARKER_SYMBOL_PROP_INT16:
                case MARKER_SYMBOL_PROP_INT16_ARRAY:
                        return FIELD_INT16;
                case MARKER_SYMBOL_PROP_INT32:
                case MARKER_SYMBOL_PROP_INT32_ARRAY:
                        return FIELD_INT32;
                case MARKER_SYMBOL_PROP_INT64:
                case MARKER_SYMBOL_PROP_INT64_ARRAY:
                        return FIELD_INT64;
                case MARKER_SYMBOL_PROP_UINT8:
                case MARKER_SYMBOL_PROP_UINT8_ARRAY:
                        return FIELD_UINT8;
                case MARKER_SYMBOL_PROP_UINT16:
                case MARKER_SYMBOL_PROP_UINT16_ARRAY:
                        return FIELD_UINT16;
                case MARKER_SYMBOL_PROP_UINT32:
                case MARKER_SYMBOL_PROP_UINT32_ARRAY:
                        return FIELD_UINT32;
                case MARKER_SYMBOL_PROP_UINT64:
                case MARKER_SYMBOL_PROP_UINT64_ARRAY:
                        return FIELD_UINT64;
                case MARKER_SYMBOL_PROP_REAL:
                case MARKER_SYMBOL_PROP_REAL_ARRAY:
                        return FIELD_FLOAT;
                case MARKER_SYMBOL_PROP_TEXT:
                case MARKER_SYMBOL_PROP_TEXT_ARRAY:
                        return FIELD_STRING;
                case MARKER_SYMBOL_PROP_OBJECT:
                case MARKER_SYMBOL_PROP_OBJECT_ARRAY:
                        return FIELD_OBJECT;
                default: {
                        ERROR_PRINT_AND_DIE(ERR_MARKERMAPPING);
                }
        }
}