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

#include <jakson/carbon/key.h>
#include <jakson/carbon/string.h>

static void write_nokey(memfile *file)
{
        u8 marker = CARBON_MNOKEY;
        memfile_write(file, &marker, sizeof(u8));
}

static void write_autokey(memfile *file)
{
        u8 marker = CARBON_MAUTOKEY;
        unique_id_t key = 0;
        memfile_write(file, &marker, sizeof(u8));
        memfile_write(file, &key, sizeof(unique_id_t));
}

static void write_ukey(memfile *file)
{
        u8 marker = CARBON_MUKEY;
        u64 key = 0;
        memfile_write(file, &marker, sizeof(u8));
        memfile_write(file, &key, sizeof(u64));
}

static void write_ikey(memfile *file)
{
        u8 marker = CARBON_MIKEY;
        i64 key = 0;
        memfile_write(file, &marker, sizeof(u8));
        memfile_write(file, &key, sizeof(u64));
}

static void write_skey(memfile *file)
{
        u8 marker = CARBON_MSKEY;
        const char *key = "";
        memfile_write(file, &marker, sizeof(u8));
        carbon_string_write(file, key);
}

bool carbon_key_create(memfile *file, carbon_key_e type, err *err)
{
        ERROR_IF_NULL(file)

        switch (type) {
                case CARBON_KEY_NOKEY:
                        write_nokey(file);
                        break;
                case CARBON_KEY_AUTOKEY:
                        write_autokey(file);
                        break;
                case CARBON_KEY_UKEY:
                        write_ukey(file);
                        break;
                case CARBON_KEY_IKEY:
                        write_ikey(file);
                        break;
                case CARBON_KEY_SKEY:
                        write_skey(file);
                        break;
                default:
                        OPTIONAL(err != NULL, ERROR(err, ERR_INTERNALERR))
                        return false;
        }
        return true;
}

bool carbon_key_skip(carbon_key_e *out, memfile *file)
{
        ERROR_IF_NULL(file)
        carbon_key_read(NULL, out, file);
        return true;
}

bool carbon_key_write_unsigned(memfile *file, u64 key)
{
        ERROR_IF_NULL(file)

        DECLARE_AND_INIT(carbon_key_e, key_type)

        carbon_key_read_type(&key_type, file);
        if (carbon_key_is_unsigned(key_type)) {
                memfile_write(file, &key, sizeof(u64));
                return true;
        } else {
                ERROR(&file->err, ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_key_write_signed(memfile *file, i64 key)
{
        ERROR_IF_NULL(file)

        DECLARE_AND_INIT(carbon_key_e, key_type)

        carbon_key_read_type(&key_type, file);
        if (carbon_key_is_signed(key_type)) {
                memfile_write(file, &key, sizeof(i64));
                return true;
        } else {
                ERROR(&file->err, ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_key_update_string(memfile *file, const char *key)
{
        return carbon_key_update_string_wnchar(file, key, strlen(key));
}

bool carbon_key_update_string_wnchar(memfile *file, const char *key, size_t length)
{
        ERROR_IF_NULL(file)
        DECLARE_AND_INIT(carbon_key_e, key_type)
        carbon_key_read_type(&key_type, file);
        if (carbon_key_is_string(key_type)) {
                carbon_string_update_wnchar(file, key, length);
                return true;
        } else {
                ERROR(&file->err, ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_key_write_string(memfile *file, const char *key)
{
        ERROR_IF_NULL(file)

        DECLARE_AND_INIT(carbon_key_e, key_type)

        carbon_key_read_type(&key_type, file);
        if (carbon_key_is_string(key_type)) {
                carbon_string_write(file, key);
                return true;
        } else {
                ERROR(&file->err, ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_key_read_type(carbon_key_e *out, memfile *file)
{
        u8 marker = *MEMFILE_READ_TYPE(file, u8);

        JAK_ASSERT(marker == CARBON_MNOKEY || marker == CARBON_MAUTOKEY || marker ==
                                                                                                       CARBON_MUKEY ||
                   marker == CARBON_MIKEY || marker == CARBON_MSKEY);

        switch (marker) {
                case CARBON_MNOKEY:
                        OPTIONAL_SET(out, CARBON_KEY_NOKEY)
                        break;
                case CARBON_MAUTOKEY:
                        OPTIONAL_SET(out, CARBON_KEY_AUTOKEY)
                        break;
                case CARBON_MUKEY:
                        OPTIONAL_SET(out, CARBON_KEY_UKEY)
                        break;
                case CARBON_MIKEY:
                        OPTIONAL_SET(out, CARBON_KEY_IKEY)
                        break;
                case CARBON_MSKEY:
                        OPTIONAL_SET(out, CARBON_KEY_SKEY)
                        break;
                default: ERROR(&file->err, ERR_INTERNALERR)
                        return false;
        }
        return true;
}

const void *carbon_key_read(u64 *len, carbon_key_e *out, memfile *file)
{
        carbon_key_e key_type = 0;
        carbon_key_read_type(&key_type, file);

        OPTIONAL_SET(out, key_type)

        switch (key_type) {
                case CARBON_KEY_NOKEY:
                        OPTIONAL_SET(len, 0)
                        return NULL;
                case CARBON_KEY_AUTOKEY:
                        OPTIONAL_SET(len, sizeof(unique_id_t))
                        return MEMFILE_READ_TYPE(file, unique_id_t);
                case CARBON_KEY_UKEY:
                        OPTIONAL_SET(len, sizeof(u64))
                        return MEMFILE_READ_TYPE(file, u64);
                case CARBON_KEY_IKEY:
                        OPTIONAL_SET(len, sizeof(i64))
                        return MEMFILE_READ_TYPE(file, i64);
                case CARBON_KEY_SKEY:
                        return carbon_string_read(len, file);
                default: ERROR(&file->err, ERR_INTERNALERR)
                        return NULL;
        }
}

const char *carbon_key_type_str(carbon_key_e type)
{
        switch (type) {
                case CARBON_KEY_NOKEY:
                        return "nokey";
                case CARBON_KEY_AUTOKEY:
                        return "autokey";
                case CARBON_KEY_UKEY:
                        return "ukey";
                case CARBON_KEY_IKEY:
                        return "ikey";
                case CARBON_KEY_SKEY:
                        return "skey";
                default: ERROR_PRINT(ERR_INTERNALERR);
                        return NULL;
        }
}