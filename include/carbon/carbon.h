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

#include "carbon/carbon-common.h"
#include "carbon/carbon-alloc.h"
#include "carbon/carbon-bitmap.h"
#include "carbon/carbon-bloom.h"
#include "carbon/carbon-archive.h"
#include "carbon/carbon-common.h"
#include "carbon/carbon-convert.h"
#include "carbon/carbon-columndoc.h"
#include "carbon/carbon-doc.h"
#include "carbon/carbon-hash.h"
#include "carbon/carbon-huffman.h"
#include "carbon/carbon-json.h"
#include "carbon/carbon-memblock.h"
#include "carbon/carbon-memfile.h"
#include "carbon/carbon-sort.h"
#include "carbon/carbon-parallel.h"
#include "carbon/carbon-reader.h"
#include "carbon/carbon-slicelist.h"
#include "carbon/carbon-slotvector.h"
#include "carbon/carbon-spinlock.h"
#include "carbon/carbon-strdic.h"
#include "carbon/carbon-strhash.h"
#include "carbon/carbon-time.h"
#include "carbon/carbon-types.h"
#include "carbon/carbon-vector.h"
#include "carbon/carbon-error.h"
#include "carbon/alloc/carbon-alloc_tracer.h"
#include "carbon/strdic/carbon-strdic-async.h"
#include "carbon/strdic/carbon-strdic-sync.h"
#include "carbon/strhash/carbon-strhash-mem.h"

#ifndef CARBON_LIBCARBON_H
#define CARBON_LIBCARBON_H

CARBON_BEGIN_DECL

CARBON_EXPORT (bool)
carbon_init(void);

CARBON_END_DECL

#endif