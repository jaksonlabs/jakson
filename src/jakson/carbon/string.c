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

#include <jakson/std/uintvar/stream.h>
#include <jakson/carbon/mime.h>
#include <jakson/carbon/field.h>
#include <jakson/carbon/string.h>

static void write_payload(memfile *file, const char *string, size_t str_len)
{
        memfile_write_uintvar_stream(NULL, file, str_len);
        memfile_ensure_space(file, str_len);
        memfile_write(file, string, str_len);
}

bool carbon_string_nomarker_write(memfile *file, const char *string)
{
        return carbon_string_nomarker_nchar_write(file, string, strlen(string));
}

bool carbon_string_nomarker_nchar_write(memfile *file, const char *string, u64 str_len)
{
        ERROR_IF_NULL(file)
        ERROR_IF_NULL(string)
        write_payload(file, string, str_len);

        return true;
}

bool carbon_string_nomarker_remove(memfile *file)
{
        ERROR_IF_NULL(file);
        u8 len_nbytes;
        u64 str_len = memfile_read_uintvar_stream(&len_nbytes, file);
        memfile_skip(file, -len_nbytes);
        memfile_inplace_remove(file, len_nbytes + str_len);
        return true;
}

bool carbon_string_remove(memfile *file)
{
        ERROR_IF_NULL(file);
        u8 marker = *MEMFILE_READ_TYPE(file, u8);
        if (LIKELY(marker == CARBON_FIELD_STRING)) {
                memfile_inplace_remove(file, sizeof(u8));
                return carbon_string_nomarker_remove(file);
        } else {
                ERROR(&file->err, ERR_MARKERMAPPING)
                return false;
        }
}

bool carbon_string_write(memfile *file, const char *string)
{
        return carbon_string_nchar_write(file, string, strlen(string));
}

bool carbon_string_nchar_write(memfile *file, const char *string, u64 str_len)
{
        ERROR_IF_NULL(file)
        ERROR_IF_NULL(string)

        memfile_ensure_space(file, sizeof(media_type));
        carbon_media_write(file, CARBON_FIELD_STRING);
        carbon_string_nomarker_nchar_write(file, string, str_len);

        return true;
}

bool carbon_string_update(memfile *file, const char *string)
{
        return carbon_string_update_wnchar(file, string, strlen(string));
}

bool carbon_string_update_wnchar(memfile *file, const char *string, size_t str_len)
{
        u8 marker = *MEMFILE_READ_TYPE(file, u8);
        if (LIKELY(marker == CARBON_FIELD_STRING)) {
                offset_t payload_start = memfile_tell(file);
                u32 old_len = memfile_read_uintvar_stream(NULL, file);
                memfile_skip(file, old_len);
                offset_t diff = memfile_tell(file) - payload_start;
                memfile_seek(file, payload_start);
                memfile_inplace_remove(file, diff);

                write_payload(file, string, str_len);
                return true;
        } else {
                ERROR(&file->err, ERR_MARKERMAPPING)
                return false;
        }
}

bool carbon_string_skip(memfile *file)
{
        return carbon_string_read(NULL, file);
}

bool carbon_string_nomarker_skip(memfile *file)
{
        return carbon_string_nomarker_read(NULL, file);
}

const char *carbon_string_read(u64 *len, memfile *file)
{
        ERROR_IF_NULL(file)
        u8 marker = *MEMFILE_READ_TYPE(file, u8);
        if (LIKELY(marker == CARBON_FIELD_STRING)) {
                return carbon_string_nomarker_read(len, file);
        } else {
                ERROR(&file->err, ERR_MARKERMAPPING)
                return false;
        }
}

const char *carbon_string_nomarker_read(u64 *len, memfile *file)
{
        u64 str_len = memfile_read_uintvar_stream(NULL, file);
        const char *result = memfile_read(file, str_len);
        OPTIONAL_SET(len, str_len);
        return result;
}