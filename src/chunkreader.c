// file: chunkreader.c

/**
 *  Copyright (C) 2018 Marcus Pinnecke
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "chunkreader.h"
#include "errno.h"

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

static Vector *toStringList(const char *contents, size_t numBytes);


int ChunkReaderCreate(ChunkReader *reader, Allocator *alloc,
                      const char *filePath, size_t chunkSizeThreshold)
{
    CHECK_NON_NULL(reader)
    CHECK_NON_NULL(filePath)
    CHECK_NON_NULL(alloc)
    CHECK_NON_NULL(chunkSizeThreshold)

    AllocatorThisOrDefault(&reader->alloc, alloc);
    reader->filePath = strdup(filePath);
    reader->chunkSizeThreshold = chunkSizeThreshold;
    reader->file = fopen(filePath, "r");
    reader->offset = 0;

    if (reader->file == NULL) {
        perror(strerror(errno));
        ChunkReaderDrop(reader);
        return STATUS_FAILED;
    }
    else {
        fseek(reader->file, 0L, SEEK_END);
        reader->fileSize = ftell(reader->file);
        fseek(reader->file, 0L, SEEK_SET);
    }

    return STATUS_OK;
}

int ChunkReaderDrop(ChunkReader *reader)
{
    CHECK_NON_NULL(reader);
    free(reader->filePath);
    if (reader->file != NULL) {
        fclose(reader->file);
        reader->file = NULL;
    }
    return STATUS_OK;
}

Vector ofType(char *) *ChunkReaderNext(ChunkReader *reader)
{
    CHECK_NON_NULL(reader);
    CHECK_NON_NULL(reader->file);

    if (reader->offset >= reader->fileSize) {
        return NULL;
    }
    else {
        fseek(reader->file, reader->offset, SEEK_SET);

        char *buffer = (char *) malloc(reader->chunkSizeThreshold * sizeof(char) + 1);

        if (buffer == NULL)
            return NULL;

        size_t bytesToRead = MIN(reader->chunkSizeThreshold, (reader->fileSize - reader->offset));
        fread(buffer, bytesToRead, 1, reader->file);

        reader->offset += bytesToRead;

        fprintf(stderr, "input read, progress %f%%\n", reader->offset * 100 / (float) reader->fileSize);

        buffer[bytesToRead] = '\0';

        fprintf(stderr, "DONE\n");

        Vector ofType(char *) *result = toStringList(buffer, bytesToRead);
        free(buffer);

        return result;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

static Vector *toStringList(const char *contents, size_t numBytes)
{
    fprintf(stderr, "converting to line list...");
    Vector *vector = malloc(sizeof(Vector));
    VectorCreate(vector, NULL, sizeof(char *), 15372804);
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
            VectorPush(vector, &string, 1);
            begin += len;
        }
        else {
            begin++;
        }
    }
    fprintf(stderr, "DONE, %zu lines\n", VectorLength(vector));
    return vector;
}