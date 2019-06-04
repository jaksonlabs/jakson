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

#include "core/bison/bison_printers.h"

struct json_formatter_extra
{
        bool intent;
        bool strict;
};

inline static bool json_formatter_has_intention(struct bison_printer *printer);
inline static bool json_formatter_is_strict(struct bison_printer *printer);

static void json_formatter_drop(struct bison_printer *self);

static void json_formatter_bison_begin(struct bison_printer *self, struct string_builder *builder);
static void json_formatter_bison_end(struct bison_printer *self, struct string_builder *builder);

static void json_formatter_header_begin(struct bison_printer *self, struct string_builder *builder);
static void json_formatter_bison_header_contents(struct bison_printer *self, struct string_builder *builder, object_id_t oid, u64 rev);
static void json_formatter_bison_header_end(struct bison_printer *self, struct string_builder *builder);

static void json_formatter_bison_payload_begin(struct bison_printer *self, struct string_builder *builder);
static void json_formatter_bison_payload_end(struct bison_printer *self, struct string_builder *builder);

NG5_EXPORT(bool) bison_json_formatter_create(struct bison_printer *printer)
{
        error_if_null(printer);
        printer->drop = json_formatter_drop;

        printer->print_bison_begin = json_formatter_bison_begin;
        printer->print_bison_end = json_formatter_bison_end;

        printer->print_bison_header_begin = json_formatter_header_begin;
        printer->print_bison_header_contents = json_formatter_bison_header_contents;
        printer->print_bison_header_end = json_formatter_bison_header_end;

        printer->print_bison_payload_begin = json_formatter_bison_payload_begin;
        printer->print_bison_payload_end = json_formatter_bison_payload_end;

        printer->extra = malloc(sizeof(struct json_formatter_extra));
        *((struct json_formatter_extra *) printer->extra) = (struct json_formatter_extra) {
                .intent = false,
                .strict = true
        };

        return true;
}

NG5_EXPORT(bool) bison_json_formatter_set_intent(struct bison_printer *printer, bool enable)
{
        error_if_null(printer);
        ((struct json_formatter_extra *) printer->extra)->intent = enable;
        return true;
}

NG5_EXPORT(bool) bison_json_formatter_set_strict(struct bison_printer *printer, bool enable)
{
        error_if_null(printer);
        ((struct json_formatter_extra *) printer->extra)->strict = enable;
        return true;
}

static void json_formatter_drop(struct bison_printer *self)
{
        free (self->extra);
}

ng5_func_unused
inline static bool json_formatter_has_intention(struct bison_printer *printer)
{
        return ((struct json_formatter_extra *) printer->extra)->intent;
}

inline static bool json_formatter_is_strict(struct bison_printer *printer)
{
        return ((struct json_formatter_extra *) printer->extra)->strict;
}

static void json_formatter_bison_begin(struct bison_printer *self, struct string_builder *builder)
{
        ng5_unused(self);
        string_builder_append(builder, "{");
}

static void json_formatter_bison_end(struct bison_printer *self, struct string_builder *builder)
{
        ng5_unused(self);
        string_builder_append(builder, "}");
}

static void json_formatter_header_begin(struct bison_printer *self, struct string_builder *builder)
{
        ng5_unused(self);
        if (json_formatter_is_strict(self)) {
                string_builder_append(builder, "\"meta\": {");
        } else {
                string_builder_append(builder, "meta: {");
        }
}

static void json_formatter_bison_header_contents(struct bison_printer *self, struct string_builder *builder, object_id_t oid, u64 rev)
{
        if (json_formatter_is_strict(self)) {
                string_builder_append(builder, "\"_id\": ");
                string_builder_append_u64(builder, oid);
                string_builder_append(builder, ", \"_rev\": ");
                string_builder_append_u64(builder, rev);
        } else {
                string_builder_append(builder, "_id: ");
                string_builder_append_u64(builder, oid);
                string_builder_append(builder, ", _rev: ");
                string_builder_append_u64(builder, rev);
        }
}

static void json_formatter_bison_header_end(struct bison_printer *self, struct string_builder *builder)
{
        ng5_unused(self);
        string_builder_append(builder, "}, ");
}

static void json_formatter_bison_payload_begin(struct bison_printer *self, struct string_builder *builder)
{
        if (json_formatter_is_strict(self)) {
                string_builder_append(builder, "\"doc\": {");
        } else {
                string_builder_append(builder, "doc: {");
        }
}

static void json_formatter_bison_payload_end(struct bison_printer *self, struct string_builder *builder)
{
        ng5_unused(self);
        string_builder_append(builder, "}");
}

