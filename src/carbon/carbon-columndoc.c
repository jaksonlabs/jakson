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

#include "carbon/carbon-columndoc.h"
#include "carbon/carbon-doc.h"

static void setup_object(carbon_columndoc_obj_t *model, carbon_columndoc_t *parent, carbon_string_id_t key, size_t idx);

static bool object_put(carbon_columndoc_obj_t *model, carbon_err_t *err, const carbon_doc_entries_t *entry, carbon_strdic_t *dic);

static bool import_object(carbon_columndoc_obj_t *dst, carbon_err_t *err, const carbon_doc_obj_t *doc, carbon_strdic_t *dic);

static bool print_object(FILE *file, carbon_err_t *err, const carbon_columndoc_obj_t *object, carbon_strdic_t *dic);

static const char *get_type_name(carbon_err_t *err, carbon_field_type_e type);

static void object_array_key_columns_create(carbon_vec_t ofType(carbon_columndoc_columngroup_t) *columns);

static carbon_columndoc_column_t *object_array_key_columns_find_or_new(carbon_vec_t ofType(carbon_columndoc_columngroup_t) *columns,
                                                            carbon_string_id_t array_key, carbon_string_id_t nested_object_entry_key,
                                                            carbon_field_type_e nested_object_entry_type);

static bool object_array_key_column_push(carbon_columndoc_column_t *col, carbon_err_t *err, const carbon_doc_entries_t *entry, uint32_t array_idx,
                                     carbon_strdic_t *dic, carbon_columndoc_obj_t *model);

bool carbon_columndoc_create(carbon_columndoc_t *columndoc,
                             carbon_err_t *err,
                             const carbon_doc_t *doc,
                             const carbon_doc_bulk_t *bulk,
                             const carbon_doc_entries_t *entries,
                             carbon_strdic_t *dic)
{
    CARBON_NON_NULL_OR_ERROR(columndoc)
    CARBON_NON_NULL_OR_ERROR(doc)
    CARBON_NON_NULL_OR_ERROR(dic)
    CARBON_NON_NULL_OR_ERROR(bulk)

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

static void object_array_key_columns_drop(carbon_vec_t ofType(carbon_columndoc_columngroup_t) *columns);

static void object_meta_model_free(carbon_columndoc_obj_t *columndoc)
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
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->bool_array_prop_vals, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->bool_array_prop_vals);

    for (size_t i = 0; i < columndoc->int8_array_prop_vals.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->int8_array_prop_vals, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int8_array_prop_vals);

    for (size_t i = 0; i < columndoc->int16_array_prop_vals.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->int16_array_prop_vals, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int16_array_prop_vals);

    for (size_t i = 0; i < columndoc->int32_array_prop_vals.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->int32_array_prop_vals, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int32_array_prop_vals);

    for (size_t i = 0; i < columndoc->int64_array_prop_vals.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->int64_array_prop_vals, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int64_array_prop_vals);

    for (size_t i = 0; i < columndoc->uint8_array_prop_vals.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->uint8_array_prop_vals, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint8_array_prop_vals);

    for (size_t i = 0; i < columndoc->uint16_array_prop_vals.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->uint16_array_prop_vals, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint16_array_prop_vals);

    for (size_t i = 0; i < columndoc->uint32_array_prop_vals.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->uint32_array_prop_vals, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint32_array_prop_vals);

    for (size_t i = 0; i < columndoc->uin64_array_prop_vals.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->uin64_array_prop_vals, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uin64_array_prop_vals);

    for (size_t i = 0; i < columndoc->float_array_prop_vals.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->float_array_prop_vals, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->float_array_prop_vals);

    for (size_t i = 0; i < columndoc->string_array_prop_vals.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->string_array_prop_vals, i, carbon_vec_t);
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
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->bool_array_idxs, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->bool_array_idxs);

    for (size_t i = 0; i < columndoc->int8_array_idxs.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->int8_array_idxs, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int8_array_idxs);

    for (size_t i = 0; i < columndoc->int16_array_idxs.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->int16_array_idxs, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int16_array_idxs);

    for (size_t i = 0; i < columndoc->int32_array_idxs.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->int32_array_idxs, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int32_array_idxs);

    for (size_t i = 0; i < columndoc->int64_array_idxs.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->int64_array_idxs, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->int64_array_idxs);

    for (size_t i = 0; i < columndoc->uint8_array_idxs.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->uint8_array_idxs, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint8_array_idxs);

    for (size_t i = 0; i < columndoc->uint16_array_idxs.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->uint16_array_idxs, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint16_array_idxs);

    for (size_t i = 0; i < columndoc->uint32_array_idxs.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->uint32_array_idxs, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint32_array_idxs);

    for (size_t i = 0; i < columndoc->uint64_array_idxs.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->uint64_array_idxs, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->uint64_array_idxs);

    for (size_t i = 0; i < columndoc->float_array_idxs.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->float_array_idxs, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->float_array_idxs);

    for (size_t i = 0; i < columndoc->string_array_idxs.num_elems; i++) {
        carbon_vec_t *vec = CARBON_VECTOR_GET(&columndoc->string_array_idxs, i, carbon_vec_t);
        carbon_vec_drop(vec);
    }
    carbon_vec_drop(&columndoc->string_array_idxs);

    for (size_t i = 0; i < columndoc->obj_prop_vals.num_elems; i++) {
        carbon_columndoc_obj_t *object = CARBON_VECTOR_GET(&columndoc->obj_prop_vals, i, carbon_columndoc_obj_t);
        object_meta_model_free(object);
    }
    carbon_vec_drop(&columndoc->obj_prop_vals);

    object_array_key_columns_drop(&columndoc->obj_array_props);
}

bool carbon_columndoc_free(carbon_columndoc_t *doc)
{
    CARBON_NON_NULL_OR_ERROR(doc);
    object_meta_model_free(&doc->columndoc);
    return true;
}

#define PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, suffix)                                             \
{                                                                                                                      \
    fprintf(file, "\"%s\": { ", type_name);                                                                            \
    if(!carbon_vec_is_empty((key_vector))) {                                                                           \
        fprintf(file, "\"Keys\": [ ");                                                                                 \
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {                                                         \
            carbon_string_id_t string_id = *CARBON_VECTOR_GET((key_vector), i, carbon_string_id_t);                    \
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (key_vector)->num_elems ? ", " : "");                      \
        }                                                                                                              \
        fprintf(file, "], ");                                                                                          \
        fprintf(file, "\"Keys Decoded\": [ ");                                                                         \
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {                                                         \
            carbon_string_id_t string_id = *CARBON_VECTOR_GET((key_vector), i, carbon_string_id_t);                    \
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
            TYPE value = *CARBON_VECTOR_GET(value_vector, i, TYPE);                                                    \
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
            carbon_boolean_t value = *CARBON_VECTOR_GET(value_vector, i, carbon_boolean_t);                                  \
            fprintf(file, "%s%s", value == 0 ? "false" : "true", i + 1 < (value_vector)->num_elems ? ", " : "");       \
        }                                                                                                              \
        fprintf(file, "]");                                                                                            \
    }                                                                                                                  \
    fprintf(file, "}, ");                                                                                              \
}

static void print_primitive_null(FILE *file, const char *type_name, const carbon_vec_t ofType(carbon_string_id_t) *key_vector,
                               carbon_strdic_t *dic)
{
    PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, "")
    fprintf(file, "}, ");
}



static bool print_primitive_objects(FILE *file, carbon_err_t *err, const char *type_name, const carbon_vec_t ofType(carbon_string_id_t) *key_vector,
                                  const carbon_vec_t ofType(carbon_columndoc_obj_t) *value_vector, carbon_strdic_t *dic)
{
    PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, ", ")
    if(!carbon_vec_is_empty((key_vector))) {
        fprintf(file, "\"Values\": [ ");
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {
            const carbon_columndoc_obj_t *object = CARBON_VECTOR_GET(value_vector, i, carbon_columndoc_obj_t);
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
            carbon_string_id_t string_id = *CARBON_VECTOR_GET((&key_vector), i, carbon_string_id_t);                   \
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (&key_vector)->num_elems ? ", " : "");                     \
        }                                                                                                              \
        fprintf(file, "], ");                                                                                          \
        fprintf(file, "\"Keys Decoded\": [ ");                                                                         \
        for (size_t i = 0; i < (&key_vector)->num_elems; i++) {                                                        \
            carbon_string_id_t string_id = *CARBON_VECTOR_GET((&key_vector), i, carbon_string_id_t);                   \
            char **encString = carbon_strdic_extract(dic, &string_id, 1);                                              \
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (&key_vector)->num_elems ? ", " : "");                     \
            carbon_strdic_free(dic, encString);                                                                        \
        }                                                                                                              \
        fprintf(file, "],");                                                                                           \
        fprintf(file, "\"Values\": [ ");                                                                               \
        for (size_t i = 0; i < (&value_vector)->num_elems; i++) {                                                      \
            const carbon_vec_t ofType(TYPE) *values = CARBON_VECTOR_GET(&value_vector, i, carbon_vec_t);               \
            fprintf(file, "[ ");                                                                                       \
            for (size_t j = 0; j < values->num_elems; j++) {                                                           \
                TYPE value = *CARBON_VECTOR_GET(values, j, TYPE);                                                      \
                if (nonnull_expr) {                                                                                    \
                    fprintf(file, "" TYPE_FORMAT "%s", value, j + 1 < values->num_elems ? ", " : "");                  \
                } else {                                                                                               \
                    fprintf(file, CARBON_NULL_TEXT "%s", j + 1 < values->num_elems ? ", " : "");                       \
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
            carbon_string_id_t string_id = *CARBON_VECTOR_GET((&key_vector), i, carbon_string_id_t);                   \
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (&key_vector)->num_elems ? ", " : "");                     \
        }                                                                                                              \
        fprintf(file, "], ");                                                                                          \
        fprintf(file, "\"Keys Decoded\": [ ");                                                                         \
        for (size_t i = 0; i < (&key_vector)->num_elems; i++) {                                                        \
            carbon_string_id_t string_id = *CARBON_VECTOR_GET((&key_vector), i, carbon_string_id_t);                   \
            char **encString = carbon_strdic_extract(dic, &string_id, 1);                                              \
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (&key_vector)->num_elems ? ", " : "");                     \
            carbon_strdic_free(dic, encString);                                                                        \
        }                                                                                                              \
        fprintf(file, "],");                                                                                           \
        fprintf(file, "\"Values\": [ ");                                                                               \
        for (size_t i = 0; i < (&value_vector)->num_elems; i++) {                                                      \
            const carbon_vec_t ofType(carbon_boolean_t) *values = CARBON_VECTOR_GET(&value_vector, i, carbon_vec_t);      \
            fprintf(file, "[ ");                                                                                       \
            for (size_t j = 0; j < values->num_elems; j++) {                                                           \
                carbon_boolean_t value = *CARBON_VECTOR_GET(values, j, carbon_boolean_t);                                    \
                fprintf(file, "%s%s", value == 0 ? "false" : "true", j + 1 < values->num_elems ? ", " : "");           \
            }                                                                                                          \
            fprintf(file, "]%s ", i + 1 < (&value_vector)->num_elems ? "," : "");                                      \
        }                                                                                                              \
        fprintf(file, "]");                                                                                            \
    }                                                                                                                  \
    fprintf(file, "}, ");                                                                                              \
}

static void print_array_null(FILE *file, const char *type_name, const carbon_vec_t ofType(carbon_string_id_t) *key_vector,
                           const carbon_vec_t ofType(uint16_t) *value_vector, carbon_strdic_t *dic)
{
    fprintf(file, "\"%s\": { ", type_name);
    if(!carbon_vec_is_empty((key_vector))) {
        fprintf(file, "\"Keys\": [ ");
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {
            carbon_string_id_t string_id = *CARBON_VECTOR_GET((key_vector), i, carbon_string_id_t);
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (key_vector)->num_elems ? ", " : "");
        }
        fprintf(file, "], ");
        fprintf(file, "\"Keys Decoded\": [ ");
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {
            carbon_string_id_t string_id = *CARBON_VECTOR_GET((key_vector), i, carbon_string_id_t);
            char **encString = carbon_strdic_extract(dic, &string_id, 1);
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (key_vector)->num_elems ? ", " : "");
            carbon_strdic_free(dic, encString);
        }
        fprintf(file, "],");
        fprintf(file, "\"Values\": [ ");
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {
            uint16_t amount = *CARBON_VECTOR_GET(value_vector, i, uint16_t);
            fprintf(file, "%d%s", amount, i + 1 < value_vector->num_elems ? ", " : "");
        }
        fprintf(file, "]");
    }
    fprintf(file, "}, ");
}

static void print_array_strings(FILE *file, const char *type_name, const carbon_vec_t ofType(carbon_string_id_t) *key_vector,
                           const carbon_vec_t ofType(Vector ofType(carbon_string_id_t)) *value_vector, carbon_strdic_t *dic)
{
    fprintf(file, "\"%s\": { ", type_name);
    if(!carbon_vec_is_empty((key_vector))) {
        fprintf(file, "\"Keys\": [ ");
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {
            carbon_string_id_t string_id = *CARBON_VECTOR_GET((key_vector), i, carbon_string_id_t);
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (key_vector)->num_elems ? ", " : "");
        }
        fprintf(file, "], ");
        fprintf(file, "\"Keys Decoded\": [ ");
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {
            carbon_string_id_t string_id_t = *CARBON_VECTOR_GET((key_vector), i, carbon_string_id_t);
            char **encString = carbon_strdic_extract(dic, &string_id_t, 1);
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (key_vector)->num_elems ? ", " : "");
            carbon_strdic_free(dic, encString);
        }
        fprintf(file, "],");
        fprintf(file, "\"Values\": [ ");
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {
            const carbon_vec_t ofType(carbon_string_id_t) *values = CARBON_VECTOR_GET(value_vector, i, carbon_vec_t);
            fprintf(file, "[");
            for (size_t j = 0; j < values->num_elems; j++) {
                carbon_string_id_t value = *CARBON_VECTOR_GET(values, j, carbon_string_id_t);
                fprintf(file, "%"PRIu64"%s", value, j + 1 < values->num_elems ? ", " : "");
            }
            fprintf(file, "]%s", i + 1 < (value_vector)->num_elems ? ", " : "");

        }
        fprintf(file, "], ");
        fprintf(file, "\"Values Decoded\": [ ");
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {
            const carbon_vec_t ofType(carbon_string_id_t) *values = CARBON_VECTOR_GET(value_vector, i, carbon_vec_t);
            fprintf(file, "[");
            for (size_t j = 0; j < values->num_elems; j++) {
                carbon_string_id_t value = *CARBON_VECTOR_GET(values, j, carbon_string_id_t);

                if (CARBON_LIKELY(value != CARBON_NULL_ENCODED_STRING)) {
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

static void print_primitive_strings(FILE *file, const char *type_name, const carbon_vec_t ofType(carbon_string_id_t) *key_vector,
                                  const carbon_vec_t ofType(carbon_string_id_t) *value_vector, carbon_strdic_t *dic)
{
    PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, ", ")
    if(!carbon_vec_is_empty((key_vector))) {
        fprintf(file, "\"Values\": [ ");
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {
            carbon_string_id_t string_id_t = *CARBON_VECTOR_GET(value_vector, i, carbon_string_id_t);
            fprintf(file, "%"PRIu64"%s", string_id_t, i + 1 < (value_vector)->num_elems ? ", " : "");
        }
        fprintf(file, "], ");
        fprintf(file, "\"Values Decoded\": [ ");
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {
            carbon_string_id_t string_id_t = *CARBON_VECTOR_GET(value_vector, i, carbon_string_id_t);
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
    const carbon_vec_t *column = CARBON_VECTOR_GET(&columnTable->values, array_idx, carbon_vec_t);                     \
    fprintf(file, "%s", column->num_elems > 1 ? "[" : "");                                                             \
    for (size_t i = 0; i < column->num_elems; i++) {                                                                   \
        fprintf(file, format_string, *CARBON_VECTOR_GET(column, i, type));                                             \
        fprintf(file, "%s", i + 1 < column->num_elems ? ", " : "");                                                    \
    }                                                                                                                  \
    fprintf(file, "%s", column->num_elems > 1 ? "]" : "");                                                             \
}

static bool print_array_objects(FILE *file, carbon_err_t *err, const char *type_name, const carbon_vec_t ofType(carbon_columndoc_columngroup_t) *key_columns,
                              carbon_strdic_t *dic)
{
    fprintf(file, "\"%s\": {", type_name);
    fprintf(file, "\"Keys\": [");
    for (size_t array_key_idx = 0; array_key_idx < key_columns->num_elems; array_key_idx++) {
        const carbon_columndoc_columngroup_t *arrayKeyColumns = CARBON_VECTOR_GET(key_columns, array_key_idx, carbon_columndoc_columngroup_t);
        fprintf(file, "%"PRIu64"%s", arrayKeyColumns->key, array_key_idx + 1 < key_columns->num_elems ? ", " : "");
    }
    fprintf(file, "], \"Keys Decoded\": [");
    for (size_t array_key_idx = 0; array_key_idx < key_columns->num_elems; array_key_idx++) {
        const carbon_columndoc_columngroup_t *arrayKeyColumns = CARBON_VECTOR_GET(key_columns, array_key_idx, carbon_columndoc_columngroup_t);
        carbon_string_id_t encKeyName = arrayKeyColumns->key;
        char **decKeyName = carbon_strdic_extract(dic, &encKeyName, 1);
        fprintf(file, "\"%s\"%s", *decKeyName, array_key_idx + 1 < key_columns->num_elems ? ", " : "");
        carbon_strdic_free(dic, decKeyName);
    }
    fprintf(file, "], ");
    fprintf(file, "\"Tables\": [");
    for (size_t array_key_idx = 0; array_key_idx < key_columns->num_elems; array_key_idx++) {
        fprintf(file, "[");
        const carbon_columndoc_columngroup_t *arrayKeyColumns = CARBON_VECTOR_GET(key_columns, array_key_idx, carbon_columndoc_columngroup_t);
        for (size_t columnIdx = 0; columnIdx < arrayKeyColumns->columns.num_elems; columnIdx++) {
            fprintf(file, "{");
            const carbon_columndoc_column_t *columnTable = CARBON_VECTOR_GET(&arrayKeyColumns->columns, columnIdx, carbon_columndoc_column_t);
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
                case carbon_field_type_null: {
                    const carbon_vec_t *column = CARBON_VECTOR_GET(&columnTable->values, array_idx, carbon_vec_t);
                    fprintf(file, "%s", column->num_elems > 1 ? "[" : "");
                    for (size_t i = 0; i < column->num_elems; i++) {
                        fprintf(file, "null");
                        fprintf(file, "%s", i + 1 < column->num_elems ? ", " : "");
                    }
                    fprintf(file, "%s", column->num_elems > 1 ? "]" : "");
                } break;
                case carbon_field_type_int8:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_int8_t, "%d")
                    break;
                case carbon_field_type_int16:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_int16_t, "%d")
                    break;
                case carbon_field_type_int32:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_int32_t, "%d")
                    break;
                case carbon_field_type_int64:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_int64_t, "%" PRIi64)
                    break;
                case carbon_field_type_uint8:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_uint8_t, "%d")
                    break;
                case carbon_field_type_uint16:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_uint16_t, "%d")
                    break;
                case carbon_field_type_uint32:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_uint32_t, "%d")
                    break;
                case carbon_field_type_uint64:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_uint64_t, "%" PRIu64)
                    break;
                case carbon_field_type_float:
                    PRINT_COLUMN(file, columnTable, array_idx, carbon_number_t , "%f")
                    break;
                case carbon_field_type_string: {
                    const carbon_vec_t *column = CARBON_VECTOR_GET(&columnTable->values, array_idx, carbon_vec_t);
                    fprintf(file, "%s", column->num_elems > 1 ? "[" : "");
                    for (size_t i = 0; i < column->num_elems; i++) {
                        carbon_string_id_t encodedString = *CARBON_VECTOR_GET(column, i, carbon_string_id_t);
                        char **decodedString = carbon_strdic_extract(dic, &encodedString, 1);
                        fprintf(file, "{\"Encoded\": %"PRIu64", \"Decoded\": \"%s\"}", encodedString, *decodedString);
                        fprintf(file, "%s", i + 1 < column->num_elems ? ", " : "");
                        carbon_strdic_free(dic, decodedString);
                    }
                    fprintf(file, "%s", column->num_elems > 1 ? "]" : "");
                } break;
                case carbon_field_type_object: {
                   // carbon_columndoc_obj_t *doc = CARBON_VECTOR_GET(&column->values, valueIdx, carbon_columndoc_obj_t);
                  //  print_object(file, doc, strdic);
                    const carbon_vec_t *column = CARBON_VECTOR_GET(&columnTable->values, array_idx, carbon_vec_t);
                    fprintf(file, "%s", column->num_elems > 1 ? "[" : "");
                    for (size_t i = 0; i < column->num_elems; i++) {
                        const carbon_columndoc_obj_t *object = CARBON_VECTOR_GET(column, i, carbon_columndoc_obj_t);
                        if(!print_object(file, err, object, dic)) {
                            return false;
                        }
                        fprintf(file, "%s", i + 1 < column->num_elems ? ", " : "");
                    }
                    fprintf(file, "%s", column->num_elems > 1 ? "]" : "");
                } break;
                default:
                    CARBON_ERROR(err, CARBON_ERR_NOTYPE)
                    return false;
                }
                fprintf(file, array_idx + 1 < columnTable->values.num_elems ? ", " : "");
            }
            fprintf(file, "],");
            fprintf(file, "\"Positions\": [");
            for (size_t positionIdx = 0; positionIdx < columnTable->array_positions.num_elems; positionIdx++) {
                fprintf(file, "%d%s", *CARBON_VECTOR_GET(&columnTable->array_positions, positionIdx, int16_t), (positionIdx + 1 < columnTable->array_positions.num_elems ? ", " : ""));
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

static bool print_object(FILE *file, carbon_err_t *err, const carbon_columndoc_obj_t *object, carbon_strdic_t *dic)
{
    char **parentKey = carbon_strdic_extract(dic, &object->parent_key, 1);
    fprintf(file, "{ ");
    fprintf(file, "\"Parent\": { \"Key\": %"PRIu64", \"Key Decoded\": \"%s\", \"Index\": %zu }, ", object->parent_key, parentKey[0], object->index);
    fprintf(file, "\"Pairs\": { ");
        fprintf(file, "\"Primitives\": { ");
            PRINT_PRIMITIVE_BOOLEAN_COLUMN(file, "Boolean", &object->bool_prop_keys, &object->bool_prop_vals, dic)
            PRINT_PRIMITIVE_COLUMN(file, "UInt8", &object->uint8_prop_keys, &object->uint8_prop_vals, &object->uint8_val_idxs, dic, carbon_uint8_t, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "UInt16", &object->uint16_prop_keys, &object->uint16_prop_vals, &object->uint16_val_idxs, dic, carbon_uint16_t, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "UInt32", &object->uin32_prop_keys, &object->uint32_prop_vals, &object->uint32_val_idxs, dic, carbon_uint32_t, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "UInt64", &object->uint64_prop_keys, &object->uint64_prop_vals, &object->uint64_val_idxs, dic, carbon_uint64_t, "%" PRIu64)
            PRINT_PRIMITIVE_COLUMN(file, "Int8", &object->int8_prop_keys, &object->int8_prop_vals, &object->int8_val_idxs, dic, carbon_int8_t, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "Int16", &object->int16_prop_keys, &object->int16_prop_vals, &object->int16_val_idxs, dic, carbon_int16_t, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "Int32", &object->int32_prop_keys, &object->int32_prop_vals, &object->int32_val_idxs, dic, carbon_int32_t, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "Int64", &object->int64_prop_keys, &object->int64_prop_vals, &object->int64_val_idxs, dic, carbon_int64_t, "%" PRIi64)
            PRINT_PRIMITIVE_COLUMN(file, "Real", &object->float_prop_keys, &object->float_prop_vals, &object->float_val_idxs, dic, carbon_number_t, "%f")
            print_primitive_strings(file, "Strings", &object->string_prop_keys, &object->string_prop_vals, dic);
            print_primitive_null(file, "Null", &object->null_prop_keys, dic);
            if(print_primitive_objects(file, err, "Objects", &object->obj_prop_keys, &object->obj_prop_vals, dic)) {
                return false;
            }
            fprintf(file, "}, ");
        fprintf(file, "\"Arrays\": { ");
            PRINT_BOOLEAN_ARRAY(file, "Boolean", object->bool_array_prop_keys, object->bool_array_prop_vals);
            PRINT_ARRAY(file, "UInt8", object->uint8_array_prop_keys, object->uint8_array_prop_vals, carbon_uint8_t, "%d", (value != CARBON_NULL_UINT8));
            PRINT_ARRAY(file, "UInt16", object->uint16_array_prop_keys, object->uint16_array_prop_vals, carbon_uint16_t, "%d", (value !=CARBON_NULL_UINT16));
            PRINT_ARRAY(file, "UInt32", object->uint32_array_prop_keys, object->uint32_array_prop_vals, carbon_uint32_t, "%d", (value !=CARBON_NULL_UINT32));
            PRINT_ARRAY(file, "UInt64", object->uint64_array_prop_keys, object->uin64_array_prop_vals, carbon_uint64_t, "%" PRIu64, (value !=CARBON_NULL_UINT64));
            PRINT_ARRAY(file, "Int8", object->int8_array_prop_keys, object->int8_array_prop_vals, carbon_int8_t, "%d", (value !=CARBON_NULL_INT8));
            PRINT_ARRAY(file, "Int16", object->int16_array_prop_keys, object->int16_array_prop_vals, carbon_int16_t, "%d", (value !=CARBON_NULL_INT16));
            PRINT_ARRAY(file, "Int32", object->int32_array_prop_keys, object->int32_array_prop_vals, carbon_int32_t, "%d", (value !=CARBON_NULL_INT32));
            PRINT_ARRAY(file, "Int64", object->int64_array_prop_keys, object->int64_array_prop_vals, carbon_int64_t, "%" PRIi64, (value !=CARBON_NULL_INT64));
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
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(doc)
    return print_object(file, &doc->err, &doc->columndoc, doc->dic);
}

bool carbon_columndoc_drop(carbon_columndoc_t *doc)
{
    CARBON_UNUSED(doc);
    CARBON_NOT_IMPLEMENTED
}

static void object_array_key_columns_create(carbon_vec_t ofType(carbon_columndoc_columngroup_t) *columns)
{
    carbon_vec_create(columns, NULL, sizeof(carbon_columndoc_columngroup_t), 20000);
}

static void object_array_key_columns_drop(carbon_vec_t ofType(carbon_columndoc_columngroup_t) *columns)
{
    for (size_t i = 0; i < columns->num_elems; i++) {
        carbon_columndoc_columngroup_t *array_columns = CARBON_VECTOR_GET(columns, i, carbon_columndoc_columngroup_t);
        for (size_t j = 0; j < array_columns->columns.num_elems; j++) {


            carbon_columndoc_column_t *column = CARBON_VECTOR_GET(&array_columns->columns, j, carbon_columndoc_column_t);

            carbon_vec_t ofType(uint32_t) *array_indices = &column->array_positions;
            carbon_vec_t ofType(carbon_vec_t ofType(<T>)) *values_for_indicies = &column->values;

            assert (array_indices->num_elems == values_for_indicies->num_elems);

            for (size_t k = 0; k < array_indices->num_elems; k++) {

                carbon_vec_t ofType(<T>) *values_for_index = CARBON_VECTOR_GET(values_for_indicies, k, carbon_vec_t);
                if (column->type == carbon_field_type_object) {
                    for (size_t l = 0; l < values_for_index->num_elems; l++) {
                        carbon_columndoc_obj_t *nested_object = CARBON_VECTOR_GET(values_for_index, l, carbon_columndoc_obj_t);
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

static const char *get_type_name(carbon_err_t *err, carbon_field_type_e type)
{
    switch (type) {
        case carbon_field_type_null:return "Null";
        case carbon_field_type_int8:return "Int8";
        case carbon_field_type_int16:return "Int16";
        case carbon_field_type_int32:return "Int32";
        case carbon_field_type_int64:return "Int64";
        case carbon_field_type_uint8:return "UInt8";
        case carbon_field_type_uint16:return "UInt16";
        case carbon_field_type_uint32:return "UInt32";
        case carbon_field_type_uint64:return "UInt64";
        case carbon_field_type_float:return "Real";
        case carbon_field_type_string:return "String";
        case carbon_field_type_object:return "Object";
        default: {
            CARBON_ERROR(err, CARBON_ERR_NOTYPE);
            return NULL;
        }
    }
}

static carbon_columndoc_column_t *object_array_key_columns_find_or_new(carbon_vec_t ofType(carbon_columndoc_columngroup_t) *columns,
                                                            carbon_string_id_t array_key, carbon_string_id_t nested_object_entry_key,
                                                            carbon_field_type_e nested_object_entry_type)
{
    carbon_columndoc_columngroup_t *key_columns;
    carbon_columndoc_column_t *key_column, *new_column;

    for (size_t i = 0; i < columns->num_elems; i++) {
        /** Find object array pair having the key `key` */
        key_columns = CARBON_VECTOR_GET(columns, i, carbon_columndoc_columngroup_t);
        if (key_columns->key == array_key) {
            /** In case such a pair is found, find column that matches the desired type */
            for (size_t j = 0; j < key_columns->columns.num_elems; j++) {
                key_column = CARBON_VECTOR_GET(&key_columns->columns, j, carbon_columndoc_column_t);
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
    carbon_vec_create(&new_column->values, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&new_column->array_positions, NULL, sizeof(uint32_t), 10);

    return new_column;
}

static bool object_array_key_column_push(carbon_columndoc_column_t *col, carbon_err_t *err, const carbon_doc_entries_t *entry, uint32_t array_idx,
                                     carbon_strdic_t *dic, carbon_columndoc_obj_t *model)
{
    assert(col->type == entry->type);

    uint32_t *entry_array_idx = VECTOR_NEW_AND_GET(&col->array_positions, uint32_t);
    *entry_array_idx = array_idx;

    carbon_vec_t ofType(<T>) *values_for_entry = VECTOR_NEW_AND_GET(&col->values, carbon_vec_t);
    carbon_vec_create(values_for_entry, NULL, GET_TYPE_SIZE(entry->type), entry->values.num_elems);

    bool is_null_by_def = entry->values.num_elems == 0;
    uint32_t num_elements = (uint32_t) entry->values.num_elems;

    carbon_field_type_e entryType = is_null_by_def ? carbon_field_type_null : entry->type;
    num_elements = is_null_by_def ? 1 : num_elements;

    switch (entryType) {
    case carbon_field_type_null: {
        carbon_vec_push(values_for_entry, &num_elements, 1);
    } break;
    case carbon_field_type_int8:
    case carbon_field_type_int16:
    case carbon_field_type_int32:
    case carbon_field_type_int64:
    case carbon_field_type_uint8:
    case carbon_field_type_uint16:
    case carbon_field_type_uint32:
    case carbon_field_type_uint64:
    case carbon_field_type_float:
        assert(!is_null_by_def);
        carbon_vec_push(values_for_entry, entry->values.base, num_elements);
        break;
    case carbon_field_type_string: {
        assert(!is_null_by_def);
        char **strings = CARBON_VECTOR_ALL(&entry->values, char *);
        carbon_string_id_t *string_ids;
        carbon_strdic_locate_fast(&string_ids, dic, (char *const *) strings, num_elements);
        carbon_vec_push(values_for_entry, string_ids, num_elements);
        carbon_strdic_free(dic, string_ids);
        //carbon_strdic_free(strdic, strings);
    } break;
    case carbon_field_type_object:
        assert(!is_null_by_def);

        carbon_string_id_t *array_key;
        carbon_strdic_locate_fast(&array_key, dic, (char *const *) &entry->key, 1);

        for (size_t array_idx = 0; array_idx < num_elements; array_idx++)
        {
            carbon_columndoc_obj_t *nested_object = VECTOR_NEW_AND_GET(values_for_entry, carbon_columndoc_obj_t);
            setup_object(nested_object, model->parent, *array_key, array_idx);
            if (!import_object(nested_object, err, CARBON_VECTOR_GET(&entry->values, array_idx, carbon_doc_obj_t), dic)) {
                return false;
            }
        }
        carbon_strdic_free(dic, array_key);
        break;
    default:
        CARBON_ERROR(err, CARBON_ERR_NOTYPE);
        return false;
    }
    return true;
}

static void setup_object(carbon_columndoc_obj_t *model, carbon_columndoc_t *parent, carbon_string_id_t key, size_t idx)
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
    carbon_vec_create(&model->int8_prop_vals, NULL, sizeof(carbon_int8_t), 10);
    carbon_vec_create(&model->int16_prop_vals, NULL, sizeof(carbon_int16_t), 10);
    carbon_vec_create(&model->int32_prop_vals, NULL, sizeof(carbon_int32_t), 10);
    carbon_vec_create(&model->int64_prop_vals, NULL, sizeof(carbon_int64_t), 10);
    carbon_vec_create(&model->uint8_prop_vals, NULL, sizeof(carbon_uint8_t), 10);
    carbon_vec_create(&model->uint16_prop_vals, NULL, sizeof(carbon_uint16_t), 10);
    carbon_vec_create(&model->uint32_prop_vals, NULL, sizeof(carbon_uint32_t), 10);
    carbon_vec_create(&model->uint64_prop_vals, NULL, sizeof(carbon_uint64_t), 10);
    carbon_vec_create(&model->float_prop_vals, NULL, sizeof(carbon_number_t), 10);
    carbon_vec_create(&model->string_prop_vals, NULL, sizeof(carbon_string_id_t), 50);

    carbon_vec_create(&model->bool_array_prop_vals, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->int8_array_prop_vals, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->int16_array_prop_vals, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->int32_array_prop_vals, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->int64_array_prop_vals, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->uint8_array_prop_vals, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->uint16_array_prop_vals, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->uint32_array_prop_vals, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->uin64_array_prop_vals, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->float_array_prop_vals, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->string_array_prop_vals, NULL, sizeof(carbon_vec_t), 50);
    carbon_vec_create(&model->null_array_prop_vals, NULL, sizeof(uint16_t), 10);

    carbon_vec_create(&model->bool_val_idxs, NULL, sizeof(uint32_t), 10);
    carbon_vec_create(&model->int8_val_idxs, NULL, sizeof(uint32_t), 10);
    carbon_vec_create(&model->int16_val_idxs, NULL, sizeof(uint32_t), 10);
    carbon_vec_create(&model->int32_val_idxs, NULL, sizeof(uint32_t), 10);
    carbon_vec_create(&model->int64_val_idxs, NULL, sizeof(uint32_t), 10);
    carbon_vec_create(&model->uint8_val_idxs, NULL, sizeof(uint32_t), 10);
    carbon_vec_create(&model->uint16_val_idxs, NULL, sizeof(uint32_t), 10);
    carbon_vec_create(&model->uint32_val_idxs, NULL, sizeof(uint32_t), 10);
    carbon_vec_create(&model->uint64_val_idxs, NULL, sizeof(uint32_t), 10);
    carbon_vec_create(&model->float_val_idxs, NULL, sizeof(uint32_t), 10);
    carbon_vec_create(&model->string_val_idxs, NULL, sizeof(uint32_t), 50);

    carbon_vec_create(&model->bool_array_idxs, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->int8_array_idxs, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->int16_array_idxs, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->int32_array_idxs, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->int64_array_idxs, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->uint8_array_idxs, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->uint16_array_idxs, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->uint32_array_idxs, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->uint64_array_idxs, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->float_array_idxs, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&model->string_array_idxs, NULL, sizeof(carbon_vec_t), 50);

    carbon_vec_create(&model->obj_prop_vals, NULL, sizeof(carbon_columndoc_obj_t), 10);

    object_array_key_columns_create(&model->obj_array_props);
}

static bool object_put_primitive(carbon_columndoc_obj_t *columndoc, carbon_err_t *err, const carbon_doc_entries_t *entry, carbon_strdic_t *dic,
                               const carbon_string_id_t *key_id)
{
    switch(entry->type) {
    case carbon_field_type_null:
        carbon_vec_push(&columndoc->null_prop_keys, key_id, 1);
        break;
    case carbon_field_type_bool:
        carbon_vec_push(&columndoc->bool_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->bool_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_int8:
        carbon_vec_push(&columndoc->int8_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->int8_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_int16:
        carbon_vec_push(&columndoc->int16_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->int16_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_int32:
        carbon_vec_push(&columndoc->int32_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->int32_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_int64:
        carbon_vec_push(&columndoc->int64_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->int64_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_uint8:
        carbon_vec_push(&columndoc->uint8_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->uint8_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_uint16:
        carbon_vec_push(&columndoc->uint16_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->uint16_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_uint32:
        carbon_vec_push(&columndoc->uin32_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->uint32_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_uint64:
        carbon_vec_push(&columndoc->uint64_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->uint64_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_float:
        carbon_vec_push(&columndoc->float_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->float_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_string: {
        carbon_string_id_t *value;
        carbon_strdic_locate_fast(&value, dic, (char *const *) entry->values.base, 1);
        carbon_vec_push(&columndoc->string_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->string_prop_vals, value, 1);
        carbon_strdic_free(dic, value);
    } break;
    case carbon_field_type_object: {
        carbon_columndoc_obj_t template, *nested_object;
        size_t position = carbon_vec_length(&columndoc->obj_prop_keys);
        carbon_vec_push(&columndoc->obj_prop_keys, key_id, 1);
        carbon_vec_push(&columndoc->obj_prop_vals, &template, 1);
        nested_object = CARBON_VECTOR_GET(&columndoc->obj_prop_vals, position, carbon_columndoc_obj_t);
        setup_object(nested_object, columndoc->parent, *key_id, 0);
        if (!import_object(nested_object, err, CARBON_VECTOR_GET(&entry->values, 0, carbon_doc_obj_t), dic)) {
            return false;
        }
    } break;
        CARBON_ERROR(err, CARBON_ERR_NOTYPE)
        return false;
    }
    return true;
}

static void object_push_array(carbon_vec_t ofType(Vector ofType(<T>)) *values, size_t TSize, uint32_t num_elements,
                            const void *data, carbon_string_id_t key_id, carbon_vec_t ofType(carbon_string_id_t) *key_vector)
{
    carbon_vec_t ofType(<T>) template, *vector;
    size_t idx = carbon_vec_length(values);
    carbon_vec_push(values, &template, 1);
    vector = CARBON_VECTOR_GET(values, idx, carbon_vec_t);
    carbon_vec_create(vector, NULL, TSize, num_elements);
    carbon_vec_push(vector, data, num_elements);
    carbon_vec_push(key_vector, &key_id, 1);
}

static bool object_put_array(carbon_columndoc_obj_t *model, carbon_err_t *err, const carbon_doc_entries_t *entry, carbon_strdic_t *dic, const carbon_string_id_t *key_id)
{
    // TODO: format for array, sort by keys, sort by values!
    CARBON_UNUSED(dic);
    uint32_t num_elements = (uint32_t) carbon_vec_length(&entry->values);

    switch(entry->type) {
    case carbon_field_type_null: {
        carbon_vec_push(&model->null_array_prop_vals, &num_elements, 1);
        carbon_vec_push(&model->null_array_prop_keys, key_id, 1);
    }
        break;
    case carbon_field_type_bool:
        object_push_array(&model->bool_array_prop_vals, sizeof(carbon_boolean_t), num_elements, entry->values.base, *key_id,
                        &model->bool_array_prop_keys);
        break;
    case carbon_field_type_int8:
        object_push_array(&model->int8_array_prop_vals, sizeof(carbon_int8_t), num_elements, entry->values.base, *key_id,
                        &model->int8_array_prop_keys);
        break;
    case carbon_field_type_int16:
        object_push_array(&model->int16_array_prop_vals, sizeof(carbon_int16_t), num_elements, entry->values.base, *key_id,
                        &model->int16_array_prop_keys);
        break;
    case carbon_field_type_int32:
        object_push_array(&model->int32_array_prop_vals, sizeof(carbon_int32_t), num_elements, entry->values.base, *key_id,
                        &model->int32_array_prop_keys);
        break;
    case carbon_field_type_int64:
        object_push_array(&model->int64_array_prop_vals, sizeof(carbon_int64_t), num_elements, entry->values.base, *key_id,
                        &model->int64_array_prop_keys);
        break;
    case carbon_field_type_uint8:
        object_push_array(&model->uint8_array_prop_vals, sizeof(carbon_uint8_t), num_elements, entry->values.base, *key_id,
                        &model->uint8_array_prop_keys);
        break;
    case carbon_field_type_uint16:
        object_push_array(&model->uint16_array_prop_vals,
                        sizeof(carbon_uint16_t),
                        num_elements,
                        entry->values.base,
                        *key_id,
                        &model->uint16_array_prop_keys);
        break;
    case carbon_field_type_uint32:
        object_push_array(&model->uint32_array_prop_vals, sizeof(carbon_uint32_t), num_elements, entry->values.base, *key_id,
                        &model->uint32_array_prop_keys);
        break;
    case carbon_field_type_uint64:
        object_push_array(&model->uin64_array_prop_vals, sizeof(carbon_uint64_t), num_elements, entry->values.base, *key_id,
                        &model->uint64_array_prop_keys);
        break;
    case carbon_field_type_float:
        object_push_array(&model->float_array_prop_vals, sizeof(carbon_number_t), num_elements, entry->values.base, *key_id,
                        &model->float_array_prop_keys);
        break;
    case carbon_field_type_string: {
        const char **strings = CARBON_VECTOR_ALL(&entry->values, const char *);
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
    case carbon_field_type_object: {
        carbon_string_id_t *nested_object_key_name;
        for (uint32_t array_idx = 0; array_idx < num_elements; array_idx++) {
            const carbon_doc_obj_t *object = CARBON_VECTOR_GET(&entry->values, array_idx, carbon_doc_obj_t);
            for (size_t pair_idx = 0; pair_idx < object->entries.num_elems; pair_idx++) {
                const carbon_doc_entries_t *pair = CARBON_VECTOR_GET(&object->entries, pair_idx, carbon_doc_entries_t);
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
        CARBON_ERROR(err, CARBON_ERR_NOTYPE)
        return false;
    } break;
    }
    return true;
}

static bool object_put(carbon_columndoc_obj_t *model, carbon_err_t *err, const carbon_doc_entries_t *entry, carbon_strdic_t *dic)
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
        CARBON_ERROR(err, CARBON_ERR_NOTYPE)
        return false;
    }

    carbon_strdic_free(dic, key_id);
    return true;
}

static bool import_object(carbon_columndoc_obj_t *dst, carbon_err_t *err, const carbon_doc_obj_t *doc, carbon_strdic_t *dic)
{
    const carbon_vec_t ofType(carbon_doc_entries_t) *objectEntries = carbon_doc_get_entries(doc);
    const carbon_doc_entries_t *entries = CARBON_VECTOR_ALL(objectEntries, carbon_doc_entries_t);
    for (size_t i = 0; i < objectEntries->num_elems; i++) {
        const carbon_doc_entries_t *entry = entries + i;
        if (!object_put(dst, err, entry, dic)) {
            return false;
        }
    }
    return true;
}