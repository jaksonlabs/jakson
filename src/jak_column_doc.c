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

#include <jak_column_doc.h>
#include <jak_doc.h>

static void
setup_object(jak_column_doc_obj *model, jak_column_doc *parent, jak_archive_field_sid_t key, size_t idx);

static bool object_put(jak_column_doc_obj *model, jak_error *err, const jak_doc_entries *entry,
                       jak_string_dict *dic);

static bool import_object(jak_column_doc_obj *dst, jak_error *err, const jak_doc_obj *doc,
                          jak_string_dict *dic);

static bool
_jak_column_doc_print_object(FILE *file, jak_error *err, const jak_column_doc_obj *object, jak_string_dict *dic);

static const char *get_type_name(jak_error *err, jak_archive_field_e type);

static void object_array_key_columns_create(jak_vector ofType(jak_column_doc_group) *columns);

static jak_column_doc_column *object_array_key_columns_find_or_new(
        jak_vector ofType(jak_column_doc_group) *columns, jak_archive_field_sid_t array_key,
        jak_archive_field_sid_t nested_object_entry_key, jak_archive_field_e nested_object_entry_type);

static bool object_array_key_column_push(jak_column_doc_column *col, jak_error *err,
                                         const jak_doc_entries *entry,
                                         jak_u32 array_idx, jak_string_dict *dic,
                                         jak_column_doc_obj *model);

bool jak_columndoc_create(jak_column_doc *columndoc, jak_error *err, const jak_doc *doc,
                      const jak_doc_bulk *bulk,
                      const jak_doc_entries *entries, jak_string_dict *dic)
{
        JAK_ERROR_IF_NULL(columndoc)
        JAK_ERROR_IF_NULL(doc)
        JAK_ERROR_IF_NULL(dic)
        JAK_ERROR_IF_NULL(bulk)

        columndoc->dic = dic;
        columndoc->doc = doc;
        columndoc->bulk = bulk;
        jak_error_init(&columndoc->err);

        const char *root_string = "/";
        jak_archive_field_sid_t *rootId;

        jak_string_dict_insert(dic, &rootId, (char *const *) &root_string, 1, 0);

        setup_object(&columndoc->columndoc, columndoc, *rootId, 0);

        jak_string_dict_free(dic, rootId);

        const jak_doc_obj *root = jak_doc_entries_get_root(entries);
        if (!import_object(&columndoc->columndoc, err, root, dic)) {
                return false;
        }

        return true;
}

static void object_array_key_columns_drop(jak_vector ofType(jak_column_doc_group) *columns);

static void object_meta_model_free(jak_column_doc_obj *columndoc)
{
        jak_vector_drop(&columndoc->bool_prop_keys);
        jak_vector_drop(&columndoc->int8_prop_keys);
        jak_vector_drop(&columndoc->int16_prop_keys);
        jak_vector_drop(&columndoc->int32_prop_keys);
        jak_vector_drop(&columndoc->int64_prop_keys);
        jak_vector_drop(&columndoc->uint8_prop_keys);
        jak_vector_drop(&columndoc->uint16_prop_keys);
        jak_vector_drop(&columndoc->uin32_prop_keys);
        jak_vector_drop(&columndoc->uint64_prop_keys);
        jak_vector_drop(&columndoc->jak_string_prop_keys);
        jak_vector_drop(&columndoc->float_prop_keys);
        jak_vector_drop(&columndoc->null_prop_keys);
        jak_vector_drop(&columndoc->obj_prop_keys);

        jak_vector_drop(&columndoc->bool_array_prop_keys);
        jak_vector_drop(&columndoc->int8_array_prop_keys);
        jak_vector_drop(&columndoc->int16_array_prop_keys);
        jak_vector_drop(&columndoc->int32_array_prop_keys);
        jak_vector_drop(&columndoc->int64_array_prop_keys);
        jak_vector_drop(&columndoc->uint8_array_prop_keys);
        jak_vector_drop(&columndoc->uint16_array_prop_keys);
        jak_vector_drop(&columndoc->uint32_array_prop_keys);
        jak_vector_drop(&columndoc->uint64_array_prop_keys);
        jak_vector_drop(&columndoc->jak_string_array_prop_keys);
        jak_vector_drop(&columndoc->float_array_prop_keys);
        jak_vector_drop(&columndoc->null_array_prop_keys);

        jak_vector_drop(&columndoc->bool_prop_vals);
        jak_vector_drop(&columndoc->int8_prop_vals);
        jak_vector_drop(&columndoc->int16_prop_vals);
        jak_vector_drop(&columndoc->int32_prop_vals);
        jak_vector_drop(&columndoc->int64_prop_vals);
        jak_vector_drop(&columndoc->uint8_prop_vals);
        jak_vector_drop(&columndoc->uint16_prop_vals);
        jak_vector_drop(&columndoc->uint32_prop_vals);
        jak_vector_drop(&columndoc->uint64_prop_vals);
        jak_vector_drop(&columndoc->float_prop_vals);
        jak_vector_drop(&columndoc->jak_string_prop_vals);

        for (size_t i = 0; i < columndoc->bool_array_prop_vals.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->bool_array_prop_vals, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->bool_array_prop_vals);

        for (size_t i = 0; i < columndoc->int8_array_prop_vals.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->int8_array_prop_vals, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->int8_array_prop_vals);

        for (size_t i = 0; i < columndoc->int16_array_prop_vals.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->int16_array_prop_vals, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->int16_array_prop_vals);

        for (size_t i = 0; i < columndoc->int32_array_prop_vals.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->int32_array_prop_vals, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->int32_array_prop_vals);

        for (size_t i = 0; i < columndoc->int64_array_prop_vals.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->int64_array_prop_vals, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->int64_array_prop_vals);

        for (size_t i = 0; i < columndoc->uint8_array_prop_vals.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->uint8_array_prop_vals, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->uint8_array_prop_vals);

        for (size_t i = 0; i < columndoc->uint16_array_prop_vals.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->uint16_array_prop_vals, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->uint16_array_prop_vals);

        for (size_t i = 0; i < columndoc->uint32_array_prop_vals.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->uint32_array_prop_vals, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->uint32_array_prop_vals);

        for (size_t i = 0; i < columndoc->ui64_array_prop_vals.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->ui64_array_prop_vals, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->ui64_array_prop_vals);

        for (size_t i = 0; i < columndoc->float_array_prop_vals.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->float_array_prop_vals, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->float_array_prop_vals);

        for (size_t i = 0; i < columndoc->jak_string_array_prop_vals.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->jak_string_array_prop_vals, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->jak_string_array_prop_vals);

        jak_vector_drop(&columndoc->null_array_prop_vals);

        jak_vector_drop(&columndoc->bool_val_idxs);
        jak_vector_drop(&columndoc->int8_val_idxs);
        jak_vector_drop(&columndoc->int16_val_idxs);
        jak_vector_drop(&columndoc->int32_val_idxs);
        jak_vector_drop(&columndoc->int64_val_idxs);
        jak_vector_drop(&columndoc->uint8_val_idxs);
        jak_vector_drop(&columndoc->uint16_val_idxs);
        jak_vector_drop(&columndoc->uint32_val_idxs);
        jak_vector_drop(&columndoc->uint64_val_idxs);
        jak_vector_drop(&columndoc->float_val_idxs);
        jak_vector_drop(&columndoc->jak_string_val_idxs);

        for (size_t i = 0; i < columndoc->bool_array_idxs.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->bool_array_idxs, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->bool_array_idxs);

        for (size_t i = 0; i < columndoc->int8_array_idxs.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->int8_array_idxs, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->int8_array_idxs);

        for (size_t i = 0; i < columndoc->int16_array_idxs.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->int16_array_idxs, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->int16_array_idxs);

        for (size_t i = 0; i < columndoc->int32_array_idxs.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->int32_array_idxs, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->int32_array_idxs);

        for (size_t i = 0; i < columndoc->int64_array_idxs.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->int64_array_idxs, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->int64_array_idxs);

        for (size_t i = 0; i < columndoc->uint8_array_idxs.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->uint8_array_idxs, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->uint8_array_idxs);

        for (size_t i = 0; i < columndoc->uint16_array_idxs.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->uint16_array_idxs, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->uint16_array_idxs);

        for (size_t i = 0; i < columndoc->uint32_array_idxs.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->uint32_array_idxs, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->uint32_array_idxs);

        for (size_t i = 0; i < columndoc->uint64_array_idxs.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->uint64_array_idxs, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->uint64_array_idxs);

        for (size_t i = 0; i < columndoc->float_array_idxs.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->float_array_idxs, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->float_array_idxs);

        for (size_t i = 0; i < columndoc->jak_string_array_idxs.num_elems; i++) {
                jak_vector *vec = JAK_VECTOR_GET(&columndoc->jak_string_array_idxs, i, jak_vector);
                jak_vector_drop(vec);
        }
        jak_vector_drop(&columndoc->jak_string_array_idxs);

        for (size_t i = 0; i < columndoc->obj_prop_vals.num_elems; i++) {
                jak_column_doc_obj *object = JAK_VECTOR_GET(&columndoc->obj_prop_vals, i, jak_column_doc_obj);
                object_meta_model_free(object);
        }
        jak_vector_drop(&columndoc->obj_prop_vals);

        object_array_key_columns_drop(&columndoc->obj_array_props);
}

bool jak_columndoc_free(jak_column_doc *doc)
{
        JAK_ERROR_IF_NULL(doc);
        object_meta_model_free(&doc->columndoc);
        return true;
}

#define PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, suffix)                                             \
{                                                                                                                      \
    fprintf(file, "\"%s\": { ", type_name);                                                                            \
    if(!jak_vector_is_empty((key_vector))) {                                                                           \
        fprintf(file, "\"Keys\": [ ");                                                                                 \
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {                                                         \
            jak_archive_field_sid_t jak_string_id = *JAK_VECTOR_GET((key_vector), i, jak_archive_field_sid_t);                    \
            fprintf(file, "%"PRIu64"%s", jak_string_id, i + 1 < (key_vector)->num_elems ? ", " : "");                      \
        }                                                                                                              \
        fprintf(file, "], ");                                                                                          \
        fprintf(file, "\"Keys Decoded\": [ ");                                                                         \
        for (size_t i = 0; i < (key_vector)->num_elems; i++) {                                                         \
            jak_archive_field_sid_t jak_string_id = *JAK_VECTOR_GET((key_vector), i, jak_archive_field_sid_t);                    \
            char **encString = jak_string_dict_extract(dic, &jak_string_id, 1);                                              \
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (key_vector)->num_elems ? ", " : "");                      \
            jak_string_dict_free(dic, encString);                                                                        \
        }                                                                                                              \
        fprintf(file, "]%s", suffix);                                                                                  \
    }                                                                                                                  \
}                                                                                                                      \

#define PRINT_PRIMITIVE_COLUMN(file, type_name, key_vector, value_vector, keyIndicesVector, dic, TYPE, FORMAT_STR)     \
{                                                                                                                      \
    PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, ", ")                                                   \
    if(!jak_vector_is_empty((key_vector))) {                                                                           \
        fprintf(file, "\"Values\": [ ");                                                                               \
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {                                                       \
            TYPE value = *JAK_VECTOR_GET(value_vector, i, TYPE);                                                    \
            fprintf(file, FORMAT_STR "%s", value, i + 1 < (value_vector)->num_elems ? ", " : "");                      \
        }                                                                                                              \
        fprintf(file, "] ");                                                                                           \
    }                                                                                                                  \
    fprintf(file, "}, ");                                                                                              \
}

#define PRINT_PRIMITIVE_BOOLEAN_COLUMN(file, type_name, key_vector, value_vector, dic)                                 \
{                                                                                                                      \
    PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, ", ")                                                   \
    if(!jak_vector_is_empty((key_vector))) {                                                                           \
        fprintf(file, "\"Values\": [ ");                                                                               \
        for (size_t i = 0; i < (value_vector)->num_elems; i++) {                                                       \
            jak_archive_field_boolean_t value = *JAK_VECTOR_GET(value_vector, i, jak_archive_field_boolean_t);                                  \
            fprintf(file, "%s%s", value == 0 ? "false" : "true", i + 1 < (value_vector)->num_elems ? ", " : "");       \
        }                                                                                                              \
        fprintf(file, "]");                                                                                            \
    }                                                                                                                  \
    fprintf(file, "}, ");                                                                                              \
}

static void print_primitive_null(FILE *file, const char *type_name,
                                 const jak_vector ofType(jak_archive_field_sid_t) *key_vector,
                                 jak_string_dict *dic)
{
        PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, "")
        fprintf(file, "}, ");
}

static bool print_primitive_objects(FILE *file, jak_error *err, const char *type_name,
                                    const jak_vector ofType(jak_archive_field_sid_t) *key_vector,
                                    const jak_vector ofType(jak_column_doc_obj) *value_vector,
                                    jak_string_dict *dic)
{
        PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, ", ")
        if (!jak_vector_is_empty((key_vector))) {
                fprintf(file, "\"Values\": [ ");
                for (size_t i = 0; i < (value_vector)->num_elems; i++) {
                        const jak_column_doc_obj *object = JAK_VECTOR_GET(value_vector, i, jak_column_doc_obj);
                        if (!_jak_column_doc_print_object(file, err, object, dic)) {
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
    if(!jak_vector_is_empty((&key_vector))) {                                                                          \
        fprintf(file, "\"Keys\": [ ");                                                                                 \
        for (size_t i = 0; i < (&key_vector)->num_elems; i++) {                                                        \
            jak_archive_field_sid_t jak_string_id = *JAK_VECTOR_GET((&key_vector), i, jak_archive_field_sid_t);                   \
            fprintf(file, "%"PRIu64"%s", jak_string_id, i + 1 < (&key_vector)->num_elems ? ", " : "");                     \
        }                                                                                                              \
        fprintf(file, "], ");                                                                                          \
        fprintf(file, "\"Keys Decoded\": [ ");                                                                         \
        for (size_t i = 0; i < (&key_vector)->num_elems; i++) {                                                        \
            jak_archive_field_sid_t jak_string_id = *JAK_VECTOR_GET((&key_vector), i, jak_archive_field_sid_t);                   \
            char **encString = jak_string_dict_extract(dic, &jak_string_id, 1);                                              \
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (&key_vector)->num_elems ? ", " : "");                     \
            jak_string_dict_free(dic, encString);                                                                        \
        }                                                                                                              \
        fprintf(file, "],");                                                                                           \
        fprintf(file, "\"Values\": [ ");                                                                               \
        for (size_t i = 0; i < (&value_vector)->num_elems; i++) {                                                      \
            const jak_vector ofType(TYPE) *values = JAK_VECTOR_GET(&value_vector, i, jak_vector);               \
            fprintf(file, "[ ");                                                                                       \
            for (size_t j = 0; j < values->num_elems; j++) {                                                           \
                TYPE value = *JAK_VECTOR_GET(values, j, TYPE);                                                      \
                if (nonnull_expr) {                                                                                    \
                    fprintf(file, "" TYPE_FORMAT "%s", value, j + 1 < values->num_elems ? ", " : "");                  \
                } else {                                                                                               \
                    fprintf(file, JAK_NULL_TEXT "%s", j + 1 < values->num_elems ? ", " : "");                       \
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
    if(!jak_vector_is_empty((&key_vector))) {                                                                          \
        fprintf(file, "\"Keys\": [ ");                                                                                 \
        for (size_t i = 0; i < (&key_vector)->num_elems; i++) {                                                        \
            jak_archive_field_sid_t jak_string_id = *JAK_VECTOR_GET((&key_vector), i, jak_archive_field_sid_t);                   \
            fprintf(file, "%"PRIu64"%s", jak_string_id, i + 1 < (&key_vector)->num_elems ? ", " : "");                     \
        }                                                                                                              \
        fprintf(file, "], ");                                                                                          \
        fprintf(file, "\"Keys Decoded\": [ ");                                                                         \
        for (size_t i = 0; i < (&key_vector)->num_elems; i++) {                                                        \
            jak_archive_field_sid_t jak_string_id = *JAK_VECTOR_GET((&key_vector), i, jak_archive_field_sid_t);                   \
            char **encString = jak_string_dict_extract(dic, &jak_string_id, 1);                                              \
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (&key_vector)->num_elems ? ", " : "");                     \
            jak_string_dict_free(dic, encString);                                                                        \
        }                                                                                                              \
        fprintf(file, "],");                                                                                           \
        fprintf(file, "\"Values\": [ ");                                                                               \
        for (size_t i = 0; i < (&value_vector)->num_elems; i++) {                                                      \
            const jak_vector ofType(jak_archive_field_boolean_t) *values = JAK_VECTOR_GET(&value_vector, i, jak_vector);      \
            fprintf(file, "[ ");                                                                                       \
            for (size_t j = 0; j < values->num_elems; j++) {                                                           \
                jak_archive_field_boolean_t value = *JAK_VECTOR_GET(values, j, jak_archive_field_boolean_t);                                    \
                fprintf(file, "%s%s", value == 0 ? "false" : "true", j + 1 < values->num_elems ? ", " : "");           \
            }                                                                                                          \
            fprintf(file, "]%s ", i + 1 < (&value_vector)->num_elems ? "," : "");                                      \
        }                                                                                                              \
        fprintf(file, "]");                                                                                            \
    }                                                                                                                  \
    fprintf(file, "}, ");                                                                                              \
}

static void
print_array_null(FILE *file, const char *type_name, const jak_vector ofType(jak_archive_field_sid_t) *key_vector,
                 const jak_vector ofType(jak_u16) *value_vector, jak_string_dict *dic)
{
        fprintf(file, "\"%s\": { ", type_name);
        if (!jak_vector_is_empty((key_vector))) {
                fprintf(file, "\"Keys\": [ ");
                for (size_t i = 0; i < (key_vector)->num_elems; i++) {
                        jak_archive_field_sid_t jak_string_id = *JAK_VECTOR_GET((key_vector), i, jak_archive_field_sid_t);
                        fprintf(file, "%"PRIu64"%s", jak_string_id, i + 1 < (key_vector)->num_elems ? ", " : "");
                }
                fprintf(file, "], ");
                fprintf(file, "\"Keys Decoded\": [ ");
                for (size_t i = 0; i < (key_vector)->num_elems; i++) {
                        jak_archive_field_sid_t jak_string_id = *JAK_VECTOR_GET((key_vector), i, jak_archive_field_sid_t);
                        char **encString = jak_string_dict_extract(dic, &jak_string_id, 1);
                        fprintf(file, "\"%s\"%s", encString[0], i + 1 < (key_vector)->num_elems ? ", " : "");
                        jak_string_dict_free(dic, encString);
                }
                fprintf(file, "],");
                fprintf(file, "\"Values\": [ ");
                for (size_t i = 0; i < (value_vector)->num_elems; i++) {
                        jak_u16 amount = *JAK_VECTOR_GET(value_vector, i, jak_u16);
                        fprintf(file, "%d%s", amount, i + 1 < value_vector->num_elems ? ", " : "");
                }
                fprintf(file, "]");
        }
        fprintf(file, "}, ");
}

static void print_array_strings(FILE *file, const char *type_name,
                                const jak_vector ofType(jak_archive_field_sid_t) *key_vector,
                                const jak_vector ofType(Vector
                                                                       ofType(jak_archive_field_sid_t)) *value_vector,
                                jak_string_dict *dic)
{
        fprintf(file, "\"%s\": { ", type_name);
        if (!jak_vector_is_empty((key_vector))) {
                fprintf(file, "\"Keys\": [ ");
                for (size_t i = 0; i < (key_vector)->num_elems; i++) {
                        jak_archive_field_sid_t jak_string_id = *JAK_VECTOR_GET((key_vector), i, jak_archive_field_sid_t);
                        fprintf(file, "%"PRIu64"%s", jak_string_id, i + 1 < (key_vector)->num_elems ? ", " : "");
                }
                fprintf(file, "], ");
                fprintf(file, "\"Keys Decoded\": [ ");
                for (size_t i = 0; i < (key_vector)->num_elems; i++) {
                        jak_archive_field_sid_t jak_string_id_t = *JAK_VECTOR_GET((key_vector), i, jak_archive_field_sid_t);
                        char **encString = jak_string_dict_extract(dic, &jak_string_id_t, 1);
                        fprintf(file, "\"%s\"%s", encString[0], i + 1 < (key_vector)->num_elems ? ", " : "");
                        jak_string_dict_free(dic, encString);
                }
                fprintf(file, "],");
                fprintf(file, "\"Values\": [ ");
                for (size_t i = 0; i < (value_vector)->num_elems; i++) {
                        const jak_vector ofType(jak_archive_field_sid_t) *values = JAK_VECTOR_GET(value_vector, i,
                                                                                                  jak_vector);
                        fprintf(file, "[");
                        for (size_t j = 0; j < values->num_elems; j++) {
                                jak_archive_field_sid_t value = *JAK_VECTOR_GET(values, j, jak_archive_field_sid_t);
                                fprintf(file, "%"PRIu64"%s", value, j + 1 < values->num_elems ? ", " : "");
                        }
                        fprintf(file, "]%s", i + 1 < (value_vector)->num_elems ? ", " : "");

                }
                fprintf(file, "], ");
                fprintf(file, "\"Values Decoded\": [ ");
                for (size_t i = 0; i < (value_vector)->num_elems; i++) {
                        const jak_vector ofType(jak_archive_field_sid_t) *values = JAK_VECTOR_GET(value_vector, i,
                                                                                                  jak_vector);
                        fprintf(file, "[");
                        for (size_t j = 0; j < values->num_elems; j++) {
                                jak_archive_field_sid_t value = *JAK_VECTOR_GET(values, j, jak_archive_field_sid_t);

                                if (JAK_LIKELY(value != JAK_NULL_ENCODED_STRING)) {
                                        char **decoded = jak_string_dict_extract(dic, &value, 1);
                                        fprintf(file, "\"%s\"%s", *decoded, j + 1 < values->num_elems ? ", " : "");
                                        jak_string_dict_free(dic, decoded);
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

static void print_primitive_strings(FILE *file, const char *type_name,
                                    const jak_vector ofType(jak_archive_field_sid_t) *key_vector,
                                    const jak_vector ofType(jak_archive_field_sid_t) *value_vector,
                                    jak_string_dict *dic)
{
        PRINT_PRIMITIVE_KEY_PART(file, type_name, key_vector, dic, ", ")
        if (!jak_vector_is_empty((key_vector))) {
                fprintf(file, "\"Values\": [ ");
                for (size_t i = 0; i < (value_vector)->num_elems; i++) {
                        jak_archive_field_sid_t jak_string_id_t = *JAK_VECTOR_GET(value_vector, i, jak_archive_field_sid_t);
                        fprintf(file, "%"PRIu64"%s", jak_string_id_t, i + 1 < (value_vector)->num_elems ? ", " : "");
                }
                fprintf(file, "], ");
                fprintf(file, "\"Values Decoded\": [ ");
                for (size_t i = 0; i < (value_vector)->num_elems; i++) {
                        jak_archive_field_sid_t jak_string_id_t = *JAK_VECTOR_GET(value_vector, i, jak_archive_field_sid_t);
                        char **values = jak_string_dict_extract(dic, &jak_string_id_t, 1);
                        fprintf(file, "\"%s\"%s", *values, i + 1 < (value_vector)->num_elems ? ", " : "");
                        jak_string_dict_free(dic, values);
                }
                fprintf(file, "]");
        }
        fprintf(file, "}, ");

}

#define PRINT_COLUMN(file, columnTable, array_idx, type, format_string)                                                \
{                                                                                                                      \
    const jak_vector *column = JAK_VECTOR_GET(&columnTable->values, array_idx, jak_vector);                     \
    fprintf(file, "%s", column->num_elems > 1 ? "[" : "");                                                             \
    for (size_t i = 0; i < column->num_elems; i++) {                                                                   \
        fprintf(file, format_string, *JAK_VECTOR_GET(column, i, type));                                             \
        fprintf(file, "%s", i + 1 < column->num_elems ? ", " : "");                                                    \
    }                                                                                                                  \
    fprintf(file, "%s", column->num_elems > 1 ? "]" : "");                                                             \
}

static bool print_array_objects(FILE *file, jak_error *err, const char *type_name,
                                const jak_vector ofType(jak_column_doc_group) *key_columns,
                                jak_string_dict *dic)
{
        fprintf(file, "\"%s\": {", type_name);
        fprintf(file, "\"Keys\": [");
        for (size_t array_key_idx = 0; array_key_idx < key_columns->num_elems; array_key_idx++) {
                const jak_column_doc_group
                        *arrayKeyColumns = JAK_VECTOR_GET(key_columns, array_key_idx, jak_column_doc_group);
                fprintf(file,
                        "%"PRIu64"%s",
                        arrayKeyColumns->key,
                        array_key_idx + 1 < key_columns->num_elems ? ", " : "");
        }
        fprintf(file, "], \"Keys Decoded\": [");
        for (size_t array_key_idx = 0; array_key_idx < key_columns->num_elems; array_key_idx++) {
                const jak_column_doc_group
                        *arrayKeyColumns = JAK_VECTOR_GET(key_columns, array_key_idx, jak_column_doc_group);
                jak_archive_field_sid_t encKeyName = arrayKeyColumns->key;
                char **decKeyName = jak_string_dict_extract(dic, &encKeyName, 1);
                fprintf(file, "\"%s\"%s", *decKeyName, array_key_idx + 1 < key_columns->num_elems ? ", " : "");
                jak_string_dict_free(dic, decKeyName);
        }
        fprintf(file, "], ");
        fprintf(file, "\"Tables\": [");
        for (size_t array_key_idx = 0; array_key_idx < key_columns->num_elems; array_key_idx++) {
                fprintf(file, "[");
                const jak_column_doc_group
                        *arrayKeyColumns = JAK_VECTOR_GET(key_columns, array_key_idx, jak_column_doc_group);
                for (size_t columnIdx = 0; columnIdx < arrayKeyColumns->columns.num_elems; columnIdx++) {
                        fprintf(file, "{");
                        const jak_column_doc_column
                                *columnTable = JAK_VECTOR_GET(&arrayKeyColumns->columns, columnIdx,
                                                       jak_column_doc_column);
                        char **decColumnKeyName = jak_string_dict_extract(dic, &columnTable->key_name, 1);

                        const char *column_type_name = get_type_name(err, columnTable->type);
                        if (!column_type_name) {
                                return false;
                        }

                        fprintf(file,
                                "\"Column Name\": %"PRIu64", "
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
                                        case JAK_FIELD_NULL: {
                                                const jak_vector
                                                        *column = JAK_VECTOR_GET(&columnTable->values, array_idx,
                                                                          jak_vector);
                                                fprintf(file, "%s", column->num_elems > 1 ? "[" : "");
                                                for (size_t i = 0; i < column->num_elems; i++) {
                                                        fprintf(file, "null");
                                                        fprintf(file, "%s", i + 1 < column->num_elems ? ", " : "");
                                                }
                                                fprintf(file, "%s", column->num_elems > 1 ? "]" : "");
                                        }
                                                break;
                                        case JAK_FIELD_INT8: PRINT_COLUMN(file, columnTable, array_idx,
                                                                          jak_archive_field_i8_t, "%d")
                                                break;
                                        case JAK_FIELD_INT16: PRINT_COLUMN(file, columnTable, array_idx,
                                                                           jak_archive_field_i16_t, "%d")
                                                break;
                                        case JAK_FIELD_INT32: PRINT_COLUMN(file, columnTable, array_idx,
                                                                           jak_archive_field_i32_t, "%d")
                                                break;
                                        case JAK_FIELD_INT64: PRINT_COLUMN(file, columnTable, array_idx,
                                                                           jak_archive_field_i64_t, "%"
                                                                                   PRIi64)
                                                break;
                                        case JAK_FIELD_UINT8: PRINT_COLUMN(file, columnTable, array_idx,
                                                                           jak_archive_field_u8_t, "%d")
                                                break;
                                        case JAK_FIELD_UINT16: PRINT_COLUMN(file, columnTable, array_idx,
                                                                            jak_archive_field_u16_t, "%d")
                                                break;
                                        case JAK_FIELD_UINT32: PRINT_COLUMN(file, columnTable, array_idx,
                                                                            jak_archive_field_u32_t, "%d")
                                                break;
                                        case JAK_FIELD_UINT64: PRINT_COLUMN(file, columnTable, array_idx,
                                                                            jak_archive_field_u64_t, "%"
                                                                                    PRIu64)
                                                break;
                                        case JAK_FIELD_FLOAT: PRINT_COLUMN(file, columnTable, array_idx,
                                                                           jak_archive_field_number_t,
                                                                           "%f")
                                                break;
                                        case JAK_FIELD_STRING: {
                                                const jak_vector
                                                        *column = JAK_VECTOR_GET(&columnTable->values, array_idx,
                                                                          jak_vector);
                                                fprintf(file, "%s", column->num_elems > 1 ? "[" : "");
                                                for (size_t i = 0; i < column->num_elems; i++) {
                                                        jak_archive_field_sid_t encodedString = *JAK_VECTOR_GET(column, i,
                                                                                                         jak_archive_field_sid_t);
                                                        char **decodedString = jak_string_dict_extract(dic, &encodedString, 1);
                                                        fprintf(file,
                                                                "{\"Encoded\": %"PRIu64", \"Decoded\": \"%s\"}",
                                                                encodedString,
                                                                *decodedString);
                                                        fprintf(file, "%s", i + 1 < column->num_elems ? ", " : "");
                                                        jak_string_dict_free(dic, decodedString);
                                                }
                                                fprintf(file, "%s", column->num_elems > 1 ? "]" : "");
                                        }
                                                break;
                                        case JAK_FIELD_OBJECT: {
                                                // jak_column_doc_obj *doc = JAK_VECTOR_GET(&column->values, valueIdx, jak_column_doc_obj);
                                                //  _jak_column_doc_print_object(file, doc, encode);
                                                const jak_vector
                                                        *column = JAK_VECTOR_GET(&columnTable->values, array_idx,
                                                                          jak_vector);
                                                fprintf(file, "%s", column->num_elems > 1 ? "[" : "");
                                                for (size_t i = 0; i < column->num_elems; i++) {
                                                        const jak_column_doc_obj
                                                                *object = JAK_VECTOR_GET(column, i, jak_column_doc_obj);
                                                        if (!_jak_column_doc_print_object(file, err, object, dic)) {
                                                                return false;
                                                        }
                                                        fprintf(file, "%s", i + 1 < column->num_elems ? ", " : "");
                                                }
                                                fprintf(file, "%s", column->num_elems > 1 ? "]" : "");
                                        }
                                                break;
                                        default: JAK_ERROR(err, JAK_ERR_NOTYPE)
                                                return false;
                                }
                                fprintf(file, array_idx + 1 < columnTable->values.num_elems ? ", " : "");
                        }
                        fprintf(file, "],");
                        fprintf(file, "\"Positions\": [");
                        for (size_t positionIdx = 0; positionIdx < columnTable->array_positions.num_elems;
                             positionIdx++) {
                                fprintf(file,
                                        "%d%s",
                                        *JAK_VECTOR_GET(&columnTable->array_positions, positionIdx, jak_i16),
                                        (positionIdx + 1 < columnTable->array_positions.num_elems ? ", " : ""));
                        }
                        fprintf(file, "]");
                        jak_string_dict_free(dic, decColumnKeyName);
                        fprintf(file, "}%s", columnIdx + 1 < arrayKeyColumns->columns.num_elems ? ", " : "");
                }
                fprintf(file, "]%s", array_key_idx + 1 < key_columns->num_elems ? ", " : "");
        }
        fprintf(file, "]");

        fprintf(file, "}");
        return true;
}

static bool
_jak_column_doc_print_object(FILE *file, jak_error *err, const jak_column_doc_obj *object, jak_string_dict *dic)
{
        char **parentKey = jak_string_dict_extract(dic, &object->parent_key, 1);
        fprintf(file, "{ ");
        fprintf(file,
                "\"Parent\": { \"Key\": %"PRIu64", \"Key Decoded\": \"%s\", \"Index\": %zu }, ",
                object->parent_key,
                parentKey[0],
                object->index);
        fprintf(file, "\"Pairs\": { ");
        fprintf(file, "\"Primitives\": { ");
        PRINT_PRIMITIVE_BOOLEAN_COLUMN(file, "Boolean", &object->bool_prop_keys, &object->bool_prop_vals, dic)
        PRINT_PRIMITIVE_COLUMN(file,
                               "UInt8",
                               &object->uint8_prop_keys,
                               &object->uint8_prop_vals,
                               &object->uint8_val_idxs,
                               dic,
                               jak_archive_field_u8_t,
                               "%d")
        PRINT_PRIMITIVE_COLUMN(file,
                               "UInt16",
                               &object->uint16_prop_keys,
                               &object->uint16_prop_vals,
                               &object->uint16_val_idxs,
                               dic,
                               jak_archive_field_u16_t,
                               "%d")
        PRINT_PRIMITIVE_COLUMN(file,
                               "UInt32",
                               &object->uin32_prop_keys,
                               &object->uint32_prop_vals,
                               &object->uint32_val_idxs,
                               dic,
                               jak_archive_field_u32_t,
                               "%d")
        PRINT_PRIMITIVE_COLUMN(file,
                               "UInt64",
                               &object->uint64_prop_keys,
                               &object->uint64_prop_vals,
                               &object->uint64_val_idxs,
                               dic,
                               jak_archive_field_u64_t,
                               "%"
                                       PRIu64)
        PRINT_PRIMITIVE_COLUMN(file,
                               "Int8",
                               &object->int8_prop_keys,
                               &object->int8_prop_vals,
                               &object->int8_val_idxs,
                               dic,
                               jak_archive_field_i8_t,
                               "%d")
        PRINT_PRIMITIVE_COLUMN(file,
                               "Int16",
                               &object->int16_prop_keys,
                               &object->int16_prop_vals,
                               &object->int16_val_idxs,
                               dic,
                               jak_archive_field_i16_t,
                               "%d")
        PRINT_PRIMITIVE_COLUMN(file,
                               "Int32",
                               &object->int32_prop_keys,
                               &object->int32_prop_vals,
                               &object->int32_val_idxs,
                               dic,
                               jak_archive_field_i32_t,
                               "%d")
        PRINT_PRIMITIVE_COLUMN(file,
                               "Int64",
                               &object->int64_prop_keys,
                               &object->int64_prop_vals,
                               &object->int64_val_idxs,
                               dic,
                               jak_archive_field_i64_t,
                               "%"
                                       PRIi64)
        PRINT_PRIMITIVE_COLUMN(file,
                               "Real",
                               &object->float_prop_keys,
                               &object->float_prop_vals,
                               &object->float_val_idxs,
                               dic,
                               jak_archive_field_number_t,
                               "%f")
        print_primitive_strings(file, "Strings", &object->jak_string_prop_keys, &object->jak_string_prop_vals, dic);
        print_primitive_null(file, "Null", &object->null_prop_keys, dic);
        if (print_primitive_objects(file, err, "Objects", &object->obj_prop_keys, &object->obj_prop_vals, dic)) {
                return false;
        }
        fprintf(file, "}, ");
        fprintf(file, "\"Arrays\": { ");
        PRINT_BOOLEAN_ARRAY(file, "Boolean", object->bool_array_prop_keys, object->bool_array_prop_vals);
        PRINT_ARRAY(file,
                    "UInt8",
                    object->uint8_array_prop_keys,
                    object->uint8_array_prop_vals,
                    jak_archive_field_u8_t,
                    "%d",
                    (value != JAK_NULL_UINT8));
        PRINT_ARRAY(file,
                    "UInt16",
                    object->uint16_array_prop_keys,
                    object->uint16_array_prop_vals,
                    jak_archive_field_u16_t,
                    "%d",
                    (value != JAK_NULL_UINT16));
        PRINT_ARRAY(file,
                    "UInt32",
                    object->uint32_array_prop_keys,
                    object->uint32_array_prop_vals,
                    jak_archive_field_u32_t,
                    "%d",
                    (value != JAK_NULL_UINT32));
        PRINT_ARRAY(file, "UInt64", object->uint64_array_prop_keys, object->ui64_array_prop_vals,
                    jak_archive_field_u64_t, "%"
                            PRIu64, (value != JAK_NULL_UINT64));
        PRINT_ARRAY(file,
                    "Int8",
                    object->int8_array_prop_keys,
                    object->int8_array_prop_vals,
                    jak_archive_field_i8_t,
                    "%d",
                    (value != JAK_NULL_INT8));
        PRINT_ARRAY(file,
                    "Int16",
                    object->int16_array_prop_keys,
                    object->int16_array_prop_vals,
                    jak_archive_field_i16_t,
                    "%d",
                    (value != JAK_NULL_INT16));
        PRINT_ARRAY(file,
                    "Int32",
                    object->int32_array_prop_keys,
                    object->int32_array_prop_vals,
                    jak_archive_field_i32_t,
                    "%d",
                    (value != JAK_NULL_INT32));
        PRINT_ARRAY(file, "Int64", object->int64_array_prop_keys, object->int64_array_prop_vals,
                    jak_archive_field_i64_t, "%"
                            PRIi64, (value != JAK_NULL_INT64));
        PRINT_ARRAY(file,
                    "Real",
                    object->float_array_prop_keys,
                    object->float_array_prop_vals,
                    jak_archive_field_number_t,
                    "%f",
                    (!isnan(value)));
        print_array_strings(file, "Strings", &object->jak_string_array_prop_keys, &object->jak_string_array_prop_vals, dic);
        print_array_null(file, "Null", &object->null_array_prop_keys, &object->null_array_prop_vals, dic);
        if (!print_array_objects(file, err, "Objects", &object->obj_array_props, dic)) {
                return false;
        }
        fprintf(file, "} ");
        fprintf(file, " }");
        fprintf(file, " }");
        jak_string_dict_free(dic, parentKey);
        return true;
}

bool jak_columndoc_print(FILE *file, jak_column_doc *doc)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(doc)
        return _jak_column_doc_print_object(file, &doc->err, &doc->columndoc, doc->dic);
}

bool jak_columndoc_drop(jak_column_doc *doc)
{
        JAK_UNUSED(doc);
        JAK_NOT_IMPLEMENTED
}

static void object_array_key_columns_create(jak_vector ofType(jak_column_doc_group) *columns)
{
        jak_vector_create(columns, NULL, sizeof(jak_column_doc_group), 20000);
}

static void object_array_key_columns_drop(jak_vector ofType(jak_column_doc_group) *columns)
{
        for (size_t i = 0; i < columns->num_elems; i++) {
                jak_column_doc_group *array_columns = JAK_VECTOR_GET(columns, i, jak_column_doc_group);
                for (size_t j = 0; j < array_columns->columns.num_elems; j++) {

                        jak_column_doc_column *column = JAK_VECTOR_GET(&array_columns->columns, j,
                                                                       jak_column_doc_column);

                        jak_vector ofType(jak_u32) *array_indices = &column->array_positions;
                        jak_vector ofType(jak_vector ofType(<T>)) *values_for_indicies = &column->values;

                        JAK_ASSERT (array_indices->num_elems == values_for_indicies->num_elems);

                        for (size_t k = 0; k < array_indices->num_elems; k++) {

                                jak_vector ofType(<T>)
                                        *values_for_index = JAK_VECTOR_GET(values_for_indicies, k, jak_vector);
                                if (column->type == JAK_FIELD_OBJECT) {
                                        for (size_t l = 0; l < values_for_index->num_elems; l++) {
                                                jak_column_doc_obj *nested_object =
                                                        JAK_VECTOR_GET(values_for_index, l, jak_column_doc_obj);
                                                object_meta_model_free(nested_object);
                                        }
                                }
                                jak_vector_drop(values_for_index);
                        }

                        jak_vector_drop(array_indices);
                        jak_vector_drop(values_for_indicies);
                }
                jak_vector_drop(&array_columns->columns);
        }
        jak_vector_drop(columns);
}

static const char *get_type_name(jak_error *err, jak_archive_field_e type)
{
        switch (type) {
                case JAK_FIELD_NULL:
                        return "Null";
                case JAK_FIELD_INT8:
                        return "Int8";
                case JAK_FIELD_INT16:
                        return "Int16";
                case JAK_FIELD_INT32:
                        return "Int32";
                case JAK_FIELD_INT64:
                        return "Int64";
                case JAK_FIELD_UINT8:
                        return "UInt8";
                case JAK_FIELD_UINT16:
                        return "UInt16";
                case JAK_FIELD_UINT32:
                        return "UInt32";
                case JAK_FIELD_UINT64:
                        return "UInt64";
                case JAK_FIELD_FLOAT:
                        return "Real";
                case JAK_FIELD_STRING:
                        return "String";
                case JAK_FIELD_OBJECT:
                        return "Object";
                default: {
                        JAK_ERROR(err, JAK_ERR_NOTYPE);
                        return NULL;
                }
        }
}

static jak_column_doc_column *object_array_key_columns_find_or_new(
        jak_vector ofType(jak_column_doc_group) *columns, jak_archive_field_sid_t array_key,
        jak_archive_field_sid_t nested_object_entry_key, jak_archive_field_e nested_object_entry_type)
{
        jak_column_doc_group *key_columns;
        jak_column_doc_column *key_column, *new_column;

        for (size_t i = 0; i < columns->num_elems; i++) {
                /** Find object array pair having the key `key` */
                key_columns = JAK_VECTOR_GET(columns, i, jak_column_doc_group);
                if (key_columns->key == array_key) {
                        /** In case such a pair is found, find column that matches the desired type */
                        for (size_t j = 0; j < key_columns->columns.num_elems; j++) {
                                key_column = JAK_VECTOR_GET(&key_columns->columns, j, jak_column_doc_column);
                                if (key_column->key_name == nested_object_entry_key
                                    && key_column->type == nested_object_entry_type) {
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
        key_columns = JAK_VECTOR_NEW_AND_GET(columns, jak_column_doc_group);
        key_columns->key = array_key;
        jak_vector_create(&key_columns->columns, NULL, sizeof(jak_column_doc_column), 10);

        objectArrayKeyColumnsNewColumn:
        new_column = JAK_VECTOR_NEW_AND_GET(&key_columns->columns, jak_column_doc_column);
        new_column->key_name = nested_object_entry_key;
        new_column->type = nested_object_entry_type;
        jak_vector_create(&new_column->values, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&new_column->array_positions, NULL, sizeof(jak_u32), 10);

        return new_column;
}

static bool object_array_key_column_push(jak_column_doc_column *col, jak_error *err,
                                         const jak_doc_entries *entry,
                                         jak_u32 array_idx, jak_string_dict *dic,
                                         jak_column_doc_obj *model)
{
        JAK_ASSERT(col->type == entry->type);

        jak_u32 *entry_array_idx = JAK_VECTOR_NEW_AND_GET(&col->array_positions, jak_u32);
        *entry_array_idx = array_idx;

        jak_vector ofType(<T>) *values_for_entry = JAK_VECTOR_NEW_AND_GET(&col->values, jak_vector);
        jak_vector_create(values_for_entry, NULL, GET_TYPE_SIZE(entry->type), entry->values.num_elems);

        bool is_null_by_def = entry->values.num_elems == 0;
        jak_u32 num_elements = (jak_u32) entry->values.num_elems;

        jak_archive_field_e entryType = is_null_by_def ? JAK_FIELD_NULL : entry->type;
        num_elements = is_null_by_def ? 1 : num_elements;

        switch (entryType) {
                case JAK_FIELD_NULL: {
                        jak_vector_push(values_for_entry, &num_elements, 1);
                }
                        break;
                case JAK_FIELD_BOOLEAN:
                case JAK_FIELD_INT8:
                case JAK_FIELD_INT16:
                case JAK_FIELD_INT32:
                case JAK_FIELD_INT64:
                case JAK_FIELD_UINT8:
                case JAK_FIELD_UINT16:
                case JAK_FIELD_UINT32:
                case JAK_FIELD_UINT64:
                case JAK_FIELD_FLOAT:
                        JAK_ASSERT(!is_null_by_def);
                        jak_vector_push(values_for_entry, entry->values.base, num_elements);
                        break;
                case JAK_FIELD_STRING: {
                        JAK_ASSERT(!is_null_by_def);
                        char **strings = JAK_VECTOR_ALL(&entry->values, char *);
                        jak_archive_field_sid_t *jak_string_ids;
                        jak_string_dict_locate_fast(&jak_string_ids, dic, (char *const *) strings, num_elements);
                        jak_vector_push(values_for_entry, jak_string_ids, num_elements);
                        jak_string_dict_free(dic, jak_string_ids);
                        //jak_string_dict_free(encode, strings);
                }
                        break;
                case JAK_FIELD_OBJECT:
                        JAK_ASSERT(!is_null_by_def);

                        jak_archive_field_sid_t *array_key;
                        jak_string_dict_locate_fast(&array_key, dic, (char *const *) &entry->key, 1);

                        for (size_t array_idx = 0; array_idx < num_elements; array_idx++) {
                                jak_column_doc_obj
                                        *nested_object = JAK_VECTOR_NEW_AND_GET(values_for_entry, jak_column_doc_obj);
                                setup_object(nested_object, model->parent, *array_key, array_idx);
                                if (!import_object(nested_object,
                                                   err,
                                                   JAK_VECTOR_GET(&entry->values, array_idx, jak_doc_obj),
                                                   dic)) {
                                        return false;
                                }
                        }
                        jak_string_dict_free(dic, array_key);
                        break;
                default: JAK_ERROR(err, JAK_ERR_NOTYPE);
                        return false;
        }
        return true;
}

static void
setup_object(jak_column_doc_obj *model, jak_column_doc *parent, jak_archive_field_sid_t key, size_t idx)
{
        model->parent = parent;
        model->parent_key = key;
        model->index = idx;

        jak_vector_create(&model->bool_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->int8_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->int16_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->int32_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->int64_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->uint8_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->uint16_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->uin32_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->uint64_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->jak_string_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 50);
        jak_vector_create(&model->float_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->null_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->obj_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);

        jak_vector_create(&model->bool_array_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->int8_array_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->int16_array_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->int32_array_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->int64_array_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->uint8_array_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->uint16_array_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->uint32_array_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->uint64_array_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->jak_string_array_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 50);
        jak_vector_create(&model->float_array_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);
        jak_vector_create(&model->null_array_prop_keys, NULL, sizeof(jak_archive_field_sid_t), 10);

        jak_vector_create(&model->bool_prop_vals, NULL, sizeof(jak_archive_field_boolean_t), 10);
        jak_vector_create(&model->int8_prop_vals, NULL, sizeof(jak_archive_field_i8_t), 10);
        jak_vector_create(&model->int16_prop_vals, NULL, sizeof(jak_archive_field_i16_t), 10);
        jak_vector_create(&model->int32_prop_vals, NULL, sizeof(jak_archive_field_i32_t), 10);
        jak_vector_create(&model->int64_prop_vals, NULL, sizeof(jak_archive_field_i64_t), 10);
        jak_vector_create(&model->uint8_prop_vals, NULL, sizeof(jak_archive_field_u8_t), 10);
        jak_vector_create(&model->uint16_prop_vals, NULL, sizeof(jak_archive_field_u16_t), 10);
        jak_vector_create(&model->uint32_prop_vals, NULL, sizeof(jak_archive_field_u32_t), 10);
        jak_vector_create(&model->uint64_prop_vals, NULL, sizeof(jak_archive_field_u64_t), 10);
        jak_vector_create(&model->float_prop_vals, NULL, sizeof(jak_archive_field_number_t), 10);
        jak_vector_create(&model->jak_string_prop_vals, NULL, sizeof(jak_archive_field_sid_t), 50);

        jak_vector_create(&model->bool_array_prop_vals, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->int8_array_prop_vals, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->int16_array_prop_vals, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->int32_array_prop_vals, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->int64_array_prop_vals, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->uint8_array_prop_vals, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->uint16_array_prop_vals, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->uint32_array_prop_vals, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->ui64_array_prop_vals, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->float_array_prop_vals, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->jak_string_array_prop_vals, NULL, sizeof(jak_vector), 50);
        jak_vector_create(&model->null_array_prop_vals, NULL, sizeof(jak_u16), 10);

        jak_vector_create(&model->bool_val_idxs, NULL, sizeof(jak_u32), 10);
        jak_vector_create(&model->int8_val_idxs, NULL, sizeof(jak_u32), 10);
        jak_vector_create(&model->int16_val_idxs, NULL, sizeof(jak_u32), 10);
        jak_vector_create(&model->int32_val_idxs, NULL, sizeof(jak_u32), 10);
        jak_vector_create(&model->int64_val_idxs, NULL, sizeof(jak_u32), 10);
        jak_vector_create(&model->uint8_val_idxs, NULL, sizeof(jak_u32), 10);
        jak_vector_create(&model->uint16_val_idxs, NULL, sizeof(jak_u32), 10);
        jak_vector_create(&model->uint32_val_idxs, NULL, sizeof(jak_u32), 10);
        jak_vector_create(&model->uint64_val_idxs, NULL, sizeof(jak_u32), 10);
        jak_vector_create(&model->float_val_idxs, NULL, sizeof(jak_u32), 10);
        jak_vector_create(&model->jak_string_val_idxs, NULL, sizeof(jak_u32), 50);

        jak_vector_create(&model->bool_array_idxs, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->int8_array_idxs, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->int16_array_idxs, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->int32_array_idxs, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->int64_array_idxs, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->uint8_array_idxs, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->uint16_array_idxs, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->uint32_array_idxs, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->uint64_array_idxs, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->float_array_idxs, NULL, sizeof(jak_vector), 10);
        jak_vector_create(&model->jak_string_array_idxs, NULL, sizeof(jak_vector), 50);

        jak_vector_create(&model->obj_prop_vals, NULL, sizeof(jak_column_doc_obj), 10);

        object_array_key_columns_create(&model->obj_array_props);
}

static bool
object_put_primitive(jak_column_doc_obj *columndoc, jak_error *err, const jak_doc_entries *entry,
                     jak_string_dict *dic, const jak_archive_field_sid_t *key_id)
{
        switch (entry->type) {
                case JAK_FIELD_NULL:
                        jak_vector_push(&columndoc->null_prop_keys, key_id, 1);
                        break;
                case JAK_FIELD_BOOLEAN:
                        jak_vector_push(&columndoc->bool_prop_keys, key_id, 1);
                        jak_vector_push(&columndoc->bool_prop_vals, entry->values.base, 1);
                        break;
                case JAK_FIELD_INT8:
                        jak_vector_push(&columndoc->int8_prop_keys, key_id, 1);
                        jak_vector_push(&columndoc->int8_prop_vals, entry->values.base, 1);
                        break;
                case JAK_FIELD_INT16:
                        jak_vector_push(&columndoc->int16_prop_keys, key_id, 1);
                        jak_vector_push(&columndoc->int16_prop_vals, entry->values.base, 1);
                        break;
                case JAK_FIELD_INT32:
                        jak_vector_push(&columndoc->int32_prop_keys, key_id, 1);
                        jak_vector_push(&columndoc->int32_prop_vals, entry->values.base, 1);
                        break;
                case JAK_FIELD_INT64:
                        jak_vector_push(&columndoc->int64_prop_keys, key_id, 1);
                        jak_vector_push(&columndoc->int64_prop_vals, entry->values.base, 1);
                        break;
                case JAK_FIELD_UINT8:
                        jak_vector_push(&columndoc->uint8_prop_keys, key_id, 1);
                        jak_vector_push(&columndoc->uint8_prop_vals, entry->values.base, 1);
                        break;
                case JAK_FIELD_UINT16:
                        jak_vector_push(&columndoc->uint16_prop_keys, key_id, 1);
                        jak_vector_push(&columndoc->uint16_prop_vals, entry->values.base, 1);
                        break;
                case JAK_FIELD_UINT32:
                        jak_vector_push(&columndoc->uin32_prop_keys, key_id, 1);
                        jak_vector_push(&columndoc->uint32_prop_vals, entry->values.base, 1);
                        break;
                case JAK_FIELD_UINT64:
                        jak_vector_push(&columndoc->uint64_prop_keys, key_id, 1);
                        jak_vector_push(&columndoc->uint64_prop_vals, entry->values.base, 1);
                        break;
                case JAK_FIELD_FLOAT:
                        jak_vector_push(&columndoc->float_prop_keys, key_id, 1);
                        jak_vector_push(&columndoc->float_prop_vals, entry->values.base, 1);
                        break;
                case JAK_FIELD_STRING: {
                        jak_archive_field_sid_t *value;
                        jak_string_dict_locate_fast(&value, dic, (char *const *) entry->values.base, 1);
                        jak_vector_push(&columndoc->jak_string_prop_keys, key_id, 1);
                        jak_vector_push(&columndoc->jak_string_prop_vals, value, 1);
                        jak_string_dict_free(dic, value);
                }
                        break;
                case JAK_FIELD_OBJECT: {
                        jak_column_doc_obj template, *nested_object;
                        size_t position = jak_vector_length(&columndoc->obj_prop_keys);
                        jak_vector_push(&columndoc->obj_prop_keys, key_id, 1);
                        jak_vector_push(&columndoc->obj_prop_vals, &template, 1);
                        nested_object = JAK_VECTOR_GET(&columndoc->obj_prop_vals, position, jak_column_doc_obj);
                        setup_object(nested_object, columndoc->parent, *key_id, 0);
                        if (!import_object(nested_object, err, JAK_VECTOR_GET(&entry->values, 0, jak_doc_obj), dic)) {
                                return false;
                        }
                }
                        break;
                        JAK_ERROR(err, JAK_ERR_NOTYPE)
                        return false;
        }
        return true;
}

static void object_push_array(jak_vector ofType(Vector
                                                               ofType( < T >)) *values, size_t TSize,
                              jak_u32 num_elements,
                              const void *data, jak_archive_field_sid_t key_id,
                              jak_vector ofType(jak_archive_field_sid_t) *key_vector)
{
        jak_vector ofType(<T>) template, *vector;
        size_t idx = jak_vector_length(values);
        jak_vector_push(values, &template, 1);
        vector = JAK_VECTOR_GET(values, idx, jak_vector);
        jak_vector_create(vector, NULL, TSize, num_elements);
        jak_vector_push(vector, data, num_elements);
        jak_vector_push(key_vector, &key_id, 1);
}

static bool
object_put_array(jak_column_doc_obj *model, jak_error *err, const jak_doc_entries *entry,
                 jak_string_dict *dic, const jak_archive_field_sid_t *key_id)
{
        JAK_UNUSED(dic);
        jak_u32 num_elements = (jak_u32) jak_vector_length(&entry->values);

        switch (entry->type) {
                case JAK_FIELD_NULL: {
                        jak_vector_push(&model->null_array_prop_vals, &num_elements, 1);
                        jak_vector_push(&model->null_array_prop_keys, key_id, 1);
                }
                        break;
                case JAK_FIELD_BOOLEAN:
                        object_push_array(&model->bool_array_prop_vals,
                                          sizeof(jak_archive_field_boolean_t),
                                          num_elements,
                                          entry->values.base,
                                          *key_id,
                                          &model->bool_array_prop_keys);
                        break;
                case JAK_FIELD_INT8:
                        object_push_array(&model->int8_array_prop_vals,
                                          sizeof(jak_archive_field_i8_t),
                                          num_elements,
                                          entry->values.base,
                                          *key_id,
                                          &model->int8_array_prop_keys);
                        break;
                case JAK_FIELD_INT16:
                        object_push_array(&model->int16_array_prop_vals,
                                          sizeof(jak_archive_field_i16_t),
                                          num_elements,
                                          entry->values.base,
                                          *key_id,
                                          &model->int16_array_prop_keys);
                        break;
                case JAK_FIELD_INT32:
                        object_push_array(&model->int32_array_prop_vals,
                                          sizeof(jak_archive_field_i32_t),
                                          num_elements,
                                          entry->values.base,
                                          *key_id,
                                          &model->int32_array_prop_keys);
                        break;
                case JAK_FIELD_INT64:
                        object_push_array(&model->int64_array_prop_vals,
                                          sizeof(jak_archive_field_i64_t),
                                          num_elements,
                                          entry->values.base,
                                          *key_id,
                                          &model->int64_array_prop_keys);
                        break;
                case JAK_FIELD_UINT8:
                        object_push_array(&model->uint8_array_prop_vals,
                                          sizeof(jak_archive_field_u8_t),
                                          num_elements,
                                          entry->values.base,
                                          *key_id,
                                          &model->uint8_array_prop_keys);
                        break;
                case JAK_FIELD_UINT16:
                        object_push_array(&model->uint16_array_prop_vals,
                                          sizeof(jak_archive_field_u16_t),
                                          num_elements,
                                          entry->values.base,
                                          *key_id,
                                          &model->uint16_array_prop_keys);
                        break;
                case JAK_FIELD_UINT32:
                        object_push_array(&model->uint32_array_prop_vals,
                                          sizeof(jak_archive_field_u32_t),
                                          num_elements,
                                          entry->values.base,
                                          *key_id,
                                          &model->uint32_array_prop_keys);
                        break;
                case JAK_FIELD_UINT64:
                        object_push_array(&model->ui64_array_prop_vals,
                                          sizeof(jak_archive_field_u64_t),
                                          num_elements,
                                          entry->values.base,
                                          *key_id,
                                          &model->uint64_array_prop_keys);
                        break;
                case JAK_FIELD_FLOAT:
                        object_push_array(&model->float_array_prop_vals,
                                          sizeof(jak_archive_field_number_t),
                                          num_elements,
                                          entry->values.base,
                                          *key_id,
                                          &model->float_array_prop_keys);
                        break;
                case JAK_FIELD_STRING: {
                        const char **strings = JAK_VECTOR_ALL(&entry->values, const char *);
                        jak_archive_field_sid_t *jak_string_ids;
                        jak_string_dict_locate_fast(&jak_string_ids, dic, (char *const *) strings, num_elements);
                        object_push_array(&model->jak_string_array_prop_vals,
                                          sizeof(jak_archive_field_sid_t),
                                          num_elements,
                                          jak_string_ids,
                                          *key_id,
                                          &model->jak_string_array_prop_keys);
                        jak_string_dict_free(dic, jak_string_ids);
                }
                        break;
                case JAK_FIELD_OBJECT: {
                        jak_archive_field_sid_t *nested_object_key_name;
                        for (jak_u32 array_idx = 0; array_idx < num_elements; array_idx++) {
                                const jak_doc_obj *object = JAK_VECTOR_GET(&entry->values, array_idx,
                                                                           jak_doc_obj);
                                for (size_t pair_idx = 0; pair_idx < object->entries.num_elems; pair_idx++) {
                                        const jak_doc_entries
                                                *pair = JAK_VECTOR_GET(&object->entries, pair_idx, jak_doc_entries);
                                        jak_string_dict_locate_fast(&nested_object_key_name, dic, (char *const *) &pair->key, 1);
                                        jak_column_doc_column *key_column =
                                                object_array_key_columns_find_or_new(&model->obj_array_props,
                                                                                     *key_id,
                                                                                     *nested_object_key_name,
                                                                                     pair->type);
                                        if (!object_array_key_column_push(key_column, err, pair, array_idx, dic,
                                                                          model)) {
                                                return false;
                                        }
                                        jak_string_dict_free(dic, nested_object_key_name);
                                }
                        }
                }
                        break;
                default: {
                        JAK_ERROR(err, JAK_ERR_NOTYPE)
                        return false;
                }
                        break;
        }
        return true;
}

static bool object_put(jak_column_doc_obj *model, jak_error *err, const jak_doc_entries *entry,
                       jak_string_dict *dic)
{
        jak_archive_field_sid_t *key_id;
        enum EntryType {
                ENTRY_TYPE_NULL, ENTRY_TYPE_PRIMITIVE, ENTRY_TYPE_ARRAY
        } entryType;

        jak_string_dict_locate_fast(&key_id, dic, (char *const *) &entry->key, 1);
        entryType =
                entry->values.num_elems == 0 ? ENTRY_TYPE_NULL : (entry->values.num_elems == 1 ? ENTRY_TYPE_PRIMITIVE
                                                                                               : ENTRY_TYPE_ARRAY);

        switch (entryType) {
                case ENTRY_TYPE_NULL:
                        /** For a key which does not jak_async_map_exec to any value, the value is defined as 'null'  */
                        jak_vector_push(&model->null_prop_keys, key_id, 1);
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
                default: JAK_ERROR(err, JAK_ERR_NOTYPE)
                        return false;
        }

        jak_string_dict_free(dic, key_id);
        return true;
}

static bool import_object(jak_column_doc_obj *dst, jak_error *err, const jak_doc_obj *doc,
                          jak_string_dict *dic)
{
        const jak_vector ofType(jak_doc_entries) *objectEntries = jak_doc_get_entries(doc);
        const jak_doc_entries *entries = JAK_VECTOR_ALL(objectEntries, jak_doc_entries);
        for (size_t i = 0; i < objectEntries->num_elems; i++) {
                const jak_doc_entries *entry = entries + i;
                if (!object_put(dst, err, entry, dic)) {
                        return false;
                }
        }
        return true;
}