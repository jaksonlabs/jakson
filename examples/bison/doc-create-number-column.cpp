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

#include <core/bison/bison.h>
#include <core/bison/bison-insert.h>


int main(void) {

        struct bison doc;
        struct bison_new context;
        struct bison_insert_column_state state;

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_OPTIMIZE);

        struct bison_insert *cins = bison_insert_column_begin(&state, ins, BISON_FIELD_TYPE_NUMBER_U8, 15);
        bison_insert_u8(cins, 0);
        bison_insert_u8(cins, 1);
        bison_insert_u8(cins, 2);
        bison_insert_u8(cins, 3);
        bison_insert_u8(cins, 4);
        bison_insert_u8(cins, 5);
        bison_insert_u8(cins, 6);
        bison_insert_column_end(&state);

        bison_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct string_builder sb;
        string_builder_create(&sb);

        printf("json formatted: '%s'\n", bison_to_json(&sb, &doc));
        bison_hexdump_print(stdout, &doc);

        string_builder_drop(&sb);
        bison_drop(&doc);

        return EXIT_SUCCESS;
}