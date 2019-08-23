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
#include <jak_json.h>
#include <jak_doc.h>
#include <jak_column_doc.h>
#include <jak_json.h>
#include <jak_utils_sort.h>

char VALUE_NULL = '\0';

static void create_doc(jak_doc_obj *model, jak_doc *doc);

static void create_typed_vector(jak_doc_entries *entry);

static void entries_drop(jak_doc_entries *entry);

static bool print_value(FILE *file, jak_archive_field_e type, const jak_vector ofType(<T>) *values);

static void _jak_doc_print_object(FILE *file, const jak_doc_obj *model);

static bool
import_json_object(jak_doc_obj *target, jak_error *err, const jak_json_object *json_obj);

static void sort_jak_columndoc_entries(jak_column_doc_obj *columndoc);

bool jak_doc_bulk_create(jak_doc_bulk *bulk, jak_string_dict *dic)
{
        JAK_ERROR_IF_NULL(bulk)
        JAK_ERROR_IF_NULL(dic)
        bulk->dic = dic;
        jak_vector_create(&bulk->keys, NULL, sizeof(char *), 500);
        jak_vector_create(&bulk->values, NULL, sizeof(char *), 1000);
        jak_vector_create(&bulk->models, NULL, sizeof(jak_doc), 50);
        return true;
}

jak_doc_obj *jak_doc_bulk_new_obj(jak_doc *model)
{
        if (!model) {
                return NULL;
        } else {
                jak_doc_obj *retval = JAK_VECTOR_NEW_AND_GET(&model->obj_model, jak_doc_obj);
                create_doc(retval, model);
                return retval;
        }
}

bool jak_doc_bulk_get_dic_contents(jak_vector ofType (const char *) **strings,
                               jak_vector ofType(jak_archive_field_sid_t) **jak_string_ids,
                               const jak_doc_bulk *context)
{
        JAK_ERROR_IF_NULL(context)

        size_t num_distinct_values;
        jak_string_dict_num_distinct(&num_distinct_values, context->dic);
        jak_vector ofType (const char *) *result_strings = JAK_MALLOC(sizeof(jak_vector));
        jak_vector ofType (jak_archive_field_sid_t) *resultjak_string_id_ts = JAK_MALLOC(sizeof(jak_vector));
        jak_vector_create(result_strings, NULL, sizeof(const char *), num_distinct_values);
        jak_vector_create(resultjak_string_id_ts, NULL, sizeof(jak_archive_field_sid_t), num_distinct_values);

        int status = jak_string_dict_get_contents(result_strings, resultjak_string_id_ts, context->dic);
        JAK_CHECK_SUCCESS(status);
        *strings = result_strings;
        *jak_string_ids = resultjak_string_id_ts;

        return status;
}

jak_doc *jak_doc_bulk_new_doc(jak_doc_bulk *context, jak_archive_field_e type)
{
        if (!context) {
                return NULL;
        }

        jak_doc template, *model;
        size_t idx = jak_vector_length(&context->models);
        jak_vector_push(&context->models, &template, 1);
        model = JAK_VECTOR_GET(&context->models, idx, jak_doc);
        model->context = context;
        model->type = type;

        jak_vector_create(&model->obj_model, NULL, sizeof(jak_doc_obj), 500);

        return model;
}

bool jak_doc_bulk_drop(jak_doc_bulk *bulk)
{
        JAK_ERROR_IF_NULL(bulk)
        for (size_t i = 0; i < bulk->keys.num_elems; i++) {
                char *string = *JAK_VECTOR_GET(&bulk->keys, i, char *);
                free(string);
        }
        for (size_t i = 0; i < bulk->values.num_elems; i++) {
                char *string = *JAK_VECTOR_GET(&bulk->values, i, char *);
                free(string);
        }
        for (size_t i = 0; i < bulk->models.num_elems; i++) {
                jak_doc *model = JAK_VECTOR_GET(&bulk->models, i, jak_doc);
                for (size_t j = 0; j < model->obj_model.num_elems; j++) {
                        jak_doc_obj *doc = JAK_VECTOR_GET(&model->obj_model, j, jak_doc_obj);
                        jak_doc_drop(doc);
                }
                jak_vector_drop(&model->obj_model);
        }

        jak_vector_drop(&bulk->keys);
        jak_vector_drop(&bulk->values);
        jak_vector_drop(&bulk->models);
        return true;
}

bool jak_doc_bulk_shrink(jak_doc_bulk *bulk)
{
        JAK_ERROR_IF_NULL(bulk)
        jak_vector_shrink(&bulk->keys);
        jak_vector_shrink(&bulk->values);
        return true;
}

bool jak_doc_bulk_print(FILE *file, jak_doc_bulk *bulk)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(bulk)

        fprintf(file, "{");
        char **key_strings = JAK_VECTOR_ALL(&bulk->keys, char *);
        fprintf(file, "\"Key Strings\": [");
        for (size_t i = 0; i < bulk->keys.num_elems; i++) {
                fprintf(file, "\"%s\"%s", key_strings[i], i + 1 < bulk->keys.num_elems ? ", " : "");
        }
        fprintf(file, "], ");

        char **valueStrings = JAK_VECTOR_ALL(&bulk->values, char *);
        fprintf(file, "\"Value Strings\": [");
        for (size_t i = 0; i < bulk->values.num_elems; i++) {
                fprintf(file, "\"%s\"%s", valueStrings[i], i + 1 < bulk->values.num_elems ? ", " : "");
        }
        fprintf(file, "]}");

        return true;
}

bool jak_doc_print(FILE *file, const jak_doc *doc)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(doc)

        if (doc->obj_model.num_elems == 0) {
                fprintf(file, "{ }");
        }

        if (doc->obj_model.num_elems > 1) {
                fprintf(file, "[");
        }

        for (size_t num_entries = 0; num_entries < doc->obj_model.num_elems; num_entries++) {
                jak_doc_obj *object = JAK_VECTOR_GET(&doc->obj_model, num_entries, jak_doc_obj);
                _jak_doc_print_object(file, object);
                fprintf(file, "%s", num_entries + 1 < doc->obj_model.num_elems ? ", " : "");
        }

        if (doc->obj_model.num_elems > 1) {
                fprintf(file, "]");
        }

        return true;
}

const jak_vector ofType(jak_doc_entries) *jak_doc_get_entries(const jak_doc_obj *model)
{
        return &model->entries;
}

void jak_doc_print_entries(FILE *file, const jak_doc_entries *entries)
{
        fprintf(file, "{\"Key\": \"%s\"", entries->key);
}

void jak_doc_drop(jak_doc_obj *model)
{
        for (size_t i = 0; i < model->entries.num_elems; i++) {
                jak_doc_entries *entry = JAK_VECTOR_GET(&model->entries, i, jak_doc_entries);
                entries_drop(entry);
        }
        jak_vector_drop(&model->entries);
}

bool jak_doc_obj_add_key(jak_doc_entries **out, jak_doc_obj *obj, const char *key, jak_archive_field_e type)
{
        JAK_ERROR_IF_NULL(out)
        JAK_ERROR_IF_NULL(obj)
        JAK_ERROR_IF_NULL(key)

        size_t entry_idx;
        char *key_dup = strdup(key);

        jak_doc_entries entry_model = {.type = type, .key = key_dup, .context = obj};

        create_typed_vector(&entry_model);
        jak_vector_push(&obj->doc->context->keys, &key_dup, 1);

        entry_idx = jak_vector_length(&obj->entries);
        jak_vector_push(&obj->entries, &entry_model, 1);

        *out = JAK_VECTOR_GET(&obj->entries, entry_idx, jak_doc_entries);

        return true;
}

bool jak_doc_obj_push_primtive(jak_doc_entries *entry, const void *value)
{
        JAK_ERROR_IF_NULL(entry)
        JAK_ERROR_IF_NULL((entry->type == JAK_FIELD_NULL) || (value != NULL))

        switch (entry->type) {
                case JAK_FIELD_NULL:
                        jak_vector_push(&entry->values, &VALUE_NULL, 1);
                        break;
                case JAK_FIELD_STRING: {
                        char *string = value ? strdup((char *) value) : NULL;
                        jak_vector_push(&entry->context->doc->context->values, &string, 1);
                        jak_vector_push(&entry->values, &string, 1);
                }
                        break;
                default:
                        jak_vector_push(&entry->values, value, 1);
                        break;
        }
        return true;
}

bool jak_doc_obj_push_object(jak_doc_obj **out, jak_doc_entries *entry)
{
        JAK_ERROR_IF_NULL(out);
        JAK_ERROR_IF_NULL(entry);

        JAK_ASSERT(entry->type == JAK_FIELD_OBJECT);

        jak_doc_obj objectModel;

        create_doc(&objectModel, entry->context->doc);
        size_t length = jak_vector_length(&entry->values);
        jak_vector_push(&entry->values, &objectModel, 1);

        *out = JAK_VECTOR_GET(&entry->values, length, jak_doc_obj);

        return true;
}

static jak_archive_field_e
value_type_for_json_number(bool *success, jak_error *err, const jak_json_number *number)
{
        *success = true;
        switch (number->value_type) {
                case JAK_JSON_NUMBER_FLOAT:
                        return JAK_FIELD_FLOAT;
                case JAK_JSON_NUMBER_UNSIGNED: {
                        jak_u64 test = number->value.unsigned_integer;
                        if (test <= JAK_LIMITS_UINT8_MAX) {
                                return JAK_FIELD_UINT8;
                        } else if (test <= JAK_LIMITS_UINT16_MAX) {
                                return JAK_FIELD_UINT16;
                        } else if (test <= JAK_LIMITS_UINT32_MAX) {
                                return JAK_FIELD_UINT32;
                        } else {
                                return JAK_FIELD_UINT64;
                        }
                }
                case JAK_JSON_NUMBER_SIGNED: {
                        jak_i64 test = number->value.signed_integer;
                        if (test >= JAK_LIMITS_INT8_MIN && test <= JAK_LIMITS_INT8_MAX) {
                                return JAK_FIELD_INT8;
                        } else if (test >= JAK_LIMITS_INT16_MIN && test <= JAK_LIMITS_INT16_MAX) {
                                return JAK_FIELD_INT16;
                        } else if (test >= JAK_LIMITS_INT32_MIN && test <= JAK_LIMITS_INT32_MAX) {
                                return JAK_FIELD_INT32;
                        } else {
                                return JAK_FIELD_INT64;
                        }
                }
                default: JAK_ERROR(err, JAK_ERR_NOJSONNUMBERT);
                        *success = false;
                        return JAK_FIELD_INT8;
        }
}

static void
import_json_object_jak_string_prop(jak_doc_obj *target, const char *key, const jak_json_string *string)
{
        jak_doc_entries *entry;
        jak_doc_obj_add_key(&entry, target, key, JAK_FIELD_STRING);
        jak_doc_obj_push_primtive(entry, string->value);
}

static bool import_json_object_number_prop(jak_doc_obj *target, jak_error *err, const char *key,
                                           const jak_json_number *number)
{
        jak_doc_entries *entry;
        bool success;
        jak_archive_field_e number_type = value_type_for_json_number(&success, err, number);
        if (!success) {
                return false;
        }
        jak_doc_obj_add_key(&entry, target, key, number_type);
        jak_doc_obj_push_primtive(entry, &number->value);
        return true;
}

static void import_json_object_bool_prop(jak_doc_obj *target, const char *key, jak_archive_field_boolean_t value)
{
        jak_doc_entries *entry;
        jak_doc_obj_add_key(&entry, target, key, JAK_FIELD_BOOLEAN);
        jak_doc_obj_push_primtive(entry, &value);
}

static void import_json_object_null_prop(jak_doc_obj *target, const char *key)
{
        jak_doc_entries *entry;
        jak_doc_obj_add_key(&entry, target, key, JAK_FIELD_NULL);
        jak_doc_obj_push_primtive(entry, NULL);
}

static bool import_json_object_object_prop(jak_doc_obj *target, jak_error *err, const char *key,
                                           const jak_json_object *object)
{
        jak_doc_entries *entry;
        jak_doc_obj *nested_object = NULL;
        jak_doc_obj_add_key(&entry, target, key, JAK_FIELD_OBJECT);
        jak_doc_obj_push_object(&nested_object, entry);
        return import_json_object(nested_object, err, object);
}

static bool import_json_object_array_prop(jak_doc_obj *target, jak_error *err, const char *key,
                                          const jak_json_array *array)
{
        jak_doc_entries *entry;

        if (!jak_vector_is_empty(&array->elements.elements)) {
                size_t num_elements = array->elements.elements.num_elems;

                /** Find first type that is not null unless the entire array is of type null */
                jak_json_value_type_e array_data_type = JAK_JSON_VALUE_NULL;
                jak_archive_field_e field_type;

                for (size_t i = 0; i < num_elements && array_data_type == JAK_JSON_VALUE_NULL; i++) {
                        const jak_json_element *element = JAK_VECTOR_GET(&array->elements.elements, i,
                                                                         jak_json_element);
                        array_data_type = element->value.value_type;
                }

                switch (array_data_type) {
                        case JAK_JSON_VALUE_OBJECT:
                                field_type = JAK_FIELD_OBJECT;
                                break;
                        case JAK_JSON_VALUE_STRING:
                                field_type = JAK_FIELD_STRING;
                                break;
                        case JAK_JSON_VALUE_NUMBER: {
                                /** find smallest fitting physical number type */
                                jak_archive_field_e array_number_type = JAK_FIELD_NULL;
                                for (size_t i = 0; i < num_elements; i++) {
                                        const jak_json_element
                                                *element = JAK_VECTOR_GET(&array->elements.elements, i,
                                                                   jak_json_element);
                                        if (JAK_UNLIKELY(element->value.value_type == JAK_JSON_VALUE_NULL)) {
                                                continue;
                                        } else {
                                                bool success;
                                                jak_archive_field_e element_number_type =
                                                        value_type_for_json_number(&success, err,
                                                                                   element->value.value.number);
                                                if (!success) {
                                                        return false;
                                                }
                                                JAK_ASSERT(element_number_type == JAK_FIELD_INT8 ||
                                                           element_number_type == JAK_FIELD_INT16
                                                           || element_number_type == JAK_FIELD_INT32
                                                           || element_number_type == JAK_FIELD_INT64
                                                           || element_number_type == JAK_FIELD_UINT8
                                                           || element_number_type == JAK_FIELD_UINT16
                                                           || element_number_type == JAK_FIELD_UINT32
                                                           || element_number_type == JAK_FIELD_UINT64
                                                           || element_number_type == JAK_FIELD_FLOAT);
                                                if (JAK_UNLIKELY(array_number_type == JAK_FIELD_NULL)) {
                                                        array_number_type = element_number_type;
                                                } else {
                                                        if (array_number_type == JAK_FIELD_INT8) {
                                                                array_number_type = element_number_type;
                                                        } else if (array_number_type == JAK_FIELD_INT16) {
                                                                if (element_number_type != JAK_FIELD_INT8) {
                                                                        array_number_type = element_number_type;
                                                                }
                                                        } else if (array_number_type == JAK_FIELD_INT32) {
                                                                if (element_number_type != JAK_FIELD_INT8
                                                                    && element_number_type != JAK_FIELD_INT16) {
                                                                        array_number_type = element_number_type;
                                                                }
                                                        } else if (array_number_type == JAK_FIELD_INT64) {
                                                                if (element_number_type != JAK_FIELD_INT8
                                                                    && element_number_type != JAK_FIELD_INT16
                                                                    && element_number_type != JAK_FIELD_INT32) {
                                                                        array_number_type = element_number_type;
                                                                }
                                                        } else if (array_number_type == JAK_FIELD_UINT8) {
                                                                array_number_type = element_number_type;
                                                        } else if (array_number_type == JAK_FIELD_UINT16) {
                                                                if (element_number_type != JAK_FIELD_UINT16) {
                                                                        array_number_type = element_number_type;
                                                                }
                                                        } else if (array_number_type == JAK_FIELD_UINT32) {
                                                                if (element_number_type != JAK_FIELD_UINT8
                                                                    && element_number_type != JAK_FIELD_UINT16) {
                                                                        array_number_type = element_number_type;
                                                                }
                                                        } else if (array_number_type == JAK_FIELD_UINT64) {
                                                                if (element_number_type != JAK_FIELD_UINT8
                                                                    && element_number_type != JAK_FIELD_UINT16
                                                                    && element_number_type != JAK_FIELD_UINT32) {
                                                                        array_number_type = element_number_type;
                                                                }
                                                        } else if (array_number_type == JAK_FIELD_FLOAT) {
                                                                break;
                                                        }
                                                }
                                        }
                                }
                                JAK_ASSERT(array_number_type != JAK_FIELD_NULL);
                                field_type = array_number_type;
                        }
                                break;
                        case JAK_JSON_VALUE_FALSE:
                        case JAK_JSON_VALUE_TRUE:
                                field_type = JAK_FIELD_BOOLEAN;
                                break;
                        case JAK_JSON_VALUE_NULL:
                                field_type = JAK_FIELD_NULL;
                                break;
                        case JAK_JSON_VALUE_ARRAY: JAK_ERROR(err, JAK_ERR_ERRINTERNAL) /** array type is illegal here */
                                return false;
                        default: JAK_ERROR(err, JAK_ERR_NOTYPE)
                                return false;
                }

                jak_doc_obj_add_key(&entry, target, key, field_type);

                for (size_t i = 0; i < num_elements; i++) {
                        const jak_json_element *element = JAK_VECTOR_GET(&array->elements.elements, i,
                                                                         jak_json_element);
                        jak_json_value_type_e ast_node_data_type = element->value.value_type;

                        switch (field_type) {
                                case JAK_FIELD_OBJECT: {
                                        jak_doc_obj *nested_object = NULL;
                                        jak_doc_obj_push_object(&nested_object, entry);
                                        if (ast_node_data_type != JAK_JSON_VALUE_NULL) {
                                                /** the object is null by definition, if no entries are contained */
                                                if (!import_json_object(nested_object, err,
                                                                        element->value.value.object)) {
                                                        return false;
                                                }
                                        }
                                }
                                        break;
                                case JAK_FIELD_STRING: {
                                        JAK_ASSERT(ast_node_data_type == array_data_type ||
                                                   ast_node_data_type == JAK_JSON_VALUE_NULL);
                                        jak_doc_obj_push_primtive(entry,
                                                              ast_node_data_type == JAK_JSON_VALUE_NULL
                                                              ? JAK_NULL_ENCODED_STRING : element->value
                                                                      .value.string->value);
                                }
                                        break;
                                case JAK_FIELD_INT8:
                                case JAK_FIELD_INT16:
                                case JAK_FIELD_INT32:
                                case JAK_FIELD_INT64:
                                case JAK_FIELD_UINT8:
                                case JAK_FIELD_UINT16:
                                case JAK_FIELD_UINT32:
                                case JAK_FIELD_UINT64:
                                case JAK_FIELD_FLOAT: {
                                        JAK_ASSERT(ast_node_data_type == array_data_type ||
                                                   ast_node_data_type == JAK_JSON_VALUE_NULL);
                                        switch (field_type) {
                                                case JAK_FIELD_INT8: {
                                                        jak_archive_field_i8_t value =
                                                                ast_node_data_type == JAK_JSON_VALUE_NULL ? JAK_NULL_INT8
                                                                                                      : (jak_archive_field_i8_t) element
                                                                        ->value.value.number->value.signed_integer;
                                                        jak_doc_obj_push_primtive(entry, &value);
                                                }
                                                        break;
                                                case JAK_FIELD_INT16: {
                                                        jak_archive_field_i16_t value =
                                                                ast_node_data_type == JAK_JSON_VALUE_NULL ? JAK_NULL_INT16
                                                                                                      : (jak_archive_field_i16_t) element
                                                                        ->value.value.number->value.signed_integer;
                                                        jak_doc_obj_push_primtive(entry, &value);
                                                }
                                                        break;
                                                case JAK_FIELD_INT32: {
                                                        jak_archive_field_i32_t value =
                                                                ast_node_data_type == JAK_JSON_VALUE_NULL ? JAK_NULL_INT32
                                                                                                      : (jak_archive_field_i32_t) element
                                                                        ->value.value.number->value.signed_integer;
                                                        jak_doc_obj_push_primtive(entry, &value);
                                                }
                                                        break;
                                                case JAK_FIELD_INT64: {
                                                        jak_archive_field_i64_t value =
                                                                ast_node_data_type == JAK_JSON_VALUE_NULL ? JAK_NULL_INT64
                                                                                                      : (jak_archive_field_i64_t) element
                                                                        ->value.value.number->value.signed_integer;
                                                        jak_doc_obj_push_primtive(entry, &value);
                                                }
                                                        break;
                                                case JAK_FIELD_UINT8: {
                                                        jak_archive_field_u8_t value =
                                                                ast_node_data_type == JAK_JSON_VALUE_NULL ? JAK_NULL_UINT8
                                                                                                      : (jak_archive_field_u8_t) element
                                                                        ->value.value.number->value.unsigned_integer;
                                                        jak_doc_obj_push_primtive(entry, &value);
                                                }
                                                        break;
                                                case JAK_FIELD_UINT16: {
                                                        jak_archive_field_u16_t value =
                                                                ast_node_data_type == JAK_JSON_VALUE_NULL ? JAK_NULL_UINT16
                                                                                                      : (jak_archive_field_u16_t) element
                                                                        ->value.value.number->value.unsigned_integer;
                                                        jak_doc_obj_push_primtive(entry, &value);
                                                }
                                                        break;
                                                case JAK_FIELD_UINT32: {
                                                        jak_archive_field_u32_t value =
                                                                ast_node_data_type == JAK_JSON_VALUE_NULL ? JAK_NULL_UINT32
                                                                                                      : (jak_archive_field_u32_t) element
                                                                        ->value.value.number->value.unsigned_integer;
                                                        jak_doc_obj_push_primtive(entry, &value);
                                                }
                                                        break;
                                                case JAK_FIELD_UINT64: {
                                                        jak_archive_field_u64_t value =
                                                                ast_node_data_type == JAK_JSON_VALUE_NULL ? JAK_NULL_UINT64
                                                                                                      : (jak_archive_field_u64_t) element
                                                                        ->value.value.number->value.unsigned_integer;
                                                        jak_doc_obj_push_primtive(entry, &value);
                                                }
                                                        break;
                                                case JAK_FIELD_FLOAT: {
                                                        jak_archive_field_number_t value = JAK_NULL_FLOAT;
                                                        if (ast_node_data_type != JAK_JSON_VALUE_NULL) {
                                                                json_number_type_e
                                                                        element_number_type = element->value.value.number->value_type;
                                                                if (element_number_type == JAK_JSON_NUMBER_FLOAT) {
                                                                        value = element->value.value.number->value.float_number;
                                                                } else if (element_number_type ==
                                                                           JAK_JSON_NUMBER_UNSIGNED) {
                                                                        value = element->value.value.number->value.unsigned_integer;
                                                                } else if (element_number_type == JAK_JSON_NUMBER_SIGNED) {
                                                                        value = element->value.value.number->value.signed_integer;
                                                                } else {
                                                                        JAK_ERROR_PRINT_AND_DIE(
                                                                                JAK_ERR_INTERNALERR) /** type mismatch */
                                                                        return false;
                                                                }
                                                        }
                                                        jak_doc_obj_push_primtive(entry, &value);
                                                }
                                                        break;
                                                default: JAK_ERROR_PRINT_AND_DIE(
                                                        JAK_ERR_INTERNALERR) /** not a number type  */
                                                        return false;
                                        }
                                }
                                        break;
                                case JAK_FIELD_BOOLEAN:
                                        if (JAK_LIKELY(ast_node_data_type == JAK_JSON_VALUE_TRUE
                                                       || ast_node_data_type == JAK_JSON_VALUE_FALSE)) {
                                                jak_archive_field_boolean_t value =
                                                        ast_node_data_type == JAK_JSON_VALUE_TRUE ? JAK_BOOLEAN_TRUE
                                                                                              : JAK_BOOLEAN_FALSE;
                                                jak_doc_obj_push_primtive(entry, &value);
                                        } else {
                                                JAK_ASSERT(ast_node_data_type == JAK_JSON_VALUE_NULL);
                                                jak_archive_field_boolean_t value = JAK_NULL_BOOLEAN;
                                                jak_doc_obj_push_primtive(entry, &value);
                                        }
                                        break;
                                case JAK_FIELD_NULL:
                                        JAK_ASSERT(ast_node_data_type == array_data_type);
                                        jak_doc_obj_push_primtive(entry, NULL);
                                        break;
                                default: JAK_ERROR(err, JAK_ERR_NOTYPE)
                                        return false;
                        }
                }
        } else {
                import_json_object_null_prop(target, key);
        }
        return true;
}

static bool
import_json_object(jak_doc_obj *target, jak_error *err, const jak_json_object *json_obj)
{
        for (size_t i = 0; i < json_obj->value->members.num_elems; i++) {
                jak_json_prop *member = JAK_VECTOR_GET(&json_obj->value->members, i, jak_json_prop);
                jak_json_value_type_e value_type = member->value.value.value_type;
                switch (value_type) {
                        case JAK_JSON_VALUE_STRING:
                                import_json_object_jak_string_prop(target, member->key.value,
                                                               member->value.value.value.string);
                                break;
                        case JAK_JSON_VALUE_NUMBER:
                                if (!import_json_object_number_prop(target,
                                                                    err,
                                                                    member->key.value,
                                                                    member->value.value.value.number)) {
                                        return false;
                                }
                                break;
                        case JAK_JSON_VALUE_TRUE:
                        case JAK_JSON_VALUE_FALSE: {
                                jak_archive_field_boolean_t value =
                                        value_type == JAK_JSON_VALUE_TRUE ? JAK_BOOLEAN_TRUE : JAK_BOOLEAN_FALSE;
                                import_json_object_bool_prop(target, member->key.value, value);
                        }
                                break;
                        case JAK_JSON_VALUE_NULL:
                                import_json_object_null_prop(target, member->key.value);
                                break;
                        case JAK_JSON_VALUE_OBJECT:
                                if (!import_json_object_object_prop(target,
                                                                    err,
                                                                    member->key.value,
                                                                    member->value.value.value.object)) {
                                        return false;
                                }
                                break;
                        case JAK_JSON_VALUE_ARRAY:
                                if (!import_json_object_array_prop(target,
                                                                   err,
                                                                   member->key.value,
                                                                   member->value.value.value.array)) {
                                        return false;
                                }
                                break;
                        default: JAK_ERROR(err, JAK_ERR_NOTYPE);
                                return false;
                }
        }
        return true;
}

static bool
import_json(jak_doc_obj *target, jak_error *err, const jak_json *jak_json,
            jak_doc_entries *partition)
{
        jak_json_value_type_e value_type = jak_json->element->value.value_type;
        switch (value_type) {
                case JAK_JSON_VALUE_OBJECT:
                        if (!import_json_object(target, err, jak_json->element->value.value.object)) {
                                return false;
                        }
                        break;
                case JAK_JSON_VALUE_ARRAY: {
                        const jak_vector ofType(jak_json_element)
                                *arrayContent = &jak_json->element->value.value.array->elements.elements;
                        if (!jak_vector_is_empty(arrayContent)) {
                                const jak_json_element *first = JAK_VECTOR_GET(arrayContent, 0,
                                                                               jak_json_element);
                                switch (first->value.value_type) {
                                        case JAK_JSON_VALUE_OBJECT:
                                                if (!import_json_object(target, err, first->value.value.object)) {
                                                        return false;
                                                }
                                                for (size_t i = 1; i < arrayContent->num_elems; i++) {
                                                        const jak_json_element
                                                                *element = JAK_VECTOR_GET(arrayContent, i,
                                                                                   jak_json_element);
                                                        jak_doc_obj *nested;
                                                        jak_doc_obj_push_object(&nested, partition);
                                                        if (!import_json_object(nested, err,
                                                                                element->value.value.object)) {
                                                                return false;
                                                        }
                                                }
                                                break;
                                        case JAK_JSON_VALUE_ARRAY:
                                        case JAK_JSON_VALUE_STRING:
                                        case JAK_JSON_VALUE_NUMBER:
                                        case JAK_JSON_VALUE_TRUE:
                                        case JAK_JSON_VALUE_FALSE:
                                        case JAK_JSON_VALUE_NULL:
                                        default: JAK_ERROR_PRINT_AND_DIE(
                                                JAK_ERR_INTERNALERR) /** Unsupported operation in arrays */
                                                break;
                                }
                        }
                }
                        break;
                case JAK_JSON_VALUE_STRING:
                case JAK_JSON_VALUE_NUMBER:
                case JAK_JSON_VALUE_TRUE:
                case JAK_JSON_VALUE_FALSE:
                case JAK_JSON_VALUE_NULL:
                default: JAK_ERROR(err, JAK_ERR_JSONTYPE);
                        return false;
        }
        return true;
}

jak_doc_obj *jak_doc_bulk_add_json(jak_doc_entries *partition, jak_json *jak_json)
{
        if (!partition || !jak_json) {
                return NULL;
        }

        jak_doc_obj *converted_json;
        jak_doc_obj_push_object(&converted_json, partition);
        if (!import_json(converted_json, &jak_json->err, jak_json, partition)) {
                return NULL;
        }

        return converted_json;
}

jak_doc_obj *jak_doc_entries_get_root(const jak_doc_entries *partition)
{
        return partition ? partition->context : NULL;
}

jak_doc_entries *jak_doc_bulk_new_entries(jak_doc_bulk *dst)
{
        jak_doc_entries *partition = NULL;
        jak_doc *model = jak_doc_bulk_new_doc(dst, JAK_FIELD_OBJECT);
        jak_doc_obj *object = jak_doc_bulk_new_obj(model);
        jak_doc_obj_add_key(&partition, object, "/", JAK_FIELD_OBJECT);
        return partition;
}

#define JAK_DEFINE_TYPE_LQ_FUNC(type)                                                                               \
static bool compare_##type##_leq(const void *lhs, const void *rhs)                                                \
{                                                                                                                      \
    jak_archive_##type a = *(jak_archive_##type *) lhs;                                                                                            \
    jak_archive_##type b = *(jak_archive_##type *) rhs;                                                                                            \
    return (a <= b);                                                                                                   \
}

JAK_DEFINE_TYPE_LQ_FUNC(field_boolean_t)

JAK_DEFINE_TYPE_LQ_FUNC(field_number_t)

JAK_DEFINE_TYPE_LQ_FUNC(field_i8_t)

JAK_DEFINE_TYPE_LQ_FUNC(field_i16_t)

JAK_DEFINE_TYPE_LQ_FUNC(field_i32_t)

JAK_DEFINE_TYPE_LQ_FUNC(field_i64_t)

JAK_DEFINE_TYPE_LQ_FUNC(field_u8_t)

JAK_DEFINE_TYPE_LQ_FUNC(field_u16_t)

JAK_DEFINE_TYPE_LQ_FUNC(field_u32_t)

JAK_DEFINE_TYPE_LQ_FUNC(field_u64_t)

static bool compare_encoded_jak_string_less_eq_func(const void *lhs, const void *rhs, void *args)
{
        jak_string_dict *dic = (jak_string_dict *) args;
        jak_archive_field_sid_t *a = (jak_archive_field_sid_t *) lhs;
        jak_archive_field_sid_t *b = (jak_archive_field_sid_t *) rhs;
        char **a_string = jak_string_dict_extract(dic, a, 1);
        char **b_string = jak_string_dict_extract(dic, b, 1);
        bool lq = strcmp(*a_string, *b_string) <= 0;
        jak_string_dict_free(dic, a_string);
        jak_string_dict_free(dic, b_string);
        return lq;
}

static void sort_nested_primitive_object(jak_column_doc_obj *columndoc)
{
        if (columndoc->parent->read_optimized) {
                for (size_t i = 0; i < columndoc->obj_prop_vals.num_elems; i++) {
                        jak_column_doc_obj *nestedModel = JAK_VECTOR_GET(&columndoc->obj_prop_vals, i,
                                                                         jak_column_doc_obj);
                        sort_jak_columndoc_entries(nestedModel);
                }
        }
}

#define JAK_DEFINE_ARRAY_TYPE_LQ_FUNC(type)                                                                         \
static bool compare_##type##_array_leq(const void *lhs, const void *rhs)                                           \
{                                                                                                                      \
    jak_vector ofType(jak_archive_##type) *a = (jak_vector *) lhs;                                                               \
    jak_vector ofType(jak_archive_##type) *b = (jak_vector *) rhs;                                                               \
    const jak_archive_##type *aValues = JAK_VECTOR_ALL(a, jak_archive_##type);                                                                  \
    const jak_archive_##type *bValues = JAK_VECTOR_ALL(b, jak_archive_##type);                                                                  \
    size_t max_compare_idx = a->num_elems < b->num_elems ? a->num_elems : b->num_elems;                                \
    for (size_t i = 0; i < max_compare_idx; i++) {                                                                     \
        if (aValues[i] > bValues[i]) {                                                                                 \
            return false;                                                                                              \
        }                                                                                                              \
    }                                                                                                                  \
    return true;                                                                                                       \
}

JAK_DEFINE_ARRAY_TYPE_LQ_FUNC(field_boolean_t)

JAK_DEFINE_ARRAY_TYPE_LQ_FUNC(field_i8_t)

JAK_DEFINE_ARRAY_TYPE_LQ_FUNC(field_i16_t)

JAK_DEFINE_ARRAY_TYPE_LQ_FUNC(field_i32_t)

JAK_DEFINE_ARRAY_TYPE_LQ_FUNC(field_i64_t)

JAK_DEFINE_ARRAY_TYPE_LQ_FUNC(field_u8_t)

JAK_DEFINE_ARRAY_TYPE_LQ_FUNC(field_u16_t)

JAK_DEFINE_ARRAY_TYPE_LQ_FUNC(field_u32_t)

JAK_DEFINE_ARRAY_TYPE_LQ_FUNC(field_u64_t)

JAK_DEFINE_ARRAY_TYPE_LQ_FUNC(field_number_t)

static bool compare_encoded_jak_string_array_less_eq_func(const void *lhs, const void *rhs, void *args)
{
        jak_string_dict *dic = (jak_string_dict *) args;
        jak_vector ofType(jak_archive_field_sid_t) *a = (jak_vector *) lhs;
        jak_vector ofType(jak_archive_field_sid_t) *b = (jak_vector *) rhs;
        const jak_archive_field_sid_t *aValues = JAK_VECTOR_ALL(a, jak_archive_field_sid_t);
        const jak_archive_field_sid_t *bValues = JAK_VECTOR_ALL(b, jak_archive_field_sid_t);
        size_t max_compare_idx = a->num_elems < b->num_elems ? a->num_elems : b->num_elems;
        for (size_t i = 0; i < max_compare_idx; i++) {
                char **aString = jak_string_dict_extract(dic, aValues + i, 1);
                char **bString = jak_string_dict_extract(dic, bValues + i, 1);
                bool greater = strcmp(*aString, *bString) > 0;
                jak_string_dict_free(dic, aString);
                jak_string_dict_free(dic, bString);
                if (greater) {
                        return false;
                }
        }
        return true;
}

static void sorted_nested_array_objects(jak_column_doc_obj *columndoc)
{
        if (columndoc->parent->read_optimized) {
                for (size_t i = 0; i < columndoc->obj_array_props.num_elems; i++) {
                        jak_column_doc_group
                                *array_columns = JAK_VECTOR_GET(&columndoc->obj_array_props, i, jak_column_doc_group);
                        for (size_t j = 0; j < array_columns->columns.num_elems; j++) {
                                jak_column_doc_column
                                        *column = JAK_VECTOR_GET(&array_columns->columns, j, jak_column_doc_column);
                                jak_vector ofType(jak_u32) *array_indices = &column->array_positions;
                                jak_vector ofType(
                                        jak_vector ofType(<T>)) *values_for_indicies = &column->values;
                                JAK_ASSERT (array_indices->num_elems == values_for_indicies->num_elems);

                                for (size_t k = 0; k < array_indices->num_elems; k++) {
                                        jak_vector ofType(<T>)
                                                *values_for_index = JAK_VECTOR_GET(values_for_indicies, k, jak_vector);
                                        if (column->type == JAK_FIELD_OBJECT) {
                                                for (size_t l = 0; l < values_for_index->num_elems; l++) {
                                                        jak_column_doc_obj *nested_object =
                                                                JAK_VECTOR_GET(values_for_index, l, jak_column_doc_obj);
                                                        sort_jak_columndoc_entries(nested_object);
                                                }
                                        }
                                }
                        }
                }
        }
}

#define SORT_META_MODEL_VALUES(key_vector, value_vector, value_type, compareValueFunc)                                 \
{                                                                                                                      \
    size_t num_elements = jak_vector_length(&key_vector);                                                              \
                                                                                                                       \
    if (num_elements > 0) {                                                                                            \
        size_t *value_indicies = JAK_MALLOC(sizeof(size_t) * num_elements);                                                \
        for (size_t i = 0; i < num_elements; i++) {                                                                    \
            value_indicies[i] = i;                                                                                     \
        }                                                                                                              \
                                                                                                                       \
        jak_vector ofType(jak_archive_field_sid_t) key_cpy;                                                               \
        jak_vector ofType(value_type) value_cpy;                                                                     \
                                                                                                                       \
        jak_vector_cpy(&key_cpy, &key_vector);                                                                         \
        jak_vector_cpy(&value_cpy, &value_vector);                                                                     \
                                                                                                                       \
        value_type *values = JAK_VECTOR_ALL(&value_cpy, value_type);                                                \
                                                                                                                       \
        jak_sort_qsort_indicies(value_indicies, values, sizeof(value_type), compareValueFunc, num_elements,         \
                      key_vector.allocator);                                                                           \
                                                                                                                       \
        for (size_t i = 0; i < num_elements; i++) {                                                                    \
            jak_vector_set(&key_vector, i, JAK_VECTOR_GET(&key_cpy, value_indicies[i], jak_archive_field_sid_t));        \
            jak_vector_set(&value_vector, i, JAK_VECTOR_GET(&value_cpy, value_indicies[i], value_type));            \
        }                                                                                                              \
                                                                                                                       \
                                                                                                                       \
        free(value_indicies);                                                                                          \
        jak_vector_drop(&key_cpy);                                                                                     \
        jak_vector_drop(&value_cpy);                                                                                   \
    }                                                                                                                  \
}

static void sort_meta_model_jak_string_values(jak_vector ofType(jak_archive_field_sid_t) *key_vector,
                                          jak_vector ofType(jak_archive_field_sid_t) *value_vector,
                                          jak_string_dict *dic)
{
        size_t num_elements = jak_vector_length(key_vector);

        if (num_elements > 0) {
                size_t *value_indicies = JAK_MALLOC(sizeof(size_t) * num_elements);
                for (size_t i = 0; i < num_elements; i++) {
                        value_indicies[i] = i;
                }

                jak_vector ofType(jak_archive_field_sid_t) key_cpy;
                jak_vector ofType(jak_archive_field_sid_t) value_cpy;

                jak_vector_cpy(&key_cpy, key_vector);
                jak_vector_cpy(&value_cpy, value_vector);

                jak_archive_field_sid_t *values = JAK_VECTOR_ALL(&value_cpy, jak_archive_field_sid_t);

                jak_sort_qsort_indicies_wargs(value_indicies,
                                          values,
                                          sizeof(jak_archive_field_sid_t),
                                          compare_encoded_jak_string_less_eq_func,
                                          num_elements,
                                          key_vector->allocator,
                                          dic);

                for (size_t i = 0; i < num_elements; i++) {
                        jak_vector_set(key_vector, i, JAK_VECTOR_GET(&key_cpy, value_indicies[i], jak_archive_field_sid_t));
                        jak_vector_set(value_vector, i, JAK_VECTOR_GET(&value_cpy, value_indicies[i], jak_archive_field_sid_t));
                }

                free(value_indicies);
                jak_vector_drop(&key_cpy);
                jak_vector_drop(&value_cpy);
        }
}

#define SORT_META_MODEL_ARRAYS(key_vector, value_array_vector, compare_func)                                           \
{                                                                                                                      \
    size_t num_elements = jak_vector_length(&key_vector);                                                              \
                                                                                                                       \
    if (num_elements > 0) {                                                                                            \
        size_t *value_indicies = JAK_MALLOC(sizeof(size_t) * num_elements);                                                \
        for (size_t i = 0; i < num_elements; i++) {                                                                    \
            value_indicies[i] = i;                                                                                     \
        }                                                                                                              \
                                                                                                                       \
        jak_vector ofType(jak_archive_field_sid_t) key_cpy;                                                               \
        jak_vector ofType(jak_vector) value_cpy;                                                                   \
                                                                                                                       \
        jak_vector_cpy(&key_cpy, &key_vector);                                                                         \
        jak_vector_cpy(&value_cpy, &value_array_vector);                                                               \
                                                                                                                       \
        const jak_vector *values = JAK_VECTOR_ALL(&value_array_vector, jak_vector);                             \
                                                                                                                       \
        jak_sort_qsort_indicies(value_indicies, values, sizeof(jak_vector), compare_func, num_elements,           \
                      key_vector.allocator);                                                                           \
                                                                                                                       \
        for (size_t i = 0; i < num_elements; i++) {                                                                    \
            jak_vector_set(&key_vector, i, JAK_VECTOR_GET(&key_cpy, value_indicies[i], jak_archive_field_sid_t));        \
            jak_vector_set(&value_array_vector, i, JAK_VECTOR_GET(&value_cpy, value_indicies[i], jak_vector));    \
        }                                                                                                              \
                                                                                                                       \
        free(value_indicies);                                                                                          \
        jak_vector_drop(&key_cpy);                                                                                     \
        jak_vector_drop(&value_cpy);                                                                                   \
    }                                                                                                                  \
}

static void sort_jak_columndoc_strings_arrays(jak_vector ofType(jak_archive_field_sid_t) *key_vector,
                                          jak_vector ofType(jak_archive_field_sid_t) *value_array_vector,
                                          jak_string_dict *dic)
{
        size_t num_elements = jak_vector_length(key_vector);

        if (num_elements > 0) {
                size_t *value_indicies = JAK_MALLOC(sizeof(size_t) * num_elements);
                for (size_t i = 0; i < num_elements; i++) {
                        value_indicies[i] = i;
                }

                jak_vector ofType(jak_archive_field_sid_t) key_cpy;
                jak_vector ofType(jak_vector) value_cpy;

                jak_vector_cpy(&key_cpy, key_vector);
                jak_vector_cpy(&value_cpy, value_array_vector);

                const jak_vector *values = JAK_VECTOR_ALL(value_array_vector, jak_vector);

                jak_sort_qsort_indicies_wargs(value_indicies,
                                          values,
                                          sizeof(jak_vector),
                                          compare_encoded_jak_string_array_less_eq_func,
                                          num_elements,
                                          key_vector->allocator,
                                          dic);

                for (size_t i = 0; i < num_elements; i++) {
                        jak_vector_set(key_vector, i, JAK_VECTOR_GET(&key_cpy, value_indicies[i], jak_archive_field_sid_t));
                        jak_vector_set(value_array_vector, i, JAK_VECTOR_GET(&value_cpy, value_indicies[i], jak_vector));
                }

                free(value_indicies);
                jak_vector_drop(&key_cpy);
                jak_vector_drop(&value_cpy);
        }
}

static bool compare_object_array_key_columns_less_eq_func(const void *lhs, const void *rhs, void *args)
{
        jak_string_dict *dic = (jak_string_dict *) args;
        jak_column_doc_group *a = (jak_column_doc_group *) lhs;
        jak_column_doc_group *b = (jak_column_doc_group *) rhs;
        char **a_column_name = jak_string_dict_extract(dic, &a->key, 1);
        char **b_column_name = jak_string_dict_extract(dic, &b->key, 1);
        bool column_name_leq = strcmp(*a_column_name, *b_column_name) <= 0;
        jak_string_dict_free(dic, a_column_name);
        jak_string_dict_free(dic, b_column_name);
        return column_name_leq;
}

static bool compare_object_array_key_column_less_eq_func(const void *lhs, const void *rhs, void *args)
{
        jak_string_dict *dic = (jak_string_dict *) args;
        jak_column_doc_column *a = (jak_column_doc_column *) lhs;
        jak_column_doc_column *b = (jak_column_doc_column *) rhs;
        char **a_column_name = jak_string_dict_extract(dic, &a->key_name, 1);
        char **b_column_name = jak_string_dict_extract(dic, &b->key_name, 1);
        int cmpResult = strcmp(*a_column_name, *b_column_name);
        bool column_name_leq = cmpResult < 0 ? true : (cmpResult == 0 ? (a->type <= b->type) : false);
        jak_string_dict_free(dic, a_column_name);
        jak_string_dict_free(dic, b_column_name);
        return column_name_leq;
}

struct com_column_leq_arg {
        jak_string_dict *dic;
        jak_archive_field_e value_type;
};

#define ARRAY_LEQ_PRIMITIVE_FUNC(max_num_elem, type, valueVectorAPtr, valueVectorBPtr)                                 \
{                                                                                                                      \
    for (size_t i = 0; i < max_num_elem; i++) {                                                                        \
        type o1 = *JAK_VECTOR_GET(valueVectorAPtr, i, type);                                                        \
        type o2 = *JAK_VECTOR_GET(valueVectorBPtr, i, type);                                                        \
        if (o1 > o2) {                                                                                                 \
            return false;                                                                                              \
        }                                                                                                              \
    }                                                                                                                  \
    return true;                                                                                                       \
}

static bool compare_column_less_eq_func(const void *lhs, const void *rhs, void *args)
{
        jak_vector ofType(<T>) *a = (jak_vector *) lhs;
        jak_vector ofType(<T>) *b = (jak_vector *) rhs;
        struct com_column_leq_arg *func_arg = (struct com_column_leq_arg *) args;

        size_t max_num_elem = JAK_MIN(a->num_elems, b->num_elems);

        switch (func_arg->value_type) {
                case JAK_FIELD_NULL:
                        return (a->num_elems <= b->num_elems);
                        break;
                case JAK_FIELD_BOOLEAN: ARRAY_LEQ_PRIMITIVE_FUNC(max_num_elem, jak_archive_field_boolean_t, a, b);
                        break;
                case JAK_FIELD_INT8: ARRAY_LEQ_PRIMITIVE_FUNC(max_num_elem, jak_archive_field_i8_t, a, b);
                        break;
                case JAK_FIELD_INT16: ARRAY_LEQ_PRIMITIVE_FUNC(max_num_elem, jak_archive_field_i16_t, a, b);
                        break;
                case JAK_FIELD_INT32: ARRAY_LEQ_PRIMITIVE_FUNC(max_num_elem, jak_archive_field_i32_t, a, b);
                        break;
                case JAK_FIELD_INT64: ARRAY_LEQ_PRIMITIVE_FUNC(max_num_elem, jak_archive_field_i64_t, a, b);
                        break;
                case JAK_FIELD_UINT8: ARRAY_LEQ_PRIMITIVE_FUNC(max_num_elem, jak_archive_field_u8_t, a, b);
                        break;
                case JAK_FIELD_UINT16: ARRAY_LEQ_PRIMITIVE_FUNC(max_num_elem, jak_archive_field_u16_t, a, b);
                        break;
                case JAK_FIELD_UINT32: ARRAY_LEQ_PRIMITIVE_FUNC(max_num_elem, jak_archive_field_u32_t, a, b);
                        break;
                case JAK_FIELD_UINT64: ARRAY_LEQ_PRIMITIVE_FUNC(max_num_elem, jak_archive_field_u64_t, a, b);
                        break;
                case JAK_FIELD_FLOAT: ARRAY_LEQ_PRIMITIVE_FUNC(max_num_elem, jak_archive_field_number_t, a, b);
                        break;
                case JAK_FIELD_STRING:
                        for (size_t i = 0; i < max_num_elem; i++) {
                                jak_archive_field_sid_t o1 = *JAK_VECTOR_GET(a, i, jak_archive_field_sid_t);
                                jak_archive_field_sid_t o2 = *JAK_VECTOR_GET(b, i, jak_archive_field_sid_t);
                                char **o1_string = jak_string_dict_extract(func_arg->dic, &o1, 1);
                                char **o2_string = jak_string_dict_extract(func_arg->dic, &o2, 1);
                                bool greater = strcmp(*o1_string, *o2_string) > 0;
                                jak_string_dict_free(func_arg->dic, o1_string);
                                jak_string_dict_free(func_arg->dic, o2_string);
                                if (greater) {
                                        return false;
                                }
                        }
                        return true;
                case JAK_FIELD_OBJECT:
                        return true;
                        break;
                default: JAK_ERROR_PRINT_AND_DIE(JAK_ERR_NOTYPE)
                        return false;
        }
}

static void sort_jak_columndoc_column(jak_column_doc_column *column, jak_string_dict *dic)
{
        /** Sort column by its value, and re-arrange the array position list according this new order */
        jak_vector ofType(jak_u32) array_position_cpy;
        jak_vector ofType(jak_vector ofType(<T>)) values_cpy;

        jak_vector_cpy(&array_position_cpy, &column->array_positions);
        jak_vector_cpy(&values_cpy, &column->values);

        JAK_ASSERT(column->array_positions.num_elems == column->values.num_elems);
        JAK_ASSERT(array_position_cpy.num_elems == values_cpy.num_elems);
        JAK_ASSERT(values_cpy.num_elems == column->array_positions.num_elems);

        size_t *indices = JAK_MALLOC(values_cpy.num_elems * sizeof(size_t));
        for (size_t i = 0; i < values_cpy.num_elems; i++) {
                indices[i] = i;
        }

        struct com_column_leq_arg func_arg = {.dic = dic, .value_type = column->type};

        jak_sort_qsort_indicies_wargs(indices,
                                  values_cpy.base,
                                  values_cpy.elem_size,
                                  compare_column_less_eq_func,
                                  values_cpy.num_elems,
                                  values_cpy.allocator,
                                  &func_arg);

        for (size_t i = 0; i < values_cpy.num_elems; i++) {
                jak_vector_set(&column->values, i, jak_vector_at(&values_cpy, indices[i]));
                jak_vector_set(&column->array_positions, i, jak_vector_at(&array_position_cpy, indices[i]));
        }

        free(indices);
        jak_vector_drop(&array_position_cpy);
        jak_vector_drop(&values_cpy);
}

static void sort_jak_columndoc_column_arrays(jak_column_doc_obj *columndoc)
{
        jak_vector ofType(jak_column_doc_group) cpy;
        jak_vector_cpy(&cpy, &columndoc->obj_array_props);
        size_t *indices = JAK_MALLOC(cpy.num_elems * sizeof(size_t));
        for (size_t i = 0; i < cpy.num_elems; i++) {
                indices[i] = i;
        }
        jak_sort_qsort_indicies_wargs(indices,
                                  cpy.base,
                                  sizeof(jak_column_doc_group),
                                  compare_object_array_key_columns_less_eq_func,
                                  cpy.num_elems,
                                  cpy.allocator,
                                  columndoc->parent->dic);
        for (size_t i = 0; i < cpy.num_elems; i++) {
                jak_vector_set(&columndoc->obj_array_props, i, JAK_VECTOR_GET(&cpy, indices[i], jak_column_doc_group));
        }
        free(indices);

        for (size_t i = 0; i < cpy.num_elems; i++) {
                jak_column_doc_group *key_columns = JAK_VECTOR_GET(&columndoc->obj_array_props, i,
                                                                   jak_column_doc_group);
                size_t *columnIndices = JAK_MALLOC(key_columns->columns.num_elems * sizeof(size_t));
                jak_vector ofType(jak_column_doc_column) columnCpy;
                jak_vector_cpy(&columnCpy, &key_columns->columns);
                for (size_t i = 0; i < key_columns->columns.num_elems; i++) {
                        columnIndices[i] = i;
                }

                /** First, sort by column name; Then, sort by columns with same name by type */
                jak_sort_qsort_indicies_wargs(columnIndices,
                                          columnCpy.base,
                                          sizeof(jak_column_doc_column),
                                          compare_object_array_key_column_less_eq_func,
                                          key_columns->columns.num_elems,
                                          key_columns->columns.allocator,
                                          columndoc->parent->dic);
                for (size_t i = 0; i < key_columns->columns.num_elems; i++) {
                        jak_vector_set(&key_columns->columns,
                                i,
                                JAK_VECTOR_GET(&columnCpy, columnIndices[i], jak_column_doc_column));
                        jak_column_doc_column *column = JAK_VECTOR_GET(&key_columns->columns, i,
                                                                       jak_column_doc_column);
                        sort_jak_columndoc_column(column, columndoc->parent->dic);
                }

                jak_vector_drop(&columnCpy);
                free(columnIndices);
        }
        jak_vector_drop(&cpy);
}

static void sort_jak_columndoc_values(jak_column_doc_obj *columndoc)
{
        if (columndoc->parent->read_optimized) {
                SORT_META_MODEL_VALUES(columndoc->bool_prop_keys,
                                       columndoc->bool_prop_vals,
                                       jak_archive_field_boolean_t,
                                       compare_field_boolean_t_leq);
                SORT_META_MODEL_VALUES(columndoc->int8_prop_keys,
                                       columndoc->int8_prop_vals,
                                       jak_archive_field_i8_t,
                                       compare_field_i8_t_leq);
                SORT_META_MODEL_VALUES(columndoc->int16_prop_keys,
                                       columndoc->int16_prop_vals,
                                       jak_archive_field_i16_t,
                                       compare_field_i16_t_leq);
                SORT_META_MODEL_VALUES(columndoc->int32_prop_keys,
                                       columndoc->int32_prop_vals,
                                       jak_archive_field_i32_t,
                                       compare_field_i32_t_leq);
                SORT_META_MODEL_VALUES(columndoc->int64_prop_keys,
                                       columndoc->int64_prop_vals,
                                       jak_archive_field_i64_t,
                                       compare_field_i64_t_leq);
                SORT_META_MODEL_VALUES(columndoc->uint8_prop_keys,
                                       columndoc->uint8_prop_vals,
                                       jak_archive_field_u8_t,
                                       compare_field_u8_t_leq);
                SORT_META_MODEL_VALUES(columndoc->uint16_prop_keys,
                                       columndoc->uint16_prop_vals,
                                       jak_archive_field_u16_t,
                                       compare_field_u16_t_leq);
                SORT_META_MODEL_VALUES(columndoc->uin32_prop_keys,
                                       columndoc->uint32_prop_vals,
                                       jak_archive_field_u32_t,
                                       compare_field_u32_t_leq);
                SORT_META_MODEL_VALUES(columndoc->uint64_prop_keys,
                                       columndoc->uint64_prop_vals,
                                       jak_archive_field_u64_t,
                                       compare_field_u64_t_leq);
                SORT_META_MODEL_VALUES(columndoc->float_prop_keys,
                                       columndoc->float_prop_vals,
                                       jak_archive_field_number_t,
                                       compare_field_number_t_leq);
                sort_meta_model_jak_string_values(&columndoc->jak_string_prop_keys,
                                              &columndoc->jak_string_prop_vals,
                                              columndoc->parent->dic);

                SORT_META_MODEL_ARRAYS(columndoc->bool_array_prop_keys,
                                       columndoc->bool_array_prop_vals,
                                       compare_field_boolean_t_array_leq);
                SORT_META_MODEL_ARRAYS(columndoc->int8_array_prop_keys,
                                       columndoc->int8_array_prop_vals,
                                       compare_field_i8_t_array_leq);
                SORT_META_MODEL_ARRAYS(columndoc->int16_array_prop_keys,
                                       columndoc->int16_array_prop_vals,
                                       compare_field_i16_t_array_leq);
                SORT_META_MODEL_ARRAYS(columndoc->int32_array_prop_keys,
                                       columndoc->int32_array_prop_vals,
                                       compare_field_i32_t_array_leq);
                SORT_META_MODEL_ARRAYS(columndoc->int64_array_prop_keys,
                                       columndoc->int64_array_prop_vals,
                                       compare_field_i64_t_array_leq);
                SORT_META_MODEL_ARRAYS(columndoc->uint8_array_prop_keys,
                                       columndoc->uint8_array_prop_vals,
                                       compare_field_u8_t_array_leq);
                SORT_META_MODEL_ARRAYS(columndoc->uint16_array_prop_keys,
                                       columndoc->uint16_array_prop_vals,
                                       compare_field_u16_t_array_leq);
                SORT_META_MODEL_ARRAYS(columndoc->uint32_array_prop_keys,
                                       columndoc->uint32_array_prop_vals,
                                       compare_field_u32_t_array_leq);
                SORT_META_MODEL_ARRAYS(columndoc->uint64_array_prop_keys,
                                       columndoc->ui64_array_prop_vals,
                                       compare_field_u64_t_array_leq);
                SORT_META_MODEL_ARRAYS(columndoc->float_array_prop_keys,
                                       columndoc->float_array_prop_vals,
                                       compare_field_number_t_array_leq);
                sort_jak_columndoc_strings_arrays(&columndoc->jak_string_array_prop_keys,
                                              &columndoc->jak_string_array_prop_vals,
                                              columndoc->parent->dic);

                sort_jak_columndoc_column_arrays(columndoc);
        }
}

static void sort_jak_columndoc_entries(jak_column_doc_obj *columndoc)
{
        if (columndoc->parent->read_optimized) {
                sort_jak_columndoc_values(columndoc);
                sort_nested_primitive_object(columndoc);
                sorted_nested_array_objects(columndoc);
        }
}

jak_column_doc *jak_doc_entries_columndoc(const jak_doc_bulk *bulk, const jak_doc_entries *partition,
                                             bool read_optimized)
{
        if (!bulk || !partition) {
                return NULL;
        }

        // Step 1: encode all strings at once in a bulk
        char *const *key_strings = JAK_VECTOR_ALL(&bulk->keys, char *);
        char *const *valueStrings = JAK_VECTOR_ALL(&bulk->values, char *);
        jak_string_dict_insert(bulk->dic, NULL, key_strings, jak_vector_length(&bulk->keys), 0);
        jak_string_dict_insert(bulk->dic, NULL, valueStrings, jak_vector_length(&bulk->values), 0);

        // Step 2: for each document doc, create a meta doc, and construct a binary compressed document
        const jak_doc *models = JAK_VECTOR_ALL(&bulk->models, jak_doc);
        JAK_ASSERT (bulk->models.num_elems == 1);

        const jak_doc *model = models;

        jak_column_doc *columndoc = JAK_MALLOC(sizeof(jak_column_doc));
        columndoc->read_optimized = read_optimized;
        jak_error err;
        if (!jak_columndoc_create(columndoc, &err, model, bulk, partition, bulk->dic)) {
                jak_error_print_and_abort(&err);
        }

        if (columndoc->read_optimized) {
                sort_jak_columndoc_entries(&columndoc->columndoc);
        }

        return columndoc;
}

bool jak_doc_entries_drop(jak_doc_entries *partition)
{
        JAK_UNUSED(partition);
        return true;
}

static void create_doc(jak_doc_obj *model, jak_doc *doc)
{
        jak_vector_create(&model->entries, NULL, sizeof(jak_doc_entries), 50);
        model->doc = doc;
}

static void create_typed_vector(jak_doc_entries *entry)
{
        size_t size;
        switch (entry->type) {
                case JAK_FIELD_NULL:
                        size = sizeof(field_null_t);
                        break;
                case JAK_FIELD_BOOLEAN:
                        size = sizeof(jak_archive_field_boolean_t);
                        break;
                case JAK_FIELD_INT8:
                        size = sizeof(jak_archive_field_i8_t);
                        break;
                case JAK_FIELD_INT16:
                        size = sizeof(jak_archive_field_i16_t);
                        break;
                case JAK_FIELD_INT32:
                        size = sizeof(jak_archive_field_i32_t);
                        break;
                case JAK_FIELD_INT64:
                        size = sizeof(jak_archive_field_i64_t);
                        break;
                case JAK_FIELD_UINT8:
                        size = sizeof(jak_archive_field_u8_t);
                        break;
                case JAK_FIELD_UINT16:
                        size = sizeof(jak_archive_field_u16_t);
                        break;
                case JAK_FIELD_UINT32:
                        size = sizeof(jak_archive_field_u32_t);
                        break;
                case JAK_FIELD_UINT64:
                        size = sizeof(jak_archive_field_u64_t);
                        break;
                case JAK_FIELD_FLOAT:
                        size = sizeof(jak_archive_field_number_t);
                        break;
                case JAK_FIELD_STRING:
                        size = sizeof(field_jak_string_t);
                        break;
                case JAK_FIELD_OBJECT:
                        size = sizeof(jak_doc_obj);
                        break;
                default: JAK_ERROR_PRINT_AND_DIE(JAK_ERR_INTERNALERR) /** unknown type */
                        return;
        }
        jak_vector_create(&entry->values, NULL, size, 50);
}

static void entries_drop(jak_doc_entries *entry)
{
        if (entry->type == JAK_FIELD_OBJECT) {
                for (size_t i = 0; i < entry->values.num_elems; i++) {
                        jak_doc_obj *model = JAK_VECTOR_GET(&entry->values, i, jak_doc_obj);
                        jak_doc_drop(model);
                }
        }
        jak_vector_drop(&entry->values);
}

static bool print_value(FILE *file, jak_archive_field_e type, const jak_vector ofType(<T>) *values)
{
        size_t num_values = values->num_elems;
        if (num_values == 0) {
                fprintf(file, "null");
                return true;
        }
        if (num_values > 1) {
                fprintf(file, "[");
        }
        switch (type) {
                case JAK_FIELD_NULL: {
                        for (size_t i = 0; i < num_values; i++) {
                                fprintf(file, "null%s", i + 1 < num_values ? ", " : "");
                        }
                }
                        break;
                case JAK_FIELD_BOOLEAN: {
                        for (size_t i = 0; i < num_values; i++) {
                                jak_archive_field_boolean_t value = *(JAK_VECTOR_GET(values, i, jak_archive_field_boolean_t));
                                if (value != JAK_NULL_BOOLEAN) {
                                        fprintf(file, "%s%s", value == 0 ? "false" : "true",
                                                i + 1 < num_values ? ", " : "");
                                } else {
                                        fprintf(file, "null%s", i + 1 < num_values ? ", " : "");
                                }
                        }
                }
                        break;
                case JAK_FIELD_INT8: {
                        for (size_t i = 0; i < num_values; i++) {
                                jak_archive_field_i8_t value = *(JAK_VECTOR_GET(values, i, jak_archive_field_i8_t));
                                if (value != JAK_NULL_INT8) {
                                        fprintf(file, "%d%s", value, i + 1 < num_values ? ", " : "");
                                } else {
                                        fprintf(file, "null%s", i + 1 < num_values ? ", " : "");
                                }
                        }
                }
                        break;
                case JAK_FIELD_INT16: {
                        for (size_t i = 0; i < num_values; i++) {
                                jak_archive_field_i16_t value = *(JAK_VECTOR_GET(values, i, jak_archive_field_i16_t));
                                if (value != JAK_NULL_INT16) {
                                        fprintf(file, "%d%s", value, i + 1 < num_values ? ", " : "");
                                } else {
                                        fprintf(file, "null%s", i + 1 < num_values ? ", " : "");
                                }
                        }
                }
                        break;
                case JAK_FIELD_INT32: {
                        for (size_t i = 0; i < num_values; i++) {
                                jak_archive_field_i32_t value = *(JAK_VECTOR_GET(values, i, jak_archive_field_i32_t));
                                if (value != JAK_NULL_INT32) {
                                        fprintf(file, "%d%s", value, i + 1 < num_values ? ", " : "");
                                } else {
                                        fprintf(file, "null%s", i + 1 < num_values ? ", " : "");
                                }
                        }
                }
                        break;
                case JAK_FIELD_INT64: {
                        for (size_t i = 0; i < num_values; i++) {
                                jak_archive_field_i64_t value = *(JAK_VECTOR_GET(values, i, jak_archive_field_i64_t));
                                if (value != JAK_NULL_INT64) {
                                        fprintf(file, "%" PRIi64 "%s", value, i + 1 < num_values ? ", " : "");
                                } else {
                                        fprintf(file, "null%s", i + 1 < num_values ? ", " : "");
                                }
                        }
                }
                        break;
                case JAK_FIELD_UINT8: {
                        for (size_t i = 0; i < num_values; i++) {
                                jak_archive_field_u8_t value = *(JAK_VECTOR_GET(values, i, jak_archive_field_u8_t));
                                if (value != JAK_NULL_UINT8) {
                                        fprintf(file, "%d%s", value, i + 1 < num_values ? ", " : "");
                                } else {
                                        fprintf(file, "null%s", i + 1 < num_values ? ", " : "");
                                }
                        }
                }
                        break;
                case JAK_FIELD_UINT16: {
                        for (size_t i = 0; i < num_values; i++) {
                                jak_archive_field_u16_t value = *(JAK_VECTOR_GET(values, i, jak_archive_field_u16_t));
                                if (value != JAK_NULL_UINT16) {
                                        fprintf(file, "%d%s", value, i + 1 < num_values ? ", " : "");
                                } else {
                                        fprintf(file, "null%s", i + 1 < num_values ? ", " : "");
                                }
                        }
                }
                        break;
                case JAK_FIELD_UINT32: {
                        for (size_t i = 0; i < num_values; i++) {
                                jak_archive_field_u32_t value = *(JAK_VECTOR_GET(values, i, jak_archive_field_u32_t));
                                if (value != JAK_NULL_UINT32) {
                                        fprintf(file, "%d%s", value, i + 1 < num_values ? ", " : "");
                                } else {
                                        fprintf(file, "null%s", i + 1 < num_values ? ", " : "");
                                }
                        }
                }
                        break;
                case JAK_FIELD_UINT64: {
                        for (size_t i = 0; i < num_values; i++) {
                                jak_archive_field_u64_t value = *(JAK_VECTOR_GET(values, i, jak_archive_field_u64_t));
                                if (value != JAK_NULL_UINT64) {
                                        fprintf(file, "%" PRIu64 "%s", value, i + 1 < num_values ? ", " : "");
                                } else {
                                        fprintf(file, "null%s", i + 1 < num_values ? ", " : "");
                                }
                        }
                }
                        break;
                case JAK_FIELD_FLOAT: {
                        for (size_t i = 0; i < num_values; i++) {
                                jak_archive_field_number_t value = *(JAK_VECTOR_GET(values, i, jak_archive_field_number_t));
                                if (!isnan(value)) {
                                        fprintf(file, "%f%s", value, i + 1 < num_values ? ", " : "");
                                } else {
                                        fprintf(file, "null%s", i + 1 < num_values ? ", " : "");
                                }
                        }
                }
                        break;
                case JAK_FIELD_STRING: {
                        for (size_t i = 0; i < num_values; i++) {
                                field_jak_string_t value = *(JAK_VECTOR_GET(values, i, field_jak_string_t));
                                if (value) {
                                        fprintf(file, "\"%s\"%s", value, i + 1 < num_values ? ", " : "");
                                } else {
                                        fprintf(file, "null%s", i + 1 < num_values ? ", " : "");
                                }
                        }
                }
                        break;
                case JAK_FIELD_OBJECT: {
                        for (size_t i = 0; i < num_values; i++) {
                                jak_doc_obj *obj = JAK_VECTOR_GET(values, i, jak_doc_obj);
                                if (!JAK_NULL_OBJECT_MODEL(obj)) {
                                        _jak_doc_print_object(file, obj);
                                } else {
                                        fprintf(file, "null");
                                }
                                fprintf(file, "%s", i + 1 < num_values ? ", " : "");
                        }
                }
                        break;
                default: JAK_NOT_IMPLEMENTED;
        }
        if (num_values > 1) {
                fprintf(file, "]");
        }
        return true;
}

static void _jak_doc_print_object(FILE *file, const jak_doc_obj *model)
{
        fprintf(file, "{");
        for (size_t i = 0; i < model->entries.num_elems; i++) {
                jak_doc_entries *entry = JAK_VECTOR_GET(&model->entries, i, jak_doc_entries);
                fprintf(file, "\"%s\": ", entry->key);
                print_value(file, entry->type, &entry->values);
                fprintf(file, "%s", i + 1 < model->entries.num_elems ? ", " : "");
        }
        fprintf(file, "}");
}

