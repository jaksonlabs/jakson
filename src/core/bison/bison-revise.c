/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#include "core/bison/bison.h"
#include "core/bison/bison-revise.h"
#include "core/bison/bison-array-it.h"
#include "core/bison/bison-int.h"
#include "core/bison/bison-dot.h"
#include "core/bison/bison-find.h"

static u64 bison_header_set_oid(struct bison *doc, object_id_t oid);

static bool internal_pack_array(struct bison_array_it *it);
static bool internal_pack_column(struct bison_column_it *it);
static bool internal_revision_inc(struct bison *doc);
static bool bison_header_rev_inc(struct bison *doc);

// ---------------------------------------------------------------------------------------------------------------------

#define event_promote(doc, event, ...)                                                                                 \
for (u32 i = 0; i < doc->handler.num_elems; i++) {                                                                     \
        struct bison_handler *handler = vec_get(&doc->handler, i, struct bison_handler);                               \
        assert(handler);                                                                                               \
        if (handler->in_use && handler->listener.event) {                                                              \
        handler->listener.event(&handler->listener, __VA_ARGS__);                                                      \
        }                                                                                                              \
}

#define fire_revision_begin(doc)                      event_promote(doc, on_revision_begin, doc);
#define fire_revision_end(doc)                        event_promote(doc, on_revision_end, doc);
#define fire_revision_abort(doc)                      event_promote(doc, on_revision_abort, doc);
#define fire_new_revision(revised, original)          event_promote(original, on_new_revision, revised, original);

// ---------------------------------------------------------------------------------------------------------------------

NG5_EXPORT(bool) bison_revise_try_begin(struct bison_revise *context, struct bison *revised_doc, struct bison *doc)
{
        error_if_null(context)
        error_if_null(doc)
        if (!doc->versioning.revision_lock) {
                return bison_revise_begin(context, revised_doc, doc);
        } else {
                return false;
        }
}

NG5_EXPORT(bool) bison_revise_begin(struct bison_revise *context, struct bison *revised_doc, struct bison *original)
{
        error_if_null(context)
        error_if_null(original)

        if (likely(original->versioning.is_latest)) {
                spin_acquire(&original->versioning.write_lock);
                original->versioning.revision_lock = true;
                context->original = original;
                context->revised_doc = revised_doc;
                error_init(&context->err);
                bison_clone(context->revised_doc, context->original);
                fire_revision_begin(original);
                return true;
        } else {
                error(&original->err, NG5_ERR_OUTDATED)
                return false;
        }
}

NG5_EXPORT(bool) bison_revise_gen_object_id(object_id_t *out, struct bison_revise *context)
{
        error_if_null(context);
        object_id_t oid;
        object_id_create(&oid);
        bison_header_set_oid(context->revised_doc, oid);
        ng5_optional_set(out, oid);
        return true;
}

NG5_EXPORT(bool) bison_revise_iterator_open(struct bison_array_it *it, struct bison_revise *context)
{
        error_if_null(it);
        error_if_null(context);
        offset_t payload_start = bison_int_payload_after_header(context->revised_doc);
        error_if(context->revised_doc->memfile.mode != READ_WRITE, &context->original->err, NG5_ERR_INTERNALERR)
        return bison_array_it_create(it, &context->revised_doc->memfile, &context->original->err, payload_start);
}

NG5_EXPORT(bool) bison_revise_iterator_close(struct bison_array_it *it)
{
        error_if_null(it);
        return bison_array_it_drop(it);
}

NG5_EXPORT(bool) bison_revise_find_open(struct bison_find *out, const char *dot_path, struct bison_revise *context)
{
        error_if_null(out)
        error_if_null(dot_path)
        error_if_null(context)
        struct bison_dot_path path;
        bison_dot_path_from_string(&path, dot_path);
        bool status = bison_find_create(out, &path, context->revised_doc);
        bison_dot_path_drop(&path);
        return status;
}

NG5_EXPORT(bool) bison_revise_find_close(struct bison_find *find)
{
        error_if_null(find)
        return bison_find_drop(find);
}

NG5_EXPORT(bool) bison_revise_remove_one(const char *dot_path, struct bison *rev_doc, struct bison *doc)
{
        struct bison_revise revise;
        bison_revise_begin(&revise, rev_doc, doc);
        bool status = bison_revise_remove(dot_path, &revise);
        bison_revise_end(&revise);
        return status;
}

NG5_EXPORT(bool) bison_revise_remove(const char *dot_path, struct bison_revise *context)
{
        error_if_null(dot_path)
        error_if_null(context)

        struct bison_dot_path dot;
        struct bison_path_evaluator eval;
        bool result;

        if (bison_dot_path_from_string(&dot, dot_path)) {
                bison_path_evaluator_begin_mutable(&eval, &dot, context);

                if (eval.status != BISON_PATH_RESOLVED) {
                        result = false;
                } else {
                        switch (eval.result.container_type) {
                        case BISON_ARRAY: {
                                struct bison_array_it *it = eval.result.containers.array.it;
                                result = bison_array_it_remove(it);
                        } break;
                        case BISON_COLUMN:  {
                                struct bison_column_it *it = eval.result.containers.column.it;
                                u32 elem_pos = eval.result.containers.column.elem_pos;
                                result = bison_column_it_remove(it, elem_pos);
                        } break;
                        default:
                                error(&context->original->err, NG5_ERR_INTERNALERR);
                                result = false;
                        }
                }
                bison_path_evaluator_end(&eval);
                return result;
        } else {
                error(&context->original->err, NG5_ERR_DOT_PATH_PARSERR);
                return false;
        }
}

NG5_EXPORT(bool) bison_revise_pack(struct bison_revise *context)
{
        error_if_null(context);
        struct bison_array_it it;
        bison_revise_iterator_open(&it, context);
        internal_pack_array(&it);
        bison_revise_iterator_close(&it);
        return true;
}

NG5_EXPORT(bool) bison_revise_shrink(struct bison_revise *context)
{
        struct bison_array_it it;
        bison_revise_iterator_open(&it, context);
        bison_array_it_fast_forward(&it);
        if (memfile_remain_size(&it.memfile) > 0) {
                offset_t first_empty_slot = memfile_tell(&it.memfile);
                assert(memfile_size(&it.memfile) > first_empty_slot);
                offset_t shrink_size = memfile_size(&it.memfile) - first_empty_slot;
                memfile_cut(&it.memfile, shrink_size);
        }

        offset_t size;
        memblock_size(&size, it.memfile.memblock);
        hexdump_print(stdout, memblock_raw_data(it.memfile.memblock), size);

        bison_revise_iterator_close(&it);
        return true;
}

NG5_EXPORT(const struct bison *) bison_revise_end(struct bison_revise *context)
{
        if (likely(context != NULL)) {
                internal_revision_inc(context->revised_doc);

                context->original->versioning.is_latest = false;
                context->original->versioning.revision_lock = false;

                fire_new_revision(context->revised_doc, context->original);
                fire_revision_end(context->original);

                spin_release(&context->original->versioning.write_lock);

                return context->revised_doc;
        } else {
                error_print(NG5_ERR_NULLPTR);
                return NULL;
        }
}

NG5_EXPORT(bool) bison_revise_abort(struct bison_revise *context)
{
        error_if_null(context)

        bison_drop(context->revised_doc);
        context->original->versioning.is_latest = true;
        context->original->versioning.revision_lock = false;
        fire_revision_abort(context->original);
        fire_revision_end(context->original);
        spin_release(&context->original->versioning.write_lock);

        return true;
}

static u64 bison_header_set_oid(struct bison *doc, object_id_t oid)
{
        assert(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        struct bison_header *header = NG5_MEMFILE_READ_TYPE(&doc->memfile, struct bison_header);
        header->oid = oid;
        memfile_seek(&doc->memfile, 0);
        memfile_write(&doc->memfile, header, sizeof(struct bison_header));
        memfile_restore_position(&doc->memfile);
        return header->oid;
}

static bool internal_pack_array(struct bison_array_it *it)
{
        assert(it);

        /* shrink this array */
        {
                struct bison_array_it this_array_it;
                bool is_empty_slot, is_array_end;

                bison_array_it_copy(&this_array_it, it);
                bison_int_array_skip_contents(&is_empty_slot, &is_array_end, &this_array_it);

                if (!is_array_end) {

                        error_if(!is_empty_slot, &it->err, NG5_ERR_CORRUPTED);
                        offset_t first_empty_slot_offset = memfile_tell(&this_array_it.memfile);
                        char final;
                        while ((final = *memfile_read(&this_array_it.memfile, sizeof(char))) == 0)
                        { }
                        assert(final == BISON_MARKER_ARRAY_END);
                        offset_t last_empty_slot_offset = memfile_tell(&this_array_it.memfile) - sizeof(char);
                        memfile_seek(&this_array_it.memfile, first_empty_slot_offset);
                        assert(last_empty_slot_offset > first_empty_slot_offset);

                        memfile_move_left(&this_array_it.memfile, last_empty_slot_offset - first_empty_slot_offset);

                        final = *memfile_read(&this_array_it.memfile, sizeof(char));
                        assert(final == BISON_MARKER_ARRAY_END);
                }

                bison_array_it_drop(&this_array_it);
        }

        /* shrink contained containers */
        {
                while (bison_array_it_next(it)) {
                        enum bison_field_type type;
                        bison_array_it_field_type(&type, it);
                        switch (type) {
                        case BISON_FIELD_TYPE_NULL:
                        case BISON_FIELD_TYPE_TRUE:
                        case BISON_FIELD_TYPE_FALSE:
                        case BISON_FIELD_TYPE_STRING:
                        case BISON_FIELD_TYPE_NUMBER_U8:
                        case BISON_FIELD_TYPE_NUMBER_U16:
                        case BISON_FIELD_TYPE_NUMBER_U32:
                        case BISON_FIELD_TYPE_NUMBER_U64:
                        case BISON_FIELD_TYPE_NUMBER_I8:
                        case BISON_FIELD_TYPE_NUMBER_I16:
                        case BISON_FIELD_TYPE_NUMBER_I32:
                        case BISON_FIELD_TYPE_NUMBER_I64:
                        case BISON_FIELD_TYPE_NUMBER_FLOAT:
                        case BISON_FIELD_TYPE_BINARY:
                        case BISON_FIELD_TYPE_BINARY_CUSTOM:
                                /* nothing to shrink, because there are no padded zeros here */
                                break;
                        case BISON_FIELD_TYPE_ARRAY: {
                                struct bison_array_it nested_array_it;
                                bison_array_it_create(&nested_array_it, &it->memfile, &it->err,
                                        it->nested_array_it->payload_start - sizeof(u8));
                                internal_pack_array(&nested_array_it);
                                char last = *memfile_peek(&nested_array_it.memfile, sizeof(char));
                                assert(last == BISON_MARKER_ARRAY_END);
                                memfile_skip(&nested_array_it.memfile, sizeof(char));
                                memfile_seek(&it->memfile, memfile_tell(&nested_array_it.memfile));
                                bison_array_it_drop(&nested_array_it);
                        } break;
                        case BISON_FIELD_TYPE_COLUMN:
                                bison_column_it_rewind(it->nested_column_it);
                                internal_pack_column(it->nested_column_it);
                                memfile_seek(&it->memfile, memfile_tell(&it->nested_column_it->memfile));
                                break;
                        case BISON_FIELD_TYPE_OBJECT:
                        error(&it->err, NG5_ERR_NOTIMPLEMENTED);
                                return false;
                        default:
                        error(&it->err, NG5_ERR_INTERNALERR);
                                return false;
                        }
                }
        }

        char last = *memfile_peek(&it->memfile, sizeof(char));
        assert(last == BISON_MARKER_ARRAY_END);

        return true;
}

static bool internal_pack_column(struct bison_column_it *it)
{
        assert(it);

        u32 free_space = (it->column_capacity - it->column_num_elements) * bison_int_get_type_value_size(it->type);
        if (free_space > 0) {
                offset_t payload_start = bison_int_column_get_payload_off(it);

                memfile_seek(&it->memfile, payload_start);
                memfile_skip(&it->memfile, it->column_num_elements * bison_int_get_type_value_size(it->type));

                memfile_move_left(&it->memfile, free_space);

                offset_t continue_off = memfile_tell(&it->memfile);

                memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
                memfile_skip_varuint(&it->memfile); // skip num of elements counter
                memfile_update_varuint(&it->memfile, it->column_num_elements); // update capacity counter to num elems

                memfile_seek(&it->memfile, continue_off);

                return true;
        } else {
                return false;
        }
}

static bool internal_revision_inc(struct bison *doc)
{
        assert(doc);
        return bison_header_rev_inc(doc);
}

static bool bison_header_rev_inc(struct bison *doc)
{
        assert(doc);
        u64 rev = bison_int_header_get_rev(doc);
        u64 new_rev = rev + 1;

        u8 nblocks_rev = varuint_required_blocks(rev);
        u8 nblocks_new_rev = varuint_required_blocks(new_rev);

        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        memfile_skip(&doc->memfile, sizeof(struct bison_header));
        error_if(memfile_remain_size(&doc->memfile) < varuint_max_blocks(), &doc->err, NG5_ERR_CORRUPTED);
        varuint_t revision = (varuint_t) memfile_peek(&doc->memfile, sizeof(char));

        assert (nblocks_rev <= nblocks_new_rev);
        assert (nblocks_rev == nblocks_new_rev ||
                (nblocks_new_rev - nblocks_rev) == 1 );

        if (nblocks_new_rev > nblocks_rev) {
                /* variable-length revision number requires more space; move remainders */
                memfile_move_right(&doc->memfile, (nblocks_new_rev - nblocks_rev));
        }

        varuint_write(revision, new_rev);
        memfile_restore_position(&doc->memfile);
        return true;
}