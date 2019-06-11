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

#include "stdx/varuint.h"
#include "core/bison/bison.h"
#include "core/bison/bison-array-it.h"
#include "core/bison/bison-insert.h"
#include "core/bison/bison-media.h"

static bool ensure_space(struct bison_insert *inserter, u64 nbytes)
{
        error_if_null(inserter)
        char c = *memfile_peek(&inserter->memfile, 1);
        if (unlikely(c != 0)) {
                /* not enough space; enlarge container */
                memfile_move(&inserter->memfile, nbytes);
        }
        return true;
}

static bool push(struct bison_insert *inserter, const void *base, u64 nbytes)
{
        ensure_space(inserter, nbytes);
        return memfile_write(&inserter->memfile, base, nbytes);
}

static bool push_media_type(struct bison_insert *inserter, enum bison_field_type type)
{
        ensure_space(inserter, sizeof(media_type_t));
        return bison_media_write(&inserter->memfile, type);
}

NG5_EXPORT(bool) bison_insert_create(struct bison_insert *inserter, struct bison_array_it *context)
{
        error_if_null(inserter)
        error_if_null(context)
        bison_array_it_lock(context);
        inserter->context = context;
        memfile_dup(&inserter->memfile, &context->memfile);
        error_init(&inserter->err);
        inserter->position = memfile_tell(&context->memfile);
        return true;
}

NG5_EXPORT(bool) bison_insert_null(struct bison_insert *inserter)
{
        return push_media_type(inserter, BISON_FIELD_TYPE_NULL);
}

NG5_EXPORT(bool) bison_insert_true(struct bison_insert *inserter)
{
        return push_media_type(inserter, BISON_FIELD_TYPE_TRUE);
}

NG5_EXPORT(bool) bison_insert_false(struct bison_insert *inserter)
{
        return push_media_type(inserter, BISON_FIELD_TYPE_FALSE);
}

NG5_EXPORT(bool) bison_insert_unsigned(struct bison_insert *inserter, u64 value)
{
        if (value <= UINT8_MAX) {
                push_media_type(inserter, BISON_FIELD_TYPE_NUMBER_U8);
                u8 data = (u8) value;
                push(inserter, &data, sizeof(u8));
                return true;
        } else if (value <= UINT16_MAX) {
                push_media_type(inserter, BISON_FIELD_TYPE_NUMBER_U16);
                u16 data = (u16) value;
                push(inserter, &data, sizeof(u16));
                return true;
        } else if (value <= UINT32_MAX) {
                push_media_type(inserter, BISON_FIELD_TYPE_NUMBER_U32);
                u32 data = (u32) value;
                push(inserter, &data, sizeof(u32));
                return true;
        } else if (value <= UINT64_MAX) {
                push_media_type(inserter, BISON_FIELD_TYPE_NUMBER_U64);
                u64 data = (u64) value;
                push(inserter, &data, sizeof(u64));
                return true;
        } else {
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
}

NG5_EXPORT(bool) bison_insert_signed(struct bison_insert *inserter, i64 value)
{
        if (value >= INT8_MIN && value <= INT8_MAX) {
                push_media_type(inserter, BISON_FIELD_TYPE_NUMBER_U8);
                i8 data = (i8) value;
                push(inserter, &data, sizeof(i8));
                return true;
        } else if (value >= INT16_MIN && value <= INT16_MAX) {
                push_media_type(inserter, BISON_FIELD_TYPE_NUMBER_U16);
                i16 data = (i16) value;
                push(inserter, &data, sizeof(i16));
                return true;
        } else if (value >= INT16_MIN && value <= INT32_MAX) {
                push_media_type(inserter, BISON_FIELD_TYPE_NUMBER_U32);
                i32 data = (i32) value;
                push(inserter, &data, sizeof(i32));
                return true;
        } else if (value >= INT16_MIN && value <= INT64_MAX) {
                push_media_type(inserter, BISON_FIELD_TYPE_NUMBER_U64);
                i64 data = (i64) value;
                push(inserter, &data, sizeof(i64));
                return true;
        } else {
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
}

NG5_EXPORT(bool) bison_insert_float(struct bison_insert *inserter, float value)
{
        error_if_null(inserter)
        push_media_type(inserter, BISON_FIELD_TYPE_NUMBER_FLOAT);
        push(inserter, &value, sizeof(float));
        return true;
}

static bool insert_byte_sequence(struct bison_insert *inserter, const void *data, u64 nbytes, enum bison_field_type type)
{
        ng5_unused(inserter);
        ng5_unused(data);
        error_if(type != BISON_FIELD_TYPE_STRING && type != BISON_FIELD_TYPE_BINARY, &inserter->err, NG5_ERR_INTERNALERR);

        push_media_type(inserter, type);

        ensure_space(inserter, varuint_required_blocks(nbytes) + nbytes);
        varuint_t enc_len = (varuint_t) memfile_peek(&inserter->memfile, sizeof(varuint_t));
        u8 varuin_nbytes = varuint_write(enc_len, nbytes);
        memfile_skip(&inserter->memfile, varuin_nbytes);
        memfile_write(&inserter->memfile, data, nbytes);
        return false;
}

NG5_EXPORT(bool) bison_insert_string(struct bison_insert *inserter, const char *value)
{
        return insert_byte_sequence(inserter, value, strlen(value), BISON_FIELD_TYPE_STRING);
}

NG5_EXPORT(bool) bison_insert_binary(struct bison_insert *inserter, const void *value, size_t nbytes)
{
        return insert_byte_sequence(inserter, value, nbytes, BISON_FIELD_TYPE_BINARY);
}

NG5_EXPORT(bool) bison_insert_drop(struct bison_insert *inserter)
{
        error_if_null(inserter)
        bison_array_it_unlock(inserter->context);
        return true;
}