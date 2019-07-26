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

#ifndef BISON_H
#define BISON_H

#include "shared/common.h"
#include "shared/error.h"
#include "core/mem/block.h"
#include "core/mem/file.h"
#include "core/oid/oid.h"
#include "std/string_builder.h"
#include "core/async/spin.h"
#include "std/vec.h"

NG5_BEGIN_DECL

struct bison; /* forwarded */
struct bison_array_it; /* forwarded from bison-array-it.h */
struct bison_find; /* forward from bison-find.h */

struct bison_event_listener
{
        void *extra;
        void (*clone)(struct bison_event_listener *dst, struct bison_event_listener *self);
        void (*drop)(struct bison_event_listener *self);

        void (*on_revision_begin)(struct bison_event_listener *self, struct bison *doc);
        void (*on_revision_end)(struct bison_event_listener *self, struct bison *doc);
        void (*on_revision_abort)(struct bison_event_listener *self, struct bison *doc);
        void (*on_new_revision)(struct bison_event_listener *self, struct bison *revised, struct bison *original);
};

struct bison_handler
{
        bool in_use;
        struct bison_event_listener listener;
};

typedef u32 listener_handle_t;

struct bison
{
        struct memblock *memblock;
        struct memfile memfile;

        struct vector ofType(struct bison_handler) handler;

        struct
        {
                struct spinlock write_lock;
                bool revision_lock;
                bool is_latest;
        } versioning;

        struct err err;
};

struct bison_revise
{
        struct bison *original;
        struct bison *revised_doc;
        struct err err;
};

struct bison_binary
{
        const char *mime_type;
        u64 mime_type_strlen;

        const void *blob;
        u64 blob_len;
};

enum bison_printer_impl
{
        JSON_FORMATTER
};

struct bison_new
{
        struct err err;
        struct bison original;
        struct bison_revise revision_context;
        struct bison_array_it *content_it;
        struct bison_insert *inserter;

        /* options shrink or compact (or both) documents, see
         * BISON_KEEP, BISON_SHRINK, BISON_COMPACT, and BISON_OPTIMIZE  */
        int mode;
};

enum bison_container_type { BISON_ARRAY, BISON_COLUMN };

enum bison_primary_key_type {
        /* no key, no revision number */
        BISON_KEY_NOKEY,
        /* auto-generated 64bit integer key */
        BISON_KEY_AUTOKEY,
        /* user-defined 64bit unsigned integer key */
        BISON_KEY_UKEY,
        /* user-defined 64bit signed integer key */
        BISON_KEY_IKEY,
        /* user-defined n-char string key */
        BISON_KEY_SKEY
};

#define BISON_MARKER_ARRAY_BEGIN '['
#define BISON_MARKER_ARRAY_END ']'

#define BISON_MARKER_COLUMN_BEGIN '('

#define BISON_MARKER_KEY_NOKEY '?'
#define BISON_MARKER_KEY_AUTOKEY '*'
#define BISON_MARKER_KEY_UKEY '+'
#define BISON_MARKER_KEY_IKEY '-'
#define BISON_MARKER_KEY_SKEY '!'

NG5_DEFINE_ERROR_GETTER(bison);
NG5_DEFINE_ERROR_GETTER(bison_new);

#define BISON_KEEP              0x0
#define BISON_SHRINK            0x1
#define BISON_COMPACT           0x2
#define BISON_OPTIMIZE          (BISON_SHRINK | BISON_COMPACT)

/**
 * Constructs a new context in which a new document can be created. The parameter <b>mode</b> controls
 * how reserved spaces should be handled after document creation is done. Set <code>mode</code> to
 * <code>BISON_KEEP</code> for no optimization. With this mode, all capacities (i.e., additional ununsed but free
 * space) in containers (objects, arrays, and columns) are kept and tailing free space after the document is
 * kept, too. Use this mode to optimize for "insertion-heavy" documents since keeping all capacities lowerst the
 * probability of reallocations and memory movements. Set <b>mode</b> to <code>BISON_COMPACT</code> if capacities in
 * containers should be removed after creation, and <code>BISON_COMPACT</code> to remove tailing free space. Use
 * <code>BISON_OPTIMIZE</code> to use both <code>BISON_SHRINK</code> and <code>BISON_COMPACT</code>.
 *
 * As a rule of thumb for <b>mode</b>. The resulting document...
 * <ul>
 *  <li>...will be updated heavily where updates may change the type-width of fields, will be target of many inserts
 *  containers, use <code>BISON_KEEP</code>. The document will have a notable portion of reserved memory contained;
 *  insertions or updates will, however, not require immediately reallocation or memory movements.</li>
 *  <li>...will <i>not</i> be target of insertion of strings or blob fields in the near future, use
 *      <code>BISON_SHRINK</code>. The document will not have padding reserved memory at the end, which means that
 *      a realloction will be required once the document grows (e.g., a container must be englarged). Typically,
 *      document growth is handled with container capacities (see <code>BISON_COMPACT</code>). However, insertions
 *      of variable-length data (i.e., strings and blobs) may require container enlargement. In this case, having
 *      padding reserved memory at the end of the document lowers the risk of a reallocation.</li>
 *  <li>...will <i>not</i> not be target of insertion operations or update operations that changes a fields type-width
 *      in the near future. In simpler words, if a document is updated and each such update keeps the (byte) size
 *      of the field, use <code>BISON_COMPACT</code>. This mode will remove all capacities in containers.</li>
 *  <li>...is read-mostly, or updates will not change the type or type-width of fields, use <code>BISON_OPTIMIZE</code>.
 *      The document will have the smallest memory footprint possible.</li>
 * </ul>
 */
NG5_EXPORT(struct bison_insert *) bison_create_begin(struct bison_new *context, struct bison *doc,
        enum bison_primary_key_type key_type, int mode);

NG5_EXPORT(bool) bison_create_end(struct bison_new *context);

NG5_EXPORT(bool) bison_create_empty(struct bison *doc, enum bison_primary_key_type key_type);

NG5_EXPORT(bool) bison_create_empty_ex(struct bison *doc, enum bison_primary_key_type key_type, u64 doc_cap_byte,
        u64 array_cap_byte);

NG5_EXPORT(bool) bison_drop(struct bison *doc);

NG5_EXPORT(bool) bison_is_up_to_date(struct bison *doc);

NG5_EXPORT(bool) bison_key_get_type(enum bison_primary_key_type *out, struct bison *doc);

NG5_EXPORT(const void *) bison_key_raw_value(u64 *key_len, enum bison_primary_key_type *type, struct bison *doc);

NG5_EXPORT(bool) bison_key_signed_value(i64 *key, struct bison *doc);

NG5_EXPORT(bool) bison_key_unsigned_value(u64 *key, struct bison *doc);

NG5_EXPORT(const char *) bison_key_string_value(u64 *str_len, struct bison *doc);

NG5_EXPORT(bool) bison_has_key(enum bison_primary_key_type type);

NG5_EXPORT(bool) bison_key_is_unsigned_type(enum bison_primary_key_type type);

NG5_EXPORT(bool) bison_key_is_signed_type(enum bison_primary_key_type type);

NG5_EXPORT(bool) bison_key_is_string_type(enum bison_primary_key_type type);

NG5_EXPORT(bool) bison_register_listener(listener_handle_t *handle, struct bison_event_listener *listener, struct bison *doc);

NG5_EXPORT(bool) bison_unregister_listener(struct bison *doc, listener_handle_t handle);

NG5_EXPORT(bool) bison_clone(struct bison *clone, struct bison *doc);

NG5_EXPORT(bool) bison_revision(u64 *rev, struct bison *doc);

NG5_EXPORT(bool) bison_to_str(struct string_builder *dst, enum bison_printer_impl printer, struct bison *doc);

NG5_EXPORT(const char *) bison_to_json(struct string_builder *dst, struct bison *doc);

NG5_EXPORT(bool) bison_iterator_open(struct bison_array_it *it, struct bison *doc);

NG5_EXPORT(bool) bison_iterator_close(struct bison_array_it *it);

NG5_EXPORT(bool) bison_print(FILE *file, struct bison *doc);

NG5_EXPORT(bool) bison_hexdump_print(FILE *file, struct bison *doc);

NG5_END_DECL

#endif
