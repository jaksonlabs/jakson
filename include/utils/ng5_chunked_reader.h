// file: ng5_chunked_reader.h

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

#ifndef NG5_CHUNKED_READER
#define NG5_CHUNKED_READER

#include <ng5_common.h>
#include <stdx/ng5_vector.h>

typedef struct ng5_chunked_reader_t
{
    ng5_allocator_t  alloc;
    char            *file_path;
    size_t           chunk_size_threshold;
    FILE            *file;
    size_t           offset;
    size_t           file_size;
} ng5_chunked_reader_t;

int ng5_chunked_reader_create(ng5_chunked_reader_t *reader, ng5_allocator_t *alloc, const char *file_path,
        size_t chunk_size_threshold);

int ng5_chunked_reader_drop(ng5_chunked_reader_t *reader);

ng5_vector_t of_type(char *) *ng5_chunked_reader_next(ng5_chunked_reader_t *reader);



#endif
