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
#include <jakson/utils/strings.h>

bool strings_contains_blank_char(const char *str)
{
        if (LIKELY(str != NULL)) {
                for (const char *it = str; *it != '\0'; it++) {
                        if (isspace(*it)) {
                                return true;
                        }
                }
                return false;
        } else {
                return false;
        }
}

bool strings_is_enquoted(const char *str)
{
        return strings_is_enquoted_wlen(str, strlen(str));
}

bool strings_is_enquoted_wlen(const char *str, size_t len)
{
        if (*str == '"') {
                if (len > 1) {
                        return str[len - 1] == '"';
                } else {
                        return false;
                }
        } else {
                return false;
        }
}

char *strings_remove_tailing_blanks(char *str_in)
{
        size_t sl = strlen(str_in);
        size_t i = sl > 0 ? sl - 1 : 0;
        while (i) {
                char c = str_in[i];
                if (isblank(c)) {
                        str_in[i] = '\0';
                } else {
                        break;
                }
                i--;
        }
        return str_in;
}

const char *strings_skip_blanks(const char *str)
{
        if (LIKELY(str != NULL)) {
                const char *ret = str;
                while (*ret != '\0') {
                        char c = *ret;
                        if (LIKELY(!isblank(c))) {
                                return ret;
                        }
                        ret++;
                }
                return ret;
        } else {
                return NULL;
        }
}