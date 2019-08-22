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

#include <jak_carbon.h>
#include <jak_carbon_revise.h>
#include <jak_carbon_array_it.h>
#include <jak_carbon_int.h>
#include <jak_carbon_dot.h>
#include <jak_carbon_find.h>
#include <jak_carbon_key.h>
#include <jak_carbon_commit.h>
#include <jak_carbon_object_it.h>

static bool internal_pack_array(struct jak_carbon_array_it *it);

static bool internal_pack_object(struct jak_carbon_object_it *it);

static bool internal_pack_column(struct jak_carbon_column_it *it);

static bool internal_commit_update(struct jak_carbon *doc);

static bool carbon_header_rev_inc(struct jak_carbon *doc);

// ---------------------------------------------------------------------------------------------------------------------

bool carbon_revise_try_begin(struct jak_carbon_revise *context, struct jak_carbon *revised_doc, struct jak_carbon *doc)
{
        error_if_null(context)
        error_if_null(doc)
        if (!doc->versioning.commit_lock) {
                return carbon_revise_begin(context, revised_doc, doc);
        } else {
                return false;
        }
}

bool carbon_revise_begin(struct jak_carbon_revise *context, struct jak_carbon *revised_doc, struct jak_carbon *original)
{
        error_if_null(context)
        error_if_null(original)

        if (likely(original->versioning.is_latest)) {
                spin_acquire(&original->versioning.write_lock);
                original->versioning.commit_lock = true;
                context->original = original;
                context->revised_doc = revised_doc;
                error_init(&context->err);
                carbon_clone(context->revised_doc, context->original);
                return true;
        } else {
                error(&original->err, JAK_ERR_OUTDATED)
                return false;
        }
}


static void key_unsigned_set(struct jak_carbon *doc, jak_u64 key)
{
        assert(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        carbon_key_write_unsigned(&doc->memfile, key);

        memfile_restore_position(&doc->memfile);
}

static void key_signed_set(struct jak_carbon *doc, jak_i64 key)
{
        assert(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        carbon_key_write_signed(&doc->memfile, key);

        memfile_restore_position(&doc->memfile);
}

static void key_string_set(struct jak_carbon *doc, const char *key)
{
        assert(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        carbon_key_update_string(&doc->memfile, key);

        memfile_restore_position(&doc->memfile);
}

bool carbon_revise_key_generate(jak_global_id_t *out, struct jak_carbon_revise *context)
{
        error_if_null(context);
        enum carbon_key_type key_type;
        carbon_key_type(&key_type, context->revised_doc);
        if (key_type == CARBON_KEY_AUTOKEY) {
                jak_global_id_t oid;
                global_id_create(&oid);
                key_unsigned_set(context->revised_doc, oid);
                JAK_optional_set(out, oid);
                return true;
        } else {
                error(&context->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_revise_key_set_unsigned(struct jak_carbon_revise *context, jak_u64 key_value)
{
        error_if_null(context);
        enum carbon_key_type key_type;
        carbon_key_type(&key_type, context->revised_doc);
        if (key_type == CARBON_KEY_UKEY) {
                key_unsigned_set(context->revised_doc, key_value);
                return true;
        } else {
                error(&context->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_revise_key_set_signed(struct jak_carbon_revise *context, jak_i64 key_value)
{
        error_if_null(context);
        enum carbon_key_type key_type;
        carbon_key_type(&key_type, context->revised_doc);
        if (key_type == CARBON_KEY_IKEY) {
                key_signed_set(context->revised_doc, key_value);
                return true;
        } else {
                error(&context->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_revise_key_set_string(struct jak_carbon_revise *context, const char *key_value)
{
        error_if_null(context);
        enum carbon_key_type key_type;
        carbon_key_type(&key_type, context->revised_doc);
        if (key_type == CARBON_KEY_SKEY) {
                key_string_set(context->revised_doc, key_value);
                return true;
        } else {
                error(&context->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_revise_iterator_open(struct jak_carbon_array_it *it, struct jak_carbon_revise *context)
{
        error_if_null(it);
        error_if_null(context);
        jak_offset_t payload_start = carbon_int_payload_after_header(context->revised_doc);
        error_if(context->revised_doc->memfile.mode != READ_WRITE, &context->original->err, JAK_ERR_INTERNALERR)
        return carbon_array_it_create(it, &context->revised_doc->memfile, &context->original->err, payload_start);
}

bool carbon_revise_iterator_close(struct jak_carbon_array_it *it)
{
        error_if_null(it);
        return carbon_array_it_drop(it);
}

bool carbon_revise_find_open(struct jak_carbon_find *out, const char *dot_path, struct jak_carbon_revise *context)
{
        error_if_null(out)
        error_if_null(dot_path)
        error_if_null(context)
        struct jak_carbon_dot_path path;
        carbon_dot_path_from_string(&path, dot_path);
        bool status = carbon_find_create(out, &path, context->revised_doc);
        carbon_dot_path_drop(&path);
        return status;
}

bool carbon_revise_find_close(struct jak_carbon_find *find)
{
        error_if_null(find)
        return carbon_find_drop(find);
}

bool carbon_revise_remove_one(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc)
{
        struct jak_carbon_revise revise;
        carbon_revise_begin(&revise, rev_doc, doc);
        bool status = carbon_revise_remove(dot_path, &revise);
        carbon_revise_end(&revise);
        return status;
}

bool carbon_revise_remove(const char *dot_path, struct jak_carbon_revise *context)
{
        error_if_null(dot_path)
        error_if_null(context)

        struct jak_carbon_dot_path dot;
        struct jak_carbon_path_evaluator eval;
        bool result;

        if (carbon_dot_path_from_string(&dot, dot_path)) {
                carbon_path_evaluator_begin_mutable(&eval, &dot, context);

                if (eval.status != CARBON_PATH_RESOLVED) {
                        result = false;
                } else {
                        switch (eval.result.container_type) {
                                case CARBON_ARRAY: {
                                        struct jak_carbon_array_it *it = &eval.result.containers.array.it;
                                        result = carbon_array_it_remove(it);
                                }
                                        break;
                                case CARBON_COLUMN: {
                                        struct jak_carbon_column_it *it = &eval.result.containers.column.it;
                                        jak_u32 elem_pos = eval.result.containers.column.elem_pos;
                                        result = carbon_column_it_remove(it, elem_pos);
                                }
                                        break;
                                default: error(&context->original->err, JAK_ERR_INTERNALERR);
                                        result = false;
                        }
                }
                carbon_path_evaluator_end(&eval);
                return result;
        } else {
                error(&context->original->err, JAK_ERR_DOT_PATH_PARSERR);
                return false;
        }
}

bool carbon_revise_pack(struct jak_carbon_revise *context)
{
        error_if_null(context);
        struct jak_carbon_array_it it;
        carbon_revise_iterator_open(&it, context);
        internal_pack_array(&it);
        carbon_revise_iterator_close(&it);
        return true;
}

bool carbon_revise_shrink(struct jak_carbon_revise *context)
{
        struct jak_carbon_array_it it;
        carbon_revise_iterator_open(&it, context);
        carbon_array_it_fast_forward(&it);
        if (memfile_remain_size(&it.memfile) > 0) {
                jak_offset_t first_empty_slot = memfile_tell(&it.memfile);
                assert(memfile_size(&it.memfile) > first_empty_slot);
                jak_offset_t shrink_size = memfile_size(&it.memfile) - first_empty_slot;
                memfile_cut(&it.memfile, shrink_size);
        }

        jak_offset_t size;
        memblock_size(&size, it.memfile.memblock);
        carbon_revise_iterator_close(&it);
        return true;
}

const struct jak_carbon *carbon_revise_end(struct jak_carbon_revise *context)
{
        if (likely(context != NULL)) {
                internal_commit_update(context->revised_doc);

                context->original->versioning.is_latest = false;
                context->original->versioning.commit_lock = false;

                spin_release(&context->original->versioning.write_lock);

                return context->revised_doc;
        } else {
                error_print(JAK_ERR_NULLPTR);
                return NULL;
        }
}

bool carbon_revise_abort(struct jak_carbon_revise *context)
{
        error_if_null(context)

        carbon_drop(context->revised_doc);
        context->original->versioning.is_latest = true;
        context->original->versioning.commit_lock = false;
        spin_release(&context->original->versioning.write_lock);

        return true;
}

static bool internal_pack_array(struct jak_carbon_array_it *it)
{
        assert(it);

        /* shrink this array */
        {
                struct jak_carbon_array_it this_array_it;
                bool is_empty_slot, is_array_end;

                carbon_array_it_copy(&this_array_it, it);
                carbon_int_array_skip_contents(&is_empty_slot, &is_array_end, &this_array_it);

                if (!is_array_end) {

                        error_if(!is_empty_slot, &it->err, JAK_ERR_CORRUPTED);
                        jak_offset_t first_empty_slot_offset = memfile_tell(&this_array_it.memfile);
                        char final;
                        while ((final = *memfile_read(&this_array_it.memfile, sizeof(char))) == 0) {}
                        assert(final == JAK_CARBON_MARKER_ARRAY_END);
                        jak_offset_t last_empty_slot_offset = memfile_tell(&this_array_it.memfile) - sizeof(char);
                        memfile_seek(&this_array_it.memfile, first_empty_slot_offset);
                        assert(last_empty_slot_offset > first_empty_slot_offset);

                        memfile_inplace_remove(&this_array_it.memfile,
                                               last_empty_slot_offset - first_empty_slot_offset);

                        final = *memfile_read(&this_array_it.memfile, sizeof(char));
                        assert(final == JAK_CARBON_MARKER_ARRAY_END);
                }

                carbon_array_it_drop(&this_array_it);
        }

        /* shrink contained containers */
        {
                while (carbon_array_it_next(it)) {
                        enum carbon_field_type type;
                        carbon_array_it_field_type(&type, it);
                        switch (type) {
                                case CARBON_JAK_FIELD_TYPE_NULL:
                                case CARBON_JAK_FIELD_TYPE_TRUE:
                                case CARBON_JAK_FIELD_TYPE_FALSE:
                                case CARBON_JAK_FIELD_TYPE_STRING:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_U8:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_U16:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_U32:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_U64:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_I8:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_I16:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_I32:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_I64:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_FLOAT:
                                case CARBON_JAK_FIELD_TYPE_BINARY:
                                case CARBON_JAK_FIELD_TYPE_BINARY_CUSTOM:
                                        /* nothing to shrink, because there are no padded zeros here */
                                        break;
                                case CARBON_JAK_FIELD_TYPE_ARRAY: {
                                        struct jak_carbon_array_it nested_array_it;
                                        carbon_array_it_create(&nested_array_it, &it->memfile, &it->err,
                                                               it->field_access.nested_array_it->payload_start -
                                                               sizeof(jak_u8));
                                        internal_pack_array(&nested_array_it);
                                        assert(*memfile_peek(&nested_array_it.memfile, sizeof(char)) ==
                                                       JAK_CARBON_MARKER_ARRAY_END);
                                        memfile_skip(&nested_array_it.memfile, sizeof(char));
                                        memfile_seek(&it->memfile, memfile_tell(&nested_array_it.memfile));
                                        carbon_array_it_drop(&nested_array_it);
                                }
                                        break;
                                case CARBON_JAK_FIELD_TYPE_COLUMN_U8:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_U16:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_U32:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_U64:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_I8:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_I16:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_I32:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_I64:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN:
                                        carbon_column_it_rewind(it->field_access.nested_column_it);
                                        internal_pack_column(it->field_access.nested_column_it);
                                        memfile_seek(&it->memfile,
                                                     memfile_tell(&it->field_access.nested_column_it->memfile));
                                        break;
                                case CARBON_JAK_FIELD_TYPE_OBJECT: {
                                        struct jak_carbon_object_it nested_object_it;
                                        carbon_object_it_create(&nested_object_it, &it->memfile, &it->err,
                                                                it->field_access.nested_object_it->object_contents_off -
                                                                sizeof(jak_u8));
                                        internal_pack_object(&nested_object_it);
                                        assert(*memfile_peek(&nested_object_it.memfile, sizeof(char)) ==
                                                       JAK_CARBON_MARKER_OBJECT_END);
                                        memfile_skip(&nested_object_it.memfile, sizeof(char));
                                        memfile_seek(&it->memfile, memfile_tell(&nested_object_it.memfile));
                                        carbon_object_it_drop(&nested_object_it);
                                }
                                        break;
                                default: error(&it->err, JAK_ERR_INTERNALERR);
                                        return false;
                        }
                }
        }

        assert(*memfile_peek(&it->memfile, sizeof(char)) == JAK_CARBON_MARKER_ARRAY_END);

        return true;
}

static bool internal_pack_object(struct jak_carbon_object_it *it)
{
        assert(it);

        /* shrink this object */
        {
                struct jak_carbon_object_it this_object_it;
                bool is_empty_slot, is_object_end;

                carbon_object_it_copy(&this_object_it, it);
                carbon_int_object_skip_contents(&is_empty_slot, &is_object_end, &this_object_it);

                if (!is_object_end) {

                        error_if(!is_empty_slot, &it->err, JAK_ERR_CORRUPTED);
                        jak_offset_t first_empty_slot_offset = memfile_tell(&this_object_it.memfile);
                        char final;
                        while ((final = *memfile_read(&this_object_it.memfile, sizeof(char))) == 0) {}
                        assert(final == JAK_CARBON_MARKER_OBJECT_END);
                        jak_offset_t last_empty_slot_offset = memfile_tell(&this_object_it.memfile) - sizeof(char);
                        memfile_seek(&this_object_it.memfile, first_empty_slot_offset);
                        assert(last_empty_slot_offset > first_empty_slot_offset);

                        memfile_inplace_remove(&this_object_it.memfile,
                                               last_empty_slot_offset - first_empty_slot_offset);

                        final = *memfile_read(&this_object_it.memfile, sizeof(char));
                        assert(final == JAK_CARBON_MARKER_OBJECT_END);
                }

                carbon_object_it_drop(&this_object_it);
        }

        /* shrink contained containers */
        {
                while (carbon_object_it_next(it)) {
                        enum carbon_field_type type;
                        carbon_object_it_prop_type(&type, it);
                        switch (type) {
                                case CARBON_JAK_FIELD_TYPE_NULL:
                                case CARBON_JAK_FIELD_TYPE_TRUE:
                                case CARBON_JAK_FIELD_TYPE_FALSE:
                                case CARBON_JAK_FIELD_TYPE_STRING:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_U8:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_U16:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_U32:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_U64:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_I8:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_I16:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_I32:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_I64:
                                case CARBON_JAK_FIELD_TYPE_NUMBER_FLOAT:
                                case CARBON_JAK_FIELD_TYPE_BINARY:
                                case CARBON_JAK_FIELD_TYPE_BINARY_CUSTOM:
                                        /* nothing to shrink, because there are no padded zeros here */
                                        break;
                                case CARBON_JAK_FIELD_TYPE_ARRAY: {
                                        struct jak_carbon_array_it nested_array_it;
                                        carbon_array_it_create(&nested_array_it, &it->memfile, &it->err,
                                                               it->field.value.data.nested_array_it->payload_start -
                                                               sizeof(jak_u8));
                                        internal_pack_array(&nested_array_it);
                                        assert(*memfile_peek(&nested_array_it.memfile, sizeof(char)) ==
                                                       JAK_CARBON_MARKER_ARRAY_END);
                                        memfile_skip(&nested_array_it.memfile, sizeof(char));
                                        memfile_seek(&it->memfile, memfile_tell(&nested_array_it.memfile));
                                        carbon_array_it_drop(&nested_array_it);
                                }
                                        break;
                                case CARBON_JAK_FIELD_TYPE_COLUMN_U8:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_U16:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_U32:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_U64:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_I8:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_I16:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_I32:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_I64:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT:
                                case CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN:
                                        carbon_column_it_rewind(it->field.value.data.nested_column_it);
                                        internal_pack_column(it->field.value.data.nested_column_it);
                                        memfile_seek(&it->memfile,
                                                     memfile_tell(&it->field.value.data.nested_column_it->memfile));
                                        break;
                                case CARBON_JAK_FIELD_TYPE_OBJECT: {
                                        struct jak_carbon_object_it nested_object_it;
                                        carbon_object_it_create(&nested_object_it, &it->memfile, &it->err,
                                                                it->field.value.data.nested_object_it->object_contents_off -
                                                                sizeof(jak_u8));
                                        internal_pack_object(&nested_object_it);
                                        assert(*memfile_peek(&nested_object_it.memfile, sizeof(char)) ==
                                                       JAK_CARBON_MARKER_OBJECT_END);
                                        memfile_skip(&nested_object_it.memfile, sizeof(char));
                                        memfile_seek(&it->memfile, memfile_tell(&nested_object_it.memfile));
                                        carbon_object_it_drop(&nested_object_it);
                                }
                                        break;
                                default: error(&it->err, JAK_ERR_INTERNALERR);
                                        return false;
                        }
                }
        }

        assert(*memfile_peek(&it->memfile, sizeof(char)) == JAK_CARBON_MARKER_OBJECT_END);

        return true;
}

static bool internal_pack_column(struct jak_carbon_column_it *it)
{
        assert(it);

        jak_u32 free_space = (it->column_capacity - it->column_num_elements) * carbon_int_get_type_value_size(it->type);
        jak_offset_t payload_start = carbon_int_column_get_payload_off(it);
        jak_u64 payload_size = it->column_num_elements * carbon_int_get_type_value_size(it->type);
        memfile_seek(&it->memfile, payload_start);
        memfile_skip(&it->memfile, payload_size);

        if (free_space > 0) {
                memfile_inplace_remove(&it->memfile, free_space);

                memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
                memfile_skip_uintvar_stream(&it->memfile); // skip num of elements counter
                memfile_update_uintvar_stream(&it->memfile, it->column_num_elements); // update capacity counter to num elems

                memfile_skip(&it->memfile, payload_size);

                return true;
        } else {
                return false;
        }
}

static bool internal_commit_update(struct jak_carbon *doc)
{
        assert(doc);
        return carbon_header_rev_inc(doc);
}

static bool carbon_header_rev_inc(struct jak_carbon *doc)
{
        assert(doc);

        enum carbon_key_type key_type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        carbon_key_read(NULL, &key_type, &doc->memfile);
        if (carbon_has_key(key_type)) {
                jak_u64 raw_data_len = 0;
                const void *raw_data = carbon_raw_data(&raw_data_len, doc);
                carbon_commit_hash_update(&doc->memfile, raw_data, raw_data_len);
        }
        memfile_restore_position(&doc->memfile);

        return true;
}