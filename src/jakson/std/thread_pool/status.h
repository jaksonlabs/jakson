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

#ifndef THREAD_POOL_STATUS_H
#define THREAD_POOL_STATUS_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>

BEGIN_DECL

typedef enum thread_status_e {
        THREAD_STATUS_IDLE = 0,
        THREAD_STATUS_WORKING = 1,
        THREAD_STATUS_ABORTED = 2,
        THREAD_STATUS_FINISHED = 3,
        THREAD_STATUS_KILLED = 4,
        THREAD_STATUS_CREATED = 5,
        THREAD_STATUS_WILL_TERMINATE = 6,
        THREAD_STATUS_COMPLETED = 7,
        THREAD_STATUS_EMPTY = 99
} thread_status_e;

END_DECL

#endif
