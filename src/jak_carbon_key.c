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

static void write_nokey(struct jak_memfile *file)
{
        jak_u8 marker = JAK_CARBON_MARKER_KEY_NOKEY;
        memfile_write(file, &marker, sizeof(jak_u8));
}

static void write_autokey(struct jak_memfile *file)
{
        jak_u8 marker = JAK_CARBON_MARKER_KEY_AUTOKEY;
        jak_global_id_t key = 0;
        memfile_write(file, &marker, sizeof(jak_u8));
        memfile_write(file, &key, sizeof(jak_global_id_t));
}

static void write_ukey(struct jak_memfile *file)
{
        jak_u8 marker = JAK_CARBON_MARKER_KEY_UKEY;
        jak_u64 key = 0;
        memfile_write(file, &marker, sizeof(jak_u8));
        memfile_write(file, &key, sizeof(jak_u64));
}

static void write_ikey(struct jak_memfile *file)
{
        jak_u8 marker = JAK_CARBON_MARKER_KEY_IKEY;
        jak_i64 key = 0;
        memfile_write(file, &marker, sizeof(jak_u8));
        memfile_write(file, &key, sizeof(jak_u64));
}

static void write_skey(struct jak_memfile *file)
{
        jak_u8 marker = JAK_CARBON_MARKER_KEY_SKEY;
        const char *key = "";
        memfile_write(file, &marker, sizeof(jak_u8));
        carbon_string_write(file, key);
}

bool carbon_key_create(struct jak_memfile *file, jak_carbon_key_e type, struct jak_error *err)
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
                        JAK_optional(err != NULL, error(err, JAK_ERR_INTERNALERR))
                        return false;
        }
        return true;
}

bool carbon_key_skip(jak_carbon_key_e *out, struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file)
        carbon_key_read(NULL, out, file);
        return true;
}

bool carbon_key_write_unsigned(struct jak_memfile *file, jak_u64 key)
{
        JAK_ERROR_IF_NULL(file)

        JAK_declare_and_init(jak_carbon_key_e, key_type)

        carbon_key_read_type(&key_type, file);
        if (jak_carbon_key_is_unsigned(key_type)) {
                memfile_write(file, &key, sizeof(jak_u64));
                return true;
        } else {
                error(&file->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_key_write_signed(struct jak_memfile *file, jak_i64 key)
{
        JAK_ERROR_IF_NULL(file)

        JAK_declare_and_init(jak_carbon_key_e, key_type)

        carbon_key_read_type(&key_type, file);
        if (jak_carbon_key_is_signed(key_type)) {
                memfile_write(file, &key, sizeof(jak_i64));
                return true;
        } else {
                error(&file->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_key_update_string(struct jak_memfile *file, const char *key)
{
        return carbon_key_update_string_wnchar(file, key, strlen(key));
}

bool carbon_key_update_string_wnchar(struct jak_memfile *file, const char *key, size_t length)
{
        JAK_ERROR_IF_NULL(file)
        JAK_declare_and_init(jak_carbon_key_e, key_type)
        carbon_key_read_type(&key_type, file);
        if (jak_carbon_key_is_string(key_type)) {
                carbon_string_update_wnchar(file, key, length);
                return true;
        } else {
                error(&file->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_key_write_string(struct jak_memfile *file, const char *key)
{
        JAK_ERROR_IF_NULL(file)

        JAK_declare_and_init(jak_carbon_key_e, key_type)

        carbon_key_read_type(&key_type, file);
        if (jak_carbon_key_is_string(key_type)) {
                carbon_string_write(file, key);
                return true;
        } else {
                error(&file->err, JAK_ERR_TYPEMISMATCH)
                return false;
        }
}

bool carbon_key_read_type(jak_carbon_key_e *out, struct jak_memfile *file)
{
        jak_u8 marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        JAK_ASSERT(marker == JAK_CARBON_MARKER_KEY_NOKEY || marker == JAK_CARBON_MARKER_KEY_AUTOKEY || marker ==
                                                                                                       JAK_CARBON_MARKER_KEY_UKEY ||
                   marker == JAK_CARBON_MARKER_KEY_IKEY || marker == JAK_CARBON_MARKER_KEY_SKEY);

        switch (marker) {
                case JAK_CARBON_MARKER_KEY_NOKEY:
                        JAK_optional_set(out, JAK_CARBON_KEY_NOKEY)
                        break;
                case JAK_CARBON_MARKER_KEY_AUTOKEY:
                        JAK_optional_set(out, JAK_CARBON_KEY_AUTOKEY)
                        break;
                case JAK_CARBON_MARKER_KEY_UKEY:
                        JAK_optional_set(out, JAK_CARBON_KEY_UKEY)
                        break;
                case JAK_CARBON_MARKER_KEY_IKEY:
                        JAK_optional_set(out, JAK_CARBON_KEY_IKEY)
                        break;
                case JAK_CARBON_MARKER_KEY_SKEY:
                        JAK_optional_set(out, JAK_CARBON_KEY_SKEY)
                        break;
                default: error(&file->err, JAK_ERR_INTERNALERR)
                        return false;
        }
        return true;
}

const void *carbon_key_read(jak_u64 *len, jak_carbon_key_e *out, struct jak_memfile *file)
{
        jak_carbon_key_e key_type = 0;
        carbon_key_read_type(&key_type, file);

        JAK_optional_set(out, key_type)

        switch (key_type) {
                case JAK_CARBON_KEY_NOKEY:
                        JAK_optional_set(len, 0)
                        return NULL;
                case JAK_CARBON_KEY_AUTOKEY:
                        JAK_optional_set(len, sizeof(jak_global_id_t))
                        return JAK_MEMFILE_READ_TYPE(file, jak_global_id_t);
                case JAK_CARBON_KEY_UKEY:
                        JAK_optional_set(len, sizeof(jak_u64))
                        return JAK_MEMFILE_READ_TYPE(file, jak_u64);
                case JAK_CARBON_KEY_IKEY:
                        JAK_optional_set(len, sizeof(jak_i64))
                        return JAK_MEMFILE_READ_TYPE(file, jak_i64);
                case JAK_CARBON_KEY_SKEY:
                        return carbon_string_read(len, file);
                default: error(&file->err, JAK_ERR_INTERNALERR)
                        return NULL;
        }
}

const char *carbon_key_type_str(jak_carbon_key_e type)
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
                default: error_print(JAK_ERR_INTERNALERR);
                        return NULL;
        }
}