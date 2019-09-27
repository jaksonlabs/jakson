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

#include <jakson/fn_result.h>
#include <stdarg.h>

void __fn_result_ok(fn_result *result)
{
        __fn_result_create(result, ERR_NOERR, NULL, 0);
}

void __fn_result_fail(fn_result *result, error_code code, const char *file, u32 line, const char *msg)
{
        assert(code != ERR_NOERR && code != ERR_NOERR_RESULT_BOOLEAN &&
                       code != ERR_NOERR_RESULT_INT && code != ERR_NOERR_RESULT_UINT &&
                       code != ERR_NOERR_RESULT_PTR);
        __fn_result_create(result, code, NULL, 0);
        error_set_wdetails_no_abort(&global_error, code, file, line, msg);
}

void __fn_result_fail_forward(fn_result *result)
{
        __fn_result_create(result, global_error.code, NULL, 0);
}

void __fn_result_ok_uint32(fn_result *result, u32 value)
{
        __fn_result_create(result, ERR_NOERR_RESULT_UINT, &value, sizeof(u32));
}

void __fn_result_ok_int32(fn_result *result, i32 value)
{
        __fn_result_create(result, ERR_NOERR_RESULT_INT, &value, sizeof(i32));
}

void __fn_result_ok_bool(fn_result *result, bool value)
{
        __fn_result_create(result, ERR_NOERR_RESULT_BOOLEAN, &value, sizeof(bool));
}

void __fn_result_ok_ptr(fn_result *result, const void *value)
{
        size_t adr = (size_t) value;
        __fn_result_create(result, ERR_NOERR_RESULT_PTR, &adr, sizeof(size_t));
}

void __fn_result_create(fn_result *result, error_code error_code, const void *payload, size_t payload_size)
{
        result->error_code = error_code;
        if (payload_size) {
                assert(error_code == ERR_NOERR || error_code == ERR_NOERR_RESULT_BOOLEAN ||
                       error_code == ERR_NOERR_RESULT_INT || error_code == ERR_NOERR_RESULT_UINT ||
                       error_code == ERR_NOERR_RESULT_PTR);
                size_t max_payload_bytes = PAYLOAD_SIZE / 8;
                payload_size = JAK_MIN(payload_size, max_payload_bytes);
                ZERO_MEMORY(&result->result, max_payload_bytes);
                memcpy(&result->result, payload, payload_size);
        } else {
                assert(error_code != ERR_NOERR_RESULT_BOOLEAN && error_code != ERR_NOERR_RESULT_INT &&
                               error_code != ERR_NOERR_RESULT_UINT && error_code != ERR_NOERR_RESULT_PTR);
        }
}

bool __fn_result_is_ok(fn_result result)
{
        return (result.error_code == ERR_NOERR || result.error_code == ERR_NOERR_RESULT_BOOLEAN ||
                result.error_code == ERR_NOERR_RESULT_INT ||result.error_code == ERR_NOERR_RESULT_UINT ||
                result.error_code == ERR_NOERR_RESULT_PTR);
}

bool __fn_result_has_value(fn_result result)
{
        return __fn_result_is_ok(result) && result.error_code != ERR_NOERR;
}

error_code __fn_result_error_code(fn_result result)
{
        return result.error_code;
}

bool __fn_result_is_int(fn_result result)
{
        return result.error_code == ERR_NOERR_RESULT_INT;
}

bool __fn_result_is_uint(fn_result result)
{
        return result.error_code == ERR_NOERR_RESULT_UINT;
}

bool __fn_result_is_bool(fn_result result)
{
        return result.error_code == ERR_NOERR_RESULT_BOOLEAN;
}

bool __fn_result_is_ptr(fn_result result)
{
        return result.error_code == ERR_NOERR_RESULT_PTR;
}

i32 __fn_result_int(fn_result result)
{
        return __fn_result_is_int(result) ? *(i32*) &result.result : 0;
}

u32 __fn_result_uint(fn_result result)
{
        return __fn_result_is_uint(result) ? *(u32*) &result.result : 0;
}

bool __fn_result_bool(fn_result result)
{
        return __fn_result_is_bool(result) ? *(bool*) &result.result : 0;
}

void *__fn_result_ptr(fn_result result)
{
        void *ret = 0;
        size_t adr = 0;
        if (__fn_result_is_ptr(result)) {
                size_t max_payload_bytes = PAYLOAD_SIZE / 8;
                memcpy(&adr, &result.result, max_payload_bytes);
                ret = (void *) adr;
                return ret;
        } else {
                return NULL;
        }
}

bool __fn_test_nonnull(size_t n,...)
{
        va_list args;
        va_start(args, n);
        bool result = true;
        for (size_t i = 0; result && i < n; ++i) {
                const void *ptr = va_arg(args, const void *);
                result = ptr != NULL;
        }
        va_end(args);
        return result;
}