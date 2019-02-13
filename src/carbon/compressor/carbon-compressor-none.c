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

#include <inttypes.h>
#include "carbon/compressor/carbon-compressor-none.h"



void compressor_none_write_dictionary(carbon_memfile_t *memfile, const carbon_vec_t ofType (const char *) *strings,
                                      const carbon_vec_t ofType(carbon_string_id_t) *string_ids)
{
    for (size_t i = 0; i < strings->num_elems; i++) {
        carbon_string_id_t *string_id_t = CARBON_VECTOR_GET(string_ids, i, carbon_string_id_t);
        const char *string = *CARBON_VECTOR_GET(strings, i, const char *);
        size_t string_length = strlen(string);

        struct embedded_string embedded_string = {
            .marker = marker_symbols[MARKER_TYPE_EMBEDDED_UNCOMP_STR].symbol,
            .strlen = string_length
        };

        carbon_memfile_write(memfile, &embedded_string, sizeof(struct embedded_string));
        carbon_memfile_write(memfile, string_id_t, sizeof(carbon_string_id_t));
        carbon_memfile_write(memfile, string, string_length);
    }
}

void compressor_none_dump_dictionary(FILE *file, carbon_memfile_t *memfile)
{
    while ((*CARBON_MEMFILE_PEEK(memfile, char)) == marker_symbols[MARKER_TYPE_EMBEDDED_UNCOMP_STR].symbol) {
        unsigned offset = CARBON_MEMFILE_TELL(memfile);
        struct embedded_string *embedded_string = CARBON_MEMFILE_READ_TYPE(memfile, struct embedded_string);
        carbon_string_id_t *string_id = CARBON_MEMFILE_READ_TYPE(memfile, carbon_string_id_t);
        const char *string = CARBON_MEMFILE_READ(memfile, embedded_string->strlen);
        char *printableString = malloc(embedded_string->strlen + 1);
        memcpy(printableString, string, embedded_string->strlen);
        printableString[embedded_string->strlen] = '\0';

        fprintf(file, "0x%04x ", offset);
        fprintf(file, "   [marker: %c] [string_length: %"PRIu64"] [string_id: %"PRIu64"] [string: '%s']\n",
            embedded_string->marker,
            embedded_string->strlen, *string_id, printableString);

        free(printableString);
    }
}


bool compressor_none_encode_string(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err,
                                   carbon_string_id_t string_id, const char *string)
{

}

char *compressor_none_decode_string(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err,
                                    carbon_string_id_t string_id)
{

}
