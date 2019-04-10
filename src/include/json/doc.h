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

#ifndef NG5_DOC_H
#define NG5_DOC_H

#include "shared/common.h"
#include "std/vec.h"
#include "stdx/strdic.h"
#include "json.h"

NG5_BEGIN_DECL

typedef struct carbon_doc_obj carbon_doc_obj_t;

typedef struct carbon_columndoc carbon_columndoc_t;

struct doc_entries {
        carbon_doc_obj_t *context;
        const char *key;
        field_e type;
        struct vector ofType(<T>) values;
};

struct doc_bulk {
        struct strdic *dic;
        struct vector ofType(char *) keys, values;
        struct vector ofType(carbon_doc_t) models;
};

typedef struct carbon_doc {
        struct doc_bulk *context;
        struct vector ofType(carbon_doc_obj_t) obj_model;
        field_e type;
} carbon_doc_t;

typedef struct carbon_doc_obj {
        struct vector ofType(struct doc_entries) entries;
        carbon_doc_t *doc;
} carbon_doc_obj_t;

NG5_EXPORT(bool) carbon_doc_bulk_create(struct doc_bulk *bulk, struct strdic *dic);

NG5_EXPORT(bool) carbon_doc_bulk_Drop(struct doc_bulk *bulk);

NG5_EXPORT(bool) carbon_doc_bulk_shrink(struct doc_bulk *bulk);

NG5_EXPORT(bool) carbon_doc_bulk_print(FILE *file, struct doc_bulk *bulk);

NG5_EXPORT(carbon_doc_t *)carbon_doc_bulk_new_doc(struct doc_bulk *context, field_e type);

NG5_EXPORT(carbon_doc_obj_t *)carbon_doc_bulk_new_obj(carbon_doc_t *model);

NG5_EXPORT(bool) carbon_doc_bulk_get_dic_contents(struct vector ofType (const char *) **strings,
        struct vector ofType(carbon_string_id_t) **string_ids, const struct doc_bulk *context);

NG5_EXPORT(bool) carbon_doc_print(FILE *file, const carbon_doc_t *doc);

NG5_EXPORT(const struct vector ofType(struct doc_entries)
        *)carbon_doc_get_entries(const carbon_doc_obj_t *model);

NG5_EXPORT(void) carbon_doc_print_entries(FILE *file, const struct doc_entries *entries);

NG5_EXPORT(void) carbon_doc_drop(carbon_doc_obj_t *model);

NG5_EXPORT(bool) carbon_doc_obj_add_key(struct doc_entries **out, carbon_doc_obj_t *obj, const char *key, field_e type);

NG5_EXPORT(bool) carbon_doc_obj_push_primtive(struct doc_entries *entry, const void *value);

NG5_EXPORT(bool) carbon_doc_obj_push_object(carbon_doc_obj_t **out, struct doc_entries *entry);

NG5_EXPORT(struct doc_entries *)carbon_doc_bulk_new_entries(struct doc_bulk *dst);

NG5_EXPORT(carbon_doc_obj_t *)carbon_doc_bulk_add_json(struct doc_entries *partition, struct json *json);

NG5_EXPORT(carbon_doc_obj_t *)carbon_doc_entries_get_root(const struct doc_entries *partition);

NG5_EXPORT(carbon_columndoc_t *)carbon_doc_entries_columndoc(const struct doc_bulk *bulk,
        const struct doc_entries *partition, bool read_optimized);

NG5_EXPORT(bool) carbon_doc_entries_drop(struct doc_entries *partition);

NG5_END_DECL

#endif