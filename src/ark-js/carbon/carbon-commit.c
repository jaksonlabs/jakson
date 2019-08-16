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

#include <ark-js/carbon/carbon-commit.h>
#include <ark-js/shared/stdx/varuint.h>

bool carbon_commit_hash_create(struct memfile *file)
{
        error_if_null(file)

        u64 init_rev = 0;
        memfile_ensure_space(file, sizeof(u64));
        memfile_write(file, &init_rev, sizeof(u64));

        return true;
}

bool carbon_commit_hash_skip(struct memfile *file)
{
        error_if_null(file)
        memfile_skip(file, sizeof(u64));
        return true;
}

bool carbon_commit_hash_read(u64 *commit_hash, struct memfile *file)
{
        error_if_null(file)
        error_if_null(commit_hash)
        *commit_hash = *ARK_MEMFILE_READ_TYPE(file, u64);
        return true;
}

bool carbon_commit_hash_peek(u64 *commit_hash, struct memfile *file)
{
        error_if_null(file)
        error_if_null(commit_hash)
        *commit_hash = *ARK_MEMFILE_PEEK(file, u64);
        return true;
}

bool carbon_commit_hash_update(struct memfile *file)
{
        ark_declare_and_init(u64, rev)

        carbon_commit_hash_peek(&rev, file);
        rev++;
        memfile_write(file, &rev, sizeof(u64));

        return true;
}
