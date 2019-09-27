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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/carbon/commit.h>
#include <jakson/stdx/unique_id.h>
#include <jakson/std/hash.h>

bool carbon_commit_hash_create(memfile *file)
{
        ERROR_IF_NULL(file)

        u64 init_rev = 0;
        unique_id_create(&init_rev);

        memfile_ensure_space(file, sizeof(u64));
        memfile_write(file, &init_rev, sizeof(u64));

        return true;
}

bool carbon_commit_hash_skip(memfile *file)
{
        ERROR_IF_NULL(file)
        memfile_skip(file, sizeof(u64));
        return true;
}

bool carbon_commit_hash_read(u64 *commit_hash, memfile *file)
{
        ERROR_IF_NULL(file)
        ERROR_IF_NULL(commit_hash)
        *commit_hash = *MEMFILE_READ_TYPE(file, u64);
        return true;
}

bool carbon_commit_hash_peek(u64 *commit_hash, memfile *file)
{
        ERROR_IF_NULL(file)
        ERROR_IF_NULL(commit_hash)
        *commit_hash = *MEMFILE_PEEK(file, u64);
        return true;
}

bool carbon_commit_hash_update(memfile *file, const char *base, u64 len)
{
        ERROR_IF_NULL(file)
        ERROR_IF_NULL(base)
        ERROR_IF_NULL(len)
        u64 commit_hash;
        carbon_commit_hash_compute(&commit_hash, base, len);
        memfile_write(file, &commit_hash, sizeof(u64));
        return true;
}

bool carbon_commit_hash_compute(u64 *commit_hash, const void *base, u64 len)
{
        ERROR_IF_NULL(commit_hash)
        ERROR_IF_NULL(base)
        ERROR_IF_NULL(len)
        *commit_hash = HASH64_FNV(len, base);
        return true;
}

const char *carbon_commit_hash_to_str(string_buffer *dst, u64 commit_hash)
{
        if (dst) {
                string_buffer_clear(dst);
                string_buffer_add_u64_as_hex(dst, commit_hash);
                return string_cstr(dst);
        } else {
                return NULL;
        }
}

bool carbon_commit_hash_append_to_str(string_buffer *dst, u64 commit_hash)
{
        ERROR_IF_NULL(dst)
        string_buffer_add_u64_as_hex(dst, commit_hash);
        return true;
}

u64 carbon_commit_hash_from_str(const char *commit_str, err *err)
{
        if (commit_str && strlen(commit_str) == 16) {
                char *illegal_char;
                errno = 0;
                u64 ret = strtoull(commit_str, &illegal_char, 16);
                if (ret == 0 && commit_str == illegal_char) {
                        OPTIONAL(err, ERROR(err, ERR_NONUMBER))
                        return 0;
                } else if (ret == ULLONG_MAX && errno) {
                        OPTIONAL(err, ERROR(err, ERR_BUFFERTOOTINY))
                        return 0;
                } else if (*illegal_char) {
                        OPTIONAL(err, ERROR(err, ERR_TAILINGJUNK))
                        return 0;
                } else {
                        return ret;
                }
        } else {
                ERROR(err, ERR_ILLEGALARG)
                return 0;
        }
}