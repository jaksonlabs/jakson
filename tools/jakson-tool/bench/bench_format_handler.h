// file: bench_format_handler.h

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

#ifndef JAKSON_BENCH_FORMAT_HANDLER_H
#define JAKSON_BENCH_FORMAT_HANDLER_H

#include <jak_stdinc.h>
#include <jak_error.h>

#include "bench_carbon.h"
#include "bench_bson.h"
#include "bench_ubjson.h"

typedef struct bench_error {
    uint32_t code;
    char *msg;
} bench_error;

typedef struct bench_format_ops bench_format_ops;

/**
typedef struct bench_format_mgr {
    void *data;
    bench_format_ops *ops;
} bench_format_mgr;

struct bench_format_ops {
    bool (*create_doc)(bench_format_mgr *, char* file);
    bool (*insert_uint32)(bench_format_mgr *, uint32_t value, unsigned int pos);
    bool (*revise_uint32)(bench_format_mgr *, uint32_t value, unsigned int pos);
    bool (*delete_uint32)(bench_format_mgr *, unsigned int pos);
    // TODO: Add more operations
};
**/

typedef struct bench_format_handler {
    bench_error *error;
    //bench_format_mgr manager;
    void *manager;
    char *format_name;
} bench_format_handler;

//bool bench_format_handler_create_carbon_handler(bench_format_handler *handler,bench_carbon_mgr *manager, bench_error *error, const char* filePath);
bool bench_format_handler_create_bson_handler(bench_format_handler *handler, bench_error *error, const char* filePath);
//bool bench_format_handler_create_ubjson_handler(bench_format_handler *handler, bench_error *error, const char* filePath);
bool bench_format_handler_destroy(bench_format_handler *handler);
bool bench_format_handler_insert_int32(bench_format_handler *handler, const char *key, uint32_t val);

#endif
