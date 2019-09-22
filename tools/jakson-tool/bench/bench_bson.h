// file: bench_bson.h

/**
 *  Copyright (C) 2018 Marcus Pinnecke
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JAKSON_BENCH_BSON_H
#define JAKSON_BENCH_BSON_H

#include <jak_stdinc.h>
#include <jak_error.h>

#include <libs/bson/bson.h>

#include "bench_fwd.h"

//typedef struct bench_format_handler bench_format_handler;
//extern struct bench_error bench_error;
//typedef struct bench_bson_error bench_bson_error;

typedef struct bench_bson_error {
    bench_error *err;
    //bson_error_t *bError;
} bench_bson_error;

typedef struct bench_bson_mgr {
    //bench_format_handler handler;
    bson_t *b;
    bench_bson_error *error;
    bson_iter_t *it;
} bench_bson_mgr;

bool bench_bson_error_create(bench_bson_error *bsonError, bench_error *benchError);
bool bench_bson_error_write(bench_bson_error *error, char *msg, size_t errOffset);
bool bench_bson_mgr_create_from_file(bench_bson_mgr *manager, const char *filePath);
bool bench_bson_mgr_create_empty(bench_bson_mgr *manager, bench_bson_error *error);
bool bench_bson_mgr_destroy(bench_bson_mgr *manager);
bool bench_bson_get_doc(char *str, bench_bson_mgr *manager);

bool bench_bson_insert_int8(bench_bson_mgr *manager, const char *key, int8_t val);
bool bench_bson_insert_int16(bench_bson_mgr *manager, const char *key, int16_t val);
bool bench_bson_insert_int32(bench_bson_mgr *manager, const char *key, int32_t val);
bool bench_bson_insert_int64(bench_bson_mgr *manager, const char *key, int64_t val);
bool bench_bson_insert_string(bench_bson_mgr *manager, const char *key, const char *val);

bool bench_bson_find_int8(bench_bson_mgr *manager, const char *key, int8_t val);
bool bench_bson_find_int16(bench_bson_mgr *manager, const char *key, int16_t val);
bool bench_bson_find_int32(bench_bson_mgr *manager, const char *key, int32_t val);
bool bench_bson_find_int64(bench_bson_mgr *manager, const char *key, int64_t val);
bool bench_bson_find_string(bench_bson_mgr *manager, const char *key, const char *val);

bool bench_bson_change_val_int8(bench_bson_mgr *manager, const char *key, int8_t newVal);
bool bench_bson_change_val_int16(bench_bson_mgr *manager, const char *key, int16_t newVal);
bool bench_bson_change_val_int32(bench_bson_mgr *manager, const char *key, int32_t newVal);
bool bench_bson_change_val_int64(bench_bson_mgr *manager, const char *key, int64_t newVal);
bool bench_bson_change_val_string(bench_bson_mgr *manager, const char *key, const char *newVal);

bool bench_bson_convert_entry_int8(bench_bson_mgr *manager, const char *key);
bool bench_bson_convert_entry_int16(bench_bson_mgr *manager, const char *key);
bool bench_bson_convert_entry_int32(bench_bson_mgr *manager, const char *key);
bool bench_bson_convert_entry_int64(bench_bson_mgr *manager, const char *key);
bool bench_bson_convert_entry_string(bench_bson_mgr *manager, const char *key, size_t size);
bool bench_bson_convert_entry_object(bench_bson_mgr *manager, const char *key);

bool bench_bson_delete_int8(bench_bson_mgr *manager, const char *key);
bool bench_bson_delete_int16(bench_bson_mgr *manager, const char *key);
bool bench_bson_delete_int32(bench_bson_mgr *manager, const char *key);
bool bench_bson_delete_int64(bench_bson_mgr *manager, const char *key);
bool bench_bson_delete_string(bench_bson_mgr *manager, const char *key);


#endif
