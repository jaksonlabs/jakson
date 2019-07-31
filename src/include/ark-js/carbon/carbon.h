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

#ifndef NG5_H
#define NG5_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#include <ark-js/shared/common.h>
#include <ark-js/carbon/alloc/alloc.h>
#include <ark-js/shared/stdx/bitmap.h>
#include <ark-js/shared/stdx/bloom.h>
#include <ark-js/carbon/carbon/archive.h>
#include <ark-js/carbon/carbon/archive_iter.h>
#include <ark-js/carbon/carbon/archive_visitor.h>
#include <ark-js/carbon/carbon/archive_converter.h>
#include <ark-js/shared/json/encoded_doc.h>
#include <ark-js/shared/common.h>
#include <ark-js/shared/utils/convert.h>
#include <ark-js/shared/json/columndoc.h>
#include <ark-js/shared/json/doc.h>
#include <ark-js/shared/error.h>
#include <ark-js/shared/stdx/hash_table.h>
#include <ark-js/shared/hash/hash.h>
#include <ark-js/carbon/coding/coding_huffman.h>
#include <ark-js/shared/json/json.h>
#include <ark-js/shared/mem/block.h>
#include <ark-js/shared/mem/file.h>
#include <ark-js/carbon/oid/oid.h>
#include <ark-js/shared/stdx/sort.h>
#include <ark-js/shared/async/parallel.h>
#include <ark-js/shared/stdx/slicelist.h>
#include <ark-js/shared/async/spin.h>
#include <ark-js/shared/stdx/strdic.h>
#include <ark-js/shared/stdx/strhash.h>
#include <ark-js/carbon/carbon/archive_strid_iter.h>
#include <ark-js/carbon/carbon/archive_sid_cache.h>
#include <ark-js/shared/utils/time.h>
#include <ark-js/shared/types.h>
#include <ark-js/carbon/carbon/archive_query.h>
#include <ark-js/shared/stdx/vec.h>
#include <ark-js/carbon/alloc/trace.h>
#include <ark-js/carbon/encode/encode_async.h>
#include <ark-js/carbon/encode/encode_sync.h>
#include <ark-js/carbon/strhash/strhash_mem.h>
#include <ark-js/carbon/string-pred/string_pred_contains.h>
#include <ark-js/carbon/string-pred/string_pred_equals.h>
#include <ark-js/shared/stdx/varuint.h>

#ifdef __cplusplus
}
#endif

#endif