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

#ifndef JAK_RESULT
#define JAK_RESULT

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_types.h>

JAK_BEGIN_DECL

#define JAK_PAYLOAD_SIZE 48 /* bits used for payload */

/* Compound function call status and return result. Supports function error code (NOERR or some error) plus
 * an optional result value, which type is encoded via the error code (JAK_ERR_RESULT_...) */
typedef struct fn_result {
        jak_error_code error_code : 16; /* error code of call */
        /* If result carries void value by JAK_ERR_NOERR, or some value by JAK_ERR_RESULT_..., the value is stored
        * in 'result'. Note that storing a pointer in 'result' works on current x64 machines bacause 64bit pointers do not
        * use the full number of bits; i.e., only 48 bits are used to store an address n a pointer */
        char result[48]; /* 48 bit for the result value */
} fn_result;


#define FN_IS_OK(result)          JAK_RESULT_IS_OK(result)

#define FN_OK()                   JAK_RESULT_OK()
#define FN_OK_INT(value)          JAK_RESULT_OK_INT(value)
#define FN_OK_UINT(value)         JAK_RESULT_OK_UINT(value)
#define FN_OK_BOOL(value)         JAK_RESULT_OK_BOOL(value)
#define FN_OK_PTR(value)          JAK_RESULT_OK_PTR(value)

#define FN_FAIL(code, msg)        JAK_RESULT_FAIL(code, msg)

#define FN_INT(result)            __JAK_RESULT_EXTRACT_VALUE(result, jak_iu32, JAK_RESULT_INT_VALUE)
#define FN_UINT(result)           __JAK_RESULT_EXTRACT_VALUE(result, jak_u32, JAK_RESULT_UINT_VALUE)
#define FN_BOOL(result)           __JAK_RESULT_EXTRACT_VALUE(result, bool, JAK_RESULT_BOOL_VALUE)
#define FN_PTR(result)            __JAK_RESULT_EXTRACT_VALUE(result, void *, JAK_RESULT_PTR_VALUE)

#define JAK_FAIL_FORWARD()                                                \
({                                                                        \
        fn_result __jak_result;                                           \
        jak_fn_result_fail_forward(&__jak_result);                        \
        __jak_result;                                                     \
})

#define JAK_RESULT_OK()                                                   \
({                                                                        \
        fn_result __jak_result;                                           \
        jak_fn_result_ok(&__jak_result);                                  \
        __jak_result;                                                     \
})

#define JAK_RESULT_OK_INT(int32_value)                                    \
({                                                                        \
        fn_result __jak_result;                                           \
        jak_fn_result_ok_int32(&__jak_result, int32_value);               \
        __jak_result;                                                     \
})

#define JAK_RESULT_OK_UINT(uint32_value)                                  \
({                                                                        \
        fn_result __jak_result;                                           \
        jak_fn_result_ok_uint32(&__jak_result, uint32_value);             \
        __jak_result;                                                     \
})

#define JAK_RESULT_OK_BOOL(bool_value)                                    \
({                                                                        \
        fn_result __jak_result;                                           \
        jak_fn_result_ok_bool(&__jak_result, (bool_value));               \
        __jak_result;                                                     \
})

#define JAK_RESULT_OK_PTR(ptr_value)                                      \
({                                                                        \
        fn_result __jak_result;                                           \
        jak_fn_result_ok_ptr(&__jak_result, ptr_value);                   \
        __jak_result;                                                     \
})

#define JAK_RESULT_FAIL(code, msg)                                        \
({                                                                        \
    fn_result __jak_result;                                               \
    jak_fn_result_fail(&__jak_result, code, __FILE__, __LINE__, msg);     \
    __jak_result;                                                         \
})

#define __JAK_RESULT_EXTRACT_VALUE(result, result_type, result_fn)        \
({                                                                        \
        result_type __jak_retval;                                         \
        if (JAK_LIKELY(JAK_RESULT_IS_OK(result))) {                       \
                __jak_retval = result_fn(result);                         \
        } else {                                                          \
                return JAK_FAIL_FORWARD();                                \
        }                                                                 \
        __jak_retval;                                                     \
})

#define JAK_RESULT_IS_OK(result)                                          \
    jak_fn_result_is_ok(result)

#define JAK_RESULT_HAS_VALUE(result)                                      \
    jak_fn_result_has_value(result)

#define JAK_RESULT_INT_VALUE(result)                                      \
    jak_fn_result_int(result)

#define JAK_RESULT_UINT_VALUE(result)                                     \
    jak_fn_result_uint(result)

#define JAK_RESULT_BOOL_VALUE(result)                                     \
    jak_fn_result_bool(result)

#define JAK_RESULT_PTR_VALUE(result)                                      \
    jak_fn_result_ptr(result)

#define JAK_RESULT_CODE(result)                                           \
    jak_fn_result_error_code(result)

#define JAK_RESULT_GET_LAST_ERROR()                                       \
    &jak_global_error;

#define FN_FAIL_IF_NULL(...)                                                                                        \
    if (!__jak_fn_test_nonnull(JAK_VA_ARGS_LENGTH(__VA_ARGS__), __VA_ARGS__)) {                                        \
        return JAK_RESULT_FAIL(JAK_ERR_NULLPTR, "function argument is not allowed to be null");                        \
    }


void jak_fn_result_ok(fn_result *result);
void jak_fn_result_fail(fn_result *result, jak_error_code code, const char *file, jak_u32 line, const char *msg);
void jak_fn_result_fail_forward(fn_result *result);
void jak_fn_result_ok_uint32(fn_result *result, jak_u32 value);
void jak_fn_result_ok_int32(fn_result *result, jak_i32 value);
void jak_fn_result_ok_bool(fn_result *result, bool value);
void jak_fn_result_ok_ptr(fn_result *result, const void *value);

void __jak_fn_result_create(fn_result *result, jak_error_code error_code, const void *payload, size_t payload_size);

bool jak_fn_result_is_ok(fn_result result);
bool jak_fn_result_has_value(fn_result result);

jak_error_code jak_fn_result_error_code(fn_result result);

bool jak_fn_result_is_int(fn_result result);
bool jak_fn_result_is_uint(fn_result result);
bool jak_fn_result_is_bool(fn_result result);
bool jak_fn_result_is_ptr(fn_result result);

jak_i32 jak_fn_result_int(fn_result result);
jak_u32 jak_fn_result_uint(fn_result result);
bool jak_fn_result_bool(fn_result result);
void *jak_fn_result_ptr(fn_result result);

bool __jak_fn_test_nonnull(size_t n,...);

JAK_END_DECL

#endif