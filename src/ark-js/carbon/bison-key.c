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

#include <ark-js/carbon/bison-key.h>
#include <ark-js/carbon/bison-string.h>

static void write_nokey(struct memfile *file)
{
        u8 marker = BISON_MARKER_KEY_NOKEY;
        memfile_write(file, &marker, sizeof(u8));
}

static void write_autokey(struct memfile *file)
{
        u8 marker = BISON_MARKER_KEY_AUTOKEY;
        object_id_t key = 0;
        memfile_write(file, &marker, sizeof(u8));
        memfile_write(file, &key, sizeof(object_id_t));
}

static void write_ukey(struct memfile *file)
{
        u8 marker = BISON_MARKER_KEY_UKEY;
        u64 key = 0;
        memfile_write(file, &marker, sizeof(u8));
        memfile_write(file, &key, sizeof(u64));
}

static void write_ikey(struct memfile *file)
{
        u8 marker = BISON_MARKER_KEY_IKEY;
        i64 key = 0;
        memfile_write(file, &marker, sizeof(u8));
        memfile_write(file, &key, sizeof(u64));
}

static void write_skey(struct memfile *file)
{
        u8 marker = BISON_MARKER_KEY_SKEY;
        const char *key = "";
        memfile_write(file, &marker, sizeof(u8));
        bison_string_write(file, key);
}

NG5_EXPORT(bool) bison_key_create(struct memfile *file, enum bison_primary_key_type type, struct err *err)
{
        error_if_null(file)

        switch (type) {
        case BISON_KEY_NOKEY:
                write_nokey(file);
                break;
        case BISON_KEY_AUTOKEY:
                write_autokey(file);
                break;
        case BISON_KEY_UKEY:
                write_ukey(file);
                break;
        case BISON_KEY_IKEY:
                write_ikey(file);
                break;
        case BISON_KEY_SKEY:
                write_skey(file);
                break;
        default:
                ng5_optional(err != NULL, error(err, NG5_ERR_INTERNALERR))
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_key_skip(enum bison_primary_key_type *out, struct memfile *file)
{
        error_if_null(file)
        bison_key_read(NULL, out, file);
        return true;
}

NG5_EXPORT(bool) bison_key_write_unsigned(struct memfile *file, u64 key)
{
        error_if_null(file)
        enum bison_primary_key_type key_type;
        bison_key_read_type(&key_type, file);
        if (bison_key_is_unsigned_type(key_type)) {
                memfile_write(file, &key, sizeof(u64));
                return true;
        } else {
                error(&file->err, NG5_ERR_TYPEMISMATCH)
                return false;
        }
}

NG5_EXPORT(bool) bison_key_write_signed(struct memfile *file, i64 key)
{
        error_if_null(file)
        enum bison_primary_key_type key_type;
        bison_key_read_type(&key_type, file);
        if (bison_key_is_signed_type(key_type)) {
                memfile_write(file, &key, sizeof(i64));
                return true;
        } else {
                error(&file->err, NG5_ERR_TYPEMISMATCH)
                return false;
        }
}

NG5_EXPORT(bool) bison_key_update_string(struct memfile *file, const char *key)
{
        error_if_null(file)
        enum bison_primary_key_type key_type;
        bison_key_read_type(&key_type, file);
        if (bison_key_is_string_type(key_type)) {
                bison_string_update(file, key);
                return true;
        } else {
                error(&file->err, NG5_ERR_TYPEMISMATCH)
                return false;
        }
}

NG5_EXPORT(bool) bison_key_write_string(struct memfile *file, const char *key)
{
        error_if_null(file)
        enum bison_primary_key_type key_type;
        bison_key_read_type(&key_type, file);
        if (bison_key_is_string_type(key_type)) {
                bison_string_write(file, key);
                return true;
        } else {
                error(&file->err, NG5_ERR_TYPEMISMATCH)
                return false;
        }
}

NG5_EXPORT(bool) bison_key_read_type(enum bison_primary_key_type *out, struct memfile *file)
{
        u8 marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        assert(marker == BISON_MARKER_KEY_NOKEY || marker == BISON_MARKER_KEY_AUTOKEY || marker ==
                BISON_MARKER_KEY_UKEY || marker == BISON_MARKER_KEY_IKEY || marker == BISON_MARKER_KEY_SKEY);

        switch (marker) {
        case BISON_MARKER_KEY_NOKEY:
                ng5_optional_set(out, BISON_KEY_NOKEY)
                break;
        case BISON_MARKER_KEY_AUTOKEY:
                ng5_optional_set(out, BISON_KEY_AUTOKEY)
                break;
        case BISON_MARKER_KEY_UKEY:
                ng5_optional_set(out, BISON_KEY_UKEY)
                break;
        case BISON_MARKER_KEY_IKEY:
                ng5_optional_set(out, BISON_KEY_IKEY)
                break;
        case BISON_MARKER_KEY_SKEY:
                ng5_optional_set(out, BISON_KEY_SKEY)
                break;
        default:
                error(&file->err, NG5_ERR_INTERNALERR)
                return false;
        }
        return true;
}

NG5_EXPORT(const void *) bison_key_read(u64 *len, enum bison_primary_key_type *out, struct memfile *file)
{
        enum bison_primary_key_type key_type;
        bison_key_read_type(&key_type, file);

        ng5_optional_set(out, key_type)

        switch (key_type) {
        case BISON_KEY_NOKEY:
                ng5_optional_set(len, 0)
                return NULL;
        case BISON_KEY_AUTOKEY:
                ng5_optional_set(len, sizeof(object_id_t))
                return NG5_MEMFILE_READ_TYPE(file, object_id_t);
        case BISON_KEY_UKEY:
                ng5_optional_set(len, sizeof(u64))
                return NG5_MEMFILE_READ_TYPE(file, u64);
        case BISON_KEY_IKEY:
                ng5_optional_set(len, sizeof(i64))
                return NG5_MEMFILE_READ_TYPE(file, i64);
        case BISON_KEY_SKEY:
                return bison_string_read(len, file);
        default:
                error(&file->err, NG5_ERR_INTERNALERR)
                return NULL;
        }
}