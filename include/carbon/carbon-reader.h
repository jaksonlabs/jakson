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

#ifndef CARBON_READER_H
#define CARBON_READER_H

#include "carbon-common.h"
#include "carbon-vector.h"

CARBON_BEGIN_DECL

typedef struct ChunkReader
{
    carbon_alloc_t alloc;
    char *filePath;
    size_t chunkSizeThreshold;
    FILE *file;
    size_t offset;
    size_t fileSize;
} ChunkReader;

CARBON_EXPORT(bool)
ChunkReaderCreate(ChunkReader *reader, carbon_alloc_t *alloc, const char *filePath,
                      size_t chunkSizeThreshold);

CARBON_EXPORT(bool)
ChunkReaderDrop(ChunkReader *reader);

CARBON_EXPORT(carbon_vec_t ofType(char *) *)
ChunkReaderNext(ChunkReader *reader);

CARBON_END_DECL

#endif
