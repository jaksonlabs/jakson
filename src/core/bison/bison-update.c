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

#include <core/bison/bison-path.h>
#include "core/bison/bison-path.h"
#include "core/bison/bison-update.h"
#include "core/bison/bison-insert.h"
#include "core/bison/bison-revise.h"

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
        if (create(&updater, context, path)) {                                                                         \
                if (resolve_path(&updater) && path_resolved(&updater)) {                                               \
                                                                                                                       \
                        switch (updater.path_evaluater.result.container_type) {                                        \
                        case BISON_ARRAY:                                                                              \
                                array_exec;                                                                            \
                                status = true;                                                                         \
                                break;                                                                                 \
                        case BISON_COLUMN: {                                                                           \
                                u32 elem_pos;                                                                          \
                                struct bison_column_it *it = column_iterator(&elem_pos, &updater);                     \
                                column_exec;                                                                           \
                                status = true;                                                                         \
                        } break;                                                                                       \
                        default:                                                                                       \
                        error(&context->original->err, NG5_ERR_INTERNALERR)                                            \
                        }                                                                                              \
                }                                                                                                      \
                drop_path_evaluator(&updater);                                                                         \
        }                                                                                                              \
        status;                                                                                                        \
})

#define try_update_value(context, path, value, array_update_fn, column_update_fn)                                      \
        try_update_generic(context, path, (array_update_fn(array_iterator(&updater), value)),                          \
                           (column_update_fn(it, elem_pos, value)) )

#define try_update(context, path, array_update_fn, column_update_fn)                                                   \
        try_update_generic(context, path, (array_update_fn(array_iterator(&updater))), (column_update_fn(it, elem_pos)))


static bool create(struct bison_update *updater, struct bison_revise *context, const struct bison_dot_path *path)
{
        error_if_null(updater)
        error_if_null(context)
        error_if_null(path)

        error_init(&updater->err);
        updater->context = context;
        updater->path = path;

        return true;
}

static bool compile_path(struct bison_dot_path *out, const char *in)
{
        return bison_dot_path_from_string(out, in);
}

static bool resolve_path(struct bison_update *updater)
{
        return bison_path_evaluator_begin_mutable(&updater->path_evaluater, updater->path, updater->context);
}

static bool path_resolved(struct bison_update *updater)
{
        return bison_path_evaluator_has_result(&updater->path_evaluater);
}

static bool drop_path_evaluator(struct bison_update *updater)
{
        error_if_null(updater)
        return bison_path_evaluator_end(&updater->path_evaluater);
}

static bool column_update_u8(struct bison_column_it *it, u32 pos, u8 value)
{
        unused(it);
        unused(pos);
        unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_u16(struct bison_column_it *it, u32 pos, u16 value)
{
        unused(it);
        unused(pos);
        unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_u32(struct bison_column_it *it, u32 pos, u32 value)
{
        unused(it);
        unused(pos);
        unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_u64(struct bison_column_it *it, u32 pos, u64 value)
{
        unused(it);
        unused(pos);
        unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_i8(struct bison_column_it *it, u32 pos, i8 value)
{
        unused(it);
        unused(pos);
        unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_i16(struct bison_column_it *it, u32 pos, i16 value)
{
        unused(it);
        unused(pos);
        unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_i32(struct bison_column_it *it, u32 pos, i32 value)
{
        unused(it);
        unused(pos);
        unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_i64(struct bison_column_it *it, u32 pos, i64 value)
{
        unused(it);
        unused(pos);
        unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_float(struct bison_column_it *it, u32 pos, float value)
{
        unused(it);
        unused(pos);
        unused(value);
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

#define compile_path_and_delegate(context, path, func)                                                                 \
({                                                                                                                     \
        error_if_null(context)                                                                                         \
        error_if_null(path)                                                                                            \
                                                                                                                       \
        struct bison_dot_path compiled_path;                                                                           \
        bool status;                                                                                                   \
        if (compile_path(&compiled_path, path)) {                                                                      \
                status = func(context, &compiled_path);                                                                \
                bison_dot_path_drop(&compiled_path);                                                                   \
        } else {                                                                                                       \
                error(&context->err, NG5_ERR_DOT_PATH_PARSERR)                                                         \
                status = false;                                                                                        \
        }                                                                                                              \
        status;                                                                                                        \
})

#define compile_path_and_delegate_wargs(context, path, func, ...)                                                      \
({                                                                                                                     \
        error_if_null(context)                                                                                         \
        error_if_null(path)                                                                                            \
                                                                                                                       \
        struct bison_dot_path compiled_path;                                                                           \
        bool status;                                                                                                   \
        if (compile_path(&compiled_path, path)) {                                                                      \
                status = func(context, &compiled_path, __VA_ARGS__);                                                   \
                bison_dot_path_drop(&compiled_path);                                                                   \
        } else {                                                                                                       \
                error(&context->err, NG5_ERR_DOT_PATH_PARSERR)                                                         \
                status = false;                                                                                        \
        }                                                                                                              \
        status;                                                                                                        \
})


NG5_EXPORT(bool) bison_update_set_null(struct bison_revise *context, const char *path)
{
        return compile_path_and_delegate(context, path, bison_update_set_null_compiled);
}

NG5_EXPORT(bool) bison_update_set_true(struct bison_revise *context, const char *path)
{
        return compile_path_and_delegate(context, path, bison_update_set_true_compiled);
}

NG5_EXPORT(bool) bison_update_set_false(struct bison_revise *context, const char *path)
{
        return compile_path_and_delegate(context, path, bison_update_set_false_compiled);
}

NG5_EXPORT(bool) bison_update_set_u8(struct bison_revise *context, const char *path, u8 value)
{
        return compile_path_and_delegate_wargs(context, path, bison_update_set_u8_compiled, value);
}

NG5_EXPORT(bool) bison_update_set_u16(struct bison_revise *context, const char *path, u16 value)
{
        return compile_path_and_delegate_wargs(context, path, bison_update_set_u16_compiled, value);
}

NG5_EXPORT(bool) bison_update_set_u32(struct bison_revise *context, const char *path, u32 value)
{
        return compile_path_and_delegate_wargs(context, path, bison_update_set_u32_compiled, value);
}

NG5_EXPORT(bool) bison_update_set_u64(struct bison_revise *context, const char *path, u64 value)
{
        return compile_path_and_delegate_wargs(context, path, bison_update_set_u64_compiled, value);
}

NG5_EXPORT(bool) bison_update_set_i8(struct bison_revise *context, const char *path, i8 value)
{
        return compile_path_and_delegate_wargs(context, path, bison_update_set_i8_compiled, value);
}

NG5_EXPORT(bool) bison_update_set_i16(struct bison_revise *context, const char *path, i16 value)
{
        return compile_path_and_delegate_wargs(context, path, bison_update_set_i16_compiled, value);
}

NG5_EXPORT(bool) bison_update_set_i32(struct bison_revise *context, const char *path, i32 value)
{
        return compile_path_and_delegate_wargs(context, path, bison_update_set_i32_compiled, value);
}

NG5_EXPORT(bool) bison_update_set_i64(struct bison_revise *context, const char *path, i64 value)
{
        return compile_path_and_delegate_wargs(context, path, bison_update_set_i64_compiled, value);
}

NG5_EXPORT(bool) bison_update_set_float(struct bison_revise *context, const char *path, float value)
{
        return compile_path_and_delegate_wargs(context, path, bison_update_set_float_compiled, value);
}

NG5_EXPORT(bool) bison_update_set_unsigned(struct bison_revise *context, const char *path, u64 value)
{
        if (value <= BISON_U8_MAX) {
                return bison_update_set_u8(context, path, (u8) value);
        } else if (value <= BISON_U16_MAX) {
                return bison_update_set_u16(context, path, (u16) value);
        } else if (value <= BISON_U32_MAX) {
                return bison_update_set_u32(context, path, (u32) value);
        } else if (value <= BISON_U64_MAX) {
                return bison_update_set_u64(context, path, (u64) value);
        } else {
                error(&context->err, NG5_ERR_INTERNALERR);
                return false;
        }
}

NG5_EXPORT(bool) bison_update_set_signed(struct bison_revise *context, const char *path, i64 value)
{
        if (value >= BISON_I8_MIN && value <= BISON_I8_MAX) {
                return bison_update_set_i8(context, path, (i8) value);
        } else if (value >= BISON_I16_MIN && value <= BISON_I16_MAX) {
                return bison_update_set_i16(context, path, (i16) value);
        } else if (value >= BISON_I32_MIN && value <= BISON_I32_MAX) {
                return bison_update_set_i32(context, path, (i32) value);
        } else if (value >= BISON_I64_MIN && value <= BISON_I64_MAX) {
                return bison_update_set_i64(context, path, (i64) value);
        } else {
                error(&context->err, NG5_ERR_INTERNALERR);
                return false;
        }
}

NG5_EXPORT(bool) bison_update_set_string(struct bison_revise *context, const char *path, const char *value)
{
        // TODO: Implement
        unused(context);
        unused(path);
        unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED)
        return false;
}

NG5_EXPORT(bool) bison_update_set_binary(struct bison_revise *context, const char *path, const void *value, size_t nbytes,
        const char *file_ext, const char *user_type)
{
        // TODO: Implement
        unused(context);
        unused(value);
        unused(nbytes);
        unused(file_ext);
        unused(user_type);
        unused(path);
        error_print(NG5_ERR_NOTIMPLEMENTED)
        return false;
}

NG5_EXPORT(struct bison_insert *) bison_update_set_array_begin(struct bison_revise *context, const char *path,
        struct bison_insert_array_state *state_out, u64 array_capacity)
{
        // TODO: Implement
        unused(context);
        unused(state_out);
        unused(array_capacity);
        unused(path);
        error_print(NG5_ERR_NOTIMPLEMENTED)
        return false;
}

NG5_EXPORT(bool) bison_update_set_array_end(struct bison_insert_array_state *state_in)
{
        // TODO: Implement
        unused(state_in);
        error_print(NG5_ERR_NOTIMPLEMENTED)
        return false;
}

NG5_EXPORT(struct bison_insert *) bison_update_set_column_begin(struct bison_revise *context, const char *path,
        struct bison_insert_column_state *state_out, enum bison_field_type type, u64 column_capacity)
{
        // TODO: Implement
        unused(state_out);
        unused(context);
        unused(type);
        unused(column_capacity);
        unused(path);
        error_print(NG5_ERR_NOTIMPLEMENTED)
        return false;
}

NG5_EXPORT(bool) bison_update_set_column_end(struct bison_insert_column_state *state_in)
{
        // TODO: Implement
        unused(state_in);
        error_print(NG5_ERR_NOTIMPLEMENTED)
        return false;
}

// ---------------------------------------------------------------------------------------------------------------------

NG5_EXPORT(bool) bison_update_set_null_compiled(struct bison_revise *context, const struct bison_dot_path *path)
{
        return try_update(context, path, bison_array_it_update_in_place_null, bison_column_it_update_set_null);
}

NG5_EXPORT(bool) bison_update_set_true_compiled(struct bison_revise *context, const struct bison_dot_path *path)
{
        return try_update(context, path, bison_array_it_update_in_place_true, bison_column_it_update_set_true);
}

NG5_EXPORT(bool) bison_update_set_false_compiled(struct bison_revise *context, const struct bison_dot_path *path)
{
        return try_update(context, path, bison_array_it_update_in_place_false, bison_column_it_update_set_false);
}

NG5_EXPORT(bool) bison_update_set_u8_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        u8 value)
{
        return try_update_value(context, path, value, array_update_u8, column_update_u8);
}

NG5_EXPORT(bool) bison_update_set_u16_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        u16 value)
{
        return try_update_value(context, path, value, array_update_u16, column_update_u16);
}

NG5_EXPORT(bool) bison_update_set_u32_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        u32 value)
{
        return try_update_value(context, path, value, array_update_u32, column_update_u32);
}

NG5_EXPORT(bool) bison_update_set_u64_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        u64 value)
{
        return try_update_value(context, path, value, array_update_u64, column_update_u64);
}

NG5_EXPORT(bool) bison_update_set_i8_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        i8 value)
{
        return try_update_value(context, path, value, array_update_i8, column_update_i8);
}

NG5_EXPORT(bool) bison_update_set_i16_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        i16 value)
{
        return try_update_value(context, path, value, array_update_i16, column_update_i16);
}

NG5_EXPORT(bool) bison_update_set_i32_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        i32 value)
{
        return try_update_value(context, path, value, array_update_i32, column_update_i32);
}

NG5_EXPORT(bool) bison_update_set_i64_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        i64 value)
{
        return try_update_value(context, path, value, array_update_i64, column_update_i64);
}

NG5_EXPORT(bool) bison_update_set_float_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        float value)
{
        return try_update_value(context, path, value, array_update_float, column_update_float);
}

NG5_EXPORT(bool) bison_update_set_unsigned_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        u64 value)
{
        if (value <= BISON_U8_MAX) {
                return bison_update_set_u8_compiled(context, path, (u8) value);
        } else if (value <= BISON_U16_MAX) {
                return bison_update_set_u16_compiled(context, path, (u16) value);
        } else if (value <= BISON_U32_MAX) {
                return bison_update_set_u32_compiled(context, path, (u32) value);
        } else if (value <= BISON_U64_MAX) {
                return bison_update_set_u64_compiled(context, path, (u64) value);
        } else {
                error(&context->err, NG5_ERR_INTERNALERR);
                return false;
        }
}

NG5_EXPORT(bool) bison_update_set_signed_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        i64 value)
{
        if (value >= BISON_I8_MIN && value <= BISON_I8_MAX) {
                return bison_update_set_i8_compiled(context, path, (i8) value);
        } else if (value >= BISON_I16_MIN && value <= BISON_I16_MAX) {
                return bison_update_set_i16_compiled(context, path, (i16) value);
        } else if (value >= BISON_I32_MIN && value <= BISON_I32_MAX) {
                return bison_update_set_i32_compiled(context, path, (i32) value);
        } else if (value >= BISON_I64_MIN && value <= BISON_I64_MAX) {
                return bison_update_set_i64_compiled(context, path, (i64) value);
        } else {
                error(&context->err, NG5_ERR_INTERNALERR);
                return false;
        }
}

NG5_EXPORT(bool) bison_update_set_string_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        const char *value)
{
        // TODO: Implement
        unused(context);
        unused(path);
        unused(value);
        error_print(NG5_ERR_NOTIMPLEMENTED)
        return false;
}

NG5_EXPORT(bool) bison_update_set_binary_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        const void *value, size_t nbytes, const char *file_ext, const char *user_type)
{
        // TODO: Implement
        unused(context);
        unused(value);
        unused(nbytes);
        unused(file_ext);
        unused(user_type);
        unused(path);
        error_print(NG5_ERR_NOTIMPLEMENTED)
        return false;
}

NG5_EXPORT(struct bison_insert *) bison_update_set_array_begin_compiled(struct bison_revise *context,
        const struct bison_dot_path *path, struct bison_insert_array_state *state_out, u64 array_capacity)
{
        // TODO: Implement
        unused(context);
        unused(state_out);
        unused(array_capacity);
        unused(path);
        error_print(NG5_ERR_NOTIMPLEMENTED)
        return false;
}

NG5_EXPORT(bool) bison_update_set_array_end_compiled(struct bison_insert_array_state *state_in)
{
        // TODO: Implement
        unused(state_in);
        error_print(NG5_ERR_NOTIMPLEMENTED)
        return false;
}

NG5_EXPORT(struct bison_insert *) bison_update_set_column_begin_compiled(struct bison_revise *context,
        const struct bison_dot_path *path, struct bison_insert_column_state *state_out, enum bison_field_type type,
        u64 column_capacity)
{
        // TODO: Implement
        unused(state_out);
        unused(context);
        unused(type);
        unused(column_capacity);
        unused(path);
        error_print(NG5_ERR_NOTIMPLEMENTED)
        return false;
}

NG5_EXPORT(bool) bison_update_set_column_end_compiled(struct bison_insert_column_state *state_in)
{
        // TODO: Implement
        unused(state_in);
        error_print(NG5_ERR_NOTIMPLEMENTED)
        return false;
}

// ---------------------------------------------------------------------------------------------------------------------

#define revision_context_delegate_func(rev_doc, doc, func, ...)                                                        \
({                                                                                                                     \
        struct bison_revise revise;                                                                                    \
        bison_revise_begin(&revise, rev_doc, doc);                                                                     \
        bool status = func(&revise, __VA_ARGS__);                                                                      \
        bison_revise_end(&revise);                                                                                     \
        status;                                                                                                        \
})

NG5_EXPORT(bool) bison_update_one_set_null(const char *dot_path, struct bison *rev_doc, struct bison *doc)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_null, dot_path);
}

NG5_EXPORT(bool) bison_update_one_set_true(const char *dot_path, struct bison *rev_doc, struct bison *doc)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_true, dot_path);
}

NG5_EXPORT(bool) bison_update_one_set_false(const char *dot_path, struct bison *rev_doc, struct bison *doc)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_false, dot_path);
}

NG5_EXPORT(bool) bison_update_one_set_u8(const char *dot_path, struct bison *rev_doc, struct bison *doc, u8 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_u8, dot_path, value);
}

NG5_EXPORT(bool) bison_update_one_set_u16(const char *dot_path, struct bison *rev_doc, struct bison *doc, u16 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_u16, dot_path, value);
}

NG5_EXPORT(bool) bison_update_one_set_u32(const char *dot_path, struct bison *rev_doc, struct bison *doc, u32 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_u32, dot_path, value);
}

NG5_EXPORT(bool) bison_update_one_set_u64(const char *dot_path, struct bison *rev_doc, struct bison *doc, u64 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_u64, dot_path, value);
}

NG5_EXPORT(bool) bison_update_one_set_i8(const char *dot_path, struct bison *rev_doc, struct bison *doc, i8 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_i8, dot_path, value);
}

NG5_EXPORT(bool) bison_update_one_set_i16(const char *dot_path, struct bison *rev_doc, struct bison *doc, i16 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_i16, dot_path, value);
}

NG5_EXPORT(bool) bison_update_one_set_i32(const char *dot_path, struct bison *rev_doc, struct bison *doc, i32 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_i32, dot_path, value);
}

NG5_EXPORT(bool) bison_update_one_set_i64(const char *dot_path, struct bison *rev_doc, struct bison *doc, i64 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_i64, dot_path, value);
}

NG5_EXPORT(bool) bison_update_one_set_float(const char *dot_path, struct bison *rev_doc, struct bison *doc,
        float value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_float, dot_path, value);
}

NG5_EXPORT(bool) bison_update_one_set_unsigned(const char *dot_path, struct bison *rev_doc, struct bison *doc,
        u64 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_unsigned, dot_path, value);
}

NG5_EXPORT(bool) bison_update_one_set_signed(const char *dot_path, struct bison *rev_doc, struct bison *doc, i64 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_signed, dot_path, value);
}

NG5_EXPORT(bool) bison_update_one_set_string(const char *dot_path, struct bison *rev_doc, struct bison *doc,
        const char *value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_string, dot_path, value);
}

NG5_EXPORT(bool) bison_update_one_set_binary(const char *dot_path, struct bison *rev_doc, struct bison *doc,
        const void *value, size_t nbytes, const char *file_ext, const char *user_type)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_binary, dot_path, value, nbytes,
                file_ext, user_type);
}

NG5_EXPORT(struct bison_insert *) bison_update_one_set_array_begin(struct bison_insert_array_state *state_out,
        const char *dot_path, struct bison *rev_doc, struct bison *doc, u64 array_capacity)
{
        struct bison_revise revise;
        bison_revise_begin(&revise, rev_doc, doc);
        struct bison_insert *result = bison_update_set_array_begin(&revise, dot_path, state_out, array_capacity);
        // ... TODO: add revision to context
        return result;
}

NG5_EXPORT(bool) bison_update_one_set_array_end(struct bison_insert_array_state *state_in)
{
        bool status = bison_update_set_array_end(state_in);
        // ... TODO: drop revision from context
        return status;
}

NG5_EXPORT(struct bison_insert *) bison_update_one_set_column_begin(struct bison_insert_column_state *state_out,
        const char *dot_path, struct bison *rev_doc, struct bison *doc, enum bison_field_type type,
        u64 column_capacity)
{
        struct bison_revise revise;
        bison_revise_begin(&revise, rev_doc, doc);
        struct bison_insert *result = bison_update_set_column_begin(&revise, dot_path, state_out, type, column_capacity);
        // ... TODO: add revision to context
        return result;
}

NG5_EXPORT(bool) bison_update_one_set_column_end(struct bison_insert_column_state *state_in)
{
        bool status = bison_update_set_column_end(state_in);
        // ... TODO: drop revision from context
        return status;
}

// ---------------------------------------------------------------------------------------------------------------------

NG5_EXPORT(bool) bison_update_one_set_null_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_null_compiled, path);
}

NG5_EXPORT(bool) bison_update_one_set_true_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_true_compiled, path);
}

NG5_EXPORT(bool) bison_update_one_set_false_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_false_compiled, path);
}

NG5_EXPORT(bool) bison_update_one_set_u8_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, u8 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_u8_compiled, path, value);
}

NG5_EXPORT(bool) bison_update_one_set_u16_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, u16 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_u16_compiled, path, value);
}

NG5_EXPORT(bool) bison_update_one_set_u32_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, u32 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_u32_compiled, path, value);
}

NG5_EXPORT(bool) bison_update_one_set_u64_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, u64 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_u64_compiled, path, value);
}

NG5_EXPORT(bool) bison_update_one_set_i8_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, i8 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_i8_compiled, path, value);
}

NG5_EXPORT(bool) bison_update_one_set_i16_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, i16 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_i16_compiled, path, value);
}

NG5_EXPORT(bool) bison_update_one_set_i32_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, i32 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_i32_compiled, path, value);
}

NG5_EXPORT(bool) bison_update_one_set_i64_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, i64 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_i64_compiled, path, value);
}

NG5_EXPORT(bool) bison_update_one_set_float_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, float value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_float_compiled, path, value);
}

NG5_EXPORT(bool) bison_update_one_set_unsigned_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, u64 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_unsigned_compiled, path, value);
}

NG5_EXPORT(bool) bison_update_one_set_signed_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, i64 value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_signed_compiled, path, value);
}

NG5_EXPORT(bool) bison_update_one_set_string_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, const char *value)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_string_compiled, path, value);
}

NG5_EXPORT(bool) bison_update_one_set_binary_compiled(const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, const void *value, size_t nbytes, const char *file_ext, const char *user_type)
{
        return revision_context_delegate_func(rev_doc, doc, bison_update_set_binary_compiled, path, value, nbytes,
                file_ext, user_type);
}

NG5_EXPORT(struct bison_insert *) bison_update_one_set_array_begin_compiled(struct bison_insert_array_state *state_out,
        const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc, u64 array_capacity)
{
        struct bison_revise revise;
        bison_revise_begin(&revise, rev_doc, doc);
        struct bison_insert *result = bison_update_set_array_begin_compiled(&revise, path, state_out, array_capacity);
        // ... TODO: add revision to context
        return result;
}

NG5_EXPORT(bool) bison_update_one_set_array_end_compiled(struct bison_insert_array_state *state_in)
{
        bool status = bison_update_set_array_end_compiled(state_in);
        // ... TODO: drop revision from context
        return status;
}

NG5_EXPORT(struct bison_insert *) bison_update_one_set_column_begin_compiled(
        struct bison_insert_column_state *state_out, const struct bison_dot_path *path, struct bison *rev_doc,
        struct bison *doc, enum bison_field_type type, u64 column_capacity)
{
        struct bison_revise revise;
        bison_revise_begin(&revise, rev_doc, doc);
        struct bison_insert *result = bison_update_set_column_begin_compiled(&revise, path, state_out, type, column_capacity);
        // ... TODO: add revision to context
        return result;
}

NG5_EXPORT(bool) bison_update_one_set_column_end_compiled(struct bison_insert_column_state *state_in)
{
        bool status = bison_update_set_column_end_compiled(state_in);
        // ... TODO: drop revision from context
        return status;
}