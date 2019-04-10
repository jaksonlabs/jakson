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

#ifndef CARBON_H
#define CARBON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#include "shared/common.h"
#include "alloc/alloc.h"
#include "bitmap/bitmap.h"
#include "bloom/bloom.h"
#include "archive/archive.h"
#include "archive/archive_iter.h"
#include "archive/archive_visitor.h"
#include "archive/converter.h"
#include "json/encoded_doc.h"
#include "shared/common.h"
#include "utils/convert.h"
#include "json/columndoc.h"
#include "json/doc.h"
#include "shared/error.h"
#include "hash/hash_table.h"
#include "hash/hash.h"
#include "huffman/huffman.h"
#include "json/json.h"
#include "mem/memblock.h"
#include "mem/memfile.h"
#include "oid/oid.h"
#include "stdx/sort.h"
#include "async/parallel.h"
#include "strdic/slicelist.h"
#include "async/spinlock.h"
#include "strdic/strdic.h"
#include "strdic/strhash.h"
#include "archive/strid-iter.h"
#include "archive/string-id-cache.h"
#include "utils/time.h"
#include "shared/types.h"
#include "archive/query.h"
#include "stdx/vec.h"
#include "alloc/alloc_tracer.h"
#include "strdic/strdic_async.h"
#include "strdic/strdic_sync.h"
#include "strhash/carbon-strhash-mem.h"
#include "string-pred/carbon-string-pred-contains.h"
#include "string-pred/carbon-string-pred-equals.h"

typedef struct types
{

} carbon_t;

CARBON_EXPORT (bool)
carbon_init(void);

#ifdef __cplusplus
}
#endif

#endif