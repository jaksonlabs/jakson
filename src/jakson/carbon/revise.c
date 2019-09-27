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

#include <jakson/carbon.h>
#include <jakson/carbon/revise.h>
#include <jakson/carbon/array_it.h>
#include <jakson/carbon/internal.h>
#include <jakson/carbon/dot.h>
#include <jakson/carbon/find.h>
#include <jakson/carbon/key.h>
#include <jakson/carbon/commit.h>
#include <jakson/carbon/object_it.h>

static bool internal_pack_array(carbon_array_it *it);

static bool internal_pack_object(carbon_object_it *it);

static bool internal_pack_column(carbon_column_it *it);

static bool internal_commit_update(carbon *doc);

static bool carbon_header_rev_inc(carbon *doc);

// ---------------------------------------------------------------------------------------------------------------------

bool carbon_revise_try_begin(carbon_revise *context, carbon *revised_doc, carbon *doc)
{
        ERROR_IF_NULL(context)
        ERROR_IF_NULL(doc)
        if (!doc->versioning.commit_lock) {
                return carbon_revise_begin(context, revised_doc, doc);
        } else {
                return false;
        }
}

bool carbon_revise_begin(carbon_revise *context, carbon *revised_doc, carbon *original)
{
        ERROR_IF_NULL(context)
        ERROR_IF_NULL(original)

        if (LIKELY(original->versioning.is_latest)) {
                spinlock_acquire(&original->versioning.write_lock);
                original->versioning.commit_lock = true;
                context->original = original;
                context->revised_doc = revised_doc;
                error_init(&context->err);
                carbon_clone(context->revised_doc, context->original);
                return true;
        } else {
                ERROR(&original->err, ERR_OUTDATED)
                return false;
        }
}


static void key_unsigned_set(carbon *doc, u64 key)
{
        JAK_ASSERT(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        carbon_key_write_unsigned(&doc->memfile, key);

        memfile_restore_position(&doc->memfile);
}

static void key_signed_set(carbon *doc, i64 key)
{
        JAK_ASSERT(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        carbon_key_write_signed(&doc->memfile, key);

        memfile_restore_position(&doc->memfile);
}

static void key_string_set(carbon *doc, const char *key)
{
        JAK_ASSERT(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        carbon_key_update_string(&doc->memfile, key);

        memfile_restore_position(&doc->memfile);
}

bool carbon_revise_key_generate(unique_id_t *out, carbon_revise *context)
{
        ERROR_IF_NULL(context);
        carbon_key_e key_type;
        carbon_key_type(&key_type, context->revised_doc);
        if (key_type == CARBON_KEY_AUTOKEY) {
                unique_id_t oid;
                unique_id_create(&oid);
                key_unsigned_set(context->revised_doc, oid);
                OPTIONAL_SET(out, oid);
                return true;
        } else {
                ERROR(&context->err, ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_revise_key_set_unsigned(carbon_revise *context, u64 key_value)
{
        ERROR_IF_NULL(context);
        carbon_key_e key_type;
        carbon_key_type(&key_type, context->revised_doc);
        if (key_type == CARBON_KEY_UKEY) {
                key_unsigned_set(context->revised_doc, key_value);
                return true;
        } else {
                ERROR(&context->err, ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_revise_key_set_signed(carbon_revise *context, i64 key_value)
{
        ERROR_IF_NULL(context);
        carbon_key_e key_type;
        carbon_key_type(&key_type, context->revised_doc);
        if (key_type == CARBON_KEY_IKEY) {
                key_signed_set(context->revised_doc, key_value);
                return true;
        } else {
                ERROR(&context->err, ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_revise_key_set_string(carbon_revise *context, const char *key_value)
{
        ERROR_IF_NULL(context);
        carbon_key_e key_type;
        carbon_key_type(&key_type, context->revised_doc);
        if (key_type == CARBON_KEY_SKEY) {
                key_string_set(context->revised_doc, key_value);
                return true;
        } else {
                ERROR(&context->err, ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_revise_iterator_open(carbon_array_it *it, carbon_revise *context)
{
        ERROR_IF_NULL(it);
        ERROR_IF_NULL(context);
        offset_t payload_start = carbon_int_payload_after_header(context->revised_doc);
        ERROR_IF(context->revised_doc->memfile.mode != READ_WRITE, &context->original->err, ERR_INTERNALERR)
        return carbon_array_it_create(it, &context->revised_doc->memfile, &context->original->err, payload_start);
}

bool carbon_revise_iterator_close(carbon_array_it *it)
{
        ERROR_IF_NULL(it);
        return carbon_array_it_drop(it);
}

bool carbon_revise_find_open(carbon_find *out, const char *dot_path, carbon_revise *context)
{
        ERROR_IF_NULL(out)
        ERROR_IF_NULL(dot_path)
        ERROR_IF_NULL(context)
        carbon_dot_path path;
        carbon_dot_path_from_string(&path, dot_path);
        bool status = carbon_find_create(out, &path, context->revised_doc);
        carbon_dot_path_drop(&path);
        return status;
}

bool carbon_revise_find_close(carbon_find *find)
{
        ERROR_IF_NULL(find)
        return carbon_find_drop(find);
}

bool carbon_revise_remove_one(const char *dot_path, carbon *rev_doc, carbon *doc)
{
        carbon_revise revise;
        carbon_revise_begin(&revise, rev_doc, doc);
        bool status = carbon_revise_remove(dot_path, &revise);
        carbon_revise_end(&revise);
        return status;
}

bool carbon_revise_remove(const char *dot_path, carbon_revise *context)
{
        ERROR_IF_NULL(dot_path)
        ERROR_IF_NULL(context)

        carbon_dot_path dot;
        carbon_path_evaluator eval;
        bool result;

        if (carbon_dot_path_from_string(&dot, dot_path)) {
                carbon_path_evaluator_begin_mutable(&eval, &dot, context);

                if (eval.status != CARBON_PATH_RESOLVED) {
                        result = false;
                } else {
                        switch (eval.result.container_type) {
                                case CARBON_ARRAY: {
                                        carbon_array_it *it = &eval.result.containers.array.it;
                                        result = carbon_array_it_remove(it);
                                }
                                        break;
                                case CARBON_COLUMN: {
                                        carbon_column_it *it = &eval.result.containers.column.it;
                                        u32 elem_pos = eval.result.containers.column.elem_pos;
                                        result = carbon_column_it_remove(it, elem_pos);
                                }
                                        break;
                                default: ERROR(&context->original->err, ERR_INTERNALERR);
                                        result = false;
                        }
                }
                carbon_path_evaluator_end(&eval);
                return result;
        } else {
                ERROR(&context->original->err, ERR_DOT_PATH_PARSERR);
                return false;
        }
}

bool carbon_revise_pack(carbon_revise *context)
{
        ERROR_IF_NULL(context);
        carbon_array_it it;
        carbon_revise_iterator_open(&it, context);
        internal_pack_array(&it);
        carbon_revise_iterator_close(&it);
        return true;
}

bool carbon_revise_shrink(carbon_revise *context)
{
        carbon_array_it it;
        carbon_revise_iterator_open(&it, context);
        carbon_array_it_fast_forward(&it);
        if (memfile_remain_size(&it.memfile) > 0) {
                offset_t first_empty_slot = memfile_tell(&it.memfile);
                JAK_ASSERT(memfile_size(&it.memfile) > first_empty_slot);
                offset_t shrink_size = memfile_size(&it.memfile) - first_empty_slot;
                memfile_cut(&it.memfile, shrink_size);
        }

        offset_t size;
        memblock_size(&size, it.memfile.memblock);
        carbon_revise_iterator_close(&it);
        return true;
}

const carbon *carbon_revise_end(carbon_revise *context)
{
        if (LIKELY(context != NULL)) {
                internal_commit_update(context->revised_doc);

                context->original->versioning.is_latest = false;
                context->original->versioning.commit_lock = false;

                spinlock_release(&context->original->versioning.write_lock);

                return context->revised_doc;
        } else {
                ERROR_PRINT(ERR_NULLPTR);
                return NULL;
        }
}

bool carbon_revise_abort(carbon_revise *context)
{
        ERROR_IF_NULL(context)

        carbon_drop(context->revised_doc);
        context->original->versioning.is_latest = true;
        context->original->versioning.commit_lock = false;
        spinlock_release(&context->original->versioning.write_lock);

        return true;
}

static bool internal_pack_array(carbon_array_it *it)
{
        JAK_ASSERT(it);

        /** shrink this array */
        {
                carbon_array_it this_array_it;
                bool is_empty_slot, is_array_end;

                carbon_array_it_copy(&this_array_it, it);
                carbon_int_array_skip_contents(&is_empty_slot, &is_array_end, &this_array_it);

                if (!is_array_end) {

                        ERROR_IF(!is_empty_slot, &it->err, ERR_CORRUPTED);
                        offset_t first_empty_slot_offset = memfile_tell(&this_array_it.memfile);
                        char final;
                        while ((final = *memfile_read(&this_array_it.memfile, sizeof(char))) == 0) {}
                        JAK_ASSERT(final == CARBON_MARRAY_END);
                        offset_t last_empty_slot_offset = memfile_tell(&this_array_it.memfile) - sizeof(char);
                        memfile_seek(&this_array_it.memfile, first_empty_slot_offset);
                        JAK_ASSERT(last_empty_slot_offset > first_empty_slot_offset);

                        memfile_inplace_remove(&this_array_it.memfile,
                                               last_empty_slot_offset - first_empty_slot_offset);

                        final = *memfile_read(&this_array_it.memfile, sizeof(char));
                        JAK_ASSERT(final == CARBON_MARRAY_END);
                }

                carbon_array_it_drop(&this_array_it);
        }

        /** shrink contained containers */
        {
                while (carbon_array_it_next(it)) {
                        carbon_field_type_e type;
                        carbon_array_it_field_type(&type, it);
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
                                        /** nothing to shrink, because there are no padded zeros here */
                                        break;
                                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET: {
                                        carbon_array_it nested_array_it;
                                        carbon_array_it_create(&nested_array_it, &it->memfile, &it->err,
                                                               it->field_access.nested_array_it->payload_start -
                                                               sizeof(u8));
                                        internal_pack_array(&nested_array_it);
                                        JAK_ASSERT(*memfile_peek(&nested_array_it.memfile, sizeof(char)) ==
                                                   CARBON_MARRAY_END);
                                        memfile_skip(&nested_array_it.memfile, sizeof(char));
                                        memfile_seek(&it->memfile, memfile_tell(&nested_array_it.memfile));
                                        carbon_array_it_drop(&nested_array_it);
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
                                        carbon_column_it_rewind(it->field_access.nested_column_it);
                                        internal_pack_column(it->field_access.nested_column_it);
                                        memfile_seek(&it->memfile,
                                                     memfile_tell(&it->field_access.nested_column_it->memfile));
                                        break;
                                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP: {
                                        carbon_object_it nested_object_it;
                                        carbon_object_it_create(&nested_object_it, &it->memfile, &it->err,
                                                                it->field_access.nested_object_it->object_contents_off -
                                                                sizeof(u8));
                                        internal_pack_object(&nested_object_it);
                                        JAK_ASSERT(*memfile_peek(&nested_object_it.memfile, sizeof(char)) ==
                                                   CARBON_MOBJECT_END);
                                        memfile_skip(&nested_object_it.memfile, sizeof(char));
                                        memfile_seek(&it->memfile, memfile_tell(&nested_object_it.memfile));
                                        carbon_object_it_drop(&nested_object_it);
                                }
                                        break;
                                default: ERROR(&it->err, ERR_INTERNALERR);
                                        return false;
                        }
                }
        }

        JAK_ASSERT(*memfile_peek(&it->memfile, sizeof(char)) == CARBON_MARRAY_END);

        return true;
}

static bool internal_pack_object(carbon_object_it *it)
{
        JAK_ASSERT(it);

        /** shrink this object */
        {
                carbon_object_it this_object_it;
                bool is_empty_slot, is_object_end;

                carbon_object_it_copy(&this_object_it, it);
                carbon_int_object_skip_contents(&is_empty_slot, &is_object_end, &this_object_it);

                if (!is_object_end) {

                        ERROR_IF(!is_empty_slot, &it->err, ERR_CORRUPTED);
                        offset_t first_empty_slot_offset = memfile_tell(&this_object_it.memfile);
                        char final;
                        while ((final = *memfile_read(&this_object_it.memfile, sizeof(char))) == 0) {}
                        JAK_ASSERT(final == CARBON_MOBJECT_END);
                        offset_t last_empty_slot_offset = memfile_tell(&this_object_it.memfile) - sizeof(char);
                        memfile_seek(&this_object_it.memfile, first_empty_slot_offset);
                        JAK_ASSERT(last_empty_slot_offset > first_empty_slot_offset);

                        memfile_inplace_remove(&this_object_it.memfile,
                                               last_empty_slot_offset - first_empty_slot_offset);

                        final = *memfile_read(&this_object_it.memfile, sizeof(char));
                        JAK_ASSERT(final == CARBON_MOBJECT_END);
                }

                carbon_object_it_drop(&this_object_it);
        }

        /** shrink contained containers */
        {
                while (carbon_object_it_next(it)) {
                        carbon_field_type_e type;
                        carbon_object_it_prop_type(&type, it);
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
                                        /** nothing to shrink, because there are no padded zeros here */
                                        break;
                                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET: {
                                        carbon_array_it nested_array_it;
                                        carbon_array_it_create(&nested_array_it, &it->memfile, &it->err,
                                                               it->field.value.data.nested_array_it->payload_start -
                                                               sizeof(u8));
                                        internal_pack_array(&nested_array_it);
                                        JAK_ASSERT(*memfile_peek(&nested_array_it.memfile, sizeof(char)) ==
                                                   CARBON_MARRAY_END);
                                        memfile_skip(&nested_array_it.memfile, sizeof(char));
                                        memfile_seek(&it->memfile, memfile_tell(&nested_array_it.memfile));
                                        carbon_array_it_drop(&nested_array_it);
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
                                        carbon_column_it_rewind(it->field.value.data.nested_column_it);
                                        internal_pack_column(it->field.value.data.nested_column_it);
                                        memfile_seek(&it->memfile,
                                                     memfile_tell(&it->field.value.data.nested_column_it->memfile));
                                        break;
                                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP: {
                                        carbon_object_it nested_object_it;
                                        carbon_object_it_create(&nested_object_it, &it->memfile, &it->err,
                                                                it->field.value.data.nested_object_it->object_contents_off -
                                                                sizeof(u8));
                                        internal_pack_object(&nested_object_it);
                                        JAK_ASSERT(*memfile_peek(&nested_object_it.memfile, sizeof(char)) ==
                                                   CARBON_MOBJECT_END);
                                        memfile_skip(&nested_object_it.memfile, sizeof(char));
                                        memfile_seek(&it->memfile, memfile_tell(&nested_object_it.memfile));
                                        carbon_object_it_drop(&nested_object_it);
                                }
                                        break;
                                default: ERROR(&it->err, ERR_INTERNALERR);
                                        return false;
                        }
                }
        }

        JAK_ASSERT(*memfile_peek(&it->memfile, sizeof(char)) == CARBON_MOBJECT_END);

        return true;
}

static bool internal_pack_column(carbon_column_it *it)
{
        JAK_ASSERT(it);

        u32 free_space = (it->column_capacity - it->column_num_elements) * carbon_int_get_type_value_size(it->type);
        offset_t payload_start = carbon_int_column_get_payload_off(it);
        u64 payload_size = it->column_num_elements * carbon_int_get_type_value_size(it->type);
        memfile_seek(&it->memfile, payload_start);
        memfile_skip(&it->memfile, payload_size);

        if (free_space > 0) {
                memfile_inplace_remove(&it->memfile, free_space);

                memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
                memfile_skip_uintvar_stream(&it->memfile); // skip num of elements counter
                memfile_update_uintvar_stream(&it->memfile,
                                              it->column_num_elements); // update capacity counter to num elems

                memfile_skip(&it->memfile, payload_size);

                return true;
        } else {
                return false;
        }
}

static bool internal_commit_update(carbon *doc)
{
        JAK_ASSERT(doc);
        return carbon_header_rev_inc(doc);
}

static bool carbon_header_rev_inc(carbon *doc)
{
        JAK_ASSERT(doc);

        carbon_key_e key_type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        carbon_key_read(NULL, &key_type, &doc->memfile);
        if (carbon_has_key(key_type)) {
                u64 raw_data_len = 0;
                const void *raw_data = carbon_raw_data(&raw_data_len, doc);
                carbon_commit_hash_update(&doc->memfile, raw_data, raw_data_len);
        }
        memfile_restore_position(&doc->memfile);

        return true;
}