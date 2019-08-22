/**
 * Copyright 2019 Marcus Pinnecke, Robert Jendersie, Johannes Wuensche, Johann Wagner, and Marten Wallewein-Eising
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without ion, including without limitation the
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

#ifndef JAK_THREAD_POOL_STATUS_H
#define JAK_THREAD_POOL_STATUS_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>

JAK_BEGIN_DECL

typedef enum jak_thread_status_e {
        JAK_THREAD_STATUS_IDLE = 0,
        JAK_THREAD_STATUS_WORKING = 1,
        JAK_THREAD_STATUS_ABORTED = 2,
        JAK_THREAD_STATUS_FINISHED = 3,
        JAK_THREAD_STATUS_KILLED = 4,
        JAK_THREAD_STATUS_CREATED = 5,
        JAK_THREAD_STATUS_WILL_TERMINATE = 6,
        JAK_THREAD_STATUS_COMPLETED = 7,
        JAK_THREAD_STATUS_EMPTY = 99
} jak_thread_status_e;

JAK_END_DECL

#endif
