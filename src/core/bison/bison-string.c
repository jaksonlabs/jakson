/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#include "stdx/varuint.h"
#include "core/bison/bison-media.h"
#include "core/bison/bison-field.h"
#include "core/bison/bison-string.h"

static void write_payload(struct memfile *file, const char *string)
{
        size_t value_strlen = strlen(string);
        memfile_write_varuint(file, value_strlen);
        memfile_ensure_space(file, value_strlen);
        memfile_write(file, string, value_strlen);
}

NG5_EXPORT(bool) bison_string_nomarker_write(struct memfile *file, const char *string)
{
        error_if_null(file)
        error_if_null(string)
        write_payload(file, string);

        return true;
}

NG5_EXPORT(bool) bison_string_write(struct memfile *file, const char *string)
{
        error_if_null(file)
        error_if_null(string)

        memfile_ensure_space(file, sizeof(media_type_t));
        bison_media_write(file, BISON_FIELD_TYPE_STRING);
        bison_string_nomarker_write(file, string);

        return true;
}

NG5_EXPORT(bool) bison_string_update(struct memfile *file, const char *string)
{
        u8 marker = *NG5_MEMFILE_READ_TYPE(file, u8);
        if (likely(marker == BISON_FIELD_TYPE_STRING)) {
                offset_t payload_start = memfile_tell(file);
                u32 old_len = memfile_read_varuint(NULL, file);
                memfile_skip(file, old_len);
                offset_t diff = memfile_tell(file) - payload_start;
                memfile_seek(file, payload_start);
                memfile_move_left(file, diff);

                write_payload(file, string);
                return true;
        } else {
                error(&file->err, NG5_ERR_MARKERMAPPING)
                return false;
        }
}

NG5_EXPORT(bool) bison_string_skip(struct memfile *file)
{
        return bison_string_read(NULL, file);
}

NG5_EXPORT(const char *) bison_string_read(u64 *len, struct memfile *file)
{
        error_if_null(file)
        u8 marker = *NG5_MEMFILE_READ_TYPE(file, u8);
        if (likely(marker == BISON_FIELD_TYPE_STRING)) {
                u64 str_len = memfile_read_varuint(NULL, file);
                const char *result = memfile_read(file, str_len);
                ng5_optional_set(len, str_len);
                return result;
        } else {
                error(&file->err, NG5_ERR_MARKERMAPPING)
                return false;
        }
}