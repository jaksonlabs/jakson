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
#include "core/bison/bison-array-it.h"
#include "core/bison/bison-column-it.h"
#include "core/bison/bison-printers.h"
#include "core/bison/bison-int.h"
#include "stdx/varuint.h"
#include "utils/hexdump.h"

#define MIN_DOC_CAPACITY 16 /* minimum number of bytes required to store header and empty document array */

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
static bool printer_bison_array_begin(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_array_end(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_null(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_true(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_false(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_comma(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_signed(struct bison_printer *printer, struct string_builder *builder, i64 value);
static bool printer_bison_unsigned(struct bison_printer *printer, struct string_builder *builder, u64 value);
static bool printer_bison_float(struct bison_printer *printer, struct string_builder *builder, float value);
static bool printer_bison_string(struct bison_printer *printer, struct string_builder *builder, const char *value, u64 strlen);
static bool printer_bison_binary(struct bison_printer *printer, struct string_builder *builder, const struct bison_binary *binary);

static bool print_array(struct bison_array_it *it, struct bison_printer *printer, struct string_builder *builder);
static bool print_column(struct bison_column_it *it, struct bison_printer *printer, struct string_builder *builder);

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
        return bison_create_ex(doc, 1024, 1);
}

NG5_EXPORT(bool) bison_create_ex(struct bison *doc, u64 doc_cap_byte, u64 array_cap_byte)
{
        error_if_null(doc);

        doc_cap_byte = ng5_max(MIN_DOC_CAPACITY, doc_cap_byte);

        error_init(&doc->err);
        memblock_create(&doc->memblock, doc_cap_byte);
        memblock_zero_out(doc->memblock);
        memfile_open(&doc->memfile, doc->memblock, READ_WRITE);
        vec_create(&doc->handler, NULL, sizeof(struct bison_handler), 1);

        spin_init(&doc->versioning.write_lock);

        doc->versioning.revision_lock = false;
        doc->versioning.is_latest = true;

        bison_header_init(doc);
        bison_int_insert_array(&doc->memfile, array_cap_byte);

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

        struct bison_array_it it;
        bison_access(&it, doc);
        print_array(&it, &p, &b);

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
        error_if(context->revised_doc->memfile.mode != READ_WRITE, &context->original->err, NG5_ERR_INTERNALERR)
        return bison_array_it_create(it, &context->revised_doc->memfile, &context->original->err, payload_start);
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

NG5_EXPORT(bool) bison_access(struct bison_array_it *it, struct bison *doc)
{
        error_if_null(it);
        error_if_null(doc);
        offset_t payload_start = internal_payload_after_header(doc);
        bison_array_it_create(it, &doc->memfile, &doc->err, payload_start);
        bison_array_it_readonly(it);
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
        case BISON_FIELD_TYPE_BINARY_CUSTOM:
        case BISON_FIELD_TYPE_BINARY:
                return BISON_FIELD_TYPE_BINARY_STR;
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

static bool printer_bison_array_begin(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_array_begin);
        printer->print_bison_array_begin(printer, builder);
        return true;
}

static bool printer_bison_array_end(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_array_end);
        printer->print_bison_array_end(printer, builder);
        return true;
}

static bool printer_bison_null(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_null);
        printer->print_bison_null(printer, builder);
        return true;
}

static bool printer_bison_true(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_true);
        printer->print_bison_true(printer, builder);
        return true;
}

static bool printer_bison_false(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_false);
        printer->print_bison_false(printer, builder);
        return true;
}

static bool printer_bison_comma(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_comma);
        printer->print_bison_comma(printer, builder);
        return true;
}

static bool printer_bison_signed(struct bison_printer *printer, struct string_builder *builder, i64 value)
{
        error_if_null(printer->print_bison_signed);
        printer->print_bison_signed(printer, builder, value);
        return true;
}

static bool printer_bison_unsigned(struct bison_printer *printer, struct string_builder *builder, u64 value)
{
        error_if_null(printer->print_bison_unsigned);
        printer->print_bison_unsigned(printer, builder, value);
        return true;
}

static bool printer_bison_float(struct bison_printer *printer, struct string_builder *builder, float value)
{
        error_if_null(printer->print_bison_float);
        printer->print_bison_float(printer, builder, value);
        return true;
}

static bool printer_bison_string(struct bison_printer *printer, struct string_builder *builder, const char *value, u64 strlen)
{
        error_if_null(printer->print_bison_string);
        printer->print_bison_string(printer, builder, value, strlen);
        return true;
}

static bool printer_bison_binary(struct bison_printer *printer, struct string_builder *builder, const struct bison_binary *binary)
{
        error_if_null(printer->print_bison_binary);
        printer->print_bison_binary(printer, builder, binary);
        return true;
}

static bool print_array(struct bison_array_it *it, struct bison_printer *printer, struct string_builder *builder)
{
        assert(it);
        assert(printer);
        assert(builder);
        bool first_entry = true;
        printer_bison_array_begin(printer, builder);

        while (bison_array_it_next(it)) {
                if (likely(!first_entry)) {
                        printer_bison_comma(printer, builder);
                }
                enum bison_field_type type;
                bison_array_it_field_type(&type, it);
                switch (type) {
                case BISON_FIELD_TYPE_NULL:
                        printer_bison_null(printer, builder);
                        break;
                case BISON_FIELD_TYPE_TRUE:
                        printer_bison_true(printer, builder);
                        break;
                case BISON_FIELD_TYPE_FALSE:
                        printer_bison_false(printer, builder);
                        break;
                case BISON_FIELD_TYPE_NUMBER_U8:
                case BISON_FIELD_TYPE_NUMBER_U16:
                case BISON_FIELD_TYPE_NUMBER_U32:
                case BISON_FIELD_TYPE_NUMBER_U64: {
                        u64 value;
                        bison_array_it_unsigned_value(&value, it);
                        printer_bison_unsigned(printer, builder, value);
                } break;
                case BISON_FIELD_TYPE_NUMBER_I8:
                case BISON_FIELD_TYPE_NUMBER_I16:
                case BISON_FIELD_TYPE_NUMBER_I32:
                case BISON_FIELD_TYPE_NUMBER_I64: {
                        i64 value;
                        bison_array_it_signed_value(&value, it);
                        printer_bison_signed(printer, builder, value);
                } break;
                case BISON_FIELD_TYPE_NUMBER_FLOAT: {
                        float value;
                        bison_array_it_float_value(&value, it);
                        printer_bison_float(printer, builder, value);
                } break;
                case BISON_FIELD_TYPE_STRING: {
                        u64 strlen;
                        const char *value = bison_array_it_string_value(&strlen, it);
                        printer_bison_string(printer, builder, value, strlen);
                } break;
                case BISON_FIELD_TYPE_BINARY:
                case BISON_FIELD_TYPE_BINARY_CUSTOM: {
                        struct bison_binary binary;
                        bison_array_it_binary_value(&binary, it);
                        printer_bison_binary(printer, builder, &binary);
                } break;
                case BISON_FIELD_TYPE_ARRAY: {
                        struct bison_array_it *array = bison_array_it_array_value(it);
                        print_array(array, printer, builder);
                } break;
                case BISON_FIELD_TYPE_COLUMN: {
                        struct bison_column_it *column = bison_array_it_column_value(it);
                        print_column(column, printer, builder);
                } break;
                case BISON_FIELD_TYPE_OBJECT:
                default:
                        printer_bison_array_end(printer, builder);
                        error(&it->err, NG5_ERR_CORRUPTED);
                        return false;
                }
                first_entry = false;
        }

        printer_bison_array_end(printer, builder);
        return true;
}

static bool print_column(struct bison_column_it *it, struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(it)
        error_if_null(printer)
        error_if_null(builder)

        enum bison_field_type type;
        u32 nvalues;
        const void *values = bison_column_it_values(&type, &nvalues, it);

        printer_bison_array_begin(printer, builder);
        for (u32 i = 0; i < nvalues; i++) {
                switch (type) {
                case BISON_FIELD_TYPE_NULL:
                        printer_bison_null(printer, builder);
                        break;
                case BISON_FIELD_TYPE_TRUE:
                        printer_bison_true(printer, builder);
                        break;
                case BISON_FIELD_TYPE_FALSE:
                        printer_bison_false(printer, builder);
                        break;
                case BISON_FIELD_TYPE_NUMBER_U8: {
                        u64 number = ((u8*) values)[i];
                        printer_bison_unsigned(printer, builder, number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_U16: {
                        u64 number = ((u16*) values)[i];
                        printer_bison_unsigned(printer, builder, number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_U32: {
                        u64 number = ((u32*) values)[i];
                        printer_bison_unsigned(printer, builder, number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_U64: {
                        u64 number = ((u64*) values)[i];
                        printer_bison_unsigned(printer, builder, number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_I8: {
                        i64 number = ((i8*) values)[i];
                        printer_bison_signed(printer, builder, number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_I16: {
                        i64 number = ((i16*) values)[i];
                        printer_bison_signed(printer, builder, number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_I32: {
                        i64 number = ((i32*) values)[i];
                        printer_bison_signed(printer, builder, number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_I64: {
                        i64 number = ((i64*) values)[i];
                        printer_bison_signed(printer, builder, number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_FLOAT: {
                        float number = ((float*) values)[i];
                        printer_bison_float(printer, builder, number);
                } break;
                default:
                        printer_bison_array_end(printer, builder);
                        error(&it->err, NG5_ERR_CORRUPTED);
                        return false;
                }
                if (i + 1 < nvalues) {
                        printer_bison_comma(printer, builder);
                }
        }
        printer_bison_array_end(printer, builder);

        return true;
}