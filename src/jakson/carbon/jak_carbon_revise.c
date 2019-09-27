/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#include <jakson/jak_carbon.h>
#include <jakson/carbon/jak_carbon_revise.h>
#include <jakson/carbon/jak_carbon_array_it.h>
#include <jakson/carbon/jak_carbon_int.h>
#include <jakson/carbon/jak_carbon_dot.h>
#include <jakson/carbon/jak_carbon_find.h>
#include <jakson/carbon/jak_carbon_key.h>
#include <jakson/carbon/jak_carbon_commit.h>
#include <jakson/carbon/jak_carbon_object_it.h>

static bool internal_jak_pack_array(jak_carbon_array_it *it);

static bool internal_jak_pack_object(jak_carbon_object_it *it);

static bool internal_jak_pack_column(jak_carbon_column_it *it);

static bool internal_commit_update(jak_carbon *doc);

static bool carbon_header_rev_inc(jak_carbon *doc);

// ---------------------------------------------------------------------------------------------------------------------

bool jak_carbon_revise_try_begin(jak_carbon_revise *context, jak_carbon *revised_doc, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(context)
        JAK_ERROR_IF_NULL(doc)
        if (!doc->versioning.commit_lock) {
                return jak_carbon_revise_begin(context, revised_doc, doc);
        } else {
                return false;
        }
}

bool jak_carbon_revise_begin(jak_carbon_revise *context, jak_carbon *revised_doc, jak_carbon *original)
{
        JAK_ERROR_IF_NULL(context)
        JAK_ERROR_IF_NULL(original)

        if (JAK_LIKELY(original->versioning.is_latest)) {
                jak_spinlock_acquire(&original->versioning.write_lock);
                original->versioning.commit_lock = true;
                context->original = original;
                context->revised_doc = revised_doc;
                jak_error_init(&context->err);
                jak_carbon_clone(context->revised_doc, context->original);
                return true;
        } else {
                JAK_ERROR(&original->err, JAK_ERR_OUTDATED)
                return false;
        }
}


static void key_unsigned_set(jak_carbon *doc, jak_u64 key)
{
        JAK_ASSERT(doc);
        jak_memfile_save_position(&doc->memfile);
        jak_memfile_seek(&doc->memfile, 0);

        jak_carbon_key_write_unsigned(&doc->memfile, key);

        jak_memfile_restore_position(&doc->memfile);
}

static void key_signed_set(jak_carbon *doc, jak_i64 key)
{
        JAK_ASSERT(doc);
        jak_memfile_save_position(&doc->memfile);
        jak_memfile_seek(&doc->memfile, 0);

        jak_carbon_key_write_signed(&doc->memfile, key);

        jak_memfile_restore_position(&doc->memfile);
}

static void key_jak_string_set(jak_carbon *doc, const char *key)
{
        JAK_ASSERT(doc);
        jak_memfile_save_position(&doc->memfile);
        jak_memfile_seek(&doc->memfile, 0);

        jak_carbon_key_update_string(&doc->memfile, key);

        jak_memfile_restore_position(&doc->memfile);
}

bool jak_carbon_revise_key_generate(jak_uid_t *out, jak_carbon_revise *context)
{
        JAK_ERROR_IF_NULL(context);
        jak_carbon_key_e key_type;
        jak_carbon_key_type(&key_type, context->revised_doc);
        if (key_type == JAK_CARBON_KEY_AUTOKEY) {
                jak_uid_t oid;
                jak_unique_id_create(&oid);
                key_unsigned_set(context->revised_doc, oid);
                JAK_OPTIONAL_SET(out, oid);
                return true;
        } else {
                JAK_ERROR(&context->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool jak_carbon_revise_key_set_unsigned(jak_carbon_revise *context, jak_u64 key_value)
{
        JAK_ERROR_IF_NULL(context);
        jak_carbon_key_e key_type;
        jak_carbon_key_type(&key_type, context->revised_doc);
        if (key_type == JAK_CARBON_KEY_UKEY) {
                key_unsigned_set(context->revised_doc, key_value);
                return true;
        } else {
                JAK_ERROR(&context->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool jak_carbon_revise_key_set_signed(jak_carbon_revise *context, jak_i64 key_value)
{
        JAK_ERROR_IF_NULL(context);
        jak_carbon_key_e key_type;
        jak_carbon_key_type(&key_type, context->revised_doc);
        if (key_type == JAK_CARBON_KEY_IKEY) {
                key_signed_set(context->revised_doc, key_value);
                return true;
        } else {
                JAK_ERROR(&context->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool jak_carbon_revise_key_set_string(jak_carbon_revise *context, const char *key_value)
{
        JAK_ERROR_IF_NULL(context);
        jak_carbon_key_e key_type;
        jak_carbon_key_type(&key_type, context->revised_doc);
        if (key_type == JAK_CARBON_KEY_SKEY) {
                key_jak_string_set(context->revised_doc, key_value);
                return true;
        } else {
                JAK_ERROR(&context->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool jak_carbon_revise_iterator_open(jak_carbon_array_it *it, jak_carbon_revise *context)
{
        JAK_ERROR_IF_NULL(it);
        JAK_ERROR_IF_NULL(context);
        jak_offset_t payload_start = jak_carbon_int_payload_after_header(context->revised_doc);
        JAK_ERROR_IF(context->revised_doc->memfile.mode != JAK_READ_WRITE, &context->original->err, JAK_ERR_INTERNALERR)
        return jak_carbon_array_it_create(it, &context->revised_doc->memfile, &context->original->err, payload_start);
}

bool jak_carbon_revise_iterator_close(jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(it);
        return jak_carbon_array_it_drop(it);
}

bool jak_carbon_revise_find_open(jak_carbon_find *out, const char *dot_path, jak_carbon_revise *context)
{
        JAK_ERROR_IF_NULL(out)
        JAK_ERROR_IF_NULL(dot_path)
        JAK_ERROR_IF_NULL(context)
        jak_carbon_dot_path path;
        jak_carbon_dot_path_from_string(&path, dot_path);
        bool status = jak_carbon_find_create(out, &path, context->revised_doc);
        jak_carbon_dot_path_drop(&path);
        return status;
}

bool jak_carbon_revise_find_close(jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(find)
        return jak_carbon_find_drop(find);
}

bool jak_carbon_revise_remove_one(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc)
{
        jak_carbon_revise revise;
        jak_carbon_revise_begin(&revise, rev_doc, doc);
        bool status = jak_carbon_revise_remove(dot_path, &revise);
        jak_carbon_revise_end(&revise);
        return status;
}

bool jak_carbon_revise_remove(const char *dot_path, jak_carbon_revise *context)
{
        JAK_ERROR_IF_NULL(dot_path)
        JAK_ERROR_IF_NULL(context)

        jak_carbon_dot_path dot;
        jak_carbon_path_evaluator eval;
        bool result;

        if (jak_carbon_dot_path_from_string(&dot, dot_path)) {
                jak_carbon_path_evaluator_begin_mutable(&eval, &dot, context);

                if (eval.status != JAK_CARBON_PATH_RESOLVED) {
                        result = false;
                } else {
                        switch (eval.result.container_type) {
                                case JAK_CARBON_ARRAY: {
                                        jak_carbon_array_it *it = &eval.result.containers.array.it;
                                        result = jak_carbon_array_it_remove(it);
                                }
                                        break;
                                case JAK_CARBON_COLUMN: {
                                        jak_carbon_column_it *it = &eval.result.containers.column.it;
                                        jak_u32 elem_pos = eval.result.containers.column.elem_pos;
                                        result = jak_carbon_column_it_remove(it, elem_pos);
                                }
                                        break;
                                default: JAK_ERROR(&context->original->err, JAK_ERR_INTERNALERR);
                                        result = false;
                        }
                }
                jak_carbon_path_evaluator_end(&eval);
                return result;
        } else {
                JAK_ERROR(&context->original->err, JAK_ERR_DOT_PATH_PARSERR);
                return false;
        }
}

bool jak_carbon_revise_pack(jak_carbon_revise *context)
{
        JAK_ERROR_IF_NULL(context);
        jak_carbon_array_it it;
        jak_carbon_revise_iterator_open(&it, context);
        internal_jak_pack_array(&it);
        jak_carbon_revise_iterator_close(&it);
        return true;
}

bool jak_carbon_revise_shrink(jak_carbon_revise *context)
{
        jak_carbon_array_it it;
        jak_carbon_revise_iterator_open(&it, context);
        jak_carbon_array_it_fast_forward(&it);
        if (jak_memfile_remain_size(&it.memfile) > 0) {
                jak_offset_t first_empty_slot = jak_memfile_tell(&it.memfile);
                JAK_ASSERT(jak_memfile_size(&it.memfile) > first_empty_slot);
                jak_offset_t shrink_size = jak_memfile_size(&it.memfile) - first_empty_slot;
                jak_memfile_cut(&it.memfile, shrink_size);
        }

        jak_offset_t size;
        jak_memblock_size(&size, it.memfile.memblock);
        jak_carbon_revise_iterator_close(&it);
        return true;
}

const jak_carbon *jak_carbon_revise_end(jak_carbon_revise *context)
{
        if (JAK_LIKELY(context != NULL)) {
                internal_commit_update(context->revised_doc);

                context->original->versioning.is_latest = false;
                context->original->versioning.commit_lock = false;

                jak_spinlock_release(&context->original->versioning.write_lock);

                return context->revised_doc;
        } else {
                JAK_ERROR_PRINT(JAK_ERR_NULLPTR);
                return NULL;
        }
}

bool jak_carbon_revise_abort(jak_carbon_revise *context)
{
        JAK_ERROR_IF_NULL(context)

        jak_carbon_drop(context->revised_doc);
        context->original->versioning.is_latest = true;
        context->original->versioning.commit_lock = false;
        jak_spinlock_release(&context->original->versioning.write_lock);

        return true;
}

static bool internal_jak_pack_array(jak_carbon_array_it *it)
{
        JAK_ASSERT(it);

        /* shrink this array */
        {
                jak_carbon_array_it this_array_it;
                bool is_empty_slot, is_array_end;

                jak_carbon_array_it_copy(&this_array_it, it);
                jak_carbon_int_array_skip_contents(&is_empty_slot, &is_array_end, &this_array_it);

                if (!is_array_end) {

                        JAK_ERROR_IF(!is_empty_slot, &it->err, JAK_ERR_CORRUPTED);
                        jak_offset_t first_empty_slot_offset = jak_memfile_tell(&this_array_it.memfile);
                        char final;
                        while ((final = *jak_memfile_read(&this_array_it.memfile, sizeof(char))) == 0) {}
                        JAK_ASSERT(final == CARBON_MARRAY_END);
                        jak_offset_t last_empty_slot_offset = jak_memfile_tell(&this_array_it.memfile) - sizeof(char);
                        jak_memfile_seek(&this_array_it.memfile, first_empty_slot_offset);
                        JAK_ASSERT(last_empty_slot_offset > first_empty_slot_offset);

                        jak_memfile_inplace_remove(&this_array_it.memfile,
                                               last_empty_slot_offset - first_empty_slot_offset);

                        final = *jak_memfile_read(&this_array_it.memfile, sizeof(char));
                        JAK_ASSERT(final == CARBON_MARRAY_END);
                }

                jak_carbon_array_it_drop(&this_array_it);
        }

        /* shrink contained containers */
        {
                while (jak_carbon_array_it_next(it)) {
                        jak_carbon_field_type_e type;
                        jak_carbon_array_it_field_type(&type, it);
                        switch (type) {
                                case CARBON_FIELD_NULL:
                                case CARBON_FIELD_TRUE:
                                case CARBON_FIELD_FALSE:
                                case CARBON_FIELD_STRING:
                                case CARBON_FIELD_NUMBER_U8:
                                case CARBON_FIELD_NUMBER_U16:
                                case CARBON_FIELD_NUMBER_U32:
                                case CARBON_FIELD_NUMBER_U64:
                                case CARBON_FIELD_NUMBER_I8:
                                case CARBON_FIELD_NUMBER_I16:
                                case CARBON_FIELD_NUMBER_I32:
                                case CARBON_FIELD_NUMBER_I64:
                                case CARBON_FIELD_NUMBER_FLOAT:
                                case CARBON_FIELD_BINARY:
                                case CARBON_FIELD_BINARY_CUSTOM:
                                        /* nothing to shrink, because there are no padded zeros here */
                                        break;
                                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET: {
                                        jak_carbon_array_it nested_array_it;
                                        jak_carbon_array_it_create(&nested_array_it, &it->memfile, &it->err,
                                                               it->field_access.nested_array_it->payload_start -
                                                               sizeof(jak_u8));
                                        internal_jak_pack_array(&nested_array_it);
                                        JAK_ASSERT(*jak_memfile_peek(&nested_array_it.memfile, sizeof(char)) ==
                                                   CARBON_MARRAY_END);
                                        jak_memfile_skip(&nested_array_it.memfile, sizeof(char));
                                        jak_memfile_seek(&it->memfile, jak_memfile_tell(&nested_array_it.memfile));
                                        jak_carbon_array_it_drop(&nested_array_it);
                                }
                                        break;
                                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                                        jak_carbon_column_it_rewind(it->field_access.nested_column_it);
                                        internal_jak_pack_column(it->field_access.nested_column_it);
                                        jak_memfile_seek(&it->memfile,
                                                     jak_memfile_tell(&it->field_access.nested_column_it->memfile));
                                        break;
                                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP: {
                                        jak_carbon_object_it nested_object_it;
                                        jak_carbon_object_it_create(&nested_object_it, &it->memfile, &it->err,
                                                                it->field_access.nested_object_it->object_contents_off -
                                                                sizeof(jak_u8));
                                        internal_jak_pack_object(&nested_object_it);
                                        JAK_ASSERT(*jak_memfile_peek(&nested_object_it.memfile, sizeof(char)) ==
                                                   CARBON_MOBJECT_END);
                                        jak_memfile_skip(&nested_object_it.memfile, sizeof(char));
                                        jak_memfile_seek(&it->memfile, jak_memfile_tell(&nested_object_it.memfile));
                                        jak_carbon_object_it_drop(&nested_object_it);
                                }
                                        break;
                                default: JAK_ERROR(&it->err, JAK_ERR_INTERNALERR);
                                        return false;
                        }
                }
        }

        JAK_ASSERT(*jak_memfile_peek(&it->memfile, sizeof(char)) == CARBON_MARRAY_END);

        return true;
}

static bool internal_jak_pack_object(jak_carbon_object_it *it)
{
        JAK_ASSERT(it);

        /* shrink this object */
        {
                jak_carbon_object_it this_object_it;
                bool is_empty_slot, is_object_end;

                jak_carbon_object_it_copy(&this_object_it, it);
                jak_carbon_int_object_skip_contents(&is_empty_slot, &is_object_end, &this_object_it);

                if (!is_object_end) {

                        JAK_ERROR_IF(!is_empty_slot, &it->err, JAK_ERR_CORRUPTED);
                        jak_offset_t first_empty_slot_offset = jak_memfile_tell(&this_object_it.memfile);
                        char final;
                        while ((final = *jak_memfile_read(&this_object_it.memfile, sizeof(char))) == 0) {}
                        JAK_ASSERT(final == CARBON_MOBJECT_END);
                        jak_offset_t last_empty_slot_offset = jak_memfile_tell(&this_object_it.memfile) - sizeof(char);
                        jak_memfile_seek(&this_object_it.memfile, first_empty_slot_offset);
                        JAK_ASSERT(last_empty_slot_offset > first_empty_slot_offset);

                        jak_memfile_inplace_remove(&this_object_it.memfile,
                                               last_empty_slot_offset - first_empty_slot_offset);

                        final = *jak_memfile_read(&this_object_it.memfile, sizeof(char));
                        JAK_ASSERT(final == CARBON_MOBJECT_END);
                }

                jak_carbon_object_it_drop(&this_object_it);
        }

        /* shrink contained containers */
        {
                while (jak_carbon_object_it_next(it)) {
                        jak_carbon_field_type_e type;
                        jak_carbon_object_it_prop_type(&type, it);
                        switch (type) {
                                case CARBON_FIELD_NULL:
                                case CARBON_FIELD_TRUE:
                                case CARBON_FIELD_FALSE:
                                case CARBON_FIELD_STRING:
                                case CARBON_FIELD_NUMBER_U8:
                                case CARBON_FIELD_NUMBER_U16:
                                case CARBON_FIELD_NUMBER_U32:
                                case CARBON_FIELD_NUMBER_U64:
                                case CARBON_FIELD_NUMBER_I8:
                                case CARBON_FIELD_NUMBER_I16:
                                case CARBON_FIELD_NUMBER_I32:
                                case CARBON_FIELD_NUMBER_I64:
                                case CARBON_FIELD_NUMBER_FLOAT:
                                case CARBON_FIELD_BINARY:
                                case CARBON_FIELD_BINARY_CUSTOM:
                                        /* nothing to shrink, because there are no padded zeros here */
                                        break;
                                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET: {
                                        jak_carbon_array_it nested_array_it;
                                        jak_carbon_array_it_create(&nested_array_it, &it->memfile, &it->err,
                                                               it->field.value.data.nested_array_it->payload_start -
                                                               sizeof(jak_u8));
                                        internal_jak_pack_array(&nested_array_it);
                                        JAK_ASSERT(*jak_memfile_peek(&nested_array_it.memfile, sizeof(char)) ==
                                                   CARBON_MARRAY_END);
                                        jak_memfile_skip(&nested_array_it.memfile, sizeof(char));
                                        jak_memfile_seek(&it->memfile, jak_memfile_tell(&nested_array_it.memfile));
                                        jak_carbon_array_it_drop(&nested_array_it);
                                }
                                        break;
                                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                                        jak_carbon_column_it_rewind(it->field.value.data.nested_column_it);
                                        internal_jak_pack_column(it->field.value.data.nested_column_it);
                                        jak_memfile_seek(&it->memfile,
                                                     jak_memfile_tell(&it->field.value.data.nested_column_it->memfile));
                                        break;
                                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP: {
                                        jak_carbon_object_it nested_object_it;
                                        jak_carbon_object_it_create(&nested_object_it, &it->memfile, &it->err,
                                                                it->field.value.data.nested_object_it->object_contents_off -
                                                                sizeof(jak_u8));
                                        internal_jak_pack_object(&nested_object_it);
                                        JAK_ASSERT(*jak_memfile_peek(&nested_object_it.memfile, sizeof(char)) ==
                                                   CARBON_MOBJECT_END);
                                        jak_memfile_skip(&nested_object_it.memfile, sizeof(char));
                                        jak_memfile_seek(&it->memfile, jak_memfile_tell(&nested_object_it.memfile));
                                        jak_carbon_object_it_drop(&nested_object_it);
                                }
                                        break;
                                default: JAK_ERROR(&it->err, JAK_ERR_INTERNALERR);
                                        return false;
                        }
                }
        }

        JAK_ASSERT(*jak_memfile_peek(&it->memfile, sizeof(char)) == CARBON_MOBJECT_END);

        return true;
}

static bool internal_jak_pack_column(jak_carbon_column_it *it)
{
        JAK_ASSERT(it);

        jak_u32 free_space = (it->column_capacity - it->column_num_elements) * jak_carbon_int_get_type_value_size(it->type);
        jak_offset_t payload_start = jak_carbon_int_column_get_payload_off(it);
        jak_u64 payload_size = it->column_num_elements * jak_carbon_int_get_type_value_size(it->type);
        jak_memfile_seek(&it->memfile, payload_start);
        jak_memfile_skip(&it->memfile, payload_size);

        if (free_space > 0) {
                jak_memfile_inplace_remove(&it->memfile, free_space);

                jak_memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
                jak_memfile_skip_uintvar_stream(&it->memfile); // skip num of elements counter
                jak_memfile_update_uintvar_stream(&it->memfile,
                                              it->column_num_elements); // update capacity counter to num elems

                jak_memfile_skip(&it->memfile, payload_size);

                return true;
        } else {
                return false;
        }
}

static bool internal_commit_update(jak_carbon *doc)
{
        JAK_ASSERT(doc);
        return carbon_header_rev_inc(doc);
}

static bool carbon_header_rev_inc(jak_carbon *doc)
{
        JAK_ASSERT(doc);

        jak_carbon_key_e key_type;
        jak_memfile_save_position(&doc->memfile);
        jak_memfile_seek(&doc->memfile, 0);
        jak_carbon_key_read(NULL, &key_type, &doc->memfile);
        if (jak_carbon_has_key(key_type)) {
                jak_u64 raw_data_len = 0;
                const void *raw_data = jak_carbon_raw_data(&raw_data_len, doc);
                jak_carbon_commit_hash_update(&doc->memfile, raw_data, raw_data_len);
        }
        jak_memfile_restore_position(&doc->memfile);

        return true;
}