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

#include <assert.h>
#include <jak_archive_int.h>

void int_read_prop_offsets(struct jak_archive_prop_offs *prop_offsets, struct memfile *memfile,
                           const union object_flags *flags)
{
        JAK_zero_memory(prop_offsets, sizeof(struct jak_archive_prop_offs));
        if (flags->bits.has_null_props) {
                prop_offsets->nulls = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_bool_props) {
                prop_offsets->bools = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int8_props) {
                prop_offsets->int8s = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int16_props) {
                prop_offsets->int16s = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int32_props) {
                prop_offsets->int32s = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int64_props) {
                prop_offsets->int64s = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint8_props) {
                prop_offsets->uint8s = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint16_props) {
                prop_offsets->uint16s = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint32_props) {
                prop_offsets->uint32s = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint64_props) {
                prop_offsets->uint64s = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_float_props) {
                prop_offsets->floats = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_string_props) {
                prop_offsets->strings = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_object_props) {
                prop_offsets->objects = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_null_array_props) {
                prop_offsets->null_arrays = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_bool_array_props) {
                prop_offsets->bool_arrays = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int8_array_props) {
                prop_offsets->int8_arrays = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int16_array_props) {
                prop_offsets->int16_arrays = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int32_array_props) {
                prop_offsets->int32_arrays = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_int64_array_props) {
                prop_offsets->int64_arrays = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint8_array_props) {
                prop_offsets->uint8_arrays = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint16_array_props) {
                prop_offsets->uint16_arrays = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint32_array_props) {
                prop_offsets->uint32_arrays = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_uint64_array_props) {
                prop_offsets->uint64_arrays = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_float_array_props) {
                prop_offsets->float_arrays = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_string_array_props) {
                prop_offsets->string_arrays = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
        if (flags->bits.has_object_array_props) {
                prop_offsets->object_arrays = *JAK_MEMFILE_READ_TYPE(memfile, offset_t);
        }
}

void int_embedded_fixed_props_read(struct fixed_prop *prop, struct memfile *memfile)
{
        prop->header = JAK_MEMFILE_READ_TYPE(memfile, struct prop_header);
        prop->keys = (field_sid_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(field_sid_t));
        prop->values = memfile_peek(memfile, 1);
}

void int_embedded_var_props_read(struct var_prop *prop, struct memfile *memfile)
{
        prop->header = JAK_MEMFILE_READ_TYPE(memfile, struct prop_header);
        prop->keys = (field_sid_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(field_sid_t));
        prop->offsets = (offset_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(offset_t));
        prop->values = memfile_peek(memfile, 1);
}

void int_embedded_null_props_read(struct null_prop *prop, struct memfile *memfile)
{
        prop->header = JAK_MEMFILE_READ_TYPE(memfile, struct prop_header);
        prop->keys = (field_sid_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(field_sid_t));
}

void int_embedded_array_props_read(struct array_prop *prop, struct memfile *memfile)
{
        prop->header = JAK_MEMFILE_READ_TYPE(memfile, struct prop_header);
        prop->keys = (field_sid_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(field_sid_t));
        prop->lengths = (u32 *) JAK_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(u32));
        prop->values_begin = memfile_tell(memfile);
}

void int_embedded_table_props_read(struct table_prop *prop, struct memfile *memfile)
{
        prop->header->marker = *JAK_MEMFILE_READ_TYPE(memfile, char);
        prop->header->num_entries = *JAK_MEMFILE_READ_TYPE(memfile, u8);
        prop->keys = (field_sid_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(field_sid_t));
        prop->groupOffs = (offset_t *) JAK_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(offset_t));
}

field_e int_get_value_type_of_char(char c)
{
        size_t len = sizeof(value_array_marker_mapping) / sizeof(value_array_marker_mapping[0]);
        for (size_t i = 0; i < len; i++) {
                if (marker_symbols[value_array_marker_mapping[i].marker].symbol == c) {
                        return value_array_marker_mapping[i].value_type;
                }
        }
        return FIELD_NULL;
}

field_e // TODO: check whether 'field_e' can be replaced by 'enum field_type'
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
                        print_error_and_die(JAK_ERR_MARKERMAPPING);
                }
        }
}