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

#include <jak_archive_strid_it.h>

bool jak_strid_iter_open(jak_strid_iter *it, jak_error *err, jak_archive *archive)
{
        JAK_ERROR_IF_NULL(it)
        JAK_ERROR_IF_NULL(archive)

        memset(&it->vector, 0, sizeof(it->vector));
        it->disk_file = fopen(archive->disk_file_path, "r");
        if (!it->disk_file) {
                JAK_OPTIONAL(err, JAK_ERROR(err, JAK_ERR_FOPEN_FAILED))
                it->is_open = false;
                return false;
        }
        fseek(it->disk_file, archive->jak_string_table.first_entry_off, SEEK_SET);
        it->is_open = true;
        it->disk_offset = archive->jak_string_table.first_entry_off;
        return true;
}

bool jak_strid_iter_next(bool *success, jak_strid_info **info, jak_error *err, size_t *info_length,
                         jak_strid_iter *it)
{
        JAK_ERROR_IF_NULL(info)
        JAK_ERROR_IF_NULL(info_length)
        JAK_ERROR_IF_NULL(it)

        if (it->disk_offset != 0 && it->is_open) {
                jak_string_entry_header header;
                size_t vec_pos = 0;
                do {
                        fseek(it->disk_file, it->disk_offset, SEEK_SET);
                        int num_read = fread(&header, sizeof(jak_string_entry_header), 1, it->disk_file);
                        if (header.marker != '-') {
                                JAK_ERROR_PRINT(JAK_ERR_INTERNALERR);
                                return false;
                        }
                        if (num_read != 1) {
                                JAK_OPTIONAL(err, JAK_ERROR(err, JAK_ERR_FREAD_FAILED))
                                *success = false;
                                return false;
                        } else {
                                it->vector[vec_pos].id = header.jak_string_id;
                                it->vector[vec_pos].offset = ftell(it->disk_file);
                                it->vector[vec_pos].strlen = header.jak_string_len;
                                it->disk_offset = header.next_entry_off;
                                vec_pos++;
                        }
                } while (header.next_entry_off != 0 && vec_pos < JAK_ARRAY_LENGTH(it->vector));

                *info_length = vec_pos;
                *success = true;
                *info = &it->vector[0];
                return true;
        } else {
                return false;
        }
}

bool jak_strid_iter_close(jak_strid_iter *it)
{
        JAK_ERROR_IF_NULL(it)
        if (it->is_open) {
                fclose(it->disk_file);
                it->is_open = false;
        }
        return true;
}
