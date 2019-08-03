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

#include <ark-js/carbon/carbon.h>
#include <ark-js/carbon/carbon-revise.h>
#include <ark-js/carbon/carbon-array-it.h>
#include <ark-js/carbon/carbon-int.h>
#include <ark-js/carbon/carbon-dot.h>
#include <ark-js/carbon/carbon-find.h>
#include <ark-js/carbon/carbon-key.h>
#include <ark-js/carbon/carbon-revision.h>
#include <ark-js/carbon/carbon-object-it.h>

static bool internal_pack_array(struct carbon_array_it *it);
static bool internal_pack_object(struct carbon_object_it *it);
static bool internal_pack_column(struct carbon_column_it *it);
static bool internal_revision_inc(struct carbon *doc);
static bool carbon_header_rev_inc(struct carbon *doc);

// ---------------------------------------------------------------------------------------------------------------------

ARK_EXPORT(bool) carbon_revise_try_begin(struct carbon_revise *context, struct carbon *revised_doc, struct carbon *doc)
{
        error_if_null(context)
        error_if_null(doc)
        if (!doc->versioning.revision_lock) {
                return carbon_revise_begin(context, revised_doc, doc);
        } else {
                return false;
        }
}

ARK_EXPORT(bool) carbon_revise_begin(struct carbon_revise *context, struct carbon *revised_doc, struct carbon *original)
{
        error_if_null(context)
        error_if_null(original)

        if (likely(original->versioning.is_latest)) {
                spin_acquire(&original->versioning.write_lock);
                original->versioning.revision_lock = true;
                context->original = original;
                context->revised_doc = revised_doc;
                error_init(&context->err);
                carbon_clone(context->revised_doc, context->original);
                return true;
        } else {
                error(&original->err, ARK_ERR_OUTDATED)
                return false;
        }
}


static void key_unsigned_set(struct carbon *doc, u64 key)
{
        assert(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        carbon_key_write_unsigned(&doc->memfile, key);

        memfile_restore_position(&doc->memfile);
}

static void key_signed_set(struct carbon *doc, i64 key)
{
        assert(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        carbon_key_write_signed(&doc->memfile, key);

        memfile_restore_position(&doc->memfile);
}

static void key_string_set(struct carbon *doc, const char *key)
{
        assert(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        carbon_key_update_string(&doc->memfile, key);

        memfile_restore_position(&doc->memfile);
}

ARK_EXPORT(bool) carbon_revise_key_generate(object_id_t *out, struct carbon_revise *context)
{
        error_if_null(context);
        enum carbon_primary_key_type key_type;
        carbon_key_get_type(&key_type, context->revised_doc);
        if (key_type == CARBON_KEY_AUTOKEY) {
                object_id_t oid;
                object_id_create(&oid);
                key_unsigned_set(context->revised_doc, oid);
                ark_optional_set(out, oid);
                return true;
        } else {
                error(&context->err, ARK_ERR_TYPEMISMATCH)
                return false;
        }
}

ARK_EXPORT(bool) carbon_revise_key_set_unsigned(struct carbon_revise *context, u64 key_value)
{
        error_if_null(context);
        enum carbon_primary_key_type key_type;
        carbon_key_get_type(&key_type, context->revised_doc);
        if (key_type == CARBON_KEY_UKEY) {
                key_unsigned_set(context->revised_doc, key_value);
                return true;
        } else {
                error(&context->err, ARK_ERR_TYPEMISMATCH)
                return false;
        }
}

ARK_EXPORT(bool) carbon_revise_key_set_signed(struct carbon_revise *context, i64 key_value)
{
        error_if_null(context);
        enum carbon_primary_key_type key_type;
        carbon_key_get_type(&key_type, context->revised_doc);
        if (key_type == CARBON_KEY_IKEY) {
                key_signed_set(context->revised_doc, key_value);
                return true;
        } else {
                error(&context->err, ARK_ERR_TYPEMISMATCH)
                return false;
        }
}

ARK_EXPORT(bool) carbon_revise_key_set_string(struct carbon_revise *context, const char *key_value)
{
        error_if_null(context);
        enum carbon_primary_key_type key_type;
        carbon_key_get_type(&key_type, context->revised_doc);
        if (key_type == CARBON_KEY_SKEY) {
                key_string_set(context->revised_doc, key_value);
                return true;
        } else {
                error(&context->err, ARK_ERR_TYPEMISMATCH)
                return false;
        }
}

ARK_EXPORT(bool) carbon_revise_iterator_open(struct carbon_array_it *it, struct carbon_revise *context)
{
        error_if_null(it);
        error_if_null(context);
        offset_t payload_start = carbon_int_payload_after_header(context->revised_doc);
        error_if(context->revised_doc->memfile.mode != READ_WRITE, &context->original->err, ARK_ERR_INTERNALERR)
        return carbon_array_it_create(it, &context->revised_doc->memfile, &context->original->err, payload_start);
}

ARK_EXPORT(bool) carbon_revise_iterator_close(struct carbon_array_it *it)
{
        error_if_null(it);
        return carbon_array_it_drop(it);
}

ARK_EXPORT(bool) carbon_revise_find_open(struct carbon_find *out, const char *dot_path, struct carbon_revise *context)
{
        error_if_null(out)
        error_if_null(dot_path)
        error_if_null(context)
        struct carbon_dot_path path;
        carbon_dot_path_from_string(&path, dot_path);
        bool status = carbon_find_create(out, &path, context->revised_doc);
        carbon_dot_path_drop(&path);
        return status;
}

ARK_EXPORT(bool) carbon_revise_find_close(struct carbon_find *find)
{
        error_if_null(find)
        return carbon_find_drop(find);
}

ARK_EXPORT(bool) carbon_revise_remove_one(const char *dot_path, struct carbon *rev_doc, struct carbon *doc)
{
        struct carbon_revise revise;
        carbon_revise_begin(&revise, rev_doc, doc);
        bool status = carbon_revise_remove(dot_path, &revise);
        carbon_revise_end(&revise);
        return status;
}

ARK_EXPORT(bool) carbon_revise_remove(const char *dot_path, struct carbon_revise *context)
{
        error_if_null(dot_path)
        error_if_null(context)

        struct carbon_dot_path dot;
        struct carbon_path_evaluator eval;
        bool result;

        if (carbon_dot_path_from_string(&dot, dot_path)) {
                carbon_path_evaluator_begin_mutable(&eval, &dot, context);

                if (eval.status != CARBON_PATH_RESOLVED) {
                        result = false;
                } else {
                        switch (eval.result.container_type) {
                        case CARBON_ARRAY: {
                                struct carbon_array_it *it = &eval.result.containers.array.it;
                                result = carbon_array_it_remove(it);
                        } break;
                        case CARBON_COLUMN:  {
                                struct carbon_column_it *it = &eval.result.containers.column.it;
                                u32 elem_pos = eval.result.containers.column.elem_pos;
                                result = carbon_column_it_remove(it, elem_pos);
                        } break;
                        default:
                                error(&context->original->err, ARK_ERR_INTERNALERR);
                                result = false;
                        }
                }
                carbon_path_evaluator_end(&eval);
                return result;
        } else {
                error(&context->original->err, ARK_ERR_DOT_PATH_PARSERR);
                return false;
        }
}

ARK_EXPORT(bool) carbon_revise_pack(struct carbon_revise *context)
{
        error_if_null(context);
        struct carbon_array_it it;
        carbon_revise_iterator_open(&it, context);
        internal_pack_array(&it);
        carbon_revise_iterator_close(&it);
        return true;
}

ARK_EXPORT(bool) carbon_revise_shrink(struct carbon_revise *context)
{
        struct carbon_array_it it;
        carbon_revise_iterator_open(&it, context);
        carbon_array_it_fast_forward(&it);
        if (memfile_remain_size(&it.memfile) > 0) {
                offset_t first_empty_slot = memfile_tell(&it.memfile);
                assert(memfile_size(&it.memfile) > first_empty_slot);
                offset_t shrink_size = memfile_size(&it.memfile) - first_empty_slot;
                memfile_cut(&it.memfile, shrink_size);
        }

        offset_t size;
        memblock_size(&size, it.memfile.memblock);
        carbon_revise_iterator_close(&it);
        return true;
}

ARK_EXPORT(const struct carbon *) carbon_revise_end(struct carbon_revise *context)
{
        if (likely(context != NULL)) {
                internal_revision_inc(context->revised_doc);

                context->original->versioning.is_latest = false;
                context->original->versioning.revision_lock = false;

                spin_release(&context->original->versioning.write_lock);

                return context->revised_doc;
        } else {
                error_print(ARK_ERR_NULLPTR);
                return NULL;
        }
}

ARK_EXPORT(bool) carbon_revise_abort(struct carbon_revise *context)
{
        error_if_null(context)

        carbon_drop(context->revised_doc);
        context->original->versioning.is_latest = true;
        context->original->versioning.revision_lock = false;
        spin_release(&context->original->versioning.write_lock);

        return true;
}

static bool internal_pack_array(struct carbon_array_it *it)
{
        assert(it);

        /* shrink this array */
        {
                struct carbon_array_it this_array_it;
                bool is_empty_slot, is_array_end;

                carbon_array_it_copy(&this_array_it, it);
                carbon_int_array_skip_contents(&is_empty_slot, &is_array_end, &this_array_it);

                if (!is_array_end) {

                        error_if(!is_empty_slot, &it->err, ARK_ERR_CORRUPTED);
                        offset_t first_empty_slot_offset = memfile_tell(&this_array_it.memfile);
                        char final;
                        while ((final = *memfile_read(&this_array_it.memfile, sizeof(char))) == 0)
                        { }
                        assert(final == CARBON_MARKER_ARRAY_END);
                        offset_t last_empty_slot_offset = memfile_tell(&this_array_it.memfile) - sizeof(char);
                        memfile_seek(&this_array_it.memfile, first_empty_slot_offset);
                        assert(last_empty_slot_offset > first_empty_slot_offset);

                        memfile_inplace_remove(&this_array_it.memfile, last_empty_slot_offset - first_empty_slot_offset);

                        final = *memfile_read(&this_array_it.memfile, sizeof(char));
                        assert(final == CARBON_MARKER_ARRAY_END);
                }

                carbon_array_it_drop(&this_array_it);
        }

        /* shrink contained containers */
        {
                while (carbon_array_it_next(it)) {
                        enum carbon_field_type type;
                        carbon_array_it_field_type(&type, it);
                        switch (type) {
                        case CARBON_FIELD_TYPE_NULL:
                        case CARBON_FIELD_TYPE_TRUE:
                        case CARBON_FIELD_TYPE_FALSE:
                        case CARBON_FIELD_TYPE_STRING:
                        case CARBON_FIELD_TYPE_NUMBER_U8:
                        case CARBON_FIELD_TYPE_NUMBER_U16:
                        case CARBON_FIELD_TYPE_NUMBER_U32:
                        case CARBON_FIELD_TYPE_NUMBER_U64:
                        case CARBON_FIELD_TYPE_NUMBER_I8:
                        case CARBON_FIELD_TYPE_NUMBER_I16:
                        case CARBON_FIELD_TYPE_NUMBER_I32:
                        case CARBON_FIELD_TYPE_NUMBER_I64:
                        case CARBON_FIELD_TYPE_NUMBER_FLOAT:
                        case CARBON_FIELD_TYPE_BINARY:
                        case CARBON_FIELD_TYPE_BINARY_CUSTOM:
                                /* nothing to shrink, because there are no padded zeros here */
                                break;
                        case CARBON_FIELD_TYPE_ARRAY: {
                                struct carbon_array_it nested_array_it;
                                carbon_array_it_create(&nested_array_it, &it->memfile, &it->err,
                                        it->field_access.nested_array_it->payload_start - sizeof(u8));
                                internal_pack_array(&nested_array_it);
                                assert(*memfile_peek(&nested_array_it.memfile, sizeof(char)) == CARBON_MARKER_ARRAY_END);
                                memfile_skip(&nested_array_it.memfile, sizeof(char));
                                memfile_seek(&it->memfile, memfile_tell(&nested_array_it.memfile));
                                carbon_array_it_drop(&nested_array_it);
                        } break;
                        case CARBON_FIELD_TYPE_COLUMN_U8:
                        case CARBON_FIELD_TYPE_COLUMN_U16:
                        case CARBON_FIELD_TYPE_COLUMN_U32:
                        case CARBON_FIELD_TYPE_COLUMN_U64:
                        case CARBON_FIELD_TYPE_COLUMN_I8:
                        case CARBON_FIELD_TYPE_COLUMN_I16:
                        case CARBON_FIELD_TYPE_COLUMN_I32:
                        case CARBON_FIELD_TYPE_COLUMN_I64:
                        case CARBON_FIELD_TYPE_COLUMN_FLOAT:
                        case CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                                carbon_column_it_rewind(it->field_access.nested_column_it);
                                internal_pack_column(it->field_access.nested_column_it);
                                memfile_seek(&it->memfile, memfile_tell(&it->field_access.nested_column_it->memfile));
                                break;
                        case CARBON_FIELD_TYPE_OBJECT: {
                                struct carbon_object_it nested_object_it;
                                carbon_object_it_create(&nested_object_it, &it->memfile, &it->err,
                                        it->field_access.nested_object_it->payload_start - sizeof(u8));
                                internal_pack_object(&nested_object_it);
                                assert(*memfile_peek(&nested_object_it.memfile, sizeof(char)) == CARBON_MARKER_OBJECT_END);
                                memfile_skip(&nested_object_it.memfile, sizeof(char));
                                memfile_seek(&it->memfile, memfile_tell(&nested_object_it.memfile));
                                carbon_object_it_drop(&nested_object_it);
                        } break;
                        default:
                        error(&it->err, ARK_ERR_INTERNALERR);
                                return false;
                        }
                }
        }

        assert(*memfile_peek(&it->memfile, sizeof(char)) == CARBON_MARKER_ARRAY_END);

        return true;
}

static bool internal_pack_object(struct carbon_object_it *it)
{
        assert(it);

        /* shrink this object */
        {
                struct carbon_object_it this_object_it;
                bool is_empty_slot, is_object_end;

                carbon_object_it_copy(&this_object_it, it);
                carbon_int_object_skip_contents(&is_empty_slot, &is_object_end, &this_object_it);

                if (!is_object_end) {

                        error_if(!is_empty_slot, &it->err, ARK_ERR_CORRUPTED);
                        offset_t first_empty_slot_offset = memfile_tell(&this_object_it.memfile);
                        char final;
                        while ((final = *memfile_read(&this_object_it.memfile, sizeof(char))) == 0)
                        { }
                        assert(final == CARBON_MARKER_OBJECT_END);
                        offset_t last_empty_slot_offset = memfile_tell(&this_object_it.memfile) - sizeof(char);
                        memfile_seek(&this_object_it.memfile, first_empty_slot_offset);
                        assert(last_empty_slot_offset > first_empty_slot_offset);

                        memfile_inplace_remove(&this_object_it.memfile,
                                last_empty_slot_offset - first_empty_slot_offset);

                        final = *memfile_read(&this_object_it.memfile, sizeof(char));
                        assert(final == CARBON_MARKER_OBJECT_END);
                }

                carbon_object_it_drop(&this_object_it);
        }

        /* shrink contained containers */
        {
                while (carbon_object_it_next(it)) {
                        enum carbon_field_type type;
                        carbon_object_it_prop_type(&type, it);
                        switch (type) {
                        case CARBON_FIELD_TYPE_NULL:
                        case CARBON_FIELD_TYPE_TRUE:
                        case CARBON_FIELD_TYPE_FALSE:
                        case CARBON_FIELD_TYPE_STRING:
                        case CARBON_FIELD_TYPE_NUMBER_U8:
                        case CARBON_FIELD_TYPE_NUMBER_U16:
                        case CARBON_FIELD_TYPE_NUMBER_U32:
                        case CARBON_FIELD_TYPE_NUMBER_U64:
                        case CARBON_FIELD_TYPE_NUMBER_I8:
                        case CARBON_FIELD_TYPE_NUMBER_I16:
                        case CARBON_FIELD_TYPE_NUMBER_I32:
                        case CARBON_FIELD_TYPE_NUMBER_I64:
                        case CARBON_FIELD_TYPE_NUMBER_FLOAT:
                        case CARBON_FIELD_TYPE_BINARY:
                        case CARBON_FIELD_TYPE_BINARY_CUSTOM:
                                /* nothing to shrink, because there are no padded zeros here */
                                break;
                        case CARBON_FIELD_TYPE_ARRAY: {
                                struct carbon_array_it nested_array_it;
                                carbon_array_it_create(&nested_array_it, &it->memfile, &it->err,
                                        it->field_access.nested_array_it->payload_start - sizeof(u8));
                                internal_pack_array(&nested_array_it);
                                assert(*memfile_peek(&nested_array_it.memfile, sizeof(char)) == CARBON_MARKER_ARRAY_END);
                                memfile_skip(&nested_array_it.memfile, sizeof(char));
                                memfile_seek(&it->memfile, memfile_tell(&nested_array_it.memfile));
                                carbon_array_it_drop(&nested_array_it);
                        } break;
                        case CARBON_FIELD_TYPE_COLUMN_U8:
                        case CARBON_FIELD_TYPE_COLUMN_U16:
                        case CARBON_FIELD_TYPE_COLUMN_U32:
                        case CARBON_FIELD_TYPE_COLUMN_U64:
                        case CARBON_FIELD_TYPE_COLUMN_I8:
                        case CARBON_FIELD_TYPE_COLUMN_I16:
                        case CARBON_FIELD_TYPE_COLUMN_I32:
                        case CARBON_FIELD_TYPE_COLUMN_I64:
                        case CARBON_FIELD_TYPE_COLUMN_FLOAT:
                        case CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                                carbon_column_it_rewind(it->field_access.nested_column_it);
                                internal_pack_column(it->field_access.nested_column_it);
                                memfile_seek(&it->memfile, memfile_tell(&it->field_access.nested_column_it->memfile));
                                memfile_hexdump_printf(stderr, &it->memfile); // TODO: Debug remove
                                break;
                        case CARBON_FIELD_TYPE_OBJECT: {
                                struct carbon_object_it nested_object_it;
                                carbon_object_it_create(&nested_object_it, &it->memfile, &it->err,
                                        it->field_access.nested_object_it->payload_start - sizeof(u8));
                                internal_pack_object(&nested_object_it);
                                assert(*memfile_peek(&nested_object_it.memfile, sizeof(char)) == CARBON_MARKER_OBJECT_END);
                                memfile_skip(&nested_object_it.memfile, sizeof(char));
                                memfile_seek(&it->memfile, memfile_tell(&nested_object_it.memfile));
                                carbon_object_it_drop(&nested_object_it);
                        } break;
                        default:
                        error(&it->err, ARK_ERR_INTERNALERR);
                                return false;
                        }
                }
        }

        assert(*memfile_peek(&it->memfile, sizeof(char)) == CARBON_MARKER_OBJECT_END);

        return true;
}

static bool internal_pack_column(struct carbon_column_it *it)
{
        assert(it);

        u32 free_space = (it->column_capacity - it->column_num_elements) * carbon_int_get_type_value_size(it->type);
        if (free_space > 0) {
                offset_t payload_start = carbon_int_column_get_payload_off(it);
                u64 payload_size = it->column_num_elements * carbon_int_get_type_value_size(it->type);

                memfile_seek(&it->memfile, payload_start);
                memfile_skip(&it->memfile, payload_size);

                memfile_inplace_remove(&it->memfile, free_space);

                memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
                memfile_skip_varuint(&it->memfile); // skip num of elements counter
                memfile_update_varuint(&it->memfile, it->column_num_elements); // update capacity counter to num elems

                memfile_skip(&it->memfile, payload_size);

                return true;
        } else {
                return false;
        }
}

static bool internal_revision_inc(struct carbon *doc)
{
        assert(doc);
        return carbon_header_rev_inc(doc);
}

static bool carbon_header_rev_inc(struct carbon *doc)
{
        assert(doc);

        enum carbon_primary_key_type key_type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        carbon_key_read(NULL, &key_type, &doc->memfile);
        if (carbon_has_key(key_type)) {
                carbon_revision_inc(&doc->memfile);
        }
        memfile_restore_position(&doc->memfile);

        return true;
}