/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements the document format itself
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

#include <inttypes.h>

#include "core/bison/bison.h"
#include "core/bison/bison_array_it.h"
#include "core/bison/bison_printers.h"
#include "stdx/varuint.h"
#include "utils/hexdump.h"

struct bison_header
{
        object_id_t oid;
};

struct bison_obj_header
{
        varuint_t obj_len;
};

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

static bool printer_drop(struct bison_printer *printer);
static bool printer_bison_begin(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_end(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_header_begin(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_header_contents(struct bison_printer *printer, struct string_builder *builder, object_id_t oid, u64 rev);
static bool printer_bison_header_end(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_payload_begin(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_payload_end(struct bison_printer *printer, struct string_builder *builder);

static bool internal_drop(struct bison *doc);
static bool internal_revision_inc(struct bison *doc);
static offset_t internal_payload_after_header(struct bison *doc);

static void bison_header_init(struct bison *doc);
static u64 bison_header_get_rev(struct bison *doc);
static bool bison_header_rev_inc(struct bison *doc);
static u64 bison_header_get_oid(struct bison *doc);
static u64 bison_header_set_oid(struct bison *doc, object_id_t oid);


// ---------------------------------------------------------------------------------------------------------------------

NG5_EXPORT(bool) bison_create(struct bison *doc)
{
        error_if_null(doc);
        error_init(&doc->err);
        memblock_create(&doc->memblock, 1024);
        memblock_zero_out(doc->memblock);
        memfile_open(&doc->memfile, doc->memblock, READ_WRITE);
        vec_create(&doc->handler, NULL, sizeof(struct bison_handler), 1);

        spin_init(&doc->versioning.write_lock);

        doc->versioning.revision_lock = false;
        doc->versioning.is_latest = true;

        bison_header_init(doc);

        return true;
}

NG5_EXPORT(bool) bison_drop(struct bison *doc)
{
        error_if_null(doc);
        return internal_drop(doc);
}

NG5_EXPORT(bool) bison_is_up_to_date(struct bison *doc)
{
        error_if_null(doc);
        return doc->versioning.is_latest;
}

NG5_EXPORT(bool) bison_register_listener(listener_handle_t *handle, struct bison_event_listener *listener, struct bison *doc)
{
        error_if_null(listener);
        error_if_null(doc);

        u32 pos = 0;
        struct bison_handler *handler;

        for (u32 pos = 0; pos < doc->handler.num_elems; pos++) {
                handler = vec_get(&doc->handler, pos, struct bison_handler);
                if (!handler->in_use) {
                        break;
                }
        }

        if (pos == doc->handler.num_elems) {
                /* append new handler */
                handler = vec_new_and_get(&doc->handler, struct bison_handler);
        }

        handler->in_use = true;
        handler->listener = *listener;
        ng5_optional_call(listener, clone, &handler->listener, listener);
        ng5_optional_set(handle, pos);
        return true;
}

NG5_EXPORT(bool) bison_unregister_listener(struct bison *doc, listener_handle_t handle)
{
        error_if_null(doc);
        if (likely(handle < doc->handler.num_elems)) {
                struct bison_handler *handler = vec_get(&doc->handler, handle, struct bison_handler);
                if (likely(handler->in_use)) {
                        handler->in_use = false;
                        ng5_optional_call(&handler->listener, drop, &handler->listener);
                        return true;
                } else {
                        error(&doc->err, NG5_ERR_NOTFOUND);
                        return false;
                }
        } else {
                error(&doc->err, NG5_ERR_OUTOFBOUNDS);
                return false;
        }
}

NG5_EXPORT(bool) bison_clone(struct bison *clone, struct bison *doc)
{
        error_if_null(clone);
        error_if_null(doc);
        ng5_check_success(memblock_cpy(&clone->memblock, doc->memblock));
        ng5_check_success(memfile_open(&clone->memfile, clone->memblock, READ_WRITE));
        ng5_check_success(error_init(&clone->err));

        vec_create(&clone->handler, NULL, sizeof(struct bison_handler), doc->handler.num_elems);
        for (u32 i = 0; i < doc->handler.num_elems; i++) {
                struct bison_handler *original = vec_get(&doc->handler, i, struct bison_handler);
                struct bison_handler *copy = vec_new_and_get(&clone->handler, struct bison_handler);
                copy->in_use = original->in_use;
                if (original->in_use) {
                        copy->listener = original->listener;
                        ng5_optional_call(&original->listener, clone, &copy->listener, &original->listener);
                }
        }

        spin_init(&clone->versioning.write_lock);
        clone->versioning.revision_lock = false;
        clone->versioning.is_latest = true;

        return true;
}

NG5_EXPORT(bool) bison_revision(u64 *rev, struct bison *doc)
{
        error_if_null(doc);
        *rev = bison_header_get_rev(doc);
        return true;
}

NG5_EXPORT(bool) bison_object_id(object_id_t *oid, struct bison *doc)
{
        error_if_null(doc);
        error_if_null(oid);
        *oid = bison_header_get_oid(doc);
        return true;
}

NG5_EXPORT(bool) bison_to_str(struct string_builder *dst, enum bison_printer_impl printer, struct bison *doc)
{
        error_if_null(doc);

        struct bison_printer p;
        struct string_builder b;
        object_id_t oid;
        u64 rev;

        memfile_save_position(&doc->memfile);

        ng5_zero_memory(&p, sizeof(struct bison_printer));
        string_builder_create(&b);
        bison_object_id(&oid, doc);
        bison_revision(&rev, doc);

        switch (printer) {
        case JSON_FORMATTER:
                bison_json_formatter_create(&p);
                break;
        default:
                error(&doc->err, NG5_ERR_NOTFOUND);
        }

        printer_bison_begin(&p, &b);
        printer_bison_header_begin(&p, &b);
        printer_bison_header_contents(&p, &b, oid, rev);
        printer_bison_header_end(&p, &b);
        printer_bison_payload_begin(&p, &b);
        printer_bison_payload_end(&p, &b);
        printer_bison_end(&p, &b);

        printer_drop(&p);
        string_builder_append(dst, string_builder_cstr(&b));
        string_builder_drop(&b);

        memfile_restore_position(&doc->memfile);
        return true;
}

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
                bison_clone(context->revised_doc, context->original);
                fire_revision_begin(original);
                return true;
        } else {
                error(&original->err, NG5_ERR_OUTOFBOUNDS)
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

NG5_EXPORT(bool) bison_revise_access(struct bison_array_it *it, struct bison_revise *context)
{
        error_if_null(it);
        error_if_null(context);
        offset_t payload_start = internal_payload_after_header(context->revised_doc);
        return bison_array_it_create(it, context->revised_doc, payload_start);
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

NG5_EXPORT(const char *) bison_field_type_str(struct err *err, enum bison_field_type type)
{
        switch (type) {
        case BISON_FIELD_TYPE_NULL: return BISON_FIELD_TYPE_NULL_STR;
        case BISON_FIELD_TYPE_TRUE: return BISON_FIELD_TYPE_TRUE_STR;
        case BISON_FIELD_TYPE_FALSE: return BISON_FIELD_TYPE_FALSE_STR;
        case BISON_FIELD_TYPE_OBJECT: return BISON_FIELD_TYPE_OBJECT_STR;
        case BISON_FIELD_TYPE_ARRAY: return BISON_FIELD_TYPE_ARRAY_STR;
        case BISON_FIELD_TYPE_STRING: return BISON_FIELD_TYPE_STRING_STR;
        case BISON_FIELD_TYPE_NUMBER_U8: return BISON_FIELD_TYPE_NUMBER_U8_STR;
        case BISON_FIELD_TYPE_NUMBER_U16: return BISON_FIELD_TYPE_NUMBER_U16_STR;
        case BISON_FIELD_TYPE_NUMBER_U32: return BISON_FIELD_TYPE_NUMBER_U32_STR;
        case BISON_FIELD_TYPE_NUMBER_U64: return BISON_FIELD_TYPE_NUMBER_U64_STR;
        case BISON_FIELD_TYPE_NUMBER_I8: return BISON_FIELD_TYPE_NUMBER_I8_STR;
        case BISON_FIELD_TYPE_NUMBER_I16: return BISON_FIELD_TYPE_NUMBER_I16_STR;
        case BISON_FIELD_TYPE_NUMBER_I32: return BISON_FIELD_TYPE_NUMBER_I32_STR;
        case BISON_FIELD_TYPE_NUMBER_I64: return BISON_FIELD_TYPE_NUMBER_I64_STR;
        case BISON_FIELD_TYPE_NUMBER_FLOAT: return BISON_FIELD_TYPE_NUMBER_FLOAT_STR;
        case BISON_FIELD_TYPE_NUMBER_U8_COLUMN: return BISON_FIELD_TYPE_NUMBER_U8_COLUMN_STR;
        case BISON_FIELD_TYPE_NUMBER_U16_COLUMN: return BISON_FIELD_TYPE_NUMBER_U16_COLUMN_STR;
        case BISON_FIELD_TYPE_NUMBER_U32_COLUMN: return BISON_FIELD_TYPE_NUMBER_U32_COLUMN_STR;
        case BISON_FIELD_TYPE_NUMBER_U64_COLUMN: return BISON_FIELD_TYPE_NUMBER_U64_COLUMN_STR;
        case BISON_FIELD_TYPE_NUMBER_I8_COLUMN: return BISON_FIELD_TYPE_NUMBER_I8_COLUMN_STR;
        case BISON_FIELD_TYPE_NUMBER_I16_COLUMN: return BISON_FIELD_TYPE_NUMBER_I16_COLUMN_STR;
        case BISON_FIELD_TYPE_NUMBER_I32_COLUMN: return BISON_FIELD_TYPE_NUMBER_I32_COLUMN_STR;
        case BISON_FIELD_TYPE_NUMBER_I64_COLUMN: return BISON_FIELD_TYPE_NUMBER_I64_COLUMN_STR;
        case BISON_FIELD_TYPE_NUMBER_FLOAT_COLUMN: return BISON_FIELD_TYPE_NUMBER_FLOAT_COLUMN_STR;
        case BISON_FIELD_TYPE_NUMBER_NCHAR_COLUMN: return BISON_FIELD_TYPE_NUMBER_NCHAR_COLUMN_STR;
        case BISON_FIELD_TYPE_NUMBER_BINARY: return BISON_FIELD_TYPE_NUMBER_BINARY_STR;
        case BISON_FIELD_TYPE_NUMBER_NBINARY_COLUMN: return BISON_FIELD_TYPE_NUMBER_NBINARY_COLUMN_STR;
        default:
                error(err, NG5_ERR_NOTFOUND);
                return NULL;
        }
}

NG5_EXPORT(bool) bison_print(FILE *file, struct bison *doc)
{
        error_if_null(file);
        error_if_null(doc);

        struct string_builder builder;
        string_builder_create(&builder);
        bison_to_str(&builder, JSON_FORMATTER, doc);
        printf("%s\n", string_builder_cstr(&builder));
        string_builder_drop(&builder);

        return true;
}

NG5_EXPORT(bool) bison_hexdump_print(FILE *file, struct bison *doc)
{
        error_if_null(file);
        error_if_null(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        bool status = hexdump_print(file, memfile_peek(&doc->memfile, 1), memfile_size(&doc->memfile));
        memfile_restore_position(&doc->memfile);
        return status;
}

// ---------------------------------------------------------------------------------------------------------------------

static bool internal_drop(struct bison *doc)
{
        assert(doc);
        memblock_drop(doc->memblock);
        for (u32 i = 0; i < doc->handler.num_elems; i++) {
                struct bison_handler *handler = vec_get(&doc->handler, i, struct bison_handler);
                if (handler->in_use) {
                        ng5_optional_call(&handler->listener, drop, &handler->listener);
                }

        }
        vec_drop(&doc->handler);
        return true;
}

static bool internal_revision_inc(struct bison *doc)
{
        assert(doc);
        return bison_header_rev_inc(doc);
}

static offset_t internal_payload_after_header(struct bison *doc)
{
        u8 rev_nbytes;
        offset_t result;

        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        memfile_skip(&doc->memfile, sizeof(struct bison_header));
        varuint_t revision = NG5_MEMFILE_PEEK(&doc->memfile, varuint_t);
        varuint_read(&rev_nbytes, revision);
        memfile_skip(&doc->memfile, rev_nbytes);
        result = memfile_tell(&doc->memfile);
        memfile_restore_position(&doc->memfile);
        return result;
}

static void bison_header_init(struct bison *doc)
{
        assert(doc);

        struct bison_header header = {
                .oid = 0
        };

        memfile_seek(&doc->memfile, 0);
        memfile_write(&doc->memfile, &header, sizeof(struct bison_header));

        varuint_t revision = NG5_MEMFILE_PEEK(&doc->memfile, varuint_t);

        /* in case not enough space is available for writing revision (variable-length) number, enlarge */
        size_t remain = memfile_remain_size(&doc->memfile);
        offset_t rev_off = memfile_tell(&doc->memfile);
        if (unlikely(remain < varuint_max_blocks())) {
                memfile_write_zero(&doc->memfile, varuint_max_blocks());
                memfile_seek(&doc->memfile, rev_off);
        }

        u8 bytes_written = varuint_write(revision, 0);
        memfile_skip(&doc->memfile, bytes_written);
        assert (bison_header_get_rev(doc) == 0);
}

static u64 bison_header_get_rev(struct bison *doc)
{
        assert(doc);
        u64 rev;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        memfile_skip(&doc->memfile, sizeof(struct bison_header));
        error_if(memfile_remain_size(&doc->memfile) < varuint_max_blocks(), &doc->err, NG5_ERR_CORRUPTED);
        varuint_t revision = NG5_MEMFILE_PEEK(&doc->memfile, varuint_t);
        rev = varuint_read(NULL, revision);
        memfile_restore_position(&doc->memfile);
        return rev;
}

static bool bison_header_rev_inc(struct bison *doc)
{
        assert(doc);
        u64 rev = bison_header_get_rev(doc);
        u64 new_rev = rev + 1;

        u8 nblocks_rev = varuint_required_blocks(rev);
        u8 nblocks_new_rev = varuint_required_blocks(new_rev);

        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        memfile_skip(&doc->memfile, sizeof(struct bison_header));
        error_if(memfile_remain_size(&doc->memfile) < varuint_max_blocks(), &doc->err, NG5_ERR_CORRUPTED);
        varuint_t revision = NG5_MEMFILE_PEEK(&doc->memfile, varuint_t);

        assert (nblocks_rev <= nblocks_new_rev);
        assert (nblocks_rev == nblocks_new_rev ||
                (nblocks_new_rev - nblocks_rev) == 1 );

        if (nblocks_new_rev > nblocks_rev) {
                /* variable-length revision number requires more space; move remainders */
                memfile_move(&doc->memfile, (nblocks_new_rev - nblocks_rev));
        }

        varuint_write(revision, new_rev);
        memfile_restore_position(&doc->memfile);
        return true;
}

static u64 bison_header_get_oid(struct bison *doc)
{
        assert(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const struct bison_header *header = NG5_MEMFILE_READ_TYPE(&doc->memfile, struct bison_header);
        memfile_restore_position(&doc->memfile);
        return header->oid;
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

static bool printer_drop(struct bison_printer *printer)
{
        error_if_null(printer->drop);
        printer->drop(printer);
        return true;
}

static bool printer_bison_begin(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_begin);
        printer->print_bison_begin(printer, builder);
        return true;
}

static bool printer_bison_end(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_end);
        printer->print_bison_end(printer, builder);
        return true;
}

static bool printer_bison_header_begin(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_header_begin);
        printer->print_bison_header_begin(printer, builder);
        return true;
}

static bool printer_bison_header_contents(struct bison_printer *printer, struct string_builder *builder, object_id_t oid, u64 rev)
{
        error_if_null(printer->drop);
        printer->print_bison_header_contents(printer, builder, oid, rev);
        return true;
}

static bool printer_bison_header_end(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_header_end);
        printer->print_bison_header_end(printer, builder);
        return true;
}

static bool printer_bison_payload_begin(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_payload_begin);
        printer->print_bison_payload_begin(printer, builder);
        return true;
}

static bool printer_bison_payload_end(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_payload_end);
        printer->print_bison_payload_end(printer, builder);
        return true;
}