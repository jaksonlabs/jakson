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

#include <math.h>
#include <inttypes.h>

#include "json/columndoc.h"
#include "json/doc.h"

static void setup_object(columndoc_obj_t *model, carbon_columndoc_t *parent, carbon_string_id_t key, size_t idx);

static bool object_put(columndoc_obj_t *model, struct err *err, const carbon_doc_entries_t *entry, struct strdic *dic);

static bool import_object(columndoc_obj_t *dst, struct err *err, const carbon_doc_obj_t *doc, struct strdic *dic);

static bool print_object(FILE *file, struct err *err, const columndoc_obj_t *object, struct strdic *dic);

static const char *get_type_name(struct err *err, field_e type);

static void object_array_key_columns_create(struct vector ofType(carbon_columndoc_columngroup_t) *columns);

static carbon_columndoc_column_t *object_array_key_columns_find_or_new(struct vector ofType(carbon_columndoc_columngroup_t) *columns,
                                                            carbon_string_id_t array_key, carbon_string_id_t nested_object_entry_key,
                                                            field_e nested_object_entry_type);

static bool object_array_key_column_push(carbon_columndoc_column_t *col, struct err *err, const carbon_doc_entries_t *entry, u32 array_idx,
                                     struct strdic *dic, columndoc_obj_t *model);

bool carbon_columndoc_create(carbon_columndoc_t *columndoc,
                             struct err *err,
                             const carbon_doc_t *doc,
                             const carbon_doc_bulk_t *bulk,
                             const carbon_doc_entries_t *entries,
                             struct strdic *dic)
{
    NG5_NON_NULL_OR_ERROR(columndoc)
    NG5_NON_NULL_OR_ERROR(doc)
    NG5_NON_NULL_OR_ERROR(dic)
    NG5_NON_NULL_OR_ERROR(bulk)

    columndoc->dic = dic;
    columndoc->doc = doc;
    columndoc->bulk = bulk;
    carbon_error_init(&columndoc->err);

    const char *root_string = "/";
    carbon_string_id_t *rootId;

    carbon_strdic_insert(dic, &rootId, (char *const *) &root_string, 1, 0);

    setup_object(&columndoc->columndoc, columndoc, *rootId, 0);

    carbon_strdic_free(dic, rootId);

    const carbon_doc_obj_t *root = carbon_doc_entries_get_root(entries);
    if (!import_object(&columndoc->columndoc, err, root, dic)) {
        return false;
    }

    return true;
}

static void object_array_key_columns_drop(struct vector ofType(carbon_columndoc_columngroup_t) *columns);

static void object_meta_model_free(columndoc_obj_t *columndoc)
{
    carbon_vec_drop(&columndoc->bool_prop_keys);
    carbon_vec_drop(&columndoc->int8_prop_keys);
    carbon_vec_drop(&columndoc->int16_prop_keys);
    carbon_vec_drop(&columndoc->int32_prop_keys);
    carbon_vec_drop(&columndoc->int64_prop_keys);
    carbon_vec_drop(&columndoc->uint8_prop_keys);
    carbon_vec_drop(&columndoc->uint16_prop_keys);
    carbon_vec_drop(&columndoc->uin32_prop_keys);
    carbon_vec_drop(&columndoc->uint64_prop_keys);
    carbon_vec_drop(&columndoc->string_prop_keys);
    carbon_vec_drop(&columndoc->float_prop_keys);
    carbon_vec_drop(&columndoc->null_prop_keys);
    carbon_vec_drop(&columndoc->obj_prop_keys);

    carbon_vec_drop(&columndoc->bool_array_prop_keys);
    carbon_vec_drop(&columndoc->int8_array_prop_keys);
    carbon_vec_drop(&columndoc->int16_array_prop_keys);
    carbon_vec_drop(&columndoc->int32_array_prop_keys);
    carbon_vec_drop(&columndoc->int64_array_prop_keys);
    carbon_vec_drop(&columndoc->uint8_array_prop_keys);
    carbon_vec_drop(&columndoc->uint16_array_prop_keys);
    carbon_vec_drop(&columndoc->uint32_array_prop_keys);
    carbon_vec_drop(&columndoc->uint64_array_prop_keys);
    carbon_vec_drop(&columndoc->string_array_prop_keys);
    carbon_vec_drop(&columndoc->float_array_prop_keys);
    carbon_vec_drop(&columndoc->null_array_prop_keys);

    carbon_vec_drop(&columndoc->bool_prop_vals);
    carbon_vec_drop(&columndoc->int8_prop_vals);
    carbon_vec_drop(&columndoc->int16_prop_vals);
    carbon_vec_drop(&columndoc->int32_prop_vals);
    carbon_vec_drop(&columndoc->int64_prop_vals);
    carbon_vec_drop(&columndoc->uint8_prop_vals);
    carbon_vec_drop(&columndoc->uint16_prop_vals);
    carbon_vec_drop(&columndoc->uint32_prop_vals);
    carbon_vec_drop(&columndoc->uint64_prop_vals);
    carbon_vec_drop(&columndoc->float_prop_vals);
    carbon_vec_drop(&columndoc->string_prop_vals);

    for (size_t i = 0; i < columndoc->bool_array_prop_vals.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->bool_array_prop_vals, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->bool_array_prop_vals);

    for (size_t i = 0; i < columndoc->int8_array_prop_vals.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->int8_array_prop_vals, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int8_array_prop_vals);

    for (size_t i = 0; i < columndoc->int16_array_prop_vals.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->int16_array_prop_vals, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int16_array_prop_vals);

    for (size_t i = 0; i < columndoc->int32_array_prop_vals.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->int32_array_prop_vals, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int32_array_prop_vals);

    for (size_t i = 0; i < columndoc->int64_array_prop_vals.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->int64_array_prop_vals, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int64_array_prop_vals);

    for (size_t i = 0; i < columndoc->uint8_array_prop_vals.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->uint8_array_prop_vals, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint8_array_prop_vals);

    for (size_t i = 0; i < columndoc->uint16_array_prop_vals.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->uint16_array_prop_vals, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint16_array_prop_vals);

    for (size_t i = 0; i < columndoc->uint32_array_prop_vals.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->uint32_array_prop_vals, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint32_array_prop_vals);

    for (size_t i = 0; i < columndoc->ui64_array_prop_vals.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->ui64_array_prop_vals, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->ui64_array_prop_vals);

    for (size_t i = 0; i < columndoc->float_array_prop_vals.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->float_array_prop_vals, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->float_array_prop_vals);

    for (size_t i = 0; i < columndoc->string_array_prop_vals.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->string_array_prop_vals, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->string_array_prop_vals);

    carbon_vec_drop(&columndoc->null_array_prop_vals);

    carbon_vec_drop(&columndoc->bool_val_idxs);
    carbon_vec_drop(&columndoc->int8_val_idxs);
    carbon_vec_drop(&columndoc->int16_val_idxs);
    carbon_vec_drop(&columndoc->int32_val_idxs);
    carbon_vec_drop(&columndoc->int64_val_idxs);
    carbon_vec_drop(&columndoc->uint8_val_idxs);
    carbon_vec_drop(&columndoc->uint16_val_idxs);
    carbon_vec_drop(&columndoc->uint32_val_idxs);
    carbon_vec_drop(&columndoc->uint64_val_idxs);
    carbon_vec_drop(&columndoc->float_val_idxs);
    carbon_vec_drop(&columndoc->string_val_idxs);

    for (size_t i = 0; i < columndoc->bool_array_idxs.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->bool_array_idxs, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->bool_array_idxs);

    for (size_t i = 0; i < columndoc->int8_array_idxs.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->int8_array_idxs, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int8_array_idxs);

    for (size_t i = 0; i < columndoc->int16_array_idxs.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->int16_array_idxs, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int16_array_idxs);

    for (size_t i = 0; i < columndoc->int32_array_idxs.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->int32_array_idxs, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int32_array_idxs);

    for (size_t i = 0; i < columndoc->int64_array_idxs.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->int64_array_idxs, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int64_array_idxs);

    for (size_t i = 0; i < columndoc->uint8_array_idxs.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->uint8_array_idxs, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint8_array_idxs);

    for (size_t i = 0; i < columndoc->uint16_array_idxs.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->uint16_array_idxs, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint16_array_idxs);

    for (size_t i = 0; i < columndoc->uint32_array_idxs.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->uint32_array_idxs, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint32_array_idxs);

    for (size_t i = 0; i < columndoc->uint64_array_idxs.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->uint64_array_idxs, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint64_array_idxs);

    for (size_t i = 0; i < columndoc->float_array_idxs.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->float_array_idxs, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->float_array_idxs);

    for (size_t i = 0; i < columndoc->string_array_idxs.num_elems; i++) {
        struct vector *vec = vec_get(&columndoc->string_array_idxs, i, struct vector);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->string_array_idxs);

    for (size_t i = 0; i < columndoc->obj_prop_vals.num_elems; i++) {
        columndoc_obj_t *object = vec_get(&columndoc->obj_prop_vals, i, columndoc_obj_t);
        object_meta_model_free(object);
    }
    carbon_vec_drop(&columndoc->obj_prop_vals);

    object_array_key_columns_drop(&columndoc->obj_array_props);
}

bool carbon_columndoc_free(carbon_columndoc_t *doc)
{
    NG5_NON_NULL_OR_ERROR(doc);
    object_meta_model_free(&doc->columndoc);
    return true;
}

#define PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, suffix)                                             \
{                                                                                                                      \
    fprintf(file, "\"%s\": { ", type_name);                                                                            \
    if(!carbon_vec_is_empty((key_vector))) {                                                                           \
        fprintf(file, "\"Keys\": [ ");                                                                                 \
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {                                                         \
            carbon_string_id_t string_id = *vec_get((key_vector), i, carbon_string_id_t);                    \
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (key_vector)->num_elems ? ", " : "");                      \
        }                                                                                                              \
        fprintf(file, "], ");                                                                                          \
        fprintf(file, "\"Keys Decoded\": [ ");                                                                         \
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {                                                         \
            carbon_string_id_t string_id = *vec_get((key_vector), i, carbon_string_id_t);                    \
            char **encString = carbon_strdic_extract(dic, &string_id, 1);                                              \
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (key_vector)->num_elems ? ", " : "");                      \
            carbon_strdic_free(dic, encString);                                                                        \
        }                                                                                                              \
        fprintf(file, "]%s", suffix);                                                                                  \
    }                                                                                                                  \
}                                                                                                                      \

#define PRINT_PRIMITIVE_COLUMN(file, type_name, key_vector, value_vector, keyIndicesVector, dic, TYPE, FORMAT_STR)     \
{                                                                                                                      \
    PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, ", ")                                                   \
    if(!carbon_vec_is_empty((key_vector))) {                                                                           \
        fprintf(file, "\"Values\": [ ");                                                                               \
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {                                                       \
            TYPE value = *vec_get(value_vector, i, TYPE);                                                    \
            fprintf(file, FORMAT_STR "%s", value, i + 1 < (value_vector)->num_elems ? ", " : "");                      \
        }                                                                                                              \
        fprintf(file, "] ");                                                                                           \
    }                                                                                                                  \
    fprintf(file, "}, ");                                                                                              \
}


#define PRINT_PRIMITIVE_BOOLEAN_COLUMN(file, type_name, key_vector, value_vector, dic)                                 \
{                                                                                                                      \
    PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, ", ")                                                   \
    if(!carbon_vec_is_empty((key_vector))) {                                                                           \
        fprintf(file, "\"Values\": [ ");                                                                               \
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {                                                       \
            carbon_boolean_t value = *vec_get(value_vector, i, carbon_boolean_t);                                  \
            fprintf(file, "%s%s", value == 0 ? "false" : "true", i + 1 < (value_vector)->num_elems ? ", " : "");       \
        }                                                                                                              \
        fprintf(file, "]");                                                                                            \
    }                                                                                                                  \
    fprintf(file, "}, ");                                                                                              \
}

static void print_primitive_null(FILE *file, const char *type_name, const struct vector ofType(carbon_string_id_t) *key_vector,
                               struct strdic *dic)
{
    PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, "")
    fprintf(file, "}, ");
}



static bool print_primitive_objects(FILE *file, struct err *err, const char *type_name, const struct vector ofType(carbon_string_id_t) *key_vector,
                                  const struct vector ofType(columndoc_obj_t) *value_vector, struct strdic *dic)
{
    PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, ", ")
    if(!carbon_vec_is_empty((key_vector))) {
        fprintf(file, "\"Values\": [ ");
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {
            const columndoc_obj_t *object = vec_get(value_vector, i, columndoc_obj_t);
            if(!print_object(file, err, object, dic)) {
                return false;
            }
            fprintf(file, "%s", i + 1 < (value_vector)->num_elems ? ", " : "");
        }
        fprintf(file, "]");
    }
    fprintf(file, "}");
    return true;
}

#define PRINT_ARRAY(file, type_name, key_vector, value_vector, TYPE, TYPE_FORMAT, nonnull_expr)                        \
{                                                                                                                      \
    fprintf(file, "\"%s\": { ", type_name);                                                                            \
    if(!carbon_vec_is_empty((&key_vector))) {                                                                          \
        fprintf(file, "\"Keys\": [ ");                                                                                 \
        for (size_t i = 0; i < (&key_vector)->num_elems; i++) {                                                        \
            carbon_string_id_t string_id = *vec_get((&key_vector), i, carbon_string_id_t);                   \
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (&key_vector)->num_elems ? ", " : "");                     \
        }                                                                                                              \
        fprintf(file, "], ");                                                                                          \
        fprintf(file, "\"Keys Decoded\": [ ");                                                                         \
        for (size_t i = 0; i < (&key_vector)->num_elems; i++) {                                                        \
            carbon_string_id_t string_id = *vec_get((&key_vector), i, carbon_string_id_t);                   \
            char **encString = carbon_strdic_extract(dic, &string_id, 1);                                              \
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (&key_vector)->num_elems ? ", " : "");                     \
            carbon_strdic_free(dic, encString);                                                                        \
        }                                                                                                              \
        fprintf(file, "],");                                                                                           \
        fprintf(file, "\"Values\": [ ");                                                                               \
        for (size_t i = 0; i < (&value_vector)->num_elems; i++) {                                                      \
            const struct vector ofType(TYPE) *values = vec_get(&value_vector, i, struct vector);               \
            fprintf(file, "[ ");                                                                                       \
            for (size_t j = 0; j < values->num_elems; j++) {                                                           \
                TYPE value = *vec_get(values, j, TYPE);                                                      \
                if (nonnull_expr) {                                                                                    \
                    fprintf(file, "" TYPE_FORMAT "%s", value, j + 1 < values->num_elems ? ", " : "");                  \
                } else {                                                                                               \
                    fprintf(file, NG5_NULL_TEXT "%s", j + 1 < values->num_elems ? ", " : "");                       \
                }                                                                                                      \
            }                                                                                                          \
            fprintf(file, "]%s ", i + 1 < (&value_vector)->num_elems ? "," : "");                                      \
        }                                                                                                              \
        fprintf(file, "]");                                                                                            \
    }                                                                                                                  \
    fprintf(file, "}, ");                                                                                              \
}

#define PRINT_BOOLEAN_ARRAY(file, type_name, key_vector, value_vector)                                                 \
{                                                                                                                      \
    fprintf(file, "\"%s\": { ", "Boolean");                                                                            \
    if(!carbon_vec_is_empty((&key_vector))) {                                                                          \
        fprintf(file, "\"Keys\": [ ");                                                                                 \
        for (size_t i = 0; i < (&key_vector)->num_elems; i++) {                                                        \
            carbon_string_id_t string_id = *vec_get((&key_vector), i, carbon_string_id_t);                   \
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (&key_vector)->num_elems ? ", " : "");                     \
        }                                                                                                              \
        fprintf(file, "], ");                                                                                          \
        fprintf(file, "\"Keys Decoded\": [ ");                                                                         \
        for (size_t i = 0; i < (&key_vector)->num_elems; i++) {                                                        \
            carbon_string_id_t string_id = *vec_get((&key_vector), i, carbon_string_id_t);                   \
            char **encString = carbon_strdic_extract(dic, &string_id, 1);                                              \
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (&key_vector)->num_elems ? ", " : "");                     \
            carbon_strdic_free(dic, encString);                                                                        \
        }                                                                                                              \
        fprintf(file, "],");                                                                                           \
        fprintf(file, "\"Values\": [ ");                                                                               \
        for (size_t i = 0; i < (&value_vector)->num_elems; i++) {                                                      \
            const struct vector ofType(carbon_boolean_t) *values = vec_get(&value_vector, i, struct vector);      \
            fprintf(file, "[ ");                                                                                       \
            for (size_t j = 0; j < values->num_elems; j++) {                                                           \
                carbon_boolean_t value = *vec_get(values, j, carbon_boolean_t);                                    \
                fprintf(file, "%s%s", value == 0 ? "false" : "true", j + 1 < values->num_elems ? ", " : "");           \
            }                                                                                                          \
            fprintf(file, "]%s ", i + 1 < (&value_vector)->num_elems ? "," : "");                                      \
        }                                                                                                              \
        fprintf(file, "]");                                                                                            \
    }                                                                                                                  \
    fprintf(file, "}, ");                                                                                              \
}

static void print_array_null(FILE *file, const char *type_name, const struct vector ofType(carbon_string_id_t) *key_vector,
                           const struct vector ofType(u16) *value_vector, struct strdic *dic)
{
    fprintf(file, "\"%s\": { ", type_name);
    if(!carbon_vec_is_empty((key_vector))) {
        fprintf(file, "\"Keys\": [ ");
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {
            carbon_string_id_t string_id = *vec_get((key_vector), i, carbon_string_id_t);
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (key_vector)->num_elems ? ", " : "");
        }
        fprintf(file, "], ");
        fprintf(file, "\"Keys Decoded\": [ ");
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {
            carbon_string_id_t string_id = *vec_get((key_vector), i, carbon_string_id_t);
            char **encString = carbon_strdic_extract(dic, &string_id, 1);
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (key_vector)->num_elems ? ", " : "");
            carbon_strdic_free(dic, encString);
        }
        fprintf(file, "],");
        fprintf(file, "\"Values\": [ ");
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {
            u16 amount = *vec_get(value_vector, i, u16);
            fprintf(file, "%d%s", amount, i + 1 < value_vector->num_elems ? ", " : "");
        }
        fprintf(file, "]");
    }
    fprintf(file, "}, ");
}

static void print_array_strings(FILE *file, const char *type_name, const struct vector ofType(carbon_string_id_t) *key_vector,
                           const struct vector ofType(Vector ofType(carbon_string_id_t)) *value_vector, struct strdic *dic)
{
    fprintf(file, "\"%s\": { ", type_name);
    if(!carbon_vec_is_empty((key_vector))) {
        fprintf(file, "\"Keys\": [ ");
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {
            carbon_string_id_t string_id = *vec_get((key_vector), i, carbon_string_id_t);
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (key_vector)->num_elems ? ", " : "");
        }
        fprintf(file, "], ");
        fprintf(file, "\"Keys Decoded\": [ ");
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {
            carbon_string_id_t string_id_t = *vec_get((key_vector), i, carbon_string_id_t);
            char **encString = carbon_strdic_extract(dic, &string_id_t, 1);
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (key_vector)->num_elems ? ", " : "");
            carbon_strdic_free(dic, encString);
        }
        fprintf(file, "],");
        fprintf(file, "\"Values\": [ ");
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {
            const struct vector ofType(carbon_string_id_t) *values = vec_get(value_vector, i, struct vector);
            fprintf(file, "[");
            for (size_t j = 0; j < values->num_elems; j++) {
                carbon_string_id_t value = *vec_get(values, j, carbon_string_id_t);
                fprintf(file, "%"PRIu64"%s", value, j + 1 < values->num_elems ? ", " : "");
            }
            fprintf(file, "]%s", i + 1 < (value_vector)->num_elems ? ", " : "");

        }
        fprintf(file, "], ");
        fprintf(file, "\"Values Decoded\": [ ");
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {
            const struct vector ofType(carbon_string_id_t) *values = vec_get(value_vector, i, struct vector);
            fprintf(file, "[");
            for (size_t j = 0; j < values->num_elems; j++) {
                carbon_string_id_t value = *vec_get(values, j, carbon_string_id_t);

                if (NG5_LIKELY(value != NG5_NULL_ENCODED_STRING)) {
                    char **decoded = carbon_strdic_extract(dic, &value, 1);
                    fprintf(file, "\"%s\"%s", *decoded, j + 1 < values->num_elems ? ", " : "");
                    carbon_strdic_free(dic, decoded);
                } else {
                    fprintf(file, "null%s", j + 1 < values->num_elems ? ", " : "");
                }

            }
            fprintf(file, "]%s", i + 1 < (value_vector)->num_elems ? ", " : "");

        }
        fprintf(file, "]");
    }
    fprintf(file, "}, ");
}

static void print_primitive_strings(FILE *file, const char *type_name, const struct vector ofType(carbon_string_id_t) *key_vector,
                                  const struct vector ofType(carbon_string_id_t) *value_vector, struct strdic *dic)
{
    PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, ", ")
    if(!carbon_vec_is_empty((key_vector))) {
        fprintf(file, "\"Values\": [ ");
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {
            carbon_string_id_t string_id_t = *vec_get(value_vector, i, carbon_string_id_t);
            fprintf(file, "%"PRIu64"%s", string_id_t, i + 1 < (value_vector)->num_elems ? ", " : "");
        }
        fprintf(file, "], ");
        fprintf(file, "\"Values Decoded\": [ ");
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {
            carbon_string_id_t string_id_t = *vec_get(value_vector, i, carbon_string_id_t);
            char **values = carbon_strdic_extract(dic, &string_id_t, 1);
            fprintf(file, "\"%s\"%s", *values, i + 1 < (value_vector)->num_elems ? ", " : "");
            carbon_strdic_free(dic, values);
        }
        fprintf(file, "]");
    }
    fprintf(file, "}, ");

}

#define PRINT_COLUMN(file, columnTable, array_idx, type, format_string)                                                \
{                                                                                                                      \
    const struct vector *column = vec_get(&columnTable->values, array_idx, struct vector);                     \
    fprintf(file, "%s", column->num_elems > 1 ? "[" : "");                                                             \
    for (size_t i = 0; i < column->num_elems; i++) {                                                                   \
        fprintf(file, format_string, *vec_get(column, i, type));                                             \
        fprintf(file, "%s", i + 1 < column->num_elems ? ", " : "");                                                    \
    }                                                                                                                  \
    fprintf(file, "%s", column->num_elems > 1 ? "]" : "");                                                             \
}

static bool print_array_objects(FILE *file, struct err *err, const char *type_name, const struct vector ofType(carbon_columndoc_columngroup_t) *key_columns,
                              struct strdic *dic)
{
    fprintf(file, "\"%s\": {", type_name);
    fprintf(file, "\"Keys\": [");
    for (size_t array_key_idx = 0; array_key_idx < key_columns->num_elems; array_key_idx++) {
        const carbon_columndoc_columngroup_t *arrayKeyColumns = vec_get(key_columns, array_key_idx, carbon_columndoc_columngroup_t);
        fprintf(file, "%"PRIu64"%s", arrayKeyColumns->key, array_key_idx + 1 < key_columns->num_elems ? ", " : "");
    }
    fprintf(file, "], \"Keys Decoded\": [");
    for (size_t array_key_idx = 0; array_key_idx < key_columns->num_elems; array_key_idx++) {
        const carbon_columndoc_columngroup_t *arrayKeyColumns = vec_get(key_columns, array_key_idx, carbon_columndoc_columngroup_t);
        carbon_string_id_t encKeyName = arrayKeyColumns->key;
        char **decKeyName = carbon_strdic_extract(dic, &encKeyName, 1);
        fprintf(file, "\"%s\"%s", *decKeyName, array_key_idx + 1 < key_columns->num_elems ? ", " : "");
        carbon_strdic_free(dic, decKeyName);
    }
    fprintf(file, "], ");
    fprintf(file, "\"Tables\": [");
    for (size_t array_key_idx = 0; array_key_idx < key_columns->num_elems; array_key_idx++) {
        fprintf(file, "[");
        const carbon_columndoc_columngroup_t *arrayKeyColumns = vec_get(key_columns, array_key_idx, carbon_columndoc_columngroup_t);
        for (size_t columnIdx = 0; columnIdx < arrayKeyColumns->columns.num_elems; columnIdx++) {
            fprintf(file, "{");
            const carbon_columndoc_column_t *columnTable = vec_get(&arrayKeyColumns->columns, columnIdx, carbon_columndoc_column_t);
            char **decColumnKeyName = carbon_strdic_extract(dic, &columnTable->key_name, 1);

            const char *column_type_name = get_type_name(err, columnTable->type);
            if (!column_type_name) {
                return false;
            }

            fprintf(file, "\"Column Name\": %"PRIu64", "
                          "\"Column Name Decoded\": \"%s\", "
                          "\"Unique Column Name Decoded\": \"{'%s'}$%s\", "
                          "\"Type\": \"%s\",",
                          columnTable->key_name,
                          *decColumnKeyName,
                          *decColumnKeyName,
                          column_type_name,
                          column_type_name);

            fprintf(file, "\"Values\": [");
            for (size_t array_idx = 0; array_idx < columnTable->values.num_elems; array_idx++) {
                switch (columnTable->type) {
                case field_null: {
                    const struct vector *column = vec_get(&columnTable->values, array_idx, struct vector);
                    fprintf(file, "%s", column->num_elems > 1 ? "[" : "");
                    for (size_t i = 0; i < column->num_elems; i++) {
                        fprintf(file, "null");
                        fprintf(file, "%s", i + 1 < column->num_elems ? ", " : "");
                    }
                    fprintf(file, "%s", column->num_elems > 1 ? "]" : "");
                } break;
                case field_int8:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_i8, "%d")
                    break;
                case field_int16:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_i16, "%d")
                    break;
                case field_int32:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_i32, "%d")
                    break;
                case field_int64:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_i64, "%" PRIi64)
                    break;
                case field_uint8:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_u8, "%d")
                    break;
                case field_uint16:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_u16, "%d")
                    break;
                case field_uint32:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_u32, "%d")
                    break;
                case field_uint64:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_u64, "%" PRIu64)
                    break;
                case field_float:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_number_t , "%f")
                    break;
                case field_string: {
                    const struct vector *column = vec_get(&columnTable->values, array_idx, struct vector);
                    fprintf(file, "%s", column->num_elems > 1 ? "[" : "");
                    for (size_t i = 0; i < column->num_elems; i++) {
                        carbon_string_id_t encodedString = *vec_get(column, i, carbon_string_id_t);
                        char **decodedString = carbon_strdic_extract(dic, &encodedString, 1);
                        fprintf(file, "{\"Encoded\": %"PRIu64", \"Decoded\": \"%s\"}", encodedString, *decodedString);
                        fprintf(file, "%s", i + 1 < column->num_elems ? ", " : "");
                        carbon_strdic_free(dic, decodedString);
                    }
                    fprintf(file, "%s", column->num_elems > 1 ? "]" : "");
                } break;
                case field_object: {
                   // columndoc_obj_t *doc = vec_get(&column->values, valueIdx, columndoc_obj_t);
                  //  print_object(file, doc, encode);
                    const struct vector *column = vec_get(&columnTable->values, array_idx, struct vector);
                    fprintf(file, "%s", column->num_elems > 1 ? "[" : "");
                    for (size_t i = 0; i < column->num_elems; i++) {
                        const columndoc_obj_t *object = vec_get(column, i, columndoc_obj_t);
                        if(!print_object(file, err, object, dic)) {
                            return false;
                        }
                        fprintf(file, "%s", i + 1 < column->num_elems ? ", " : "");
                    }
                    fprintf(file, "%s", column->num_elems > 1 ? "]" : "");
                } break;
                default:
                    error(err, NG5_ERR_NOTYPE)
                    return false;
                }
                fprintf(file, array_idx + 1 < columnTable->values.num_elems ? ", " : "");
            }
            fprintf(file, "],");
            fprintf(file, "\"Positions\": [");
            for (size_t positionIdx = 0; positionIdx < columnTable->array_positions.num_elems; positionIdx++) {
                fprintf(file, "%d%s", *vec_get(&columnTable->array_positions, positionIdx, i16), (positionIdx + 1 < columnTable->array_positions.num_elems ? ", " : ""));
            }
            fprintf(file, "]");
            carbon_strdic_free(dic, decColumnKeyName);
            fprintf(file, "}%s", columnIdx + 1 < arrayKeyColumns->columns.num_elems ? ", " : "");
        }
        fprintf(file, "]%s", array_key_idx + 1 < key_columns->num_elems ? ", " : "");
    }
    fprintf(file, "]");

    fprintf(file, "}");
    return true;
}

static bool print_object(FILE *file, struct err *err, const columndoc_obj_t *object, struct strdic *dic)
{
    char **parentKey = carbon_strdic_extract(dic, &object->parent_key, 1);
    fprintf(file, "{ ");
    fprintf(file, "\"Parent\": { \"Key\": %"PRIu64", \"Key Decoded\": \"%s\", \"Index\": %zu }, ", object->parent_key, parentKey[0], object->index);
    fprintf(file, "\"Pairs\": { ");
        fprintf(file, "\"Primitives\": { ");
            PRINT_PRIMITIVE_BOOLEAN_COLUMN(file, "Boolean", &object->bool_prop_keys, &object->bool_prop_vals, dic)
            PRINT_PRIMITIVE_COLUMN(file, "UInt8", &object->uint8_prop_keys, &object->uint8_prop_vals, &object->uint8_val_idxs, dic, carbon_u8, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "UInt16", &object->uint16_prop_keys, &object->uint16_prop_vals, &object->uint16_val_idxs, dic, carbon_u16, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "UInt32", &object->uin32_prop_keys, &object->uint32_prop_vals, &object->uint32_val_idxs, dic, carbon_u32, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "UInt64", &object->uint64_prop_keys, &object->uint64_prop_vals, &object->uint64_val_idxs, dic, carbon_u64, "%" PRIu64)
            PRINT_PRIMITIVE_COLUMN(file, "Int8", &object->int8_prop_keys, &object->int8_prop_vals, &object->int8_val_idxs, dic, carbon_i8, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "Int16", &object->int16_prop_keys, &object->int16_prop_vals, &object->int16_val_idxs, dic, carbon_i16, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "Int32", &object->int32_prop_keys, &object->int32_prop_vals, &object->int32_val_idxs, dic, carbon_i32, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "Int64", &object->int64_prop_keys, &object->int64_prop_vals, &object->int64_val_idxs, dic, carbon_i64, "%" PRIi64)
            PRINT_PRIMITIVE_COLUMN(file, "Real", &object->float_prop_keys, &object->float_prop_vals, &object->float_val_idxs, dic, carbon_number_t, "%f")
            print_primitive_strings(file, "Strings", &object->string_prop_keys, &object->string_prop_vals, dic);
            print_primitive_null(file, "Null", &object->null_prop_keys, dic);
            if(print_primitive_objects(file, err, "Objects", &object->obj_prop_keys, &object->obj_prop_vals, dic)) {
                return false;
            }
            fprintf(file, "}, ");
        fprintf(file, "\"Arrays\": { ");
            PRINT_BOOLEAN_ARRAY(file, "Boolean", object->bool_array_prop_keys, object->bool_array_prop_vals);
            PRINT_ARRAY(file, "UInt8", object->uint8_array_prop_keys, object->uint8_array_prop_vals, carbon_u8, "%d", (value != NG5_NULL_UINT8));
            PRINT_ARRAY(file, "UInt16", object->uint16_array_prop_keys, object->uint16_array_prop_vals, carbon_u16, "%d", (value !=NG5_NULL_UINT16));
            PRINT_ARRAY(file, "UInt32", object->uint32_array_prop_keys, object->uint32_array_prop_vals, carbon_u32, "%d", (value !=NG5_NULL_UINT32));
            PRINT_ARRAY(file, "UInt64", object->uint64_array_prop_keys, object->ui64_array_prop_vals, carbon_u64, "%" PRIu64, (value !=NG5_NULL_UINT64));
            PRINT_ARRAY(file, "Int8", object->int8_array_prop_keys, object->int8_array_prop_vals, carbon_i8, "%d", (value !=NG5_NULL_INT8));
            PRINT_ARRAY(file, "Int16", object->int16_array_prop_keys, object->int16_array_prop_vals, carbon_i16, "%d", (value !=NG5_NULL_INT16));
            PRINT_ARRAY(file, "Int32", object->int32_array_prop_keys, object->int32_array_prop_vals, carbon_i32, "%d", (value !=NG5_NULL_INT32));
            PRINT_ARRAY(file, "Int64", object->int64_array_prop_keys, object->int64_array_prop_vals, carbon_i64, "%" PRIi64, (value !=NG5_NULL_INT64));
            PRINT_ARRAY(file, "Real", object->float_array_prop_keys, object->float_array_prop_vals, carbon_number_t, "%f", (!isnan(value)));
            print_array_strings(file, "Strings", &object->string_array_prop_keys, &object->string_array_prop_vals, dic);
            print_array_null(file, "Null", &object->null_array_prop_keys, &object->null_array_prop_vals, dic);
            if(!print_array_objects(file, err, "Objects", &object->obj_array_props, dic)) {
                return false;
            }
        fprintf(file, "} ");
    fprintf(file, " }");
    fprintf(file, " }");
    carbon_strdic_free(dic, parentKey);
    return true;
}

bool carbon_columndoc_print(FILE *file, carbon_columndoc_t *doc)
{
    NG5_NON_NULL_OR_ERROR(file)
    NG5_NON_NULL_OR_ERROR(doc)
    return print_object(file, &doc->err, &doc->columndoc, doc->dic);
}

bool carbon_columndoc_drop(carbon_columndoc_t *doc)
{
    NG5_UNUSED(doc);
    NG5_NOT_IMPLEMENTED
}

static void object_array_key_columns_create(struct vector ofType(carbon_columndoc_columngroup_t) *columns)
{
    carbon_vec_create(columns, NULL, sizeof(carbon_columndoc_columngroup_t), 20000);
}

static void object_array_key_columns_drop(struct vector ofType(carbon_columndoc_columngroup_t) *columns)
{
    for (size_t i = 0; i < columns->num_elems; i++) {
        carbon_columndoc_columngroup_t *array_columns = vec_get(columns, i, carbon_columndoc_columngroup_t);
        for (size_t j = 0; j < array_columns->columns.num_elems; j++) {


            carbon_columndoc_column_t *column = vec_get(&array_columns->columns, j, carbon_columndoc_column_t);

            struct vector ofType(u32) *array_indices = &column->array_positions;
            struct vector ofType(struct vector ofType(<T>)) *values_for_indicies = &column->values;

            assert (array_indices->num_elems == values_for_indicies->num_elems);

            for (size_t k = 0; k < array_indices->num_elems; k++) {

                struct vector ofType(<T>) *values_for_index = vec_get(values_for_indicies, k, struct vector);
                if (column->type == field_object) {
                    for (size_t l = 0; l < values_for_index->num_elems; l++) {
                        columndoc_obj_t *nested_object = vec_get(values_for_index, l, columndoc_obj_t);
                        object_meta_model_free(nested_object);
                    }
                }
                carbon_vec_drop(values_for_index);
            }

            carbon_vec_drop(array_indices);
            carbon_vec_drop(values_for_indicies);
        }
        carbon_vec_drop(&array_columns->columns);
    }
    carbon_vec_drop(columns);
}

static const char *get_type_name(struct err *err, field_e type)
{
    switch (type) {
        case field_null:return "Null";
        case field_int8:return "Int8";
        case field_int16:return "Int16";
        case field_int32:return "Int32";
        case field_int64:return "Int64";
        case field_uint8:return "UInt8";
        case field_uint16:return "UInt16";
        case field_uint32:return "UInt32";
        case field_uint64:return "UInt64";
        case field_float:return "Real";
        case field_string:return "String";
        case field_object:return "Object";
        default: {
            error(err, NG5_ERR_NOTYPE);
            return NULL;
        }
    }
}

static carbon_columndoc_column_t *object_array_key_columns_find_or_new(struct vector ofType(carbon_columndoc_columngroup_t) *columns,
                                                            carbon_string_id_t array_key, carbon_string_id_t nested_object_entry_key,
                                                            field_e nested_object_entry_type)
{
    carbon_columndoc_columngroup_t *key_columns;
    carbon_columndoc_column_t *key_column, *new_column;

    for (size_t i = 0; i < columns->num_elems; i++) {
        /** Find object array pair having the key `key` */
        key_columns = vec_get(columns, i, carbon_columndoc_columngroup_t);
        if (key_columns->key == array_key) {
            /** In case such a pair is found, find column that matches the desired type */
            for (size_t j = 0; j < key_columns->columns.num_elems; j++) {
                key_column = vec_get(&key_columns->columns, j, carbon_columndoc_column_t);
                if (key_column->key_name == nested_object_entry_key && key_column->type == nested_object_entry_type) {
                    /** Column for the object array with the desired key, the nested object entry with the desired key
                     * and a matching type is found */
                    return key_column;
                }
            }
            /** In this case, the requested array_key is found, but the nested object entry does not match, hence
             * create a new one */
            goto objectArrayKeyColumnsNewColumn;
        }
    }
    /** In this case, the array key is also not known. Create a new one array entry with the fitting key column and
     * return that newly created column */
    key_columns = VECTOR_NEW_AND_GET(columns, carbon_columndoc_columngroup_t);
    key_columns->key = array_key;
    carbon_vec_create(&key_columns->columns, NULL, sizeof(carbon_columndoc_column_t), 10);

objectArrayKeyColumnsNewColumn:
    new_column = VECTOR_NEW_AND_GET(&key_columns->columns, carbon_columndoc_column_t);
    new_column->key_name = nested_object_entry_key;
    new_column->type = nested_object_entry_type;
    carbon_vec_create(&new_column->values, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&new_column->array_positions, NULL, sizeof(u32), 10);

    return new_column;
}

static bool object_array_key_column_push(carbon_columndoc_column_t *col, struct err *err, const carbon_doc_entries_t *entry, u32 array_idx,
                                     struct strdic *dic, columndoc_obj_t *model)
{
    assert(col->type == entry->type);

    u32 *entry_array_idx = VECTOR_NEW_AND_GET(&col->array_positions, u32);
    *entry_array_idx = array_idx;

    struct vector ofType(<T>) *values_for_entry = VECTOR_NEW_AND_GET(&col->values, struct vector);
    carbon_vec_create(values_for_entry, NULL, GET_TYPE_SIZE(entry->type), entry->values.num_elems);

    bool is_null_by_def = entry->values.num_elems == 0;
    u32 num_elements = (u32) entry->values.num_elems;

    field_e entryType = is_null_by_def ? field_null : entry->type;
    num_elements = is_null_by_def ? 1 : num_elements;

    switch (entryType) {
    case field_null: {
        carbon_vec_push(values_for_entry, &num_elements, 1);
    } break;
    case field_bool:
    case field_int8:
    case field_int16:
    case field_int32:
    case field_int64:
    case field_uint8:
    case field_uint16:
    case field_uint32:
    case field_uint64:
    case field_float:
        assert(!is_null_by_def);
        carbon_vec_push(values_for_entry, entry->values.base, num_elements);
        break;
    case field_string: {
        assert(!is_null_by_def);
        char **strings = vec_all(&entry->values, char *);
        carbon_string_id_t *string_ids;
        carbon_strdic_locate_fast(&string_ids, dic, (char *const *) strings, num_elements);
        carbon_vec_push(values_for_entry, string_ids, num_elements);
        carbon_strdic_free(dic, string_ids);
        //carbon_strdic_free(encode, strings);
    } break;
    case field_object:
        assert(!is_null_by_def);

        carbon_string_id_t *array_key;
        carbon_strdic_locate_fast(&array_key, dic, (char *const *) &entry->key, 1);

        for (size_t array_idx = 0; array_idx < num_elements; array_idx++)
        {
            columndoc_obj_t *nested_object = VECTOR_NEW_AND_GET(values_for_entry, columndoc_obj_t);
            setup_object(nested_object, model->parent, *array_key, array_idx);
            if (!import_object(nested_object, err, vec_get(&entry->values, array_idx, carbon_doc_obj_t), dic)) {
                return false;
            }
        }
        carbon_strdic_free(dic, array_key);
        break;
    default:
        error(err, NG5_ERR_NOTYPE);
        return false;
    }
    return true;
}

static void setup_object(columndoc_obj_t *model, carbon_columndoc_t *parent, carbon_string_id_t key, size_t idx)
{
    model->parent = parent;
    model->parent_key = key;
    model->index = idx;

    carbon_vec_create(&model->bool_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->int8_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->int16_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->int32_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->int64_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->uint8_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->uint16_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->uin32_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->uint64_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->string_prop_keys, NULL, sizeof(carbon_string_id_t), 50);
    carbon_vec_create(&model->float_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->null_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->obj_prop_keys, NULL, sizeof(carbon_string_id_t), 10);

    carbon_vec_create(&model->bool_array_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->int8_array_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->int16_array_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->int32_array_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->int64_array_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->uint8_array_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->uint16_array_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->uint32_array_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->uint64_array_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->string_array_prop_keys, NULL, sizeof(carbon_string_id_t), 50);
    carbon_vec_create(&model->float_array_prop_keys, NULL, sizeof(carbon_string_id_t), 10);
    carbon_vec_create(&model->null_array_prop_keys, NULL, sizeof(carbon_string_id_t), 10);

    carbon_vec_create(&model->bool_prop_vals, NULL, sizeof(carbon_boolean_t), 10);
    carbon_vec_create(&model->int8_prop_vals, NULL, sizeof(carbon_i8), 10);
    carbon_vec_create(&model->int16_prop_vals, NULL, sizeof(carbon_i16), 10);
    carbon_vec_create(&model->int32_prop_vals, NULL, sizeof(carbon_i32), 10);
    carbon_vec_create(&model->int64_prop_vals, NULL, sizeof(carbon_i64), 10);
    carbon_vec_create(&model->uint8_prop_vals, NULL, sizeof(carbon_u8), 10);
    carbon_vec_create(&model->uint16_prop_vals, NULL, sizeof(carbon_u16), 10);
    carbon_vec_create(&model->uint32_prop_vals, NULL, sizeof(carbon_u32), 10);
    carbon_vec_create(&model->uint64_prop_vals, NULL, sizeof(carbon_u64), 10);
    carbon_vec_create(&model->float_prop_vals, NULL, sizeof(carbon_number_t), 10);
    carbon_vec_create(&model->string_prop_vals, NULL, sizeof(carbon_string_id_t), 50);

    carbon_vec_create(&model->bool_array_prop_vals, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->int8_array_prop_vals, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->int16_array_prop_vals, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->int32_array_prop_vals, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->int64_array_prop_vals, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->uint8_array_prop_vals, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->uint16_array_prop_vals, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->uint32_array_prop_vals, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->ui64_array_prop_vals, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->float_array_prop_vals, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->string_array_prop_vals, NULL, sizeof(struct vector), 50);
    carbon_vec_create(&model->null_array_prop_vals, NULL, sizeof(u16), 10);

    carbon_vec_create(&model->bool_val_idxs, NULL, sizeof(u32), 10);
    carbon_vec_create(&model->int8_val_idxs, NULL, sizeof(u32), 10);
    carbon_vec_create(&model->int16_val_idxs, NULL, sizeof(u32), 10);
    carbon_vec_create(&model->int32_val_idxs, NULL, sizeof(u32), 10);
    carbon_vec_create(&model->int64_val_idxs, NULL, sizeof(u32), 10);
    carbon_vec_create(&model->uint8_val_idxs, NULL, sizeof(u32), 10);
    carbon_vec_create(&model->uint16_val_idxs, NULL, sizeof(u32), 10);
    carbon_vec_create(&model->uint32_val_idxs, NULL, sizeof(u32), 10);
    carbon_vec_create(&model->uint64_val_idxs, NULL, sizeof(u32), 10);
    carbon_vec_create(&model->float_val_idxs, NULL, sizeof(u32), 10);
    carbon_vec_create(&model->string_val_idxs, NULL, sizeof(u32), 50);

    carbon_vec_create(&model->bool_array_idxs, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->int8_array_idxs, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->int16_array_idxs, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->int32_array_idxs, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->int64_array_idxs, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->uint8_array_idxs, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->uint16_array_idxs, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->uint32_array_idxs, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->uint64_array_idxs, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->float_array_idxs, NULL, sizeof(struct vector), 10);
    carbon_vec_create(&model->string_array_idxs, NULL, sizeof(struct vector), 50);

    carbon_vec_create(&model->obj_prop_vals, NULL, sizeof(columndoc_obj_t), 10);

    object_array_key_columns_create(&model->obj_array_props);
}

static bool object_put_primitive(columndoc_obj_t *columndoc, struct err *err, const carbon_doc_entries_t *entry, struct strdic *dic,
                               const carbon_string_id_t *key_id)
{
    switch(entry->type) {
    case field_null:
        carbon_vec_push(&columndoc->null_prop_keys, key_id, 1);
        break;
    case field_bool:
        carbon_vec_push(&columndoc->bool_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->bool_prop_vals, entry->values.base, 1);
        break;
    case field_int8:
        carbon_vec_push(&columndoc->int8_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->int8_prop_vals, entry->values.base, 1);
        break;
    case field_int16:
        carbon_vec_push(&columndoc->int16_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->int16_prop_vals, entry->values.base, 1);
        break;
    case field_int32:
        carbon_vec_push(&columndoc->int32_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->int32_prop_vals, entry->values.base, 1);
        break;
    case field_int64:
        carbon_vec_push(&columndoc->int64_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->int64_prop_vals, entry->values.base, 1);
        break;
    case field_uint8:
        carbon_vec_push(&columndoc->uint8_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->uint8_prop_vals, entry->values.base, 1);
        break;
    case field_uint16:
        carbon_vec_push(&columndoc->uint16_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->uint16_prop_vals, entry->values.base, 1);
        break;
    case field_uint32:
        carbon_vec_push(&columndoc->uin32_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->uint32_prop_vals, entry->values.base, 1);
        break;
    case field_uint64:
        carbon_vec_push(&columndoc->uint64_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->uint64_prop_vals, entry->values.base, 1);
        break;
    case field_float:
        carbon_vec_push(&columndoc->float_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->float_prop_vals, entry->values.base, 1);
        break;
    case field_string: {
        carbon_string_id_t *value;
        carbon_strdic_locate_fast(&value, dic, (char *const *) entry->values.base, 1);
        carbon_vec_push(&columndoc->string_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->string_prop_vals, value, 1);
        carbon_strdic_free(dic, value);
    } break;
    case field_object: {
        columndoc_obj_t template, *nested_object;
        size_t position = carbon_vec_length(&columndoc->obj_prop_keys);
        carbon_vec_push(&columndoc->obj_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->obj_prop_vals, &template, 1);
        nested_object = vec_get(&columndoc->obj_prop_vals, position, columndoc_obj_t);
        setup_object(nested_object, columndoc->parent, *key_id, 0);
        if (!import_object(nested_object, err, vec_get(&entry->values, 0, carbon_doc_obj_t), dic)) {
            return false;
        }
    } break;
        error(err, NG5_ERR_NOTYPE)
        return false;
    }
    return true;
}

static void object_push_array(struct vector ofType(Vector ofType(<T>)) *values, size_t TSize, u32 num_elements,
                            const void *data, carbon_string_id_t key_id, struct vector ofType(carbon_string_id_t) *key_vector)
{
    struct vector ofType(<T>) template, *vector;
    size_t idx = carbon_vec_length(values);
    carbon_vec_push(values, &template, 1);
    vector = vec_get(values, idx, struct vector);
    carbon_vec_create(vector, NULL, TSize, num_elements);
    carbon_vec_push(vector, data, num_elements);
    carbon_vec_push(key_vector, &key_id, 1);
}

static bool object_put_array(columndoc_obj_t *model, struct err *err, const carbon_doc_entries_t *entry, struct strdic *dic, const carbon_string_id_t *key_id)
{
    // TODO: format for array, sort by keys, sort by values!
    NG5_UNUSED(dic);
    u32 num_elements = (u32) carbon_vec_length(&entry->values);

    switch(entry->type) {
    case field_null: {
        carbon_vec_push(&model->null_array_prop_vals, &num_elements, 1);
        carbon_vec_push(&model->null_array_prop_keys, key_id, 1);
    }
        break;
    case field_bool:
        object_push_array(&model->bool_array_prop_vals, sizeof(carbon_boolean_t), num_elements, entry->values.base, *key_id,
                        &model->bool_array_prop_keys);
        break;
    case field_int8:
        object_push_array(&model->int8_array_prop_vals, sizeof(carbon_i8), num_elements, entry->values.base, *key_id,
                        &model->int8_array_prop_keys);
        break;
    case field_int16:
        object_push_array(&model->int16_array_prop_vals, sizeof(carbon_i16), num_elements, entry->values.base, *key_id,
                        &model->int16_array_prop_keys);
        break;
    case field_int32:
        object_push_array(&model->int32_array_prop_vals, sizeof(carbon_i32), num_elements, entry->values.base, *key_id,
                        &model->int32_array_prop_keys);
        break;
    case field_int64:
        object_push_array(&model->int64_array_prop_vals, sizeof(carbon_i64), num_elements, entry->values.base, *key_id,
                        &model->int64_array_prop_keys);
        break;
    case field_uint8:
        object_push_array(&model->uint8_array_prop_vals, sizeof(carbon_u8), num_elements, entry->values.base, *key_id,
                        &model->uint8_array_prop_keys);
        break;
    case field_uint16:
        object_push_array(&model->uint16_array_prop_vals,
                        sizeof(carbon_u16),
                        num_elements,
                        entry->values.base,
                        *key_id,
                        &model->uint16_array_prop_keys);
        break;
    case field_uint32:
        object_push_array(&model->uint32_array_prop_vals, sizeof(carbon_u32), num_elements, entry->values.base, *key_id,
                        &model->uint32_array_prop_keys);
        break;
    case field_uint64:
        object_push_array(&model->ui64_array_prop_vals, sizeof(carbon_u64), num_elements, entry->values.base, *key_id,
                        &model->uint64_array_prop_keys);
        break;
    case field_float:
        object_push_array(&model->float_array_prop_vals, sizeof(carbon_number_t), num_elements, entry->values.base, *key_id,
                        &model->float_array_prop_keys);
        break;
    case field_string: {
        const char **strings = vec_all(&entry->values, const char *);
        carbon_string_id_t *string_ids;
        carbon_strdic_locate_fast(&string_ids, dic, (char *const *) strings, num_elements);
        object_push_array(&model->string_array_prop_vals,
                        sizeof(carbon_string_id_t),
                        num_elements,
                        string_ids,
                        *key_id,
                        &model->string_array_prop_keys);
        carbon_strdic_free(dic, string_ids);
    }
        break;
    case field_object: {
        carbon_string_id_t *nested_object_key_name;
        for (u32 array_idx = 0; array_idx < num_elements; array_idx++) {
            const carbon_doc_obj_t *object = vec_get(&entry->values, array_idx, carbon_doc_obj_t);
            for (size_t pair_idx = 0; pair_idx < object->entries.num_elems; pair_idx++) {
                const carbon_doc_entries_t *pair = vec_get(&object->entries, pair_idx, carbon_doc_entries_t);
                carbon_strdic_locate_fast(&nested_object_key_name, dic, (char *const *) &pair->key, 1);
                carbon_columndoc_column_t *key_column = object_array_key_columns_find_or_new(&model->obj_array_props, *key_id,
                                                                                      *nested_object_key_name, pair->type);
                if (!object_array_key_column_push(key_column, err, pair, array_idx, dic, model)) {
                    return false;
                }
                carbon_strdic_free(dic, nested_object_key_name);
            }
        }
    }
        break;
    default: {
        error(err, NG5_ERR_NOTYPE)
        return false;
    } break;
    }
    return true;
}

static bool object_put(columndoc_obj_t *model, struct err *err, const carbon_doc_entries_t *entry, struct strdic *dic)
{
    carbon_string_id_t *key_id;
    enum EntryType { ENTRY_TYPE_NULL, ENTRY_TYPE_PRIMITIVE, ENTRY_TYPE_ARRAY } entryType;

    carbon_strdic_locate_fast(&key_id, dic, (char *const *) &entry->key, 1);
    entryType = entry->values.num_elems == 0 ? ENTRY_TYPE_NULL :
                (entry->values.num_elems == 1 ? ENTRY_TYPE_PRIMITIVE : ENTRY_TYPE_ARRAY );

    switch (entryType) {
    case ENTRY_TYPE_NULL:
        /** For a key which does not carbon_parallel_map_exec to any value, the value is defined as 'null'  */
        carbon_vec_push(&model->null_prop_keys, key_id, 1);
        break;
    case ENTRY_TYPE_PRIMITIVE:
        if (!object_put_primitive(model, err, entry, dic, key_id)) {
            return false;
        }
        break;
    case ENTRY_TYPE_ARRAY:
        if (!object_put_array(model, err, entry, dic, key_id)) {
            return false;
        }
        break;
    default:
        error(err, NG5_ERR_NOTYPE)
        return false;
    }

    carbon_strdic_free(dic, key_id);
    return true;
}

static bool import_object(columndoc_obj_t *dst, struct err *err, const carbon_doc_obj_t *doc, struct strdic *dic)
{
    const struct vector ofType(carbon_doc_entries_t) *objectEntries = carbon_doc_get_entries(doc);
    const carbon_doc_entries_t *entries = vec_all(objectEntries, carbon_doc_entries_t);
    for (size_t i = 0; i < objectEntries->num_elems; i++) {
        const carbon_doc_entries_t *entry = entries + i;
        if (!object_put(dst, err, entry, dic)) {
            return false;
        }
    }
    return true;
}