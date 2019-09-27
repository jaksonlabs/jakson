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

#include <ctype.h>
#include <jakson/hexdump.h>

bool hexdump(string_buffer *dst, const void *base, u64 nbytes)
{
        ERROR_IF_NULL(dst);
        ERROR_IF_NULL(base);
        char buffer[11];

        sprintf(buffer, "%08x  ", 0);
        string_buffer_add(dst, buffer);

        for (u64 hex_block_id = 0; hex_block_id < nbytes;) {

                u8 step = JAK_MIN(16, nbytes - hex_block_id);

                for (u64 i = 0; i < step; i++) {
                        char c = *((const char *) (base + hex_block_id + i));
                        sprintf(buffer, "%02x ", (unsigned char) c);
                        string_buffer_add(dst, buffer);
                        if (i == 7) {
                                string_buffer_add_char(dst, ' ');
                        }
                }

                if (UNLIKELY(step == 7)) {
                        string_buffer_add_char(dst, ' ');
                }

                if (UNLIKELY(step < 16)) {
                        for (u8 pad = 0; pad < 16 - step; pad++) {
                                sprintf(buffer, "   ");
                                string_buffer_add(dst, buffer);
                        }
                }

                string_buffer_add(dst, " | ");

                for (u64 i = 0; i < step; i++) {
                        char c = *((const char *) (base + hex_block_id + i));
                        if (isgraph(c)) {
                                sprintf(buffer, "%c", c);
                        } else {
                                sprintf(buffer, ".");
                        }

                        string_buffer_add(dst, buffer);
                }

                if (UNLIKELY(step < 16)) {
                        for (u8 pad = 0; pad < 16 - step; pad++) {
                                sprintf(buffer, " ");
                                string_buffer_add(dst, buffer);
                        }
                }

                string_buffer_add_char(dst, '|');


                if (LIKELY(hex_block_id + step < nbytes)) {
                        string_buffer_add(dst, "\n");
                        sprintf(buffer, "%08x  ", ((u32) hex_block_id + 16));
                        string_buffer_add(dst, buffer);
                }


                hex_block_id += step;
        }

        string_buffer_add(dst, "\n");

        return true;
}

bool hexdump_print(FILE *file, const void *base, u64 nbytes)
{
        bool status;
        string_buffer sb;
        string_buffer_create(&sb);
        if ((status = hexdump(&sb, base, nbytes))) {
                fprintf(file, "%s", string_cstr(&sb));
        }
        string_buffer_drop(&sb);
        return status;

}