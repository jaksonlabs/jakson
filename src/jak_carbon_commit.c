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

#include <jak_carbon_commit.h>
#include <jak_unique_id.h>
#include <jak_hash.h>

bool jak_carbon_commit_hash_create(struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file)

        jak_u64 init_rev = 0;
        jak_unique_id_create(&init_rev);

        memfile_ensure_space(file, sizeof(jak_u64));
        memfile_write(file, &init_rev, sizeof(jak_u64));

        return true;
}

bool jak_carbon_commit_hash_skip(struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file)
        memfile_skip(file, sizeof(jak_u64));
        return true;
}

bool jak_carbon_commit_hash_read(jak_u64 *commit_hash, struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(commit_hash)
        *commit_hash = *JAK_MEMFILE_READ_TYPE(file, jak_u64);
        return true;
}

bool jak_carbon_commit_hash_peek(jak_u64 *commit_hash, struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(commit_hash)
        *commit_hash = *JAK_MEMFILE_PEEK(file, jak_u64);
        return true;
}

bool jak_carbon_commit_hash_update(struct jak_memfile *file, const char *base, jak_u64 len)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(base)
        JAK_ERROR_IF_NULL(len)
        jak_u64 commit_hash;
        jak_carbon_commit_hash_compute(&commit_hash, base, len);
        memfile_write(file, &commit_hash, sizeof(jak_u64));
        return true;
}

bool jak_carbon_commit_hash_compute(jak_u64 *commit_hash, const void *base, jak_u64 len)
{
        JAK_ERROR_IF_NULL(commit_hash)
        JAK_ERROR_IF_NULL(base)
        JAK_ERROR_IF_NULL(len)
        *commit_hash = JAK_HASH64_FNV(len, base);
        return true;
}

const char *jak_carbon_commit_hash_to_str(jak_string *dst, jak_u64 commit_hash)
{
        if (dst) {
                jak_string_clear(dst);
                jak_string_add_u64_as_hex(dst, commit_hash);
                return jak_string_cstr(dst);
        } else {
                return NULL;
        }
}

bool jak_carbon_commit_hash_append_to_str(jak_string *dst, jak_u64 commit_hash)
{
        JAK_ERROR_IF_NULL(dst)
        jak_string_add_u64_as_hex(dst, commit_hash);
        return true;
}

jak_u64 jak_carbon_commit_hash_from_str(const char *commit_str, jak_error *err)
{
        if (commit_str && strlen(commit_str) == 16) {
                char *illegal_char;
                errno = 0;
                jak_u64 ret = strtoull(commit_str, &illegal_char, 16);
                if (ret == 0 && commit_str == illegal_char) {
                        JAK_OPTIONAL(err, JAK_ERROR(err, JAK_ERR_NONUMBER))
                        return 0;
                } else if (ret == ULLONG_MAX && errno) {
                        JAK_OPTIONAL(err, JAK_ERROR(err, JAK_ERR_BUFFERTOOTINY))
                        return 0;
                } else if (*illegal_char) {
                        JAK_OPTIONAL(err, JAK_ERROR(err, JAK_ERR_TAILINGJUNK))
                        return 0;
                } else {
                        return ret;
                }
        } else {
                JAK_ERROR(err, JAK_ERR_ILLEGALARG)
                return 0;
        }
}