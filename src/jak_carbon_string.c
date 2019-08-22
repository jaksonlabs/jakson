/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#include <jak_uintvar_stream.h>
#include <jak_carbon_media.h>
#include <jak_carbon_field.h>
#include <jak_carbon_string.h>

static void write_payload(struct jak_memfile *file, const char *string, size_t str_len)
{
        memfile_write_uintvar_stream(NULL, file, str_len);
        memfile_ensure_space(file, str_len);
        memfile_write(file, string, str_len);
}

bool jak_carbon_jak_string_nomarker_write(struct jak_memfile *file, const char *string)
{
        return jak_carbon_jak_string_nomarker_nchar_write(file, string, strlen(string));
}

bool jak_carbon_jak_string_nomarker_nchar_write(struct jak_memfile *file, const char *string, jak_u64 str_len)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(string)
        write_payload(file, string, str_len);

        return true;
}

bool jak_carbon_jak_string_nomarker_remove(struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file);
        jak_u8 len_nbytes;
        jak_u64 str_len = memfile_read_uintvar_stream(&len_nbytes, file);
        memfile_skip(file, -len_nbytes);
        memfile_inplace_remove(file, len_nbytes + str_len);
        return true;
}

bool jak_carbon_jak_string_remove(struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file);
        jak_u8 marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);
        if (JAK_LIKELY(marker == JAK_CARBON_FIELD_TYPE_STRING)) {
                memfile_inplace_remove(file, sizeof(jak_u8));
                return jak_carbon_jak_string_nomarker_remove(file);
        } else {
                JAK_ERROR(&file->err, JAK_ERR_MARKERMAPPING)
                return false;
        }
}

bool jak_carbon_jak_string_write(struct jak_memfile *file, const char *string)
{
        return jak_carbon_jak_string_nchar_write(file, string, strlen(string));
}

bool jak_carbon_jak_string_nchar_write(struct jak_memfile *file, const char *string, jak_u64 str_len)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(string)

        memfile_ensure_space(file, sizeof(jak_media_type));
        jak_carbon_media_write(file, JAK_CARBON_FIELD_TYPE_STRING);
        jak_carbon_jak_string_nomarker_nchar_write(file, string, str_len);

        return true;
}

bool jak_carbon_jak_string_update(struct jak_memfile *file, const char *string)
{
        return jak_carbon_jak_string_update_wnchar(file, string, strlen(string));
}

bool jak_carbon_jak_string_update_wnchar(struct jak_memfile *file, const char *string, size_t str_len)
{
        jak_u8 marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);
        if (JAK_LIKELY(marker == JAK_CARBON_FIELD_TYPE_STRING)) {
                jak_offset_t payload_start = memfile_tell(file);
                jak_u32 old_len = memfile_read_uintvar_stream(NULL, file);
                memfile_skip(file, old_len);
                jak_offset_t diff = memfile_tell(file) - payload_start;
                memfile_seek(file, payload_start);
                memfile_inplace_remove(file, diff);

                write_payload(file, string, str_len);
                return true;
        } else {
                JAK_ERROR(&file->err, JAK_ERR_MARKERMAPPING)
                return false;
        }
}

bool jak_carbon_jak_string_skip(struct jak_memfile *file)
{
        return jak_carbon_jak_string_read(NULL, file);
}

bool jak_carbon_jak_string_nomarker_skip(struct jak_memfile *file)
{
        return jak_carbon_jak_string_nomarker_read(NULL, file);
}

const char *jak_carbon_jak_string_read(jak_u64 *len, struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file)
        jak_u8 marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);
        if (JAK_LIKELY(marker == JAK_CARBON_FIELD_TYPE_STRING)) {
                return jak_carbon_jak_string_nomarker_read(len, file);
        } else {
                JAK_ERROR(&file->err, JAK_ERR_MARKERMAPPING)
                return false;
        }
}

const char *jak_carbon_jak_string_nomarker_read(jak_u64 *len, struct jak_memfile *file)
{
        jak_u64 str_len = memfile_read_uintvar_stream(NULL, file);
        const char *result = memfile_read(file, str_len);
        JAK_OPTIONAL_SET(len, str_len);
        return result;
}