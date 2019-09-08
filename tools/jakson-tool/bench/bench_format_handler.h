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

typedef struct bench_error {
    uint32_t code;
    char *msg;
} bench_error;

typedef struct bench_format_reader {
    void *data;
    bool (*get_reader) (int argc, char **argv, void *memory, char *file);
} bench_format_reader;

typedef struct bench_format_handler {
    bench_error error;
    bench_format_reader reader;
    char *format_name;
} bench_format_handler;

bool bench_format_handler_create(bench_format_handler *handler, bench_error error, char *format_name);
//bool bench_format_handler_insert_char(bench_format_handler *handler, char value);


#endif
