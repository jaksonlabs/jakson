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

#include <errno.h>

#include "carbon/carbon-reader.h"

static carbon_vec_t *toStringList(const char *contents, size_t numBytes);

CARBON_EXPORT(bool)
ChunkReaderCreate(ChunkReader *reader, carbon_alloc_t *alloc,
                      const char *filePath, size_t chunkSizeThreshold)
{
    CARBON_NON_NULL_OR_ERROR(reader)
    CARBON_NON_NULL_OR_ERROR(filePath)
    CARBON_NON_NULL_OR_ERROR(alloc)
    CARBON_NON_NULL_OR_ERROR(chunkSizeThreshold)

    carbon_alloc_this_or_std(&reader->alloc, alloc);
    reader->filePath = strdup(filePath);
    reader->chunkSizeThreshold = chunkSizeThreshold;
    reader->file = fopen(filePath, "r");
    reader->offset = 0;

    if (reader->file == NULL) {
        perror(strerror(errno));
        ChunkReaderDrop(reader);
        return false;
    }
    else {
        fseek(reader->file, 0L, SEEK_END);
        reader->fileSize = ftell(reader->file);
        fseek(reader->file, 0L, SEEK_SET);
    }

    return true;
}

CARBON_EXPORT(bool)
ChunkReaderDrop(ChunkReader *reader)
{
    CARBON_NON_NULL_OR_ERROR(reader);
    free(reader->filePath);
    if (reader->file != NULL) {
        fclose(reader->file);
        reader->file = NULL;
    }
    return true;
}

carbon_vec_t ofType(char *) *ChunkReaderNext(ChunkReader *reader)
{
    CARBON_NON_NULL_OR_ERROR(reader);
    CARBON_NON_NULL_OR_ERROR(reader->file);

    if (reader->offset >= reader->fileSize) {
        return NULL;
    }
    else {
        fseek(reader->file, reader->offset, SEEK_SET);

        char *buffer = (char *) malloc(reader->chunkSizeThreshold * sizeof(char) + 1);

        if (buffer == NULL)
            return NULL;

        size_t bytesToRead = CARBON_MIN(reader->chunkSizeThreshold, (reader->fileSize - reader->offset));
        fread(buffer, bytesToRead, 1, reader->file);

        reader->offset += bytesToRead;

        fprintf(stderr, "input read, progress %f%%\n", reader->offset * 100 / (float) reader->fileSize);

        buffer[bytesToRead] = '\0';

        fprintf(stderr, "DONE\n");

        carbon_vec_t ofType(char *) *result = toStringList(buffer, bytesToRead);
        free(buffer);

        return result;
    }
}

static carbon_vec_t *toStringList(const char *contents, size_t numBytes)
{
    fprintf(stderr, "converting to line list...");
    carbon_vec_t *vector = malloc(sizeof(carbon_vec_t));
    carbon_vec_create(vector, NULL, sizeof(char *), 15372804);
    char *begin, *end;
    begin = (char *) contents;
    end = (char *) contents + numBytes;
    while (*begin != '\0') {

        char *it;
        for (it = begin; it < end; it++) {
            if (*it == '\n') {
                break;
            }
        }

        size_t len = it - begin;
        if (len > 0) {
            char *string = malloc(len + 1);
            memcpy(string, begin, len);
            string[len] = '\0';
            carbon_vec_push(vector, &string, 1);
            begin += len;
        }
        else {
            begin++;
        }
    }
    fprintf(stderr, "DONE, %zu lines\n", carbon_vec_length(vector));
    return vector;
}