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

#ifndef FN_RESULT
#define FN_RESULT

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>
#include <jakson/types.h>

BEGIN_DECL

#define PAYLOAD_SIZE 48 /** bits used for payload */

/** Compound function call status and return result. Supports function err code (NOERR or some err) plus
 * an optional result value, which type is encoded via the err code (ERR_RESULT_...) */
typedef struct fn_result {
        error_code error_code : 16; /** err code of call */
        /** If result carries void value by ERR_NOERR, or some value by ERR_RESULT_..., the value is stored
        * in 'result'. Note that storing a pointer in 'result' works on current x64 machines bacause 64bit pointers do not
        * use the full number of bits; i.e., only 48 bits are used to store an address n a pointer */
        char result[48]; /** 48 bit for the result value */
} fn_result;

// ---------------------------------------------------------------------------------------------------------------------
//  function result creation macros
//
//  For ease of understanding, use the 'ofType(X)' annotation in case the function carries any value in addition to
//  its success or fail state, e.g., fn_result ofType(bool) foo();
//  Annotating a function in this way helps the caller to figure out if there is any value to get for a successful
//  function call.
//
// ---------------------------------------------------------------------------------------------------------------------

/** constructs a new fn_result that encodes a successful call */
#define FN_OK()                   RESULT_OK()

/** constructs a new fn_result that encodes a successful call, and carries a 32bit integer (function) return value */
#define FN_OK_INT(value)          RESULT_OK_INT(value)

/** constructs a new fn_result that encodes a successful call, and carries a 32bit unsigned (function) return value */
#define FN_OK_UINT(value)         RESULT_OK_UINT(value)

/** constructs a new fn_result that encodes a successful call, and carries a boolean return value */
#define FN_OK_BOOL(value)         RESULT_OK_BOOL(value)

/** constructs a new fn_result that encodes a successful call, and carries a (48bit) pointer value */
#define FN_OK_PTR(value)          RESULT_OK_PTR(value)

/** constructs a new fn_result that encodes a successful call, and carries 'true' as boolean value */
#define FN_OK_TRUE()              FN_OK_BOOL(true)

/** constructs a new fn_result that encodes a successful call, and carries 'false' as boolean value */
#define FN_OK_FALSE()             FN_OK_BOOL(false)

/** constructs a new fn_result that encodes an unsuccessful call with err code and message */
#define FN_FAIL(code, msg)        RESULT_FAIL(code, msg)

/** use this macro for the return statement inside a fn_result-returning function (outer), if a function call within
 * outer results in a failed fn_result object, and the err information (err code, and message) should be return by
 * outer without modifying it */
#define FN_FAIL_FORWARD()                                                 \
({                                                                        \
        fn_result __result;                                           \
        __fn_result_fail_forward(&__result);                          \
        __result;                                                     \
})

/** use this macro to produce a function fail with forwarded error message from expr that returns a fn_result object if
 * this fn_result object encodes a function call fail */
#define FN_FAIL_FORWARD_IF_NOT_OK(expr)                                 \
        if (!FN_IS_OK(expr)) {                                          \
                return FN_FAIL_FORWARD();                               \
        }

/** use this macro to produce a function fail with forwarded error message from expr that returns a fn_result object if
 * this fn_result object encodes a function call fail */
#define FN_IF_NOT_OK_RETURN(expr, ret)                                  \
        if (!FN_IS_OK(expr)) {                                          \
                return (ret);                                           \
        }

/** use this macro to produce a function fail with forwarded error message from fn_result_obj that returns a fn_result
 * object carrying a boolean value, if this boolean value is not true or the function call for fn_result_obj had a
 * failure */
#define FN_FAIL_FORWARD_IF_NOT_TRUE(fn_result_obj)                      \
        if (!FN_BOOL(fn_result_obj)) {                                  \
                return FN_FAIL_FORWARD();                               \
        }

// ---------------------------------------------------------------------------------------------------------------------
//  function err state resolver macro about success or fail of a function call
// ---------------------------------------------------------------------------------------------------------------------

/** checks if a fn_result 'result' encodes a successful call */
#define FN_IS_OK(result)          RESULT_IS_OK(result)

// ---------------------------------------------------------------------------------------------------------------------
//  function return value resolver macro for successful calls
// ---------------------------------------------------------------------------------------------------------------------

/** returns true if 'expr' returns a fn_result object that encodes success and carries a boolean value true */
#define FN_IS_TRUE(expr)                                                                                               \
({                                                                                                                     \
        fn_result fn_true_result;                                                                                      \
        bool fn_true_ret = FN_IS_OK((fn_true_result = (expr))) && FN_BOOL(fn_true_result);                             \
        fn_true_ret;                                                                                                   \
})

/** returns the signed 32bit integer value carried by a successful function call; undefined for not successful calls */
#define FN_INT(result)            __RESULT_EXTRACT_VALUE(result, iu32, RESULT_INT_VALUE)

/** returns the unsigned 32bit integer value carried by a successful function call; undefined for not successful calls */
#define FN_UINT(result)           __RESULT_EXTRACT_VALUE(result, u32, RESULT_UINT_VALUE)

/** returns the boolean value carried by a successful function call; undefined for not successful calls */
#define FN_BOOL(result)           __RESULT_EXTRACT_VALUE(result, bool, RESULT_BOOL_VALUE)

/** returns the pointer value carried by a successful function call and cast it to T*; undefined for not successful calls */
#define FN_PTR(T, result)         ((T *) __RESULT_EXTRACT_VALUE(result, void *, RESULT_PTR_VALUE))

// ---------------------------------------------------------------------------------------------------------------------
//  helper macro to check if a list of (up to eleven) function parameters is non-null
// ---------------------------------------------------------------------------------------------------------------------

/** constructs a new fn_result that encodes an unsuccessful call with err code 'null pointer' if one of the arguments
 * is a pointer to NULL */
#define FN_FAIL_IF_NULL(...)                                                                                           \
    if (!__fn_test_nonnull(VA_ARGS_LENGTH(__VA_ARGS__), __VA_ARGS__)) {                                                \
        return RESULT_FAIL(ERR_NULLPTR, "function argument is not allowed to be null");                                \
    }

// ---------------------------------------------------------------------------------------------------------------------
//  breakouts of the fn_result environment
// ---------------------------------------------------------------------------------------------------------------------

/** break outside the fn_result environment by converting an fn_result object to a boolean, which is true if
 * the object does not carry any value but the function call was successful, or which is true if
 * the object carries a non-zero value and the function call was successful, or false otherwise */
#define FN_STATUS(expr)                                                                                                \
({                                                                                                                     \
        fn_result result = (expr);                                                                                     \
        (__fn_result_is_ok(result) && (!RESULT_HAS_VALUE(result) ||                                                    \
                                       (RESULT_HAS_VALUE(result) &&                                                    \
                                                ((__fn_result_is_bool(result) && __fn_result_bool(result)) ||          \
                                                 (__fn_result_is_int(result) &&  __fn_result_int(result))) ||          \
                                                 (__fn_result_is_uint(result)  &&  __fn_result_uint(result)) ||        \
                                                 (__fn_result_is_ptr(result)  &&  __fn_result_ptr(result)))));         \
})

#define FN_GET_PTR(T, expr)                                                                                            \
({                                                                                                                     \
        void *__get_ptr_ret = NULL;                                                                                    \
        fn_result __get_ptr_result = (expr);                                                                           \
        if (__fn_result_is_ok(__get_ptr_result) && RESULT_HAS_VALUE(__get_ptr_result) &&                               \
            __fn_result_is_ptr(__get_ptr_result)) {                                                                    \
                __get_ptr_ret = __fn_result_ptr(__get_ptr_result);                                                     \
        }                                                                                                              \
        ((T *)__get_ptr_ret);                                                                                          \
})

#define FN_GET_BOOL(expr)                                                                                              \
({                                                                                                                     \
        bool __get_bool_ret = false;                                                                                   \
        fn_result __get_bool_result = (expr);                                                                          \
        if (__fn_result_is_ok(__get_bool_result) && RESULT_HAS_VALUE(__get_bool_result) &&                             \
            __fn_result_is_bool(__get_bool_result)) {                                                                  \
                __get_bool_ret = __fn_result_bool(__get_bool_result);                                                  \
        }                                                                                                              \
        __get_bool_ret;                                                                                                \
})

// ---------------------------------------------------------------------------------------------------------------------
//  lower-level macros, no need to use them outside this module
// ---------------------------------------------------------------------------------------------------------------------

#define RESULT_OK()                                                   \
({                                                                    \
        fn_result __result;                                           \
        __fn_result_ok(&__result);                                    \
        __result;                                                     \
})

#define RESULT_OK_INT(int32_value)                                    \
({                                                                    \
        fn_result __result;                                           \
        __fn_result_ok_int32(&__result, int32_value);                 \
        __result;                                                     \
})

#define RESULT_OK_UINT(uint32_value)                                  \
({                                                                        \
        fn_result __result;                                           \
        __fn_result_ok_uint32(&__result, uint32_value);               \
        __result;                                                     \
})

#define RESULT_OK_BOOL(bool_value)                                    \
({                                                                        \
        fn_result __result;                                           \
        __fn_result_ok_bool(&__result, (bool_value));                 \
        __result;                                                     \
})

#define RESULT_OK_PTR(ptr_value)                                      \
({                                                                        \
        fn_result __result;                                           \
        __fn_result_ok_ptr(&__result, ptr_value);                     \
        __result;                                                     \
})

#define RESULT_FAIL(code, msg)                                        \
({                                                                        \
    fn_result __result;                                               \
    __fn_result_fail(&__result, code, __FILE__, __LINE__, msg);       \
    __result;                                                         \
})

#define __RESULT_EXTRACT_VALUE(result, result_type, result_fn)        \
({                                                                        \
        result_type __retval;                                         \
        fn_result __call_result = (result);                     \
        if (LIKELY(RESULT_IS_OK(__call_result))) {                       \
                __retval = result_fn(__call_result);                         \
        } else {                                                          \
                return FN_FAIL_FORWARD();                                 \
        }                                                                 \
        __retval;                                                     \
})

#define RESULT_IS_OK(result)                                          \
    __fn_result_is_ok(result)

#define RESULT_HAS_VALUE(result)                                      \
    __fn_result_has_value(result)

#define RESULT_INT_VALUE(result)                                      \
    __fn_result_int(result)

#define RESULT_UINT_VALUE(result)                                     \
    __fn_result_uint(result)

#define RESULT_BOOL_VALUE(result)                                     \
    __fn_result_bool(result)

#define RESULT_PTR_VALUE(result)                                      \
    __fn_result_ptr(result)

#define RESULT_CODE(result)                                           \
    __fn_result_error_code(result)

#define RESULT_GET_LAST_ERROR()                                       \
    (&global_error)

void __fn_result_ok(fn_result *result);
void __fn_result_fail(fn_result *result, error_code code, const char *file, u32 line, const char *msg);
void __fn_result_fail_forward(fn_result *result);
void __fn_result_ok_uint32(fn_result *result, u32 value);
void __fn_result_ok_int32(fn_result *result, i32 value);
void __fn_result_ok_bool(fn_result *result, bool value);
void __fn_result_ok_ptr(fn_result *result, const void *value);

void __fn_result_create(fn_result *result, error_code error_code, const void *payload, size_t payload_size);

bool __fn_result_is_ok(fn_result result);
bool __fn_result_has_value(fn_result result);

error_code __fn_result_error_code(fn_result result);

bool __fn_result_is_int(fn_result result);
bool __fn_result_is_uint(fn_result result);
bool __fn_result_is_bool(fn_result result);
bool __fn_result_is_ptr(fn_result result);

i32 __fn_result_int(fn_result result);
u32 __fn_result_uint(fn_result result);
bool __fn_result_bool(fn_result result);
void *__fn_result_ptr(fn_result result);

bool __fn_test_nonnull(size_t n,...);

END_DECL

#endif