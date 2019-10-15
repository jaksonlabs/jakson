/**
 * Copyright 2019 Marcus Pinnecke
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

#ifndef CARBON_CONTAINERS
#define CARBON_CONTAINERS

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/carbon/markers.h>

BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//  container types (lists/maps)
// ---------------------------------------------------------------------------------------------------------------------

typedef enum carbon_container_type {
    CARBON_OBJECT, CARBON_ARRAY, CARBON_COLUMN
} carbon_container_e;

// ---------------------------------------------------------------------------------------------------------------------
//  implementation types (and marker) for list/map container types
// ---------------------------------------------------------------------------------------------------------------------

typedef enum carbon_list_container_sub_type {
    CARBON_CONTAINER_OBJECT = CARBON_MOBJECT_BEGIN,
    CARBON_CONTAINER_ARRAY = CARBON_MARRAY_BEGIN,
    CARBON_CONTAINER_COLUMN_U8 = CARBON_MCOLUMN_U8,
    CARBON_CONTAINER_COLUMN_U16 = CARBON_MCOLUMN_U16,
    CARBON_CONTAINER_COLUMN_U32 = CARBON_MCOLUMN_U32,
    CARBON_CONTAINER_COLUMN_U64 = CARBON_MCOLUMN_U64,
    CARBON_CONTAINER_COLUMN_I8 = CARBON_MCOLUMN_I8,
    CARBON_CONTAINER_COLUMN_I16 = CARBON_MCOLUMN_I16,
    CARBON_CONTAINER_COLUMN_I32 = CARBON_MCOLUMN_I32,
    CARBON_CONTAINER_COLUMN_I64 = CARBON_MCOLUMN_I64,
    CARBON_CONTAINER_COLUMN_BOOLEAN = CARBON_MCOLUMN_BOOLEAN,
    CARBON_CONTAINER_COLUMN_FLOAT = CARBON_MCOLUMN_FLOAT
} carbon_container_sub_type_e;

// ---------------------------------------------------------------------------------------------------------------------
//  implementation types (and marker) for list-only container types
// ---------------------------------------------------------------------------------------------------------------------

typedef enum carbon_list_container {
    CARBON_LIST_CONTAINER_ARRAY = CARBON_MARRAY_BEGIN,
    CARBON_LIST_CONTAINER_COLUMN_U8 = CARBON_MCOLUMN_U8,
    CARBON_LIST_CONTAINER_COLUMN_U16 = CARBON_MCOLUMN_U16,
    CARBON_LIST_CONTAINER_COLUMN_U32 = CARBON_MCOLUMN_U32,
    CARBON_LIST_CONTAINER_COLUMN_U64 = CARBON_MCOLUMN_U64,
    CARBON_LIST_CONTAINER_COLUMN_I8 = CARBON_MCOLUMN_I8,
    CARBON_LIST_CONTAINER_COLUMN_I16 = CARBON_MCOLUMN_I16,
    CARBON_LIST_CONTAINER_COLUMN_I32 = CARBON_MCOLUMN_I32,
    CARBON_LIST_CONTAINER_COLUMN_I64 = CARBON_MCOLUMN_I64,
    CARBON_LIST_CONTAINER_COLUMN_BOOLEAN = CARBON_MCOLUMN_BOOLEAN,
    CARBON_LIST_CONTAINER_COLUMN_FLOAT = CARBON_MCOLUMN_FLOAT
} carbon_list_container_e;

bool carbon_list_container_type_by_column_type(carbon_list_container_e *out, int field_type);

END_DECL

#endif