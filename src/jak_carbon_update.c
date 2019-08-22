/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#include <jak_carbon_path.h>
#include <jak_carbon_path.h>
#include <jak_carbon_update.h>
#include <jak_carbon_insert.h>
#include <jak_carbon_revise.h>
#include <jak_utils_numbers.h>

#define try_array_update(type_match, in_place_update_fn, insert_fn)                                                    \
({                                                                                                                     \
        enum carbon_field_type type_is = 0;                                                                            \
        carbon_array_it_field_type(&type_is, it);                                                                      \
        bool status = false;                                                                                           \
        switch (type_is) {                                                                                             \
                case type_match:                                                                                       \
                        status = in_place_update_fn(it, value);                                                        \
                break;                                                                                                 \
                default: {                                                                                             \
                        struct jak_carbon_insert inserter;                                                                 \
                        carbon_array_it_remove(it);                                                                    \
                        carbon_array_it_next(it);                                                                      \
                        carbon_array_it_insert_begin(&inserter, it);                                                   \
                        status = insert_fn(&inserter, value);                                                          \
                        carbon_array_it_insert_end(&inserter);                                                         \
                break;                                                                                                 \
                }                                                                                                      \
        }                                                                                                              \
        status;                                                                                                        \
})

#define DEFINE_ARRAY_UPDATE_FUNCTION(type_name, type_match, in_place_update_fn, insert_fn)                             \
static bool array_update_##type_name(struct jak_carbon_array_it *it, jak_##type_name value)                                       \
{                                                                                                                      \
        return try_array_update(type_match, in_place_update_fn, insert_fn);                                            \
}

DEFINE_ARRAY_UPDATE_FUNCTION(u8, CARBON_JAK_FIELD_TYPE_NUMBER_U8, carbon_array_it_update_in_place_u8, carbon_insert_u8)

DEFINE_ARRAY_UPDATE_FUNCTION(u16, CARBON_JAK_FIELD_TYPE_NUMBER_U16, carbon_array_it_update_in_place_u16, carbon_insert_u16)

DEFINE_ARRAY_UPDATE_FUNCTION(u32, CARBON_JAK_FIELD_TYPE_NUMBER_U32, carbon_array_it_update_in_place_u32, carbon_insert_u32)

DEFINE_ARRAY_UPDATE_FUNCTION(u64, CARBON_JAK_FIELD_TYPE_NUMBER_U64, carbon_array_it_update_in_place_u64, carbon_insert_u64)

DEFINE_ARRAY_UPDATE_FUNCTION(i8, CARBON_JAK_FIELD_TYPE_NUMBER_I8, carbon_array_it_update_in_place_i8, carbon_insert_i8)

DEFINE_ARRAY_UPDATE_FUNCTION(i16, CARBON_JAK_FIELD_TYPE_NUMBER_I16, carbon_array_it_update_in_place_i16, carbon_insert_i16)

DEFINE_ARRAY_UPDATE_FUNCTION(i32, CARBON_JAK_FIELD_TYPE_NUMBER_I32, carbon_array_it_update_in_place_i32, carbon_insert_i32)

DEFINE_ARRAY_UPDATE_FUNCTION(i64, CARBON_JAK_FIELD_TYPE_NUMBER_I64, carbon_array_it_update_in_place_i64, carbon_insert_i64)

DEFINE_ARRAY_UPDATE_FUNCTION(float, CARBON_JAK_FIELD_TYPE_NUMBER_FLOAT, carbon_array_it_update_in_place_float,
                             carbon_insert_float)

#define try_update_generic(context, path, array_exec, column_exec)                                                     \
({                                                                                                                     \
        error_if_null(context)                                                                                         \
        error_if_null(path)                                                                                            \
        bool status = false;                                                                                           \
        struct jak_carbon_update updater;                                                                                   \
        if (create(&updater, context, path)) {                                                                         \
                if (resolve_path(&updater) && path_resolved(&updater)) {                                               \
                                                                                                                       \
                        switch (updater.path_evaluater.result.container_type) {                                        \
                        case CARBON_ARRAY:                                                                              \
                                array_exec;                                                                            \
                                status = true;                                                                         \
                                break;                                                                                 \
                        case CARBON_COLUMN: {                                                                           \
                                jak_u32 elem_pos;                                                                          \
                                struct jak_carbon_column_it *it = column_iterator(&elem_pos, &updater);                     \
                                column_exec;                                                                           \
                                status = true;                                                                         \
                        } break;                                                                                       \
                        default:                                                                                       \
                        error(&context->original->err, JAK_ERR_INTERNALERR)                                            \
                        }                                                                                              \
                }                                                                                                      \
                carbon_path_evaluator_end(&updater.path_evaluater);                                                    \
                }                                                                                                              \
        status;                                                                                                        \
})

#define try_update_value(context, path, value, array_update_fn, column_update_fn)                                      \
        try_update_generic(context, path, (array_update_fn(array_iterator(&updater), value)),                          \
                           (column_update_fn(it, elem_pos, value)) )

#define try_update(context, path, array_update_fn, column_update_fn)                                                   \
        try_update_generic(context, path, (array_update_fn(array_iterator(&updater))), (column_update_fn(it, elem_pos)))


static bool create(struct jak_carbon_update *updater, struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path)
{
        error_if_null(updater)
        error_if_null(context)
        error_if_null(path)

        error_init(&updater->err);
        updater->context = context;
        updater->path = path;

        return true;
}

static bool compile_path(struct jak_carbon_dot_path *out, const char *in)
{
        return carbon_dot_path_from_string(out, in);
}

static bool resolve_path(struct jak_carbon_update *updater)
{
        return carbon_path_evaluator_begin_mutable(&updater->path_evaluater, updater->path, updater->context);
}

static bool path_resolved(struct jak_carbon_update *updater)
{
        return carbon_path_evaluator_has_result(&updater->path_evaluater);
}

static bool column_update_u8(struct jak_carbon_column_it *it, jak_u32 pos, jak_u8 value)
{
        JAK_UNUSED(it);
        JAK_UNUSED(pos);
        JAK_UNUSED(value);
        error_print(JAK_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_u16(struct jak_carbon_column_it *it, jak_u32 pos, jak_u16 value)
{
        JAK_UNUSED(it);
        JAK_UNUSED(pos);
        JAK_UNUSED(value);
        error_print(JAK_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_u32(struct jak_carbon_column_it *it, jak_u32 pos, jak_u32 value)
{
        JAK_UNUSED(it);
        JAK_UNUSED(pos);
        JAK_UNUSED(value);
        error_print(JAK_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_u64(struct jak_carbon_column_it *it, jak_u32 pos, jak_u64 value)
{
        JAK_UNUSED(it);
        JAK_UNUSED(pos);
        JAK_UNUSED(value);
        error_print(JAK_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_i8(struct jak_carbon_column_it *it, jak_u32 pos, jak_i8 value)
{
        JAK_UNUSED(it);
        JAK_UNUSED(pos);
        JAK_UNUSED(value);
        error_print(JAK_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_i16(struct jak_carbon_column_it *it, jak_u32 pos, jak_i16 value)
{
        JAK_UNUSED(it);
        JAK_UNUSED(pos);
        JAK_UNUSED(value);
        error_print(JAK_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_i32(struct jak_carbon_column_it *it, jak_u32 pos, jak_i32 value)
{
        JAK_UNUSED(it);
        JAK_UNUSED(pos);
        JAK_UNUSED(value);
        error_print(JAK_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_i64(struct jak_carbon_column_it *it, jak_u32 pos, jak_i64 value)
{
        JAK_UNUSED(it);
        JAK_UNUSED(pos);
        JAK_UNUSED(value);
        error_print(JAK_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}

static bool column_update_float(struct jak_carbon_column_it *it, jak_u32 pos, float value)
{
        JAK_UNUSED(it);
        JAK_UNUSED(pos);
        JAK_UNUSED(value);
        error_print(JAK_ERR_NOTIMPLEMENTED);    // TODO: Implement
        return false;
}


static inline struct jak_carbon_array_it *array_iterator(struct jak_carbon_update *updater)
{
        return &updater->path_evaluater.result.containers.array.it;
}

static inline struct jak_carbon_column_it *column_iterator(jak_u32 *elem_pos, struct jak_carbon_update *updater)
{
        *elem_pos = updater->path_evaluater.result.containers.column.elem_pos;
        return &updater->path_evaluater.result.containers.column.it;
}

#define compile_path_and_delegate(context, path, func)                                                                 \
({                                                                                                                     \
        error_if_null(context)                                                                                         \
        error_if_null(path)                                                                                            \
                                                                                                                       \
        struct jak_carbon_dot_path compiled_path;                                                                           \
        bool status;                                                                                                   \
        if (compile_path(&compiled_path, path)) {                                                                      \
                status = func(context, &compiled_path);                                                                \
                carbon_dot_path_drop(&compiled_path);                                                                   \
        } else {                                                                                                       \
                error(&context->err, JAK_ERR_DOT_PATH_PARSERR)                                                         \
                status = false;                                                                                        \
        }                                                                                                              \
        status;                                                                                                        \
})

#define compile_path_and_delegate_wargs(context, path, func, ...)                                                      \
({                                                                                                                     \
        error_if_null(context)                                                                                         \
        error_if_null(path)                                                                                            \
                                                                                                                       \
        struct jak_carbon_dot_path compiled_path;                                                                           \
        bool status;                                                                                                   \
        if (compile_path(&compiled_path, path)) {                                                                      \
                status = func(context, &compiled_path, __VA_ARGS__);                                                   \
                carbon_dot_path_drop(&compiled_path);                                                                   \
        } else {                                                                                                       \
                error(&context->err, JAK_ERR_DOT_PATH_PARSERR)                                                         \
                status = false;                                                                                        \
        }                                                                                                              \
        status;                                                                                                        \
})


bool carbon_update_set_null(struct jak_carbon_revise *context, const char *path)
{
        return compile_path_and_delegate(context, path, carbon_update_set_null_compiled);
}

bool carbon_update_set_true(struct jak_carbon_revise *context, const char *path)
{
        return compile_path_and_delegate(context, path, carbon_update_set_true_compiled);
}

bool carbon_update_set_false(struct jak_carbon_revise *context, const char *path)
{
        return compile_path_and_delegate(context, path, carbon_update_set_false_compiled);
}

bool carbon_update_set_u8(struct jak_carbon_revise *context, const char *path, jak_u8 value)
{
        return compile_path_and_delegate_wargs(context, path, carbon_update_set_u8_compiled, value);
}

bool carbon_update_set_u16(struct jak_carbon_revise *context, const char *path, jak_u16 value)
{
        return compile_path_and_delegate_wargs(context, path, carbon_update_set_u16_compiled, value);
}

bool carbon_update_set_u32(struct jak_carbon_revise *context, const char *path, jak_u32 value)
{
        return compile_path_and_delegate_wargs(context, path, carbon_update_set_u32_compiled, value);
}

bool carbon_update_set_u64(struct jak_carbon_revise *context, const char *path, jak_u64 value)
{
        return compile_path_and_delegate_wargs(context, path, carbon_update_set_u64_compiled, value);
}

bool carbon_update_set_i8(struct jak_carbon_revise *context, const char *path, jak_i8 value)
{
        return compile_path_and_delegate_wargs(context, path, carbon_update_set_i8_compiled, value);
}

bool carbon_update_set_i16(struct jak_carbon_revise *context, const char *path, jak_i16 value)
{
        return compile_path_and_delegate_wargs(context, path, carbon_update_set_i16_compiled, value);
}

bool carbon_update_set_i32(struct jak_carbon_revise *context, const char *path, jak_i32 value)
{
        return compile_path_and_delegate_wargs(context, path, carbon_update_set_i32_compiled, value);
}

bool carbon_update_set_i64(struct jak_carbon_revise *context, const char *path, jak_i64 value)
{
        return compile_path_and_delegate_wargs(context, path, carbon_update_set_i64_compiled, value);
}

bool carbon_update_set_float(struct jak_carbon_revise *context, const char *path, float value)
{
        return compile_path_and_delegate_wargs(context, path, carbon_update_set_float_compiled, value);
}

bool carbon_update_set_unsigned(struct jak_carbon_revise *context, const char *path, jak_u64 value)
{
        switch (number_min_type_unsigned(value)) {
                case NUMBER_U8:
                        return carbon_update_set_u8(context, path, (jak_u8) value);
                case NUMBER_U16:
                        return carbon_update_set_u16(context, path, (jak_u16) value);
                case NUMBER_U32:
                        return carbon_update_set_u32(context, path, (jak_u32) value);
                case NUMBER_U64:
                        return carbon_update_set_u64(context, path, (jak_u64) value);
                default: error(&context->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_update_set_signed(struct jak_carbon_revise *context, const char *path, jak_i64 value)
{
        switch (number_min_type_signed(value)) {
                case NUMBER_I8:
                        return carbon_update_set_i8(context, path, (jak_i8) value);
                case NUMBER_I16:
                        return carbon_update_set_i16(context, path, (jak_i16) value);
                case NUMBER_I32:
                        return carbon_update_set_i32(context, path, (jak_i32) value);
                case NUMBER_I64:
                        return carbon_update_set_i64(context, path, (jak_i64) value);
                default:
                        error(&context->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_update_set_string(struct jak_carbon_revise *context, const char *path, const char *value)
{
        // TODO: Implement
        JAK_UNUSED(context);
        JAK_UNUSED(path);
        JAK_UNUSED(value);
        error_print(JAK_ERR_NOTIMPLEMENTED)
        return false;
}

bool carbon_update_set_binary(struct jak_carbon_revise *context, const char *path, const void *value, size_t nbytes,
                              const char *file_ext, const char *user_type)
{
        // TODO: Implement
        JAK_UNUSED(context);
        JAK_UNUSED(value);
        JAK_UNUSED(nbytes);
        JAK_UNUSED(file_ext);
        JAK_UNUSED(user_type);
        JAK_UNUSED(path);
        error_print(JAK_ERR_NOTIMPLEMENTED)
        return false;
}

struct jak_carbon_insert *carbon_update_set_array_begin(struct jak_carbon_revise *context, const char *path,
                                                    struct jak_carbon_insert_array_state *state_out, jak_u64 array_capacity)
{
        // TODO: Implement
        JAK_UNUSED(context);
        JAK_UNUSED(state_out);
        JAK_UNUSED(array_capacity);
        JAK_UNUSED(path);
        error_print(JAK_ERR_NOTIMPLEMENTED)
        return false;
}

bool carbon_update_set_array_end(struct jak_carbon_insert_array_state *state_in)
{
        // TODO: Implement
        JAK_UNUSED(state_in);
        error_print(JAK_ERR_NOTIMPLEMENTED)
        return false;
}

struct jak_carbon_insert *carbon_update_set_column_begin(struct jak_carbon_revise *context, const char *path,
                                                     struct jak_carbon_insert_column_state *state_out,
                                                     enum carbon_field_type type, jak_u64 column_capacity)
{
        // TODO: Implement
        JAK_UNUSED(state_out);
        JAK_UNUSED(context);
        JAK_UNUSED(type);
        JAK_UNUSED(column_capacity);
        JAK_UNUSED(path);
        error_print(JAK_ERR_NOTIMPLEMENTED)
        return false;
}

bool carbon_update_set_column_end(struct jak_carbon_insert_column_state *state_in)
{
        // TODO: Implement
        JAK_UNUSED(state_in);
        error_print(JAK_ERR_NOTIMPLEMENTED)
        return false;
}

// ---------------------------------------------------------------------------------------------------------------------

bool carbon_update_set_null_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path)
{
        return try_update(context, path, carbon_array_it_update_in_place_null, carbon_column_it_update_set_null);
}

bool carbon_update_set_true_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path)
{
        return try_update(context, path, carbon_array_it_update_in_place_true, carbon_column_it_update_set_true);
}

bool carbon_update_set_false_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path)
{
        return try_update(context, path, carbon_array_it_update_in_place_false, carbon_column_it_update_set_false);
}

bool carbon_update_set_u8_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path,
                                   jak_u8 value)
{
        return try_update_value(context, path, value, array_update_u8, column_update_u8);
}

bool carbon_update_set_u16_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path,
                                    jak_u16 value)
{
        return try_update_value(context, path, value, array_update_u16, column_update_u16);
}

bool carbon_update_set_u32_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path,
                                    jak_u32 value)
{
        return try_update_value(context, path, value, array_update_u32, column_update_u32);
}

bool carbon_update_set_u64_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path,
                                    jak_u64 value)
{
        return try_update_value(context, path, value, array_update_u64, column_update_u64);
}

bool carbon_update_set_i8_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path,
                                   jak_i8 value)
{
        return try_update_value(context, path, value, array_update_i8, column_update_i8);
}

bool carbon_update_set_i16_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path,
                                    jak_i16 value)
{
        return try_update_value(context, path, value, array_update_i16, column_update_i16);
}

bool carbon_update_set_i32_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path,
                                    jak_i32 value)
{
        return try_update_value(context, path, value, array_update_i32, column_update_i32);
}

bool carbon_update_set_i64_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path,
                                    jak_i64 value)
{
        return try_update_value(context, path, value, array_update_i64, column_update_i64);
}

bool carbon_update_set_float_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path,
                                      float value)
{
        return try_update_value(context, path, value, array_update_float, column_update_float);
}

bool carbon_update_set_unsigned_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path,
                                         jak_u64 value)
{
        switch (number_min_type_unsigned(value)) {
                case NUMBER_U8:
                        return carbon_update_set_u8_compiled(context, path, (jak_u8) value);
                case NUMBER_U16:
                        return carbon_update_set_u16_compiled(context, path, (jak_u16) value);
                case NUMBER_U32:
                        return carbon_update_set_u32_compiled(context, path, (jak_u32) value);
                case NUMBER_U64:
                        return carbon_update_set_u64_compiled(context, path, (jak_u64) value);
                default: error(&context->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_update_set_signed_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path,
                                       jak_i64 value)
{
        switch (number_min_type_signed(value)) {
                case NUMBER_I8:
                        return carbon_update_set_i8_compiled(context, path, (jak_i8) value);
                case NUMBER_I16:
                        return carbon_update_set_i16_compiled(context, path, (jak_i16) value);
                case NUMBER_I32:
                        return carbon_update_set_i32_compiled(context, path, (jak_i32) value);
                case NUMBER_I64:
                        return carbon_update_set_i64_compiled(context, path, (jak_i64) value);
                default: error(&context->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_update_set_string_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path,
                                       const char *value)
{
        // TODO: Implement
        JAK_UNUSED(context);
        JAK_UNUSED(path);
        JAK_UNUSED(value);
        error_print(JAK_ERR_NOTIMPLEMENTED)
        return false;
}

bool carbon_update_set_binary_compiled(struct jak_carbon_revise *context, const struct jak_carbon_dot_path *path,
                                       const void *value, size_t nbytes, const char *file_ext, const char *user_type)
{
        // TODO: Implement
        JAK_UNUSED(context);
        JAK_UNUSED(value);
        JAK_UNUSED(nbytes);
        JAK_UNUSED(file_ext);
        JAK_UNUSED(user_type);
        JAK_UNUSED(path);
        error_print(JAK_ERR_NOTIMPLEMENTED)
        return false;
}

struct jak_carbon_insert *carbon_update_set_array_begin_compiled(struct jak_carbon_revise *context,
                                                             const struct jak_carbon_dot_path *path,
                                                             struct jak_carbon_insert_array_state *state_out,
                                                             jak_u64 array_capacity)
{
        // TODO: Implement
        JAK_UNUSED(context);
        JAK_UNUSED(state_out);
        JAK_UNUSED(array_capacity);
        JAK_UNUSED(path);
        error_print(JAK_ERR_NOTIMPLEMENTED)
        return false;
}

bool carbon_update_set_array_end_compiled(struct jak_carbon_insert_array_state *state_in)
{
        // TODO: Implement
        JAK_UNUSED(state_in);
        error_print(JAK_ERR_NOTIMPLEMENTED)
        return false;
}

struct jak_carbon_insert *carbon_update_set_column_begin_compiled(struct jak_carbon_revise *context,
                                                              const struct jak_carbon_dot_path *path,
                                                              struct jak_carbon_insert_column_state *state_out,
                                                              enum carbon_field_type type,
                                                              jak_u64 column_capacity)
{
        // TODO: Implement
        JAK_UNUSED(state_out);
        JAK_UNUSED(context);
        JAK_UNUSED(type);
        JAK_UNUSED(column_capacity);
        JAK_UNUSED(path);
        error_print(JAK_ERR_NOTIMPLEMENTED)
        return false;
}

bool carbon_update_set_column_end_compiled(struct jak_carbon_insert_column_state *state_in)
{
        // TODO: Implement
        JAK_UNUSED(state_in);
        error_print(JAK_ERR_NOTIMPLEMENTED)
        return false;
}

// ---------------------------------------------------------------------------------------------------------------------

#define revision_context_delegate_func(rev_doc, doc, func, ...)                                                        \
({                                                                                                                     \
        struct jak_carbon_revise revise;                                                                                    \
        carbon_revise_begin(&revise, rev_doc, doc);                                                                     \
        bool status = func(&revise, __VA_ARGS__);                                                                      \
        carbon_revise_end(&revise);                                                                                     \
        status;                                                                                                        \
})

bool carbon_update_one_set_null(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_null, dot_path);
}

bool carbon_update_one_set_true(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_true, dot_path);
}

bool carbon_update_one_set_false(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_false, dot_path);
}

bool carbon_update_one_set_u8(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc, jak_u8 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_u8, dot_path, value);
}

bool carbon_update_one_set_u16(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc, jak_u16 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_u16, dot_path, value);
}

bool carbon_update_one_set_u32(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc, jak_u32 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_u32, dot_path, value);
}

bool carbon_update_one_set_u64(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc, jak_u64 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_u64, dot_path, value);
}

bool carbon_update_one_set_i8(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc, jak_i8 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_i8, dot_path, value);
}

bool carbon_update_one_set_i16(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc, jak_i16 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_i16, dot_path, value);
}

bool carbon_update_one_set_i32(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc, jak_i32 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_i32, dot_path, value);
}

bool carbon_update_one_set_i64(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc, jak_i64 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_i64, dot_path, value);
}

bool carbon_update_one_set_float(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc,
                                 float value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_float, dot_path, value);
}

bool carbon_update_one_set_unsigned(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc,
                                    jak_u64 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_unsigned, dot_path, value);
}

bool carbon_update_one_set_signed(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc, jak_i64 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_signed, dot_path, value);
}

bool carbon_update_one_set_string(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc,
                                  const char *value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_string, dot_path, value);
}

bool carbon_update_one_set_binary(const char *dot_path, struct jak_carbon *rev_doc, struct jak_carbon *doc,
                                  const void *value, size_t nbytes, const char *file_ext, const char *user_type)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_binary, dot_path, value, nbytes,
                                              file_ext, user_type);
}

struct jak_carbon_insert *carbon_update_one_set_array_begin(struct jak_carbon_insert_array_state *state_out,
                                                        const char *dot_path, struct jak_carbon *rev_doc,
                                                        struct jak_carbon *doc, jak_u64 array_capacity)
{
        struct jak_carbon_revise revise;
        carbon_revise_begin(&revise, rev_doc, doc);
        struct jak_carbon_insert *result = carbon_update_set_array_begin(&revise, dot_path, state_out, array_capacity);
        // ... TODO: add revision to context
        return result;
}

bool carbon_update_one_set_array_end(struct jak_carbon_insert_array_state *state_in)
{
        bool status = carbon_update_set_array_end(state_in);
        // ... TODO: drop revision from context
        return status;
}

struct jak_carbon_insert *carbon_update_one_set_column_begin(struct jak_carbon_insert_column_state *state_out,
                                                         const char *dot_path, struct jak_carbon *rev_doc,
                                                         struct jak_carbon *doc, enum carbon_field_type type,
                                                         jak_u64 column_capacity)
{
        struct jak_carbon_revise revise;
        carbon_revise_begin(&revise, rev_doc, doc);
        struct jak_carbon_insert *result = carbon_update_set_column_begin(&revise, dot_path, state_out, type,
                                                                      column_capacity);
        // ... TODO: add revision to context
        return result;
}

bool carbon_update_one_set_column_end(struct jak_carbon_insert_column_state *state_in)
{
        bool status = carbon_update_set_column_end(state_in);
        // ... TODO: drop revision from context
        return status;
}

// ---------------------------------------------------------------------------------------------------------------------

bool carbon_update_one_set_null_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                         struct jak_carbon *doc)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_null_compiled, path);
}

bool carbon_update_one_set_true_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                         struct jak_carbon *doc)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_true_compiled, path);
}

bool carbon_update_one_set_false_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                          struct jak_carbon *doc)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_false_compiled, path);
}

bool carbon_update_one_set_u8_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                       struct jak_carbon *doc, jak_u8 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_u8_compiled, path, value);
}

bool carbon_update_one_set_u16_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                        struct jak_carbon *doc, jak_u16 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_u16_compiled, path, value);
}

bool carbon_update_one_set_u32_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                        struct jak_carbon *doc, jak_u32 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_u32_compiled, path, value);
}

bool carbon_update_one_set_u64_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                        struct jak_carbon *doc, jak_u64 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_u64_compiled, path, value);
}

bool carbon_update_one_set_i8_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                       struct jak_carbon *doc, jak_i8 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_i8_compiled, path, value);
}

bool carbon_update_one_set_i16_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                        struct jak_carbon *doc, jak_i16 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_i16_compiled, path, value);
}

bool carbon_update_one_set_i32_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                        struct jak_carbon *doc, jak_i32 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_i32_compiled, path, value);
}

bool carbon_update_one_set_i64_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                        struct jak_carbon *doc, jak_i64 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_i64_compiled, path, value);
}

bool carbon_update_one_set_float_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                          struct jak_carbon *doc, float value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_float_compiled, path, value);
}

bool carbon_update_one_set_unsigned_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                             struct jak_carbon *doc, jak_u64 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_unsigned_compiled, path, value);
}

bool carbon_update_one_set_signed_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                           struct jak_carbon *doc, jak_i64 value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_signed_compiled, path, value);
}

bool carbon_update_one_set_string_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                           struct jak_carbon *doc, const char *value)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_string_compiled, path, value);
}

bool carbon_update_one_set_binary_compiled(const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
                                           struct jak_carbon *doc, const void *value, size_t nbytes, const char *file_ext,
                                           const char *user_type)
{
        return revision_context_delegate_func(rev_doc, doc, carbon_update_set_binary_compiled, path, value, nbytes,
                                              file_ext, user_type);
}

struct jak_carbon_insert *carbon_update_one_set_array_begin_compiled(struct jak_carbon_insert_array_state *state_out,
                                                                 const struct jak_carbon_dot_path *path,
                                                                 struct jak_carbon *rev_doc, struct jak_carbon *doc,
                                                                 jak_u64 array_capacity)
{
        struct jak_carbon_revise revise;
        carbon_revise_begin(&revise, rev_doc, doc);
        struct jak_carbon_insert *result = carbon_update_set_array_begin_compiled(&revise, path, state_out, array_capacity);
        // ... TODO: add revision to context
        return result;
}

bool carbon_update_one_set_array_end_compiled(struct jak_carbon_insert_array_state *state_in)
{
        bool status = carbon_update_set_array_end_compiled(state_in);
        // ... TODO: drop revision from context
        return status;
}

struct jak_carbon_insert *carbon_update_one_set_column_begin_compiled(
        struct jak_carbon_insert_column_state *state_out, const struct jak_carbon_dot_path *path, struct jak_carbon *rev_doc,
        struct jak_carbon *doc, enum carbon_field_type type, jak_u64 column_capacity)
{
        struct jak_carbon_revise revise;
        carbon_revise_begin(&revise, rev_doc, doc);
        struct jak_carbon_insert *result = carbon_update_set_column_begin_compiled(&revise, path, state_out, type,
                                                                               column_capacity);
        // ... TODO: add revision to context
        return result;
}

bool carbon_update_one_set_column_end_compiled(struct jak_carbon_insert_column_state *state_in)
{
        bool status = carbon_update_set_column_end_compiled(state_in);
        // ... TODO: drop revision from context
        return status;
}