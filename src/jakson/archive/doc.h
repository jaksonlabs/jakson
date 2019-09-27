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

#ifndef DOC_H
#define DOC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/std/vector.h>
#include <jakson/std/string_dict.h>
#include <jakson/json/parser.h>

BEGIN_DECL

typedef struct doc_entries {
        doc_obj *context;
        const char *key;
        archive_field_e type;
        vector ofType(<T>) values;
} doc_entries;

typedef struct doc_bulk {
        string_dict *dic;
        vector ofType(char *) keys, values;
        vector ofType(doc) models;
} doc_bulk;

typedef struct doc {
        doc_bulk *context;
        vector ofType(doc_obj) obj_model;
        archive_field_e type;
} doc;

typedef struct doc_obj {
        vector ofType(doc_entries) entries;
        doc *doc;
} doc_obj;

bool doc_bulk_create(doc_bulk *bulk, string_dict *dic);
bool doc_bulk_drop(doc_bulk *bulk);

bool doc_bulk_shrink(doc_bulk *bulk);
bool doc_bulk_print(FILE *file, doc_bulk *bulk);

doc *doc_bulk_new_doc(doc_bulk *context, archive_field_e type);
doc_obj *doc_bulk_new_obj(doc *model);
bool doc_bulk_get_dic_contents(vector ofType (const char *) **strings, vector ofType(archive_field_sid_t) **string_ids, const doc_bulk *context);

bool doc_print(FILE *file, const doc *doc);
const vector ofType(doc_entries) *doc_get_entries(const doc_obj *model);
void doc_print_entries(FILE *file, const doc_entries *entries);
void doc_drop(doc_obj *model);

bool doc_obj_add_key(doc_entries **out, doc_obj *obj, const char *key, archive_field_e type);
bool doc_obj_push_primtive(doc_entries *entry, const void *value);
bool doc_obj_push_object(doc_obj **out, doc_entries *entry);

doc_entries *doc_bulk_new_entries(doc_bulk *dst);
doc_obj *doc_bulk_add_json(doc_entries *partition, json *json);
doc_obj *doc_entries_get_root(const doc_entries *partition);
column_doc *doc_entries_columndoc(const doc_bulk *bulk, const doc_entries *partition, bool read_optimized);
bool doc_entries_drop(doc_entries *partition);

END_DECL

#endif