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
#include <jakson/carbon/patch.h>
#include <jakson/carbon/internal.h>
#include <jakson/carbon/find.h>

fn_result carbon_patch_begin(carbon_array_it *it, carbon *doc)
{
        FN_FAIL_IF_NULL(it, doc);
        offset_t payload_start = carbon_int_payload_after_header(doc);
        carbon_array_it_create(it, &doc->memfile, &doc->err, payload_start);
        carbon_array_it_set_mode(it, READ_WRITE);
        return FN_OK();
}

fn_result carbon_patch_end(carbon_array_it *it)
{
        FN_FAIL_IF_NULL(it);
        return carbon_array_it_drop(it);
}

bool carbon_patch_find_begin(carbon_find *out, const char *dot_path, carbon *doc)
{
        doc->memfile.mode = READ_WRITE;
        return carbon_find_begin(out, dot_path, doc);
}

fn_result carbon_patch_find_end(carbon_find *find)
{
        find->doc->memfile.mode = READ_ONLY;
        return carbon_find_end(find);
}