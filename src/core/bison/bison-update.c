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

#include "core/bison/bison-path.h"
#include "core/bison/bison-update.h"
#include "core/bison/bison-insert.h"

#define try_array_update(type_match, in_place_update_fn, insert_fn)                                                    \
({                                                                                                                     \
        enum bison_field_type type_is;                                                                                 \
        bison_array_it_field_type(&type_is, it);                                                                       \
        bool status;                                                                                                   \
        switch (type_is) {                                                                                             \
                case type_match:                                                                                       \
                        status = in_place_update_fn(it, value);                                                        \
                break;                                                                                                 \
                default: {                                                                                             \
                        struct bison_insert inserter;                                                                  \
                        bison_array_it_remove(it);                                                                     \
                        bison_array_it_insert_begin(&inserter, it);                                                    \
                        status = insert_fn(&inserter, value);                                                          \
                        bison_array_it_insert_end(&inserter);                                                          \
                break;                                                                                                 \
                }                                                                                                      \
        }                                                                                                              \
        status;                                                                                                        \
})

#define DEFINE_ARRAY_UPDATE_FUNCTION(type_name, type_match, in_place_update_fn, insert_fn)                             \
static bool array_update_##type_name(struct bison_array_it *it, type_name value)                                       \
{                                                                                                                      \
        return try_array_update(type_match, in_place_update_fn, insert_fn);                                            \
}

DEFINE_ARRAY_UPDATE_FUNCTION(u8, BISON_FIELD_TYPE_NUMBER_U8, bison_array_it_update_in_place_u8, bison_insert_u8)
DEFINE_ARRAY_UPDATE_FUNCTION(u16, BISON_FIELD_TYPE_NUMBER_U16, bison_array_it_update_in_place_u16, bison_insert_u16)
DEFINE_ARRAY_UPDATE_FUNCTION(u32, BISON_FIELD_TYPE_NUMBER_U32, bison_array_it_update_in_place_u32, bison_insert_u32)
DEFINE_ARRAY_UPDATE_FUNCTION(u64, BISON_FIELD_TYPE_NUMBER_U64, bison_array_it_update_in_place_u64, bison_insert_u64)
DEFINE_ARRAY_UPDATE_FUNCTION(i8, BISON_FIELD_TYPE_NUMBER_I8, bison_array_it_update_in_place_i8, bison_insert_i8)
DEFINE_ARRAY_UPDATE_FUNCTION(i16, BISON_FIELD_TYPE_NUMBER_I16, bison_array_it_update_in_place_i16, bison_insert_i16)
DEFINE_ARRAY_UPDATE_FUNCTION(i32, BISON_FIELD_TYPE_NUMBER_I32, bison_array_it_update_in_place_i32, bison_insert_i32)
DEFINE_ARRAY_UPDATE_FUNCTION(i64, BISON_FIELD_TYPE_NUMBER_I64, bison_array_it_update_in_place_i64, bison_insert_i64)
DEFINE_ARRAY_UPDATE_FUNCTION(float, BISON_FIELD_TYPE_NUMBER_FLOAT, bison_array_it_update_in_place_float, bison_insert_float)

#define try_update_generic(context, path, array_exec, column_exec)                                                     \
({                                                                                                                     \
        error_if_null(context)                                                                                         \
        error_if_null(path)                                                                                            \
        bool status = false;                                                                                           \
        struct bison_update updater;                                                                                   \
        if (create(&updater, context) && compile_path(&updater, path)) {                                               \
                if (resolve_path(&updater) && path_resolved(&updater)) {                                               \
                switch (updater.path_evaluater.result.container_type) {                                                \
                        case BISON_ARRAY:                                                                              \
                                array_exec;                                                                            \
                                status = true;                                                                         \
                        break;                                                                                         \
                        case BISON_COLUMN: {                                                                           \
                                u32 elem_pos;                                                                          \
                                struct bison_column_it *it = column_iterator(&elem_pos, &updater);                     \
                                column_exec;                                                                           \
                                status = true;                                                                         \
                        } break;                                                                                       \
                        default:                                                                                       \
                                error(&context->original->err, NG5_ERR_INTERNALERR)                                    \
                        }                                                                                              \
                }                                                                                                      \
                drop_path(&updater);                                                                                   \
        }                                                                                                              \
        status;                                                                                                        \
})

#define try_update_value(context, path, value, array_update_fn, column_update_fn)                                      \
        try_update_generic(context, path, (array_update_fn(array_iterator(&updater), value)), (column_update_fn(it, elem_pos, value)) )


static bool create(struct bison_update *updater, struct bison_revise *context)
{
        error_if_null(updater)
        error_if_null(context)

        error_init(&updater->err);
        updater->context = context;

        return true;
}

static bool compile_path(struct bison_update *updater, const char *in)
{
        return bison_dot_path_from_string(&updater->path, in);
}

static bool resolve_path(struct bison_update *updater)
{
        return bison_path_evaluator_begin_mutable(&updater->path_evaluater, &updater->path, updater->context);
}

static bool path_resolved(struct bison_update *updater)
{
        return bison_path_evaluator_has_result(&updater->path_evaluater);
}

static bool drop_path(struct bison_update *updater)
{
        error_if_null(updater)
        return bison_path_evaluator_end(&updater->path_evaluater) && bison_dot_path_drop(&updater->path);
}

static bool column_update_u8(struct bison_column_it *it, u32 pos, u8 value)
{
        ng5_unused(it);
        ng5_unused(pos);
        ng5_unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_u16(struct bison_column_it *it, u32 pos, u16 value)
{
        ng5_unused(it);
        ng5_unused(pos);
        ng5_unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_u32(struct bison_column_it *it, u32 pos, u32 value)
{
        ng5_unused(it);
        ng5_unused(pos);
        ng5_unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_u64(struct bison_column_it *it, u32 pos, u64 value)
{
        ng5_unused(it);
        ng5_unused(pos);
        ng5_unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_i8(struct bison_column_it *it, u32 pos, i8 value)
{
        ng5_unused(it);
        ng5_unused(pos);
        ng5_unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_i16(struct bison_column_it *it, u32 pos, i16 value)
{
        ng5_unused(it);
        ng5_unused(pos);
        ng5_unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_i32(struct bison_column_it *it, u32 pos, i32 value)
{
        ng5_unused(it);
        ng5_unused(pos);
        ng5_unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_i64(struct bison_column_it *it, u32 pos, i64 value)
{
        ng5_unused(it);
        ng5_unused(pos);
        ng5_unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_float(struct bison_column_it *it, u32 pos, float value)
{
        ng5_unused(it);
        ng5_unused(pos);
        ng5_unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}


static inline struct bison_array_it *array_iterator(struct bison_update *updater)
{
        return updater->path_evaluater.result.containers.array.it;
}

static inline struct bison_column_it *column_iterator(u32 *elem_pos, struct bison_update *updater)
{
        *elem_pos = updater->path_evaluater.result.containers.column.elem_pos;
        return updater->path_evaluater.result.containers.column.it;
}

NG5_EXPORT(bool) bison_update_null(struct bison_revise *context, const char *path)
{
        ng5_unused(context);
        ng5_unused(path);
        error_print(NG5_ERR_NOTIMPLEMENTED) // TODO: implement
        return false;
}

NG5_EXPORT(bool) bison_update_true(struct bison_revise *context, const char *path)
{
        ng5_unused(context);
        ng5_unused(path);
        error_print(NG5_ERR_NOTIMPLEMENTED) // TODO: implement
        return false;
}

NG5_EXPORT(bool) bison_update_false(struct bison_revise *context, const char *path)
{
        ng5_unused(context);
        ng5_unused(path);
        error_print(NG5_ERR_NOTIMPLEMENTED) // TODO: implement
        return false;
}

NG5_EXPORT(bool) bison_update_u8(struct bison_revise *context, const char *path, u8 value)
{
        return try_update_value(context, path, value, array_update_u8, column_update_u8);
}

NG5_EXPORT(bool) bison_update_u16(struct bison_revise *context, const char *path, u16 value)
{
        return try_update_value(context, path, value, array_update_u16, column_update_u16);
}

NG5_EXPORT(bool) bison_update_u32(struct bison_revise *context, const char *path, u32 value)
{
        return try_update_value(context, path, value, array_update_u32, column_update_u32);
}

NG5_EXPORT(bool) bison_update_u64(struct bison_revise *context, const char *path, u64 value)
{
        return try_update_value(context, path, value, array_update_u64, column_update_u64);
}

NG5_EXPORT(bool) bison_update_i8(struct bison_revise *context, const char *path, i8 value)
{
        return try_update_value(context, path, value, array_update_i8, column_update_i8);
}

NG5_EXPORT(bool) bison_update_i16(struct bison_revise *context, const char *path, i16 value)
{
        return try_update_value(context, path, value, array_update_i16, column_update_i16);
}

NG5_EXPORT(bool) bison_update_i32(struct bison_revise *context, const char *path, i32 value)
{
        return try_update_value(context, path, value, array_update_i32, column_update_i32);
}

NG5_EXPORT(bool) bison_update_i64(struct bison_revise *context, const char *path, i64 value)
{
        return try_update_value(context, path, value, array_update_i64, column_update_i64);
}

NG5_EXPORT(bool) bison_update_float(struct bison_revise *context, const char *path, float value)
{
        return try_update_value(context, path, value, array_update_float, column_update_float);
}
