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

struct jak_doc_obj;

struct jak_column_doc;

struct jak_doc_entries {
    struct jak_doc_obj *context;
    const char *key;
    jak_archive_field_e type;
    struct vector ofType(<T>) values;
};

struct jak_doc_bulk {
    struct jak_string_dict *dic;
    struct vector ofType(char *) keys, values;
    struct vector ofType(struct jak_doc) models;
};

struct jak_doc {
    struct jak_doc_bulk *context;
    struct vector ofType(struct jak_doc_obj) obj_model;
    jak_archive_field_e type;
};

struct jak_doc_obj {
    struct vector ofType(struct jak_doc_entries) entries;
    struct jak_doc *doc;
};

bool doc_bulk_create(struct jak_doc_bulk *bulk, struct jak_string_dict *dic);

bool doc_bulk_Drop(struct jak_doc_bulk *bulk);

bool doc_bulk_shrink(struct jak_doc_bulk *bulk);

bool doc_bulk_print(FILE *file, struct jak_doc_bulk *bulk);

struct jak_doc *doc_bulk_new_doc(struct jak_doc_bulk *context, jak_archive_field_e type);

struct jak_doc_obj *doc_bulk_new_obj(struct jak_doc *model);

bool doc_bulk_get_dic_contents(struct vector ofType (const char *) **strings,
                               struct vector ofType(jak_field_sid) **string_ids, const struct jak_doc_bulk *context);

bool doc_print(FILE *file, const struct jak_doc *doc);

const struct vector ofType(struct jak_doc_entries) *doc_get_entries(const struct jak_doc_obj *model);

void doc_print_entries(FILE *file, const struct jak_doc_entries *entries);

void doc_drop(struct jak_doc_obj *model);

bool doc_obj_add_key(struct jak_doc_entries **out, struct jak_doc_obj *obj, const char *key, jak_archive_field_e type);

bool doc_obj_push_primtive(struct jak_doc_entries *entry, const void *value);

bool doc_obj_push_object(struct jak_doc_obj **out, struct jak_doc_entries *entry);

struct jak_doc_entries *doc_bulk_new_entries(struct jak_doc_bulk *dst);

struct jak_doc_obj *doc_bulk_add_json(struct jak_doc_entries *partition, struct jak_json *json);

struct jak_doc_obj *doc_entries_get_root(const struct jak_doc_entries *partition);

struct jak_column_doc *doc_entries_columndoc(const struct jak_doc_bulk *bulk, const struct jak_doc_entries *partition,
                                             bool read_optimized);

bool doc_entries_drop(struct jak_doc_entries *partition);

JAK_END_DECL

#endif