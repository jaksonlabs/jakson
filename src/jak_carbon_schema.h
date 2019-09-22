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

#ifndef JAK_CARBON_SCHEMA_H
#define JAK_CARBON_SCHEMA_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------
#include <jak_stdinc.h>
#include <jak_error.h>

JAK_BEGIN_DECL

typedef struct jak_carbon_schema_content {
    const char *key;
    void *value;
} jak_carbon_schema_content;

typedef struct jak_carbon_schema {
    jak_carbon_schema_content *content;
    unsigned int content_size;
    jak_error err;
} jak_carbon_schema;

bool jak_carbon_schema_validate(jak_carbon *schemaCarbon, jak_carbon **filesToVal);
bool jak_carbon_schema_createSchema(jak_carbon_schema *schema, jak_carbon_object_it *oit, jak_carbon **filesToVal);
bool jak_carbon_schema_handleKeys(jak_carbon_schema *schema, jak_carbon **filesToVal);
unsigned int jak_carbon_schema_getSchemaSize(jak_carbon_object_it *oit);

JAK_END_DECL

#endif
