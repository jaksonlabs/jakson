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
#include "carbon/carbon-int-archive.h"

void
carbon_int_read_prop_offsets(carbon_archive_prop_offs_t *prop_offsets,
                             carbon_memfile_t *memfile,
                             const carbon_archive_object_flags_t *flags)
{
    CARBON_ZERO_MEMORY(prop_offsets, sizeof(carbon_archive_prop_offs_t));
    if (flags->bits.has_null_props) {
        prop_offsets->nulls = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_bool_props) {
        prop_offsets->bools = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int8_props) {
        prop_offsets->int8s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int16_props) {
        prop_offsets->int16s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int32_props) {
        prop_offsets->int32s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int64_props) {
        prop_offsets->int64s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint8_props) {
        prop_offsets->uint8s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint16_props) {
        prop_offsets->uint16s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint32_props) {
        prop_offsets->uint32s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint64_props) {
        prop_offsets->uint64s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_float_props) {
        prop_offsets->floats = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_string_props) {
        prop_offsets->strings = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_object_props) {
        prop_offsets->objects = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_null_array_props) {
        prop_offsets->null_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_bool_array_props) {
        prop_offsets->bool_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int8_array_props) {
        prop_offsets->int8_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int16_array_props) {
        prop_offsets->int16_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int32_array_props) {
        prop_offsets->int32_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int64_array_props) {
        prop_offsets->int64_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint8_array_props) {
        prop_offsets->uint8_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint16_array_props) {
        prop_offsets->uint16_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint32_array_props) {
        prop_offsets->uint32_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint64_array_props) {
        prop_offsets->uint64_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_float_array_props) {
        prop_offsets->float_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_string_array_props) {
        prop_offsets->string_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_object_array_props) {
        prop_offsets->object_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
}

void
carbon_int_embedded_fixed_props_read(carbon_fixed_prop_t *prop, carbon_memfile_t *memfile) {
    prop->header = CARBON_MEMFILE_READ_TYPE(memfile, carbon_prop_header_t);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_string_id_t));
    prop->values = carbon_memfile_peek(memfile, 1);
}

void
carbon_int_embedded_var_props_read(carbon_var_prop_t *prop, carbon_memfile_t *memfile) {
    prop->header = CARBON_MEMFILE_READ_TYPE(memfile, carbon_prop_header_t);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_string_id_t));
    prop->offsets = (carbon_off_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_off_t));
    prop->values = carbon_memfile_peek(memfile, 1);
}

void
carbon_int_embedded_null_props_read(carbon_null_prop_t *prop, carbon_memfile_t *memfile) {
    prop->header = CARBON_MEMFILE_READ_TYPE(memfile, carbon_prop_header_t);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_string_id_t));
}

void
carbon_int_embedded_array_props_read(carbon_array_prop_t *prop, carbon_memfile_t *memfile) {
    prop->header = CARBON_MEMFILE_READ_TYPE(memfile, carbon_prop_header_t);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_string_id_t));
    prop->lengths = (uint32_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(uint32_t));
    prop->values_begin = CARBON_MEMFILE_TELL(memfile);
}

void
carbon_int_embedded_table_props_read(carbon_table_prop_t *prop, carbon_memfile_t *memfile) {
    prop->header->marker = *CARBON_MEMFILE_READ_TYPE(memfile, char);
    prop->header->num_entries = *CARBON_MEMFILE_READ_TYPE(memfile, uint8_t);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_string_id_t));
    prop->groupOffs = (carbon_off_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_off_t));
}

carbon_field_type_e
carbon_int_get_value_type_of_char(char c)
{
    size_t len = sizeof(value_array_marker_mapping)/ sizeof(value_array_marker_mapping[0]);
    for (size_t i = 0; i < len; i++) {
        if (marker_symbols[value_array_marker_mapping[i].marker].symbol == c) {
            return value_array_marker_mapping[i].value_type;
        }
    }
    return carbon_field_type_null;
}
