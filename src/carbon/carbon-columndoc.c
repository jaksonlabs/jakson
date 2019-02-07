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

#include "carbon/carbon-columndoc.h"
#include "carbon/carbon-doc.h"

static void setupObject(carbon_columndoc_obj_t *model, carbon_columndoc_t *parent, carbon_string_id_t key, size_t idx);

static bool objectPut(carbon_columndoc_obj_t *model, carbon_err_t *err, const carbon_doc_entries_t *entry, carbon_strdic_t *dic);

static bool importObject(carbon_columndoc_obj_t *dst, carbon_err_t *err, const carbon_doc_obj_t *objectModel, carbon_strdic_t *dic);

static bool printObject(FILE *file, carbon_err_t *err, const carbon_columndoc_obj_t *object, carbon_strdic_t *dic);

static const char *getTypeName(carbon_err_t *err, carbon_field_type_e type);

static void objectArrayKeyColumnsCreate(carbon_vec_t ofType(carbon_columndoc_columngroup_t) *columns);

static carbon_columndoc_column_t *objectArrayKeyColumnsFindOrNew(carbon_vec_t ofType(carbon_columndoc_columngroup_t) *columns,
                                                            carbon_string_id_t arrayKey, carbon_string_id_t nestedObjectEntryKey,
                                                            carbon_field_type_e nestedObjectEntryType);

static bool objectArrayKeyColumnPush(carbon_columndoc_column_t *col, carbon_err_t *err, const carbon_doc_entries_t *entry, uint32_t arrayIdx,
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

    const char *rootString = "/";
    carbon_string_id_t *rootId;

    carbon_strdic_insert(dic, &rootId, (char *const *) &rootString, 1, 0);

    setupObject(&columndoc->columndoc, columndoc, *rootId, 0);

    carbon_strdic_free(dic, rootId);

    const carbon_doc_obj_t *root = carbon_doc_entries_get_root(entries);
    if (!importObject(&columndoc->columndoc, err, root, dic)) {
        return false;
    }

    return true;
}

static void objectArrayKeyColumnsDrop(carbon_vec_t ofType(carbon_columndoc_columngroup_t) *columns);

static void objectMetaModelFree(carbon_columndoc_obj_t *metaModel)
{
    VectorDrop(&metaModel->bool_prop_keys);
    VectorDrop(&metaModel->int8_prop_keys);
    VectorDrop(&metaModel->int16_prop_keys);
    VectorDrop(&metaModel->int32_prop_keys);
    VectorDrop(&metaModel->int64_prop_keys);
    VectorDrop(&metaModel->uint8_prop_keys);
    VectorDrop(&metaModel->uint16_prop_keys);
    VectorDrop(&metaModel->uin32_prop_keys);
    VectorDrop(&metaModel->uint64_prop_keys);
    VectorDrop(&metaModel->string_prop_keys);
    VectorDrop(&metaModel->float_prop_keys);
    VectorDrop(&metaModel->null_prop_keys);
    VectorDrop(&metaModel->obj_prop_keys);

    VectorDrop(&metaModel->bool_array_prop_keys);
    VectorDrop(&metaModel->int8_array_prop_keys);
    VectorDrop(&metaModel->int16_array_prop_keys);
    VectorDrop(&metaModel->int32_array_prop_keys);
    VectorDrop(&metaModel->int64_array_prop_keys);
    VectorDrop(&metaModel->uint8_array_prop_keys);
    VectorDrop(&metaModel->uint16_array_prop_keys);
    VectorDrop(&metaModel->uint32_array_prop_keys);
    VectorDrop(&metaModel->uint64_array_prop_keys);
    VectorDrop(&metaModel->string_array_prop_keys);
    VectorDrop(&metaModel->float_array_prop_keys);
    VectorDrop(&metaModel->null_array_prop_keys);

    VectorDrop(&metaModel->bool_prop_vals);
    VectorDrop(&metaModel->int8_prop_vals);
    VectorDrop(&metaModel->int16_prop_vals);
    VectorDrop(&metaModel->int32_prop_vals);
    VectorDrop(&metaModel->int64_prop_vals);
    VectorDrop(&metaModel->uint8_prop_vals);
    VectorDrop(&metaModel->uint16_prop_vals);
    VectorDrop(&metaModel->uint32_prop_vals);
    VectorDrop(&metaModel->uint64_prop_vals);
    VectorDrop(&metaModel->float_prop_vals);
    VectorDrop(&metaModel->string_prop_vals);

    for (size_t i = 0; i < metaModel->bool_array_prop_vals.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->bool_array_prop_vals, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->bool_array_prop_vals);

    for (size_t i = 0; i < metaModel->int8_array_prop_vals.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->int8_array_prop_vals, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->int8_array_prop_vals);

    for (size_t i = 0; i < metaModel->int16_array_prop_vals.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->int16_array_prop_vals, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->int16_array_prop_vals);

    for (size_t i = 0; i < metaModel->int32_array_prop_vals.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->int32_array_prop_vals, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->int32_array_prop_vals);

    for (size_t i = 0; i < metaModel->int64_array_prop_vals.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->int64_array_prop_vals, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->int64_array_prop_vals);

    for (size_t i = 0; i < metaModel->uint8_array_prop_vals.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->uint8_array_prop_vals, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->uint8_array_prop_vals);

    for (size_t i = 0; i < metaModel->uint16_array_prop_vals.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->uint16_array_prop_vals, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->uint16_array_prop_vals);

    for (size_t i = 0; i < metaModel->uint32_array_prop_vals.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->uint32_array_prop_vals, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->uint32_array_prop_vals);

    for (size_t i = 0; i < metaModel->uin64_array_prop_vals.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->uin64_array_prop_vals, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->uin64_array_prop_vals);

    for (size_t i = 0; i < metaModel->float_array_prop_vals.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->float_array_prop_vals, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->float_array_prop_vals);

    for (size_t i = 0; i < metaModel->string_array_prop_vals.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->string_array_prop_vals, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->string_array_prop_vals);

    VectorDrop(&metaModel->null_array_prop_vals);

    VectorDrop(&metaModel->bool_val_idxs);
    VectorDrop(&metaModel->int8_val_idxs);
    VectorDrop(&metaModel->int16_val_idxs);
    VectorDrop(&metaModel->int32_val_idxs);
    VectorDrop(&metaModel->int64_val_idxs);
    VectorDrop(&metaModel->uint8_val_idxs);
    VectorDrop(&metaModel->uint16_val_idxs);
    VectorDrop(&metaModel->uint32_val_idxs);
    VectorDrop(&metaModel->uint64_val_idxs);
    VectorDrop(&metaModel->float_val_idxs);
    VectorDrop(&metaModel->string_val_idxs);

    for (size_t i = 0; i < metaModel->bool_array_idxs.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->bool_array_idxs, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->bool_array_idxs);

    for (size_t i = 0; i < metaModel->int8_array_idxs.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->int8_array_idxs, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->int8_array_idxs);

    for (size_t i = 0; i < metaModel->int16_array_idxs.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->int16_array_idxs, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->int16_array_idxs);

    for (size_t i = 0; i < metaModel->int32_array_idxs.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->int32_array_idxs, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->int32_array_idxs);

    for (size_t i = 0; i < metaModel->int64_array_idxs.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->int64_array_idxs, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->int64_array_idxs);

    for (size_t i = 0; i < metaModel->uint8_array_idxs.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->uint8_array_idxs, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->uint8_array_idxs);

    for (size_t i = 0; i < metaModel->uint16_array_idxs.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->uint16_array_idxs, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->uint16_array_idxs);

    for (size_t i = 0; i < metaModel->uint32_array_idxs.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->uint32_array_idxs, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->uint32_array_idxs);

    for (size_t i = 0; i < metaModel->uint64_array_idxs.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->uint64_array_idxs, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->uint64_array_idxs);

    for (size_t i = 0; i < metaModel->float_array_idxs.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->float_array_idxs, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->float_array_idxs);

    for (size_t i = 0; i < metaModel->string_array_idxs.numElems; i++) {
        carbon_vec_t *vec = VECTOR_GET(&metaModel->string_array_idxs, i, carbon_vec_t);
        VectorDrop(vec);
    }
    VectorDrop(&metaModel->string_array_idxs);

    for (size_t i = 0; i < metaModel->obj_prop_vals.numElems; i++) {
        carbon_columndoc_obj_t *object = VECTOR_GET(&metaModel->obj_prop_vals, i, carbon_columndoc_obj_t);
        objectMetaModelFree(object);
    }
    VectorDrop(&metaModel->obj_prop_vals);

    objectArrayKeyColumnsDrop(&metaModel->obj_array_props);
}

bool carbon_columndoc_free(carbon_columndoc_t *doc)
{
    CARBON_NON_NULL_OR_ERROR(doc);
    objectMetaModelFree(&doc->columndoc);
    return true;
}

#define PRINT_PRIMITIVE_KEY_PART(file, typeName, keyVector, dic, suffix)                                \
{                                                                                                       \
    fprintf(file, "\"%s\": { ", typeName);                                                              \
    if(!carbon_vec_is_empty((keyVector))) {                                                                   \
        fprintf(file, "\"Keys\": [ ");                                                                  \
        for (size_t i = 0; i < (keyVector)->numElems; i++) {                                            \
            carbon_string_id_t string_id = *VECTOR_GET((keyVector), i, carbon_string_id_t);              \
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (keyVector)->numElems ? ", " : "");                \
        }                                                                                               \
        fprintf(file, "], ");                                                                           \
        fprintf(file, "\"Keys Decoded\": [ ");                                                          \
        for (size_t i = 0; i < (keyVector)->numElems; i++) {                                            \
            carbon_string_id_t string_id = *VECTOR_GET((keyVector), i, carbon_string_id_t);              \
            char **encString = carbon_strdic_extract(dic, &string_id, 1);                              \
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (keyVector)->numElems ? ", " : "");         \
            carbon_strdic_free(dic, encString);                                                       \
        }                                                                                               \
        fprintf(file, "]%s", suffix);                                                                   \
    }                                                                                                   \
}                                                                                                       \

#define PRINT_PRIMITIVE_COLUMN(file, typeName, keyVector, valueVector, keyIndicesVector, dic, TYPE, FORMAT_STR)           \
{                                                                                                       \
    PRINT_PRIMITIVE_KEY_PART(file, typeName, keyVector, dic, ", ")                                      \
    if(!carbon_vec_is_empty((keyVector))) {                                                                   \
        fprintf(file, "\"Values\": [ ");                                                                \
        for (size_t i = 0; i < (valueVector)->numElems; i++) {                                          \
            TYPE value = *VECTOR_GET(valueVector, i, TYPE);                                             \
            fprintf(file, FORMAT_STR "%s", value, i + 1 < (valueVector)->numElems ? ", " : "");         \
        }                                                                                               \
        fprintf(file, "] ");                                                                            \
    }                                                                                                   \
    fprintf(file, "}, ");                                                                               \
}


#define PRINT_PRIMITIVE_BOOLEAN_COLUMN(file, typeName, keyVector, valueVector, dic)                             \
{                                                                                                               \
    PRINT_PRIMITIVE_KEY_PART(file, typeName, keyVector, dic, ", ")                                              \
    if(!carbon_vec_is_empty((keyVector))) {                                                                           \
        fprintf(file, "\"Values\": [ ");                                                                        \
        for (size_t i = 0; i < (valueVector)->numElems; i++) {                                                  \
            carbon_bool_t value = *VECTOR_GET(valueVector, i, carbon_bool_t);                                     \
            fprintf(file, "%s%s", value == 0 ? "false" : "true", i + 1 < (valueVector)->numElems ? ", " : "");  \
        }                                                                                                       \
        fprintf(file, "]");                                                                                     \
    }                                                                                                           \
    fprintf(file, "}, ");                                                                                       \
}

static void printPrimitiveNull(FILE *file, const char *typeName, const carbon_vec_t ofType(carbon_string_id_t) *keyVector,
                               carbon_strdic_t *dic)
{
    PRINT_PRIMITIVE_KEY_PART(file, typeName, keyVector, dic, "")
    fprintf(file, "}, ");
}



static bool printPrimitiveObjects(FILE *file, carbon_err_t *err, const char *typeName, const carbon_vec_t ofType(carbon_string_id_t) *keyVector,
                                  const carbon_vec_t ofType(carbon_columndoc_obj_t) *valueVector, carbon_strdic_t *dic)
{
    PRINT_PRIMITIVE_KEY_PART(file, typeName, keyVector, dic, ", ")
    if(!carbon_vec_is_empty((keyVector))) {
        fprintf(file, "\"Values\": [ ");
        for (size_t i = 0; i < (valueVector)->numElems; i++) {
            const carbon_columndoc_obj_t *object = VECTOR_GET(valueVector, i, carbon_columndoc_obj_t);
            if(!printObject(file, err, object, dic)) {
                return false;
            }
            fprintf(file, "%s", i + 1 < (valueVector)->numElems ? ", " : "");
        }
        fprintf(file, "]");
    }
    fprintf(file, "}");
    return true;
}

#define PRINT_ARRAY(file, typeName, keyVector, valueVector, TYPE, TYPE_FORMAT, nonnull_expr)            \
{                                                                                                       \
    fprintf(file, "\"%s\": { ", typeName);                                                              \
    if(!carbon_vec_is_empty((&keyVector))) {                                                                  \
        fprintf(file, "\"Keys\": [ ");                                                                  \
        for (size_t i = 0; i < (&keyVector)->numElems; i++) {                                           \
            carbon_string_id_t string_id = *VECTOR_GET((&keyVector), i, carbon_string_id_t);             \
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (&keyVector)->numElems ? ", " : "");               \
        }                                                                                               \
        fprintf(file, "], ");                                                                           \
        fprintf(file, "\"Keys Decoded\": [ ");                                                          \
        for (size_t i = 0; i < (&keyVector)->numElems; i++) {                                           \
            carbon_string_id_t string_id = *VECTOR_GET((&keyVector), i, carbon_string_id_t);             \
            char **encString = carbon_strdic_extract(dic, &string_id, 1);                              \
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (&keyVector)->numElems ? ", " : "");        \
            carbon_strdic_free(dic, encString);                                                       \
        }                                                                                               \
        fprintf(file, "],");                                                                            \
        fprintf(file, "\"Values\": [ ");                                                                \
        for (size_t i = 0; i < (&valueVector)->numElems; i++) {                                         \
            const carbon_vec_t ofType(TYPE) *values = VECTOR_GET(&valueVector, i, carbon_vec_t);      \
            fprintf(file, "[ ");                                                                        \
            for (size_t j = 0; j < values->numElems; j++) {                                             \
                TYPE value = *VECTOR_GET(values, j, TYPE);                                              \
                if (nonnull_expr) {                                                                     \
                    fprintf(file, "" TYPE_FORMAT "%s", value, j + 1 < values->numElems ? ", " : "");    \
                } else {                                                                                \
                    fprintf(file, CARBON_NULL_TEXT "%s", j + 1 < values->numElems ? ", " : "");          \
                }                                                                                       \
            }                                                                                           \
            fprintf(file, "]%s ", i + 1 < (&valueVector)->numElems ? "," : "");                         \
        }                                                                                               \
        fprintf(file, "]");                                                                             \
    }                                                                                                   \
    fprintf(file, "}, ");                                                                               \
}

#define PRINT_BOOLEAN_ARRAY(file, typeName, keyVector, valueVector)                                         \
{                                                                                                           \
    fprintf(file, "\"%s\": { ", "Boolean");                                                                 \
    if(!carbon_vec_is_empty((&keyVector))) {                                                                      \
        fprintf(file, "\"Keys\": [ ");                                                                      \
        for (size_t i = 0; i < (&keyVector)->numElems; i++) {                                               \
            carbon_string_id_t string_id = *VECTOR_GET((&keyVector), i, carbon_string_id_t);                 \
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (&keyVector)->numElems ? ", " : "");                   \
        }                                                                                                   \
        fprintf(file, "], ");                                                                               \
        fprintf(file, "\"Keys Decoded\": [ ");                                                              \
        for (size_t i = 0; i < (&keyVector)->numElems; i++) {                                               \
            carbon_string_id_t string_id = *VECTOR_GET((&keyVector), i, carbon_string_id_t);                 \
            char **encString = carbon_strdic_extract(dic, &string_id, 1);                                  \
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (&keyVector)->numElems ? ", " : "");            \
            carbon_strdic_free(dic, encString);                                                           \
        }                                                                                                   \
        fprintf(file, "],");                                                                                \
        fprintf(file, "\"Values\": [ ");                                                                    \
        for (size_t i = 0; i < (&valueVector)->numElems; i++) {                                             \
            const carbon_vec_t ofType(carbon_bool_t) *values = VECTOR_GET(&valueVector, i, carbon_vec_t);  \
            fprintf(file, "[ ");                                                                            \
            for (size_t j = 0; j < values->numElems; j++) {                                                 \
                carbon_bool_t value = *VECTOR_GET(values, j, carbon_bool_t);                                  \
                fprintf(file, "%s%s", value == 0 ? "false" : "true", j + 1 < values->numElems ? ", " : ""); \
            }                                                                                               \
            fprintf(file, "]%s ", i + 1 < (&valueVector)->numElems ? "," : "");                             \
        }                                                                                                   \
        fprintf(file, "]");                                                                                 \
    }                                                                                                       \
    fprintf(file, "}, ");                                                                                   \
}

static void printArrayNull(FILE *file, const char *typeName, const carbon_vec_t ofType(carbon_string_id_t) *keyVector,
                           const carbon_vec_t ofType(uint16_t) *valueVector, carbon_strdic_t *dic)
{
    fprintf(file, "\"%s\": { ", typeName);
    if(!carbon_vec_is_empty((keyVector))) {
        fprintf(file, "\"Keys\": [ ");
        for (size_t i = 0; i < (keyVector)->numElems; i++) {
            carbon_string_id_t string_id = *VECTOR_GET((keyVector), i, carbon_string_id_t);
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (keyVector)->numElems ? ", " : "");
        }
        fprintf(file, "], ");
        fprintf(file, "\"Keys Decoded\": [ ");
        for (size_t i = 0; i < (keyVector)->numElems; i++) {
            carbon_string_id_t string_id = *VECTOR_GET((keyVector), i, carbon_string_id_t);
            char **encString = carbon_strdic_extract(dic, &string_id, 1);
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (keyVector)->numElems ? ", " : "");
            carbon_strdic_free(dic, encString);
        }
        fprintf(file, "],");
        fprintf(file, "\"Values\": [ ");
        for (size_t i = 0; i < (valueVector)->numElems; i++) {
            uint16_t amount = *VECTOR_GET(valueVector, i, uint16_t);
            fprintf(file, "%d%s", amount, i + 1 < valueVector->numElems ? ", " : "");
        }
        fprintf(file, "]");
    }
    fprintf(file, "}, ");
}

static void printArrayStrings(FILE *file, const char *typeName, const carbon_vec_t ofType(carbon_string_id_t) *keyVector,
                           const carbon_vec_t ofType(Vector ofType(carbon_string_id_t)) *valueVector, carbon_strdic_t *dic)
{
    fprintf(file, "\"%s\": { ", typeName);
    if(!carbon_vec_is_empty((keyVector))) {
        fprintf(file, "\"Keys\": [ ");
        for (size_t i = 0; i < (keyVector)->numElems; i++) {
            carbon_string_id_t string_id = *VECTOR_GET((keyVector), i, carbon_string_id_t);
            fprintf(file, "%"PRIu64"%s", string_id, i + 1 < (keyVector)->numElems ? ", " : "");
        }
        fprintf(file, "], ");
        fprintf(file, "\"Keys Decoded\": [ ");
        for (size_t i = 0; i < (keyVector)->numElems; i++) {
            carbon_string_id_t string_id_t = *VECTOR_GET((keyVector), i, carbon_string_id_t);
            char **encString = carbon_strdic_extract(dic, &string_id_t, 1);
            fprintf(file, "\"%s\"%s", encString[0], i + 1 < (keyVector)->numElems ? ", " : "");
            carbon_strdic_free(dic, encString);
        }
        fprintf(file, "],");
        fprintf(file, "\"Values\": [ ");
        for (size_t i = 0; i < (valueVector)->numElems; i++) {
            const carbon_vec_t ofType(carbon_string_id_t) *values = VECTOR_GET(valueVector, i, carbon_vec_t);
            fprintf(file, "[");
            for (size_t j = 0; j < values->numElems; j++) {
                carbon_string_id_t value = *VECTOR_GET(values, j, carbon_string_id_t);
                fprintf(file, "%"PRIu64"%s", value, j + 1 < values->numElems ? ", " : "");
            }
            fprintf(file, "]%s", i + 1 < (valueVector)->numElems ? ", " : "");

        }
        fprintf(file, "], ");
        fprintf(file, "\"Values Decoded\": [ ");
        for (size_t i = 0; i < (valueVector)->numElems; i++) {
            const carbon_vec_t ofType(carbon_string_id_t) *values = VECTOR_GET(valueVector, i, carbon_vec_t);
            fprintf(file, "[");
            for (size_t j = 0; j < values->numElems; j++) {
                carbon_string_id_t value = *VECTOR_GET(values, j, carbon_string_id_t);

                if (CARBON_BRANCH_LIKELY(value != CARBON_NULL_ENCODED_STRING)) {
                    char **decoded = carbon_strdic_extract(dic, &value, 1);
                    fprintf(file, "\"%s\"%s", *decoded, j + 1 < values->numElems ? ", " : "");
                    carbon_strdic_free(dic, decoded);
                } else {
                    fprintf(file, "null%s", j + 1 < values->numElems ? ", " : "");
                }

            }
            fprintf(file, "]%s", i + 1 < (valueVector)->numElems ? ", " : "");

        }
        fprintf(file, "]");
    }
    fprintf(file, "}, ");
}

static void printPrimitiveStrings(FILE *file, const char *typeName, const carbon_vec_t ofType(carbon_string_id_t) *keyVector,
                                  const carbon_vec_t ofType(carbon_string_id_t) *valueVector, carbon_strdic_t *dic)
{
    PRINT_PRIMITIVE_KEY_PART(file, typeName, keyVector, dic, ", ")
    if(!carbon_vec_is_empty((keyVector))) {
        fprintf(file, "\"Values\": [ ");
        for (size_t i = 0; i < (valueVector)->numElems; i++) {
            carbon_string_id_t string_id_t = *VECTOR_GET(valueVector, i, carbon_string_id_t);
            fprintf(file, "%"PRIu64"%s", string_id_t, i + 1 < (valueVector)->numElems ? ", " : "");
        }
        fprintf(file, "], ");
        fprintf(file, "\"Values Decoded\": [ ");
        for (size_t i = 0; i < (valueVector)->numElems; i++) {
            carbon_string_id_t string_id_t = *VECTOR_GET(valueVector, i, carbon_string_id_t);
            char **values = carbon_strdic_extract(dic, &string_id_t, 1);
            fprintf(file, "\"%s\"%s", *values, i + 1 < (valueVector)->numElems ? ", " : "");
            carbon_strdic_free(dic, values);
        }
        fprintf(file, "]");
    }
    fprintf(file, "}, ");

}

#define PRINT_COLUMN(file, columnTable, arrayIdx, type, formatString)               \
{                                                                                   \
    const carbon_vec_t *column = VECTOR_GET(&columnTable->values, arrayIdx, carbon_vec_t);      \
    fprintf(file, "%s", column->numElems > 1 ? "[" : "");                           \
    for (size_t i = 0; i < column->numElems; i++) {                                 \
        fprintf(file, formatString, *VECTOR_GET(column, i, type));                  \
        fprintf(file, "%s", i + 1 < column->numElems ? ", " : "");                  \
    }                                                                               \
    fprintf(file, "%s", column->numElems > 1 ? "]" : "");                           \
}

static bool printArrayObjects(FILE *file, carbon_err_t *err, const char *typeName, const carbon_vec_t ofType(carbon_columndoc_columngroup_t) *keyColumns,
                              carbon_strdic_t *dic)
{
    fprintf(file, "\"%s\": {", typeName);
    fprintf(file, "\"Keys\": [");
    for (size_t arrayKeyIdx = 0; arrayKeyIdx < keyColumns->numElems; arrayKeyIdx++) {
        const carbon_columndoc_columngroup_t *arrayKeyColumns = VECTOR_GET(keyColumns, arrayKeyIdx, carbon_columndoc_columngroup_t);
        fprintf(file, "%"PRIu64"%s", arrayKeyColumns->key, arrayKeyIdx + 1 < keyColumns->numElems ? ", " : "");
    }
    fprintf(file, "], \"Keys Decoded\": [");
    for (size_t arrayKeyIdx = 0; arrayKeyIdx < keyColumns->numElems; arrayKeyIdx++) {
        const carbon_columndoc_columngroup_t *arrayKeyColumns = VECTOR_GET(keyColumns, arrayKeyIdx, carbon_columndoc_columngroup_t);
        carbon_string_id_t encKeyName = arrayKeyColumns->key;
        char **decKeyName = carbon_strdic_extract(dic, &encKeyName, 1);
        fprintf(file, "\"%s\"%s", *decKeyName, arrayKeyIdx + 1 < keyColumns->numElems ? ", " : "");
        carbon_strdic_free(dic, decKeyName);
    }
    fprintf(file, "], ");
    fprintf(file, "\"Tables\": [");
    for (size_t arrayKeyIdx = 0; arrayKeyIdx < keyColumns->numElems; arrayKeyIdx++) {
        fprintf(file, "[");
        const carbon_columndoc_columngroup_t *arrayKeyColumns = VECTOR_GET(keyColumns, arrayKeyIdx, carbon_columndoc_columngroup_t);
        for (size_t columnIdx = 0; columnIdx < arrayKeyColumns->columns.numElems; columnIdx++) {
            fprintf(file, "{");
            const carbon_columndoc_column_t *columnTable = VECTOR_GET(&arrayKeyColumns->columns, columnIdx, carbon_columndoc_column_t);
            char **decColumnKeyName = carbon_strdic_extract(dic, &columnTable->key_name, 1);

            const char *column_type_name = getTypeName(err, columnTable->type);
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
            for (size_t arrayIdx = 0; arrayIdx < columnTable->values.numElems; arrayIdx++) {
                switch (columnTable->type) {
                case carbon_field_type_null: {
                    const carbon_vec_t *column = VECTOR_GET(&columnTable->values, arrayIdx, carbon_vec_t);
                    fprintf(file, "%s", column->numElems > 1 ? "[" : "");
                    for (size_t i = 0; i < column->numElems; i++) {
                        fprintf(file, "null");
                        fprintf(file, "%s", i + 1 < column->numElems ? ", " : "");
                    }
                    fprintf(file, "%s", column->numElems > 1 ? "]" : "");
                } break;
                case carbon_field_type_int8:
                    PRINT_COLUMN(file, columnTable, arrayIdx, carbon_int8_t, "%d")
                    break;
                case carbon_field_type_int16:
                    PRINT_COLUMN(file, columnTable, arrayIdx, carbon_int16_t, "%d")
                    break;
                case carbon_field_type_int32:
                    PRINT_COLUMN(file, columnTable, arrayIdx, carbon_int32_t, "%d")
                    break;
                case carbon_field_type_int64:
                    PRINT_COLUMN(file, columnTable, arrayIdx, carbon_int64_t, "%" PRIi64)
                    break;
                case carbon_field_type_uint8:
                    PRINT_COLUMN(file, columnTable, arrayIdx, carbon_uint8_t, "%d")
                    break;
                case carbon_field_type_uint16:
                    PRINT_COLUMN(file, columnTable, arrayIdx, carbon_uint16_t, "%d")
                    break;
                case carbon_field_type_uint32:
                    PRINT_COLUMN(file, columnTable, arrayIdx, carbon_uin32_t, "%d")
                    break;
                case carbon_field_type_uint64:
                    PRINT_COLUMN(file, columnTable, arrayIdx, carbon_uin64_t, "%" PRIu64)
                    break;
                case carbon_field_type_float:
                    PRINT_COLUMN(file, columnTable, arrayIdx, carbon_float_t , "%f")
                    break;
                case carbon_field_type_string: {
                    const carbon_vec_t *column = VECTOR_GET(&columnTable->values, arrayIdx, carbon_vec_t);
                    fprintf(file, "%s", column->numElems > 1 ? "[" : "");
                    for (size_t i = 0; i < column->numElems; i++) {
                        carbon_string_id_t encodedString = *VECTOR_GET(column, i, carbon_string_id_t);
                        char **decodedString = carbon_strdic_extract(dic, &encodedString, 1);
                        fprintf(file, "{\"Encoded\": %"PRIu64", \"Decoded\": \"%s\"}", encodedString, *decodedString);
                        fprintf(file, "%s", i + 1 < column->numElems ? ", " : "");
                        carbon_strdic_free(dic, decodedString);
                    }
                    fprintf(file, "%s", column->numElems > 1 ? "]" : "");
                } break;
                case carbon_field_type_object: {
                   // carbon_columndoc_obj_t *doc = VECTOR_GET(&column->values, valueIdx, carbon_columndoc_obj_t);
                  //  printObject(file, doc, strdic);
                    const carbon_vec_t *column = VECTOR_GET(&columnTable->values, arrayIdx, carbon_vec_t);
                    fprintf(file, "%s", column->numElems > 1 ? "[" : "");
                    for (size_t i = 0; i < column->numElems; i++) {
                        const carbon_columndoc_obj_t *object = VECTOR_GET(column, i, carbon_columndoc_obj_t);
                        if(!printObject(file, err, object, dic)) {
                            return false;
                        }
                        fprintf(file, "%s", i + 1 < column->numElems ? ", " : "");
                    }
                    fprintf(file, "%s", column->numElems > 1 ? "]" : "");
                } break;
                default:
                    CARBON_ERROR(err, CARBON_ERR_NOTYPE)
                    return false;
                }
                fprintf(file, arrayIdx + 1 < columnTable->values.numElems ? ", " : "");
            }
            fprintf(file, "],");
            fprintf(file, "\"Positions\": [");
            for (size_t positionIdx = 0; positionIdx < columnTable->array_positions.numElems; positionIdx++) {
                fprintf(file, "%d%s", *VECTOR_GET(&columnTable->array_positions, positionIdx, int16_t), (positionIdx + 1 < columnTable->array_positions.numElems ? ", " : ""));
            }
            fprintf(file, "]");
            carbon_strdic_free(dic, decColumnKeyName);
            fprintf(file, "}%s", columnIdx + 1 < arrayKeyColumns->columns.numElems ? ", " : "");
        }
        fprintf(file, "]%s", arrayKeyIdx + 1 < keyColumns->numElems ? ", " : "");
    }
    fprintf(file, "]");

    fprintf(file, "}");
    return true;
}

static bool printObject(FILE *file, carbon_err_t *err, const carbon_columndoc_obj_t *object, carbon_strdic_t *dic)
{
    char **parentKey = carbon_strdic_extract(dic, &object->parent_key, 1);
    fprintf(file, "{ ");
    fprintf(file, "\"Parent\": { \"Key\": %"PRIu64", \"Key Decoded\": \"%s\", \"Index\": %zu }, ", object->parent_key, parentKey[0], object->index);
    fprintf(file, "\"Pairs\": { ");
        fprintf(file, "\"Primitives\": { ");
            PRINT_PRIMITIVE_BOOLEAN_COLUMN(file, "Boolean", &object->bool_prop_keys, &object->bool_prop_vals, dic)
            PRINT_PRIMITIVE_COLUMN(file, "UInt8", &object->uint8_prop_keys, &object->uint8_prop_vals, &object->uint8_val_idxs, dic, carbon_uint8_t, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "UInt16", &object->uint16_prop_keys, &object->uint16_prop_vals, &object->uint16_val_idxs, dic, carbon_uint16_t, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "UInt32", &object->uin32_prop_keys, &object->uint32_prop_vals, &object->uint32_val_idxs, dic, carbon_uin32_t, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "UInt64", &object->uint64_prop_keys, &object->uint64_prop_vals, &object->uint64_val_idxs, dic, carbon_uin64_t, "%" PRIu64)
            PRINT_PRIMITIVE_COLUMN(file, "Int8", &object->int8_prop_keys, &object->int8_prop_vals, &object->int8_val_idxs, dic, carbon_int8_t, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "Int16", &object->int16_prop_keys, &object->int16_prop_vals, &object->int16_val_idxs, dic, carbon_int16_t, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "Int32", &object->int32_prop_keys, &object->int32_prop_vals, &object->int32_val_idxs, dic, carbon_int32_t, "%d")
            PRINT_PRIMITIVE_COLUMN(file, "Int64", &object->int64_prop_keys, &object->int64_prop_vals, &object->int64_val_idxs, dic, carbon_int64_t, "%" PRIi64)
            PRINT_PRIMITIVE_COLUMN(file, "Real", &object->float_prop_keys, &object->float_prop_vals, &object->float_val_idxs, dic, carbon_float_t, "%f")
            printPrimitiveStrings(file, "Strings", &object->string_prop_keys, &object->string_prop_vals, dic);
            printPrimitiveNull(file, "Null", &object->null_prop_keys, dic);
            if(printPrimitiveObjects(file, err, "Objects", &object->obj_prop_keys, &object->obj_prop_vals, dic)) {
                return false;
            }
            fprintf(file, "}, ");
        fprintf(file, "\"Arrays\": { ");
            PRINT_BOOLEAN_ARRAY(file, "Boolean", object->bool_array_prop_keys, object->bool_array_prop_vals);
            PRINT_ARRAY(file, "UInt8", object->uint8_array_prop_keys, object->uint8_array_prop_vals, carbon_uint8_t, "%d", (value != CARBON_NULL_UINT8));
            PRINT_ARRAY(file, "UInt16", object->uint16_array_prop_keys, object->uint16_array_prop_vals, carbon_uint16_t, "%d", (value !=CARBON_NULL_UINT16));
            PRINT_ARRAY(file, "UInt32", object->uint32_array_prop_keys, object->uint32_array_prop_vals, carbon_uin32_t, "%d", (value !=CARBON_NULL_UINT32));
            PRINT_ARRAY(file, "UInt64", object->uint64_array_prop_keys, object->uin64_array_prop_vals, carbon_uin64_t, "%" PRIu64, (value !=CARBON_NULL_UINT64));
            PRINT_ARRAY(file, "Int8", object->int8_array_prop_keys, object->int8_array_prop_vals, carbon_int8_t, "%d", (value !=CARBON_NULL_INT8));
            PRINT_ARRAY(file, "Int16", object->int16_array_prop_keys, object->int16_array_prop_vals, carbon_int16_t, "%d", (value !=CARBON_NULL_INT16));
            PRINT_ARRAY(file, "Int32", object->int32_array_prop_keys, object->int32_array_prop_vals, carbon_int32_t, "%d", (value !=CARBON_NULL_INT32));
            PRINT_ARRAY(file, "Int64", object->int64_array_prop_keys, object->int64_array_prop_vals, carbon_int64_t, "%" PRIi64, (value !=CARBON_NULL_INT64));
            PRINT_ARRAY(file, "Real", object->float_array_prop_keys, object->float_array_prop_vals, carbon_float_t, "%f", (!isnan(value)));
            printArrayStrings(file, "Strings", &object->string_array_prop_keys, &object->string_array_prop_vals, dic);
            printArrayNull(file, "Null", &object->null_array_prop_keys, &object->null_array_prop_vals, dic);
            if(!printArrayObjects(file, err, "Objects", &object->obj_array_props, dic)) {
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
    return printObject(file, &doc->err, &doc->columndoc, doc->dic);
}

bool carbon_columndoc_drop(carbon_columndoc_t *doc)
{
    CARBON_UNUSED(doc);
    CARBON_NOT_IMPLEMENTED
}

static void objectArrayKeyColumnsCreate(carbon_vec_t ofType(carbon_columndoc_columngroup_t) *columns)
{
    carbon_vec_create(columns, NULL, sizeof(carbon_columndoc_columngroup_t), 20000);
}

static void objectArrayKeyColumnsDrop(carbon_vec_t ofType(carbon_columndoc_columngroup_t) *columns)
{
    for (size_t i = 0; i < columns->numElems; i++) {
        carbon_columndoc_columngroup_t *arrayColumns = VECTOR_GET(columns, i, carbon_columndoc_columngroup_t);
        for (size_t j = 0; j < arrayColumns->columns.numElems; j++) {


            carbon_columndoc_column_t *column = VECTOR_GET(&arrayColumns->columns, j, carbon_columndoc_column_t);

            carbon_vec_t ofType(uint32_t) *arrayIndices = &column->array_positions;
            carbon_vec_t ofType(carbon_vec_t ofType(<T>)) *valuesForIndicies = &column->values;

            assert (arrayIndices->numElems == valuesForIndicies->numElems);

            for (size_t k = 0; k < arrayIndices->numElems; k++) {

                carbon_vec_t ofType(<T>) *valuesForIndex = VECTOR_GET(valuesForIndicies, k, carbon_vec_t);
                if (column->type == carbon_field_type_object) {
                    for (size_t l = 0; l < valuesForIndex->numElems; l++) {
                        carbon_columndoc_obj_t *nestedObject = VECTOR_GET(valuesForIndex, l, carbon_columndoc_obj_t);
                        objectMetaModelFree(nestedObject);
                    }
                }
                VectorDrop(valuesForIndex);
            }

            VectorDrop(arrayIndices);
            VectorDrop(valuesForIndicies);
        }
        VectorDrop(&arrayColumns->columns);
    }
    VectorDrop(columns);
}

static const char *getTypeName(carbon_err_t *err, carbon_field_type_e type)
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

static carbon_columndoc_column_t *objectArrayKeyColumnsFindOrNew(carbon_vec_t ofType(carbon_columndoc_columngroup_t) *columns,
                                                            carbon_string_id_t arrayKey, carbon_string_id_t nestedObjectEntryKey,
                                                            carbon_field_type_e nestedObjectEntryType)
{
    carbon_columndoc_columngroup_t *keyColumns;
    carbon_columndoc_column_t *keyColumn, *newColumn;

    for (size_t i = 0; i < columns->numElems; i++) {
        /** Find object array pair having the key `key` */
        keyColumns = VECTOR_GET(columns, i, carbon_columndoc_columngroup_t);
        if (keyColumns->key == arrayKey) {
            /** In case such a pair is found, find column that matches the desired type */
            for (size_t j = 0; j < keyColumns->columns.numElems; j++) {
                keyColumn = VECTOR_GET(&keyColumns->columns, j, carbon_columndoc_column_t);
                if (keyColumn->key_name == nestedObjectEntryKey && keyColumn->type == nestedObjectEntryType) {
                    /** Column for the object array with the desired key, the nested object entry with the desired key
                     * and a matching type is found */
                    return keyColumn;
                }
            }
            /** In this case, the requested arrayKey is found, but the nested object entry does not match, hence
             * create a new one */
            goto objectArrayKeyColumnsNewColumn;
        }
    }
    /** In this case, the array key is also not known. Create a new one array entry with the fitting key column and
     * return that newly created column */
    keyColumns = VECTOR_NEW_AND_GET(columns, carbon_columndoc_columngroup_t);
    keyColumns->key = arrayKey;
    carbon_vec_create(&keyColumns->columns, NULL, sizeof(carbon_columndoc_column_t), 10);

objectArrayKeyColumnsNewColumn:
    newColumn = VECTOR_NEW_AND_GET(&keyColumns->columns, carbon_columndoc_column_t);
    newColumn->key_name = nestedObjectEntryKey;
    newColumn->type = nestedObjectEntryType;
    carbon_vec_create(&newColumn->values, NULL, sizeof(carbon_vec_t), 10);
    carbon_vec_create(&newColumn->array_positions, NULL, sizeof(uint32_t), 10);

    return newColumn;
}

static bool objectArrayKeyColumnPush(carbon_columndoc_column_t *col, carbon_err_t *err, const carbon_doc_entries_t *entry, uint32_t arrayIdx,
                                     carbon_strdic_t *dic, carbon_columndoc_obj_t *model)
{
    assert(col->type == entry->type);

    uint32_t *entryArrayIdx = VECTOR_NEW_AND_GET(&col->array_positions, uint32_t);
    *entryArrayIdx = arrayIdx;

    carbon_vec_t ofType(<T>) *valuesForEntry = VECTOR_NEW_AND_GET(&col->values, carbon_vec_t);
    carbon_vec_create(valuesForEntry, NULL, GET_TYPE_SIZE(entry->type), entry->values.numElems);

    bool isNullByDef = entry->values.numElems == 0;
    uint32_t numElements = (uint32_t) entry->values.numElems;

    carbon_field_type_e entryType = isNullByDef ? carbon_field_type_null : entry->type;
    numElements = isNullByDef ? 1 : numElements;

    switch (entryType) {
    case carbon_field_type_null: {
        carbon_vec_push(valuesForEntry, &numElements, 1);
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
        assert(!isNullByDef);
        carbon_vec_push(valuesForEntry, entry->values.base, numElements);
        break;
    case carbon_field_type_string: {
        assert(!isNullByDef);
        char **strings = VECTOR_ALL(&entry->values, char *);
        carbon_string_id_t *carbon_string_id_ts;
        carbon_strdic_locate_fast(&carbon_string_id_ts, dic, (char *const *) strings, numElements);
        carbon_vec_push(valuesForEntry, carbon_string_id_ts, numElements);
        carbon_strdic_free(dic, carbon_string_id_ts);
        //carbon_strdic_free(strdic, strings);
    } break;
    case carbon_field_type_object:
        assert(!isNullByDef);

        carbon_string_id_t *arrayKey;
        carbon_strdic_locate_fast(&arrayKey, dic, (char *const *) &entry->key, 1);

        for (size_t arrayIdx = 0; arrayIdx < numElements; arrayIdx++)
        {
            carbon_columndoc_obj_t *nestedObject = VECTOR_NEW_AND_GET(valuesForEntry, carbon_columndoc_obj_t);
            setupObject(nestedObject, model->parent, *arrayKey, arrayIdx);
            if (!importObject(nestedObject, err, VECTOR_GET(&entry->values, arrayIdx, carbon_doc_obj_t), dic)) {
                return false;
            }
        }
        carbon_strdic_free(dic, arrayKey);
        break;
    default:
        CARBON_ERROR(err, CARBON_ERR_NOTYPE);
        return false;
    }
    return true;
}

static void setupObject(carbon_columndoc_obj_t *model, carbon_columndoc_t *parent, carbon_string_id_t key, size_t idx)
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

    carbon_vec_create(&model->bool_prop_vals, NULL, sizeof(carbon_bool_t), 10);
    carbon_vec_create(&model->int8_prop_vals, NULL, sizeof(carbon_int8_t), 10);
    carbon_vec_create(&model->int16_prop_vals, NULL, sizeof(carbon_int16_t), 10);
    carbon_vec_create(&model->int32_prop_vals, NULL, sizeof(carbon_int32_t), 10);
    carbon_vec_create(&model->int64_prop_vals, NULL, sizeof(carbon_int64_t), 10);
    carbon_vec_create(&model->uint8_prop_vals, NULL, sizeof(carbon_uint8_t), 10);
    carbon_vec_create(&model->uint16_prop_vals, NULL, sizeof(carbon_uint16_t), 10);
    carbon_vec_create(&model->uint32_prop_vals, NULL, sizeof(carbon_uin32_t), 10);
    carbon_vec_create(&model->uint64_prop_vals, NULL, sizeof(carbon_uin64_t), 10);
    carbon_vec_create(&model->float_prop_vals, NULL, sizeof(carbon_float_t), 10);
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

    objectArrayKeyColumnsCreate(&model->obj_array_props);
}

static bool objectPutPrimitive(carbon_columndoc_obj_t *model, carbon_err_t *err, const carbon_doc_entries_t *entry, carbon_strdic_t *dic,
                               const carbon_string_id_t *keyId)
{
    switch(entry->type) {
    case carbon_field_type_null:
        carbon_vec_push(&model->null_prop_keys, keyId, 1);
        break;
    case carbon_field_type_bool:
        carbon_vec_push(&model->bool_prop_keys, keyId, 1);
        carbon_vec_push(&model->bool_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_int8:
        carbon_vec_push(&model->int8_prop_keys, keyId, 1);
        carbon_vec_push(&model->int8_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_int16:
        carbon_vec_push(&model->int16_prop_keys, keyId, 1);
        carbon_vec_push(&model->int16_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_int32:
        carbon_vec_push(&model->int32_prop_keys, keyId, 1);
        carbon_vec_push(&model->int32_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_int64:
        carbon_vec_push(&model->int64_prop_keys, keyId, 1);
        carbon_vec_push(&model->int64_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_uint8:
        carbon_vec_push(&model->uint8_prop_keys, keyId, 1);
        carbon_vec_push(&model->uint8_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_uint16:
        carbon_vec_push(&model->uint16_prop_keys, keyId, 1);
        carbon_vec_push(&model->uint16_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_uint32:
        carbon_vec_push(&model->uin32_prop_keys, keyId, 1);
        carbon_vec_push(&model->uint32_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_uint64:
        carbon_vec_push(&model->uint64_prop_keys, keyId, 1);
        carbon_vec_push(&model->uint64_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_float:
        carbon_vec_push(&model->float_prop_keys, keyId, 1);
        carbon_vec_push(&model->float_prop_vals, entry->values.base, 1);
        break;
    case carbon_field_type_string: {
        carbon_string_id_t *value;
        carbon_strdic_locate_fast(&value, dic, (char *const *) entry->values.base, 1);
        carbon_vec_push(&model->string_prop_keys, keyId, 1);
        carbon_vec_push(&model->string_prop_vals, value, 1);
        carbon_strdic_free(dic, value);
    } break;
    case carbon_field_type_object: {
        carbon_columndoc_obj_t template, *nestedObject;
        size_t position = carbon_vec_length(&model->obj_prop_keys);
        carbon_vec_push(&model->obj_prop_keys, keyId, 1);
        carbon_vec_push(&model->obj_prop_vals, &template, 1);
        nestedObject = VECTOR_GET(&model->obj_prop_vals, position, carbon_columndoc_obj_t);
        setupObject(nestedObject, model->parent, *keyId, 0);
        if (!importObject(nestedObject, err, VECTOR_GET(&entry->values, 0, carbon_doc_obj_t), dic)) {
            return false;
        }
    } break;
        CARBON_ERROR(err, CARBON_ERR_NOTYPE)
        return false;
    }
    return true;
}

static void objectPushArray(carbon_vec_t ofType(Vector ofType(<T>)) *values, size_t TSize, uint32_t numElements,
                            const void *data, carbon_string_id_t keyId, carbon_vec_t ofType(carbon_string_id_t) *keyVector)
{
    carbon_vec_t ofType(<T>) template, *vector;
    size_t idx = carbon_vec_length(values);
    carbon_vec_push(values, &template, 1);
    vector = VECTOR_GET(values, idx, carbon_vec_t);
    carbon_vec_create(vector, NULL, TSize, numElements);
    carbon_vec_push(vector, data, numElements);
    carbon_vec_push(keyVector, &keyId, 1);
}

static bool objectPutArray(carbon_columndoc_obj_t *model, carbon_err_t *err, const carbon_doc_entries_t *entry, carbon_strdic_t *dic, const carbon_string_id_t *keyId)
{
    // TODO: format for array, sort by keys, sort by values!
    CARBON_UNUSED(dic);
    uint32_t numElements = (uint32_t) carbon_vec_length(&entry->values);

    switch(entry->type) {
    case carbon_field_type_null: {
        carbon_vec_push(&model->null_array_prop_vals, &numElements, 1);
        carbon_vec_push(&model->null_array_prop_keys, keyId, 1);
    }
        break;
    case carbon_field_type_bool:
        objectPushArray(&model->bool_array_prop_vals, sizeof(carbon_bool_t), numElements, entry->values.base, *keyId,
                        &model->bool_array_prop_keys);
        break;
    case carbon_field_type_int8:
        objectPushArray(&model->int8_array_prop_vals, sizeof(carbon_int8_t), numElements, entry->values.base, *keyId,
                        &model->int8_array_prop_keys);
        break;
    case carbon_field_type_int16:
        objectPushArray(&model->int16_array_prop_vals, sizeof(carbon_int16_t), numElements, entry->values.base, *keyId,
                        &model->int16_array_prop_keys);
        break;
    case carbon_field_type_int32:
        objectPushArray(&model->int32_array_prop_vals, sizeof(carbon_int32_t), numElements, entry->values.base, *keyId,
                        &model->int32_array_prop_keys);
        break;
    case carbon_field_type_int64:
        objectPushArray(&model->int64_array_prop_vals, sizeof(carbon_int64_t), numElements, entry->values.base, *keyId,
                        &model->int64_array_prop_keys);
        break;
    case carbon_field_type_uint8:
        objectPushArray(&model->uint8_array_prop_vals, sizeof(carbon_uint8_t), numElements, entry->values.base, *keyId,
                        &model->uint8_array_prop_keys);
        break;
    case carbon_field_type_uint16:
        objectPushArray(&model->uint16_array_prop_vals,
                        sizeof(carbon_uint16_t),
                        numElements,
                        entry->values.base,
                        *keyId,
                        &model->uint16_array_prop_keys);
        break;
    case carbon_field_type_uint32:
        objectPushArray(&model->uint32_array_prop_vals, sizeof(carbon_uin32_t), numElements, entry->values.base, *keyId,
                        &model->uint32_array_prop_keys);
        break;
    case carbon_field_type_uint64:
        objectPushArray(&model->uin64_array_prop_vals, sizeof(carbon_uin64_t), numElements, entry->values.base, *keyId,
                        &model->uint64_array_prop_keys);
        break;
    case carbon_field_type_float:
        objectPushArray(&model->float_array_prop_vals, sizeof(carbon_float_t), numElements, entry->values.base, *keyId,
                        &model->float_array_prop_keys);
        break;
    case carbon_field_type_string: {
        const char **strings = VECTOR_ALL(&entry->values, const char *);
        carbon_string_id_t *carbon_string_id_ts;
        carbon_strdic_locate_fast(&carbon_string_id_ts, dic, (char *const *) strings, numElements);
        objectPushArray(&model->string_array_prop_vals,
                        sizeof(carbon_string_id_t),
                        numElements,
                        carbon_string_id_ts,
                        *keyId,
                        &model->string_array_prop_keys);
        carbon_strdic_free(dic, carbon_string_id_ts);
    }
        break;
    case carbon_field_type_object: {
        carbon_string_id_t *nestedObjectKeyName;
        for (uint32_t arrayIdx = 0; arrayIdx < numElements; arrayIdx++) {
            const carbon_doc_obj_t *object = VECTOR_GET(&entry->values, arrayIdx, carbon_doc_obj_t);
            for (size_t pairIdx = 0; pairIdx < object->entries.numElems; pairIdx++) {
                const carbon_doc_entries_t *pair = VECTOR_GET(&object->entries, pairIdx, carbon_doc_entries_t);
                carbon_strdic_locate_fast(&nestedObjectKeyName, dic, (char *const *) &pair->key, 1);
                carbon_columndoc_column_t *keyColumn = objectArrayKeyColumnsFindOrNew(&model->obj_array_props, *keyId,
                                                                                      *nestedObjectKeyName, pair->type);
                if (!objectArrayKeyColumnPush(keyColumn, err, pair, arrayIdx, dic, model)) {
                    return false;
                }
                carbon_strdic_free(dic, nestedObjectKeyName);
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

static bool objectPut(carbon_columndoc_obj_t *model, carbon_err_t *err, const carbon_doc_entries_t *entry, carbon_strdic_t *dic)
{
    carbon_string_id_t *keyId;
    enum EntryType { ENTRY_TYPE_NULL, ENTRY_TYPE_PRIMITIVE, ENTRY_TYPE_ARRAY } entryType;

    carbon_strdic_locate_fast(&keyId, dic, (char *const *) &entry->key, 1);
    entryType = entry->values.numElems == 0 ? ENTRY_TYPE_NULL :
                (entry->values.numElems == 1 ? ENTRY_TYPE_PRIMITIVE : ENTRY_TYPE_ARRAY );

    switch (entryType) {
    case ENTRY_TYPE_NULL:
        /** For a key which does not map to any value, the value is defined as 'null'  */
        carbon_vec_push(&model->null_prop_keys, keyId, 1);
        break;
    case ENTRY_TYPE_PRIMITIVE:
        if (!objectPutPrimitive(model, err, entry, dic, keyId)) {
            return false;
        }
        break;
    case ENTRY_TYPE_ARRAY:
        if (!objectPutArray(model, err, entry, dic, keyId)) {
            return false;
        }
        break;
    default:
        CARBON_ERROR(err, CARBON_ERR_NOTYPE)
        return false;
    }

    carbon_strdic_free(dic, keyId);
    return true;
}

static bool importObject(carbon_columndoc_obj_t *dst, carbon_err_t *err, const carbon_doc_obj_t *objectModel, carbon_strdic_t *dic)
{
    const carbon_vec_t ofType(carbon_doc_entries_t) *objectEntries = carbon_doc_get_entries(objectModel);
    const carbon_doc_entries_t *entries = VECTOR_ALL(objectEntries, carbon_doc_entries_t);
    for (size_t i = 0; i < objectEntries->numElems; i++) {
        const carbon_doc_entries_t *entry = entries + i;
        if (!objectPut(dst, err, entry, dic)) {
            return false;
        }
    }
    return true;
}