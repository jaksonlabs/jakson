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

#ifndef JAK_DOC_H
#define JAK_DOC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>
#include <jak_string_dict.h>
#include <jak_json.h>

JAK_BEGIN_DECL

typedef struct jak_doc_entries {
        jak_doc_obj *context;
        const char *key;
        jak_archive_field_e type;
        struct jak_vector ofType(<T>) values;
} jak_doc_entries;

typedef struct jak_doc_bulk {
        struct jak_string_dict *dic;
        struct jak_vector ofType(char *) keys, values;
        struct jak_vector ofType(jak_doc) models;
} jak_doc_bulk;

typedef struct jak_doc {
        jak_doc_bulk *context;
        struct jak_vector ofType(jak_doc_obj) obj_model;
        jak_archive_field_e type;
} jak_doc;

typedef struct jak_doc_obj {
        struct jak_vector ofType(jak_doc_entries) entries;
        jak_doc *doc;
} jak_doc_obj;

bool jak_doc_bulk_create(jak_doc_bulk *bulk, struct jak_string_dict *dic);
bool jak_doc_bulk_drop(jak_doc_bulk *bulk);

bool jak_doc_bulk_shrink(jak_doc_bulk *bulk);
bool jak_doc_bulk_print(FILE *file, jak_doc_bulk *bulk);

jak_doc *jak_doc_bulk_new_doc(jak_doc_bulk *context, jak_archive_field_e type);
jak_doc_obj *jak_doc_bulk_new_obj(jak_doc *model);
bool jak_doc_bulk_get_dic_contents(struct jak_vector ofType (const char *) **strings, struct jak_vector ofType(jak_archive_field_sid_t) **string_ids, const jak_doc_bulk *context);

bool jak_doc_print(FILE *file, const jak_doc *doc);
const struct jak_vector ofType(jak_doc_entries) *jak_doc_get_entries(const jak_doc_obj *model);
void jak_doc_print_entries(FILE *file, const jak_doc_entries *entries);
void jak_doc_drop(jak_doc_obj *model);

bool jak_doc_obj_add_key(jak_doc_entries **out, jak_doc_obj *obj, const char *key, jak_archive_field_e type);
bool jak_doc_obj_push_primtive(jak_doc_entries *entry, const void *value);
bool jak_doc_obj_push_object(jak_doc_obj **out, jak_doc_entries *entry);

jak_doc_entries *jak_doc_bulk_new_entries(jak_doc_bulk *dst);
jak_doc_obj *jak_doc_bulk_add_json(jak_doc_entries *partition, jak_json *jak_json);
jak_doc_obj *jak_doc_entries_get_root(const jak_doc_entries *partition);
jak_column_doc *jak_doc_entries_columndoc(const jak_doc_bulk *bulk, const jak_doc_entries *partition, bool read_optimized);
bool jak_doc_entries_drop(jak_doc_entries *partition);

JAK_END_DECL

#endif