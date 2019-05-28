/**
 * BISON Binary Universal Document -- Copyright 2019 Marcus Pinnecke
 * This file contains internal functions and macros used for BISON at implementation level.
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

#ifndef BISON_INT_H
#define BISON_INT_H

#include <stdbool.h>

#ifdef __cplusplus
#define BISON_BEGIN_DECL  extern "C" {
#define BISON_END_DECL    }
#else
#define BISON_BEGIN_DECL
#define BISON_END_DECL
#endif

#ifndef BISON_EXPORT
#ifndef BISON_STATIC
#ifdef _WIN32
#define BISON_EXPORT(x) __declspec(dllimport) x
#elif defined(__GNUC__) && __GNUC__ >= 4
#define BISON_EXPORT(x) __attribute__((visibility("default"))) x
#else
#define NG5_EXPORT(x) x
#endif
#else
#define NG5_EXPORT(x) x
#endif
#endif

#endif //LIBCARBON_BISON_INT_H
