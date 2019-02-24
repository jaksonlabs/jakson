/**
 * Copyright 2019 Marcus Pinnecke
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

#include "carbon/carbon-encoded-doc.h"

CARBON_EXPORT(bool)
carbon_encoded_doc_collection_create(carbon_encoded_doc_collection_t *collection, carbon_err_t *err,
                                     carbon_archive_t *archive)
{
    CARBON_UNUSED(collection);
    CARBON_UNUSED(err);
    CARBON_UNUSED(archive);
    //abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_collection_drop(carbon_encoded_doc_collection_t *collection)
{
    CARBON_UNUSED(collection);
   // abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_collection_print(FILE *file, carbon_encoded_doc_collection_t *collection)
{
    CARBON_UNUSED(file);
    CARBON_UNUSED(collection);
  //  abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_create(carbon_encoded_doc_t *doc, carbon_err_t err, carbon_object_id_t object_id,
                          carbon_encoded_doc_collection_t *collection)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(err);
    CARBON_UNUSED(object_id);
    CARBON_UNUSED(collection);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_drop(carbon_encoded_doc_t *doc)
{
    CARBON_UNUSED(doc);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_get_object_id(carbon_object_id_t *oid, carbon_encoded_doc_t *doc)
{
    CARBON_UNUSED(oid);
    CARBON_UNUSED(doc);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_int8(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_int8_t value)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(value);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_int16(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_int16_t value)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(value);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_int32(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_int32_t value)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(value);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_int64(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_int64_t value)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(value);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_uint8(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_int8_t value)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(value);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_uint16(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_int16_t value)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(value);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_uint32(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_int32_t value)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(value);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_uint64(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_int64_t value)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(value);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_number(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_number_t value)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(value);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_boolean(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_boolean_t value)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(value);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_string(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_string_id_t value)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(value);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_null(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_string_id_t value)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(value);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_object(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_encoded_doc_t *value)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(value);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_array_int8(carbon_encoded_doc_prop_array_t *handle, carbon_encoded_doc_t *doc,
                                       carbon_string_id_t key)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(handle);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_array_int16(carbon_encoded_doc_prop_array_t *handle, carbon_encoded_doc_t *doc,
                                        carbon_string_id_t key)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(handle);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_array_int32(carbon_encoded_doc_prop_array_t *handle, carbon_encoded_doc_t *doc,
                                        carbon_string_id_t key)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(handle);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_array_int64(carbon_encoded_doc_prop_array_t *handle, carbon_encoded_doc_t *doc,
                                        carbon_string_id_t key)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(handle);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_array_uint8(carbon_encoded_doc_prop_array_t *handle, carbon_encoded_doc_t *doc,
                                        carbon_string_id_t key)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(handle);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_array_uint16(carbon_encoded_doc_prop_array_t *handle, carbon_encoded_doc_t *doc,
                                         carbon_string_id_t key)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(handle);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_array_uint32(carbon_encoded_doc_prop_array_t *handle, carbon_encoded_doc_t *doc,
                                         carbon_string_id_t key)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(handle);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_array_uint64(carbon_encoded_doc_prop_array_t *handle, carbon_encoded_doc_t *doc,
                                         carbon_string_id_t key)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(handle);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_array_number(carbon_encoded_doc_prop_array_t *handle, carbon_encoded_doc_t *doc,
                                         carbon_string_id_t key)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(handle);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_array_boolean(carbon_encoded_doc_prop_array_t *handle, carbon_encoded_doc_t *doc,
                                          carbon_string_id_t key)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(handle);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_array_string(carbon_encoded_doc_prop_array_t *handle, carbon_encoded_doc_t *doc,
                                         carbon_string_id_t key)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(handle);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_array_null(carbon_encoded_doc_prop_array_t *handle, carbon_encoded_doc_t *doc,
                                       carbon_string_id_t key)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(handle);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_int8(carbon_encoded_doc_prop_array_t *handle, const carbon_int8_t *values,
                                   uint32_t num_values)
{
    CARBON_UNUSED(handle);
    CARBON_UNUSED(values);
    CARBON_UNUSED(num_values);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_int16(carbon_encoded_doc_prop_array_t *handle, const carbon_int16_t *values,
                                    uint32_t num_values)
{
    CARBON_UNUSED(handle);
    CARBON_UNUSED(values);
    CARBON_UNUSED(num_values);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_int32(carbon_encoded_doc_prop_array_t *handle, const carbon_int32_t *values,
                                    uint32_t num_values)
{
    CARBON_UNUSED(handle);
    CARBON_UNUSED(values);
    CARBON_UNUSED(num_values);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_int64(carbon_encoded_doc_prop_array_t *handle, const carbon_int64_t *values,
                                    uint32_t num_values)
{
    CARBON_UNUSED(handle);
    CARBON_UNUSED(values);
    CARBON_UNUSED(num_values);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_uint8(carbon_encoded_doc_prop_array_t *handle, const carbon_uint8_t *values,
                                    uint32_t num_values)
{
    CARBON_UNUSED(handle);
    CARBON_UNUSED(values);
    CARBON_UNUSED(num_values);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_uint16(carbon_encoded_doc_prop_array_t *handle, const carbon_uint16_t *values,
                                     uint32_t num_values)
{
    CARBON_UNUSED(handle);
    CARBON_UNUSED(values);
    CARBON_UNUSED(num_values);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_uint32(carbon_encoded_doc_prop_array_t *handle, const carbon_uint32_t *values,
                                     uint32_t num_values)
{
    CARBON_UNUSED(handle);
    CARBON_UNUSED(values);
    CARBON_UNUSED(num_values);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_uint64(carbon_encoded_doc_prop_array_t *handle, const carbon_uint64_t *values,
                                     uint32_t num_values)
{
    CARBON_UNUSED(handle);
    CARBON_UNUSED(values);
    CARBON_UNUSED(num_values);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_number(carbon_encoded_doc_prop_array_t *handle, const carbon_number_t *values,
                                     uint32_t num_values)
{
    CARBON_UNUSED(handle);
    CARBON_UNUSED(values);
    CARBON_UNUSED(num_values);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_boolean(carbon_encoded_doc_prop_array_t *handle, const carbon_boolean_t *values,
                                      uint32_t num_values)
{
    CARBON_UNUSED(handle);
    CARBON_UNUSED(values);
    CARBON_UNUSED(num_values);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_string(carbon_encoded_doc_prop_array_t *handle, const carbon_string_id_t *values,
                                     uint32_t num_values)
{
    CARBON_UNUSED(handle);
    CARBON_UNUSED(values);
    CARBON_UNUSED(num_values);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_null(carbon_encoded_doc_prop_array_t *handle, uint32_t how_many)
{
    CARBON_UNUSED(handle);
    CARBON_UNUSED(how_many);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_object(carbon_encoded_doc_prop_array_t *handle, carbon_object_id_t id)
{
    CARBON_UNUSED(handle);
    CARBON_UNUSED(id);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_get_nested_object(carbon_encoded_doc_t *nested, carbon_object_id_t oid, carbon_encoded_doc_t *doc)
{
    CARBON_UNUSED(nested);
    CARBON_UNUSED(oid);
    CARBON_UNUSED(doc);
    abort(); // TODO: implement
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_print(FILE *file, const carbon_encoded_doc_t *doc)
{
    CARBON_UNUSED(file);
    CARBON_UNUSED(doc);
    abort(); // TODO: implement
    return false;
}

