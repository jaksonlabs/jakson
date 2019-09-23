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

#include <jak_carbon_key.h>
#include <jak_carbon_string.h>

static void write_nokey(jak_memfile *file)
{
        jak_u8 marker = CARBON_MNOKEY;
        jak_memfile_write(file, &marker, sizeof(jak_u8));
}

static void write_autokey(jak_memfile *file)
{
        jak_u8 marker = CARBON_MAUTOKEY;
        jak_uid_t key = 0;
        jak_memfile_write(file, &marker, sizeof(jak_u8));
        jak_memfile_write(file, &key, sizeof(jak_uid_t));
}

static void write_ukey(jak_memfile *file)
{
        jak_u8 marker = CARBON_MUKEY;
        jak_u64 key = 0;
        jak_memfile_write(file, &marker, sizeof(jak_u8));
        jak_memfile_write(file, &key, sizeof(jak_u64));
}

static void write_ikey(jak_memfile *file)
{
        jak_u8 marker = CARBON_MIKEY;
        jak_i64 key = 0;
        jak_memfile_write(file, &marker, sizeof(jak_u8));
        jak_memfile_write(file, &key, sizeof(jak_u64));
}

static void write_skey(jak_memfile *file)
{
        jak_u8 marker = CARBON_MSKEY;
        const char *key = "";
        jak_memfile_write(file, &marker, sizeof(jak_u8));
        jak_carbon_jak_string_write(file, key);
}

bool jak_carbon_key_create(jak_memfile *file, jak_carbon_key_e type, jak_error *err)
{
        JAK_ERROR_IF_NULL(file)

        switch (type) {
                case JAK_CARBON_KEY_NOKEY:
                        write_nokey(file);
                        break;
                case JAK_CARBON_KEY_AUTOKEY:
                        write_autokey(file);
                        break;
                case JAK_CARBON_KEY_UKEY:
                        write_ukey(file);
                        break;
                case JAK_CARBON_KEY_IKEY:
                        write_ikey(file);
                        break;
                case JAK_CARBON_KEY_SKEY:
                        write_skey(file);
                        break;
                default:
                        JAK_OPTIONAL(err != NULL, JAK_ERROR(err, JAK_ERR_INTERNALERR))
                        return false;
        }
        return true;
}

bool jak_carbon_key_skip(jak_carbon_key_e *out, jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file)
        jak_carbon_key_read(NULL, out, file);
        return true;
}

bool jak_carbon_key_write_unsigned(jak_memfile *file, jak_u64 key)
{
        JAK_ERROR_IF_NULL(file)

        JAK_DECLARE_AND_INIT(jak_carbon_key_e, key_type)

        jak_carbon_key_read_type(&key_type, file);
        if (jak_carbon_key_is_unsigned(key_type)) {
                jak_memfile_write(file, &key, sizeof(jak_u64));
                return true;
        } else {
                JAK_ERROR(&file->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool jak_carbon_key_write_signed(jak_memfile *file, jak_i64 key)
{
        JAK_ERROR_IF_NULL(file)

        JAK_DECLARE_AND_INIT(jak_carbon_key_e, key_type)

        jak_carbon_key_read_type(&key_type, file);
        if (jak_carbon_key_is_signed(key_type)) {
                jak_memfile_write(file, &key, sizeof(jak_i64));
                return true;
        } else {
                JAK_ERROR(&file->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool jak_carbon_key_update_string(jak_memfile *file, const char *key)
{
        return jak_carbon_key_update_jak_string_wnchar(file, key, strlen(key));
}

bool jak_carbon_key_update_jak_string_wnchar(jak_memfile *file, const char *key, size_t length)
{
        JAK_ERROR_IF_NULL(file)
        JAK_DECLARE_AND_INIT(jak_carbon_key_e, key_type)
        jak_carbon_key_read_type(&key_type, file);
        if (jak_carbon_key_is_string(key_type)) {
                jak_carbon_jak_string_update_wnchar(file, key, length);
                return true;
        } else {
                JAK_ERROR(&file->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool jak_carbon_key_write_string(jak_memfile *file, const char *key)
{
        JAK_ERROR_IF_NULL(file)

        JAK_DECLARE_AND_INIT(jak_carbon_key_e, key_type)

        jak_carbon_key_read_type(&key_type, file);
        if (jak_carbon_key_is_string(key_type)) {
                jak_carbon_jak_string_write(file, key);
                return true;
        } else {
                JAK_ERROR(&file->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool jak_carbon_key_read_type(jak_carbon_key_e *out, jak_memfile *file)
{
        jak_u8 marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        JAK_ASSERT(marker == CARBON_MNOKEY || marker == CARBON_MAUTOKEY || marker ==
                                                                                                       CARBON_MUKEY ||
                   marker == CARBON_MIKEY || marker == CARBON_MSKEY);

        switch (marker) {
                case CARBON_MNOKEY:
                        JAK_OPTIONAL_SET(out, JAK_CARBON_KEY_NOKEY)
                        break;
                case CARBON_MAUTOKEY:
                        JAK_OPTIONAL_SET(out, JAK_CARBON_KEY_AUTOKEY)
                        break;
                case CARBON_MUKEY:
                        JAK_OPTIONAL_SET(out, JAK_CARBON_KEY_UKEY)
                        break;
                case CARBON_MIKEY:
                        JAK_OPTIONAL_SET(out, JAK_CARBON_KEY_IKEY)
                        break;
                case CARBON_MSKEY:
                        JAK_OPTIONAL_SET(out, JAK_CARBON_KEY_SKEY)
                        break;
                default: JAK_ERROR(&file->err, JAK_ERR_INTERNALERR)
                        return false;
        }
        return true;
}

const void *jak_carbon_key_read(jak_u64 *len, jak_carbon_key_e *out, jak_memfile *file)
{
        jak_carbon_key_e key_type = 0;
        jak_carbon_key_read_type(&key_type, file);

        JAK_OPTIONAL_SET(out, key_type)

        switch (key_type) {
                case JAK_CARBON_KEY_NOKEY:
                        JAK_OPTIONAL_SET(len, 0)
                        return NULL;
                case JAK_CARBON_KEY_AUTOKEY:
                        JAK_OPTIONAL_SET(len, sizeof(jak_uid_t))
                        return JAK_MEMFILE_READ_TYPE(file, jak_uid_t);
                case JAK_CARBON_KEY_UKEY:
                        JAK_OPTIONAL_SET(len, sizeof(jak_u64))
                        return JAK_MEMFILE_READ_TYPE(file, jak_u64);
                case JAK_CARBON_KEY_IKEY:
                        JAK_OPTIONAL_SET(len, sizeof(jak_i64))
                        return JAK_MEMFILE_READ_TYPE(file, jak_i64);
                case JAK_CARBON_KEY_SKEY:
                        return jak_carbon_jak_string_read(len, file);
                default: JAK_ERROR(&file->err, JAK_ERR_INTERNALERR)
                        return NULL;
        }
}

const char *jak_carbon_key_type_str(jak_carbon_key_e type)
{
        switch (type) {
                case JAK_CARBON_KEY_NOKEY:
                        return "nokey";
                case JAK_CARBON_KEY_AUTOKEY:
                        return "autokey";
                case JAK_CARBON_KEY_UKEY:
                        return "ukey";
                case JAK_CARBON_KEY_IKEY:
                        return "ikey";
                case JAK_CARBON_KEY_SKEY:
                        return "skey";
                default: JAK_ERROR_PRINT(JAK_ERR_INTERNALERR);
                        return NULL;
        }
}