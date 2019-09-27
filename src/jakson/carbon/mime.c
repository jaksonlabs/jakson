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

#include <jakson/carbon/mime.h>

#define find_mime_by_ext(needle_ext)                                                    \
({                                                                                      \
        register size_t l = 0;                                                          \
        register size_t r = _global_mime_type_register - 1;                                   \
        u32 result = _global_mime_type_register;                                              \
        while (l <= r && r < SIZE_MAX) {                                                \
                register size_t m = l + (r - l) / 2;                                    \
                register int comp = strcmp(global_mime_type_register[m].ext, needle_ext);      \
                if (comp == 0) {                                                        \
                        result = m;                                                     \
                        break;                                                          \
                }                                                                       \
                if (comp < 0) {                                                         \
                        l = m + 1;                                                      \
                } else {                                                                \
                        r = m - 1;                                                      \
                }                                                                       \
        }                                                                               \
        result;                                                                         \
})

bool carbon_media_write(memfile *dst, carbon_field_type_e type)
{
        ERROR_IF_NULL(dst);
        media_type t = type;
        memfile_write(dst, &t, sizeof(media_type));
        return true;
}

u32 carbon_media_mime_type_by_ext(const char *ext)
{
        u32 id;
        if (LIKELY(ext != NULL)) {
                if (LIKELY((id = find_mime_by_ext(ext)) < (u32) _global_mime_type_register)) {
                        return id;
                }
        }
        id = find_mime_by_ext("bin");
        JAK_ASSERT(id < _global_mime_type_register);
        return id;
}

const char *carbon_media_mime_type_by_id(u32 id)
{
        if (UNLIKELY(id >= _global_mime_type_register)) {
                id = find_mime_by_ext("bin");
                JAK_ASSERT(id < _global_mime_type_register);
        }
        return global_mime_type_register[id].type;
}

const char *carbon_media_mime_ext_by_id(u32 id)
{
        if (UNLIKELY(id >= _global_mime_type_register)) {
                id = find_mime_by_ext("bin");
                JAK_ASSERT(id < _global_mime_type_register);
        }
        return global_mime_type_register[id].ext;
}


