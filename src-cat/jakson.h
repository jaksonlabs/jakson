/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_ALLOC_H
#define JAK_ALLOC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>

JAK_BEGIN_DECL

/**
 * Allocates <code>num</code> elements of size <code>sizeof(type)</code> using the allocator <code>alloc</code> and
 * creates a new stack variable <code>type *name</code>.
 */
#define JAK_ALLOC_MALLOC(type, name, num, alloc)                                                                          \
    type *name = jak_alloc_malloc(alloc, num *sizeof(type))

/**
 * Invokes a free operation in <code>alloc</code> allocator to free up memory assigned to pointer <code>name</code>
 */
#define JAK_ALLOC_FREE(name, alloc)                                                                                       \
    jak_alloc_free(alloc, name)

typedef struct jak_allocator {
        /**
         *  Implementation-specific data (private fields etc.)
         *  This pointer may point to NULL.
         */
        void *extra;

        /**
         *  Error information
         */
        jak_error err;

        /**
         *  Implementation to call memory allocation.
         */
        void *(*malloc)(jak_allocator *self, size_t size);

        /**
         *  Implementation to call memory re-allocation.
         */
        void *(*realloc)(jak_allocator *self, void *ptr, size_t size);

        /**
         *  Implementation to call freeing up memory.
         *  Depending on the strategy, freeing up memory might be lazy.
         */
        void (*free)(jak_allocator *self, void *ptr);

        /**
         *  Perform a deep copy of this allocator including implementation-specific data stored in 'extra'
         *
         * @param dst non-null target in which 'self' should be cloned
         * @param self non-null source which should be clones in 'dst'
         */
        void (*clone)(jak_allocator *dst, const jak_allocator *self);
} jak_allocator;

/**
 * Returns standard c-lib allocator (malloc, realloc, free)
 *
 * @param alloc must be non-null
 * @return STATUS_OK in case of non-null parameter alloc, STATUS_NULLPTR otherwise
 */
bool jak_alloc_create_std(jak_allocator *alloc);

/**
 * Creates a new allocator 'dst' with default constructor (in case of 'this' is null), or as copy of
 * 'this' (in case 'this' is non-null)
 * @param dst non-null destination in which the allocator should be stored
 * @param self possibly null-pointer to an allocator implementation
 * @return a value unequal to STATUS_OK in case the operation is not successful
 */
bool jak_alloc_this_or_std(jak_allocator *dst, const jak_allocator *self);

/**
 * Performs a deep copy of the allocator 'src' into the allocator 'dst'.
 *
 * @param dst non-null pointer to allocator implementation (of same implementation as src)
 * @param src non-null pointer to allocator implementation (of same implementation as dst)
 * @return STATUS_OK in case of success, otherwise a value unequal to STATUS_OK describing the JAK_ERROR
 */
bool jak_alloc_clone(jak_allocator *dst, const jak_allocator *src);

/**
 * Invokes memory allocation of 'size' bytes using the allocator 'alloc'.
 *
 * If allocation fails, the system may panic.
 *
 * @param alloc non-null pointer to allocator implementation
 * @param size number of bytes requested
 * @return non-null pointer to memory allocated with 'alloc'
 */
void *jak_alloc_malloc(jak_allocator *alloc, size_t size);

/**
 * Invokes memory re-allocation for pointer 'ptr' (that is managed by 'alloc') to size 'size' in bytes.
 *
 * @param alloc non-null pointer to allocator implementation
 * @param ptr non-null pointer manged by 'alloc'
 * @param size new number of bytes for 'ptr'
 * @return non-null pointer that points to reallocated memory for 'ptr'
 */
void *jak_alloc_realloc(jak_allocator *alloc, void *ptr, size_t size);

/**
 * Invokes memory freeing for pointer 'ptr' (that is managed by 'alloc').
 * Depending on the implementation, this operation might be lazy.
 *
 * @param alloc non-null pointer to allocator implementation
 * @param ptr non-null pointer manged by 'alloc'
 * @return STATUS_OK if success, STATUS_NULLPTR if <code>alloc</code> or <code>ptr</ptr> is <b>NULL</b>
 */
bool jak_alloc_free(jak_allocator *alloc, void *ptr);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_ALLOC_TRACER_H
#define JAK_ALLOC_TRACER_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include "jak_alloc.h"

JAK_BEGIN_DECL

/**
 * Returns standard c-lib allocator (malloc, realloc, free) that collects some statistics
 * for inspection purposes. Generally, this implementation is slow and should not be used
 * in productive mode
 *
 * @param alloc must be non-null
 * @return STATUS_OK in case of non-null parameter alloc, STATUS_NULLPTR otherwise
 */
int jak_trace_alloc_create(jak_allocator *alloc);

JAK_END_DECL

#endif
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

#ifndef JAK_CACHE_H
#define JAK_CACHE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_types.h>
#include <jak_archive_query.h>

JAK_BEGIN_DECL

typedef struct jak_sid_cache_stats {
        size_t num_hits;
        size_t num_misses;
        size_t num_evicted;
} jak_sid_cache_stats;

bool jak_string_id_cache_create_lru(struct jak_string_cache **cache, jak_archive *archive);
bool jak_string_id_cache_create_lru_ex(struct jak_string_cache **cache, jak_archive *archive, size_t capacity);
bool jak_string_id_cache_get_error(jak_error *err, const struct jak_string_cache *cache);
bool jak_string_id_cache_get_size(size_t *size, const struct jak_string_cache *cache);
char *jak_string_id_cache_get(struct jak_string_cache *cache, jak_archive_field_sid_t id);
bool jak_string_id_cache_get_statistics(jak_sid_cache_stats *statistics, struct jak_string_cache *cache);
bool jak_string_id_cache_reset_statistics(struct jak_string_cache *cache);
bool jak_string_id_cache_drop(struct jak_string_cache *cache);

JAK_END_DECL

#endif
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

#ifndef JAK_ARCHIVE_DOC_CONVERTER_H
#define JAK_ARCHIVE_DOC_CONVERTER_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_encoded_doc.h>

JAK_BEGIN_DECL

bool jak_archive_converter(jak_encoded_doc_list *collection, jak_archive *archive);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_ARCHIVE_H
#define JAK_ARCHIVE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_memblock.h>
#include <jak_memfile.h>
#include <jak_pack.h>
#include <jak_column_doc.h>
#include <jak_archive_io.h>
#include <jak_archive_int.h>

JAK_BEGIN_DECL

typedef struct jak_archive {
        jak_archive_info info;
        char *disk_file_path;
        jak_string_table jak_string_table;
        jak_record_table record_table;
        jak_error err;
        struct jak_sid_to_offset *query_index_jak_string_id_to_offset;
        struct jak_string_cache *jak_string_id_cache;
        jak_archive_query *default_query;
} jak_archive;

typedef struct jak_archive_callback {
        void (*begin_create_from_model)();
        void (*end_create_from_model)();
        void (*begin_create_from_json)();
        void (*end_create_from_json)();
        void (*begin_jak_archive_stream_from_json)();
        void (*end_jak_archive_stream_from_json)();
        void (*begin_write_archive_file_to_disk)();
        void (*end_write_archive_file_to_disk)();
        void (*begin_load_archive)();
        void (*end_load_archive)();
        void (*begin_setup_jak_string_dict_ionary)();
        void (*end_setup_jak_string_dict_ionary)();
        void (*begin_parse_json)();
        void (*end_parse_json)();
        void (*begin_test_json)();
        void (*end_test_json)();
        void (*begin_import_json)();
        void (*end_import_json)();
        void (*begin_cleanup)();
        void (*end_cleanup)();
        void (*begin_write_jak_string_table)();
        void (*end_write_jak_string_table)();
        void (*begin_write_record_table)();
        void (*end_write_record_table)();
        void (*skip_jak_string_id_index_baking)();
        void (*begin_jak_string_id_index_baking)();
        void (*end_jak_string_id_index_baking)();
} jak_archive_callback;

JAK_DEFINE_GET_ERROR_FUNCTION(archive, jak_archive, archive);

bool jak_archive_from_json(jak_archive *out, const char *file, jak_error *err, const char *json_string, jak_packer_e compressor, jak_str_dict_tag_e dictionary, size_t num_async_dic_threads, bool read_optimized, bool bake_jak_string_id_index, jak_archive_callback *callback);
bool jak_archive_stream_from_json(jak_memblock **stream, jak_error *err, const char *json_string, jak_packer_e compressor, jak_str_dict_tag_e dictionary, size_t num_async_dic_threads, bool read_optimized, bool bake_id_index, jak_archive_callback *callback);
bool jak_archive_from_model(jak_memblock **stream, jak_error *err, jak_column_doc *model, jak_packer_e compressor, bool bake_jak_string_id_index, jak_archive_callback *callback);
bool jak_archive_write(FILE *file, const jak_memblock *stream);
bool jak_archive_load(jak_memblock **stream, FILE *file);
bool jak_archive_print(FILE *file, jak_error *err, jak_memblock *stream);
bool jak_archive_open(jak_archive *out, const char *file_path);
bool jak_archive_get_info(jak_archive_info *info, const jak_archive *archive);
bool jak_archive_close(jak_archive *archive);
bool jak_archive_drop_indexes(jak_archive *archive);
bool jak_archive_query_run(jak_archive_query *query, jak_archive *archive);
bool jak_archive_has_query_index_jak_string_id_to_offset(bool *state, jak_archive *archive);
bool jak_archive_hash_query_jak_string_id_cache(bool *has_cache, jak_archive *archive);
bool jak_archive_drop_query_jak_string_id_cache(jak_archive *archive);
struct jak_string_cache *jak_archive_get_query_jak_string_id_cache(jak_archive *archive);
jak_archive_query *jak_archive_query_default(jak_archive *archive);

/**
 * Creates a new <code>jak_archive_io_context</code> to access the archives underlying file for unsafe operations.
 *
 * An unsafe operation directly seeks randomly in the underlying file. To avoid creation of multiple file
 * descriptors while at the same time allow to access unsafe operations in a multi-threading environment, an
 * <code>jak_archive_io_context</code> is used. Roughly, such a context is a regular FILE that is protected by a lock.
 *
 * @param archive The archive
 * @return a heap-allocated instance of <code>jak_archive_io_context</code>, or NULL if not successful
 */
jak_archive_io_context *jak_archive_io_context_create(jak_archive *archive);

JAK_END_DECL

#endif/**
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

#ifndef JAK_INTERNALS_ARCHIVE_H
#define JAK_INTERNALS_ARCHIVE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_memfile.h>
#include <jak_types.h>
#include <jak_unique_id.h>
#include <jak_pack.h>

JAK_BEGIN_DECL

typedef struct __attribute__((packed)) jak_archive_header {
        char magic[9];
        jak_u8 version;
        jak_offset_t root_object_header_offset;
        jak_offset_t jak_string_id_to_offset_index_offset;
} jak_archive_header;

typedef struct __attribute__((packed)) jak_record_header {
        char marker;
        jak_u8 flags;
        jak_u64 record_size;
} jak_record_header;

typedef struct __attribute__((packed)) jak_object_header {
        char marker;
        jak_uid_t oid;
        jak_u32 flags;
} jak_object_header;

typedef struct __attribute__((packed)) jak_prop_header {
        char marker;
        jak_u32 num_entries;
} jak_prop_header;

typedef union __attribute__((packed)) jak_string_tab_flags {
        struct {
                jak_u8 compressor_none
                        : 1;
                jak_u8 compressed_huffman
                        : 1;
        } bits;
        jak_u8 value;
} jak_string_tab_flags_u;

typedef struct __attribute__((packed)) jak_string_table_header {
        char marker;
        jak_u32 num_entries;
        jak_u8 flags;
        jak_offset_t first_entry;
        jak_offset_t compressor_extra_size;
} jak_string_table_header;

typedef struct __attribute__((packed)) jak_object_array_header {
        char marker;
        jak_u8 num_entries;
} jak_object_array_header;

typedef struct __attribute__((packed)) jak_column_group_header {
        char marker;
        jak_u32 num_columns;
        jak_u32 num_objects;
} jak_column_group_header;

typedef struct __attribute__((packed)) jak_column_header {
        char marker;
        jak_archive_field_sid_t column_name;
        char value_type;
        jak_u32 num_entries;
} jak_column_header;

typedef union jak_object_flags {
        struct {
                jak_u32 has_null_props
                        : 1;
                jak_u32 has_bool_props
                        : 1;
                jak_u32 has_int8_props
                        : 1;
                jak_u32 has_int16_props
                        : 1;
                jak_u32 has_int32_props
                        : 1;
                jak_u32 has_int64_props
                        : 1;
                jak_u32 has_uint8_props
                        : 1;
                jak_u32 has_uint16_props
                        : 1;
                jak_u32 has_uint32_props
                        : 1;
                jak_u32 has_uint64_props
                        : 1;
                jak_u32 has_float_props
                        : 1;
                jak_u32 has_jak_string_props
                        : 1;
                jak_u32 has_object_props
                        : 1;
                jak_u32 has_null_array_props
                        : 1;
                jak_u32 has_bool_array_props
                        : 1;
                jak_u32 has_int8_array_props
                        : 1;
                jak_u32 has_int16_array_props
                        : 1;
                jak_u32 has_int32_array_props
                        : 1;
                jak_u32 has_int64_array_props
                        : 1;
                jak_u32 has_uint8_array_props
                        : 1;
                jak_u32 has_uint16_array_props
                        : 1;
                jak_u32 has_uint32_array_props
                        : 1;
                jak_u32 has_uint64_array_props
                        : 1;
                jak_u32 has_float_array_props
                        : 1;
                jak_u32 has_jak_string_array_props
                        : 1;
                jak_u32 has_object_array_props
                        : 1;
                jak_u32 RESERVED_27
                        : 1;
                jak_u32 RESERVED_28
                        : 1;
                jak_u32 RESERVED_29
                        : 1;
                jak_u32 RESERVED_30
                        : 1;
                jak_u32 RESERVED_31
                        : 1;
                jak_u32 RESERVED_32
                        : 1;
        } bits;
        jak_u32 value;
} jak_object_flags_u;

typedef struct jak_archive_prop_offs {
        jak_offset_t nulls;
        jak_offset_t bools;
        jak_offset_t int8s;
        jak_offset_t int16s;
        jak_offset_t int32s;
        jak_offset_t int64s;
        jak_offset_t uint8s;
        jak_offset_t uint16s;
        jak_offset_t uint32s;
        jak_offset_t uint64s;
        jak_offset_t floats;
        jak_offset_t strings;
        jak_offset_t objects;
        jak_offset_t null_arrays;
        jak_offset_t bool_arrays;
        jak_offset_t int8_arrays;
        jak_offset_t int16_arrays;
        jak_offset_t int32_arrays;
        jak_offset_t int64_arrays;
        jak_offset_t uint8_arrays;
        jak_offset_t uint16_arrays;
        jak_offset_t uint32_arrays;
        jak_offset_t uint64_arrays;
        jak_offset_t float_arrays;
        jak_offset_t jak_string_arrays;
        jak_offset_t object_arrays;
} jak_archive_prop_offs;

typedef struct jak_fixed_prop {
        jak_prop_header *header;
        const jak_archive_field_sid_t *keys;
        const void *values;
} jak_fixed_prop;

typedef struct jak_table_prop {
        jak_prop_header *header;
        const jak_archive_field_sid_t *keys;
        const jak_offset_t *group_offs;
} jak_table_prop;

typedef struct jak_var_prop {
        jak_prop_header *header;
        const jak_archive_field_sid_t *keys;
        const jak_offset_t *offsets;
        const void *values;
} jak_var_prop;

typedef struct jak_array_prop {
        jak_prop_header *header;
        const jak_archive_field_sid_t *keys;
        const jak_u32 *lengths;
        jak_offset_t values_begin;
} jak_array_prop;

typedef struct jak_null_prop {
        jak_prop_header *header;
        const jak_archive_field_sid_t *keys;
} jak_null_prop;

typedef enum jak_archive_marker {
        JAK_MARKER_TYPE_OBJECT_BEGIN = 0,
        JAK_MARKER_TYPE_OBJECT_END = 1,
        JAK_MARKER_TYPE_PROP_NULL = 2,
        JAK_MARKER_TYPE_PROP_BOOLEAN = 3,
        JAK_MARKER_TYPE_PROP_INT8 = 4,
        JAK_MARKER_TYPE_PROP_INT16 = 5,
        JAK_MARKER_TYPE_PROP_INT32 = 6,
        JAK_MARKER_TYPE_PROP_INT64 = 7,
        JAK_MARKER_TYPE_PROP_UINT8 = 8,
        JAK_MARKER_TYPE_PROP_UINT16 = 9,
        JAK_MARKER_TYPE_PROP_UINT32 = 10,
        JAK_MARKER_TYPE_PROP_UINT64 = 11,
        JAK_MARKER_TYPE_PROP_REAL = 12,
        JAK_MARKER_TYPE_PROP_TEXT = 13,
        JAK_MARKER_TYPE_PROP_OBJECT = 14,
        JAK_MARKER_TYPE_PROP_NULL_ARRAY = 15,
        JAK_MARKER_TYPE_PROP_BOOLEAN_ARRAY = 16,
        JAK_MARKER_TYPE_PROP_INT8_ARRAY = 17,
        JAK_MARKER_TYPE_PROP_INT16_ARRAY = 18,
        JAK_MARKER_TYPE_PROP_INT32_ARRAY = 19,
        JAK_MARKER_TYPE_PROP_INT64_ARRAY = 20,
        JAK_MARKER_TYPE_PROP_UINT8_ARRAY = 21,
        JAK_MARKER_TYPE_PROP_UINT16_ARRAY = 22,
        JAK_MARKER_TYPE_PROP_UINT32_ARRAY = 23,
        JAK_MARKER_TYPE_PROP_UINT64_ARRAY = 24,
        JAK_MARKER_TYPE_PROP_REAL_ARRAY = 25,
        JAK_MARKER_TYPE_PROP_TEXT_ARRAY = 26,
        JAK_MARKER_TYPE_PROP_OBJECT_ARRAY = 27,
        JAK_MARKER_TYPE_EMBEDDED_STR_DIC = 28,
        JAK_MARKER_TYPE_EMBEDDED_UNCOMP_STR = 29,
        JAK_MARKER_TYPE_COLUMN_GROUP = 30,
        JAK_MARKER_TYPE_COLUMN = 31,
        JAK_MARKER_TYPE_HUFFMAN_DIC_ENTRY = 32,
        JAK_MARKER_TYPE_RECORD_HEADER = 33,
} jak_archive_marker_e;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static jak_archive_header this_file_header = {.version = JAK_CARBON_ARCHIVE_VERSION, .root_object_header_offset = 0};

static struct {
        jak_archive_marker_e type;
        char symbol;
} jak_global_marker_symbols[] =
        {{JAK_MARKER_TYPE_OBJECT_BEGIN,        JAK_MARKER_SYMBOL_OBJECT_BEGIN},
         {JAK_MARKER_TYPE_OBJECT_END,          JAK_MARKER_SYMBOL_OBJECT_END},
         {JAK_MARKER_TYPE_PROP_NULL,           JAK_MARKER_SYMBOL_PROP_NULL},
         {JAK_MARKER_TYPE_PROP_BOOLEAN,        JAK_MARKER_SYMBOL_PROP_BOOLEAN},
         {JAK_MARKER_TYPE_PROP_INT8,           JAK_MARKER_SYMBOL_PROP_INT8},
         {JAK_MARKER_TYPE_PROP_INT16,          JAK_MARKER_SYMBOL_PROP_INT16},
         {JAK_MARKER_TYPE_PROP_INT32,          JAK_MARKER_SYMBOL_PROP_INT32},
         {JAK_MARKER_TYPE_PROP_INT64,          JAK_MARKER_SYMBOL_PROP_INT64},
         {JAK_MARKER_TYPE_PROP_UINT8,          JAK_MARKER_SYMBOL_PROP_UINT8},
         {JAK_MARKER_TYPE_PROP_UINT16,         JAK_MARKER_SYMBOL_PROP_UINT16},
         {JAK_MARKER_TYPE_PROP_UINT32,         JAK_MARKER_SYMBOL_PROP_UINT32},
         {JAK_MARKER_TYPE_PROP_UINT64,         JAK_MARKER_SYMBOL_PROP_UINT64},
         {JAK_MARKER_TYPE_PROP_REAL,           JAK_MARKER_SYMBOL_PROP_REAL},
         {JAK_MARKER_TYPE_PROP_TEXT,           JAK_MARKER_SYMBOL_PROP_TEXT},
         {JAK_MARKER_TYPE_PROP_OBJECT,         JAK_MARKER_SYMBOL_PROP_OBJECT},
         {JAK_MARKER_TYPE_PROP_NULL_ARRAY,     JAK_MARKER_SYMBOL_PROP_NULL_ARRAY},
         {JAK_MARKER_TYPE_PROP_BOOLEAN_ARRAY,  JAK_MARKER_SYMBOL_PROP_BOOLEAN_ARRAY},
         {JAK_MARKER_TYPE_PROP_INT8_ARRAY,     JAK_MARKER_SYMBOL_PROP_INT8_ARRAY},
         {JAK_MARKER_TYPE_PROP_INT16_ARRAY,    JAK_MARKER_SYMBOL_PROP_INT16_ARRAY},
         {JAK_MARKER_TYPE_PROP_INT32_ARRAY,    JAK_MARKER_SYMBOL_PROP_INT32_ARRAY},
         {JAK_MARKER_TYPE_PROP_INT64_ARRAY,    JAK_MARKER_SYMBOL_PROP_INT64_ARRAY},
         {JAK_MARKER_TYPE_PROP_UINT8_ARRAY,    JAK_MARKER_SYMBOL_PROP_UINT8_ARRAY},
         {JAK_MARKER_TYPE_PROP_UINT16_ARRAY,   JAK_MARKER_SYMBOL_PROP_UINT16_ARRAY},
         {JAK_MARKER_TYPE_PROP_UINT32_ARRAY,   JAK_MARKER_SYMBOL_PROP_UINT32_ARRAY},
         {JAK_MARKER_TYPE_PROP_UINT64_ARRAY,   JAK_MARKER_SYMBOL_PROP_UINT64_ARRAY},
         {JAK_MARKER_TYPE_PROP_REAL_ARRAY,     JAK_MARKER_SYMBOL_PROP_REAL_ARRAY},
         {JAK_MARKER_TYPE_PROP_TEXT_ARRAY,     JAK_MARKER_SYMBOL_PROP_TEXT_ARRAY},
         {JAK_MARKER_TYPE_PROP_OBJECT_ARRAY,   JAK_MARKER_SYMBOL_PROP_OBJECT_ARRAY},
         {JAK_MARKER_TYPE_EMBEDDED_STR_DIC,    JAK_MARKER_SYMBOL_EMBEDDED_STR_DIC},
         {JAK_MARKER_TYPE_EMBEDDED_UNCOMP_STR, JAK_MARKER_SYMBOL_EMBEDDED_STR},
         {JAK_MARKER_TYPE_COLUMN_GROUP,        JAK_MARKER_SYMBOL_COLUMN_GROUP},
         {JAK_MARKER_TYPE_COLUMN,              JAK_MARKER_SYMBOL_COLUMN},
         {JAK_MARKER_TYPE_HUFFMAN_DIC_ENTRY,   JAK_MARKER_SYMBOL_HUFFMAN_DIC_ENTRY},
         {JAK_MARKER_TYPE_RECORD_HEADER,       JAK_MARKER_SYMBOL_RECORD_HEADER}};

static struct {
        jak_archive_field_e value_type;
        jak_archive_marker_e marker;
} jak_global_value_array_marker_mapping[] =
        {{JAK_FIELD_NULL,    JAK_MARKER_TYPE_PROP_NULL_ARRAY},
         {JAK_FIELD_BOOLEAN, JAK_MARKER_TYPE_PROP_BOOLEAN_ARRAY},
         {JAK_FIELD_INT8,    JAK_MARKER_TYPE_PROP_INT8_ARRAY},
         {JAK_FIELD_INT16,   JAK_MARKER_TYPE_PROP_INT16_ARRAY},
         {JAK_FIELD_INT32,   JAK_MARKER_TYPE_PROP_INT32_ARRAY},
         {JAK_FIELD_INT64,   JAK_MARKER_TYPE_PROP_INT64_ARRAY},
         {JAK_FIELD_UINT8,   JAK_MARKER_TYPE_PROP_UINT8_ARRAY},
         {JAK_FIELD_UINT16,  JAK_MARKER_TYPE_PROP_UINT16_ARRAY},
         {JAK_FIELD_UINT32,  JAK_MARKER_TYPE_PROP_UINT32_ARRAY},
         {JAK_FIELD_UINT64,  JAK_MARKER_TYPE_PROP_UINT64_ARRAY},
         {JAK_FIELD_FLOAT,   JAK_MARKER_TYPE_PROP_REAL_ARRAY},
         {JAK_FIELD_STRING,  JAK_MARKER_TYPE_PROP_TEXT_ARRAY},
         {JAK_FIELD_OBJECT,  JAK_MARKER_TYPE_PROP_OBJECT_ARRAY}}, valueMarkerMapping[] =
        {{JAK_FIELD_NULL,    JAK_MARKER_TYPE_PROP_NULL},
         {JAK_FIELD_BOOLEAN, JAK_MARKER_TYPE_PROP_BOOLEAN},
         {JAK_FIELD_INT8,    JAK_MARKER_TYPE_PROP_INT8},
         {JAK_FIELD_INT16,   JAK_MARKER_TYPE_PROP_INT16},
         {JAK_FIELD_INT32,   JAK_MARKER_TYPE_PROP_INT32},
         {JAK_FIELD_INT64,   JAK_MARKER_TYPE_PROP_INT64},
         {JAK_FIELD_UINT8,   JAK_MARKER_TYPE_PROP_UINT8},
         {JAK_FIELD_UINT16,  JAK_MARKER_TYPE_PROP_UINT16},
         {JAK_FIELD_UINT32,  JAK_MARKER_TYPE_PROP_UINT32},
         {JAK_FIELD_UINT64,  JAK_MARKER_TYPE_PROP_UINT64},
         {JAK_FIELD_FLOAT,   JAK_MARKER_TYPE_PROP_REAL},
         {JAK_FIELD_STRING,  JAK_MARKER_TYPE_PROP_TEXT},
         {JAK_FIELD_OBJECT,  JAK_MARKER_TYPE_PROP_OBJECT}};

#pragma GCC diagnostic pop

typedef struct jak_record_flags {
        struct {
                jak_u8 is_sorted
                        : 1;
                jak_u8 RESERVED_2
                        : 1;
                jak_u8 RESERVED_3
                        : 1;
                jak_u8 RESERVED_4
                        : 1;
                jak_u8 RESERVED_5
                        : 1;
                jak_u8 RESERVED_6
                        : 1;
                jak_u8 RESERVED_7
                        : 1;
                jak_u8 RESERVED_8
                        : 1;
        } bits;
        jak_u8 value;
} jak_record_flags;

typedef struct jak_string_table {
        jak_packer compressor;
        jak_offset_t first_entry_off;
        jak_u32 num_embeddded_strings;
} jak_string_table;

typedef struct jak_record_table {
        jak_record_flags flags;
        jak_memblock *record_db;
} jak_record_table;

typedef struct jak_archive_info {
        size_t jak_string_table_size;
        size_t record_table_size;
        size_t jak_string_id_index_size;
        jak_u32 num_embeddded_strings;
} jak_archive_info;

typedef struct __attribute__((packed)) jak_string_entry_header {
        char marker;
        jak_offset_t next_entry_off;
        jak_archive_field_sid_t jak_string_id;
        jak_u32 jak_string_len;
} jak_string_entry_header;

void jak_int_read_prop_offsets(jak_archive_prop_offs *prop_offsets, jak_memfile *memfile, const jak_object_flags_u *flags);
void jak_int_embedded_fixed_props_read(jak_fixed_prop *prop, jak_memfile *memfile);
void jak_int_embedded_var_props_read(jak_var_prop *prop, jak_memfile *memfile);
void jak_int_embedded_null_props_read(jak_null_prop *prop, jak_memfile *memfile);
void jak_int_embedded_array_props_read(jak_array_prop *prop, jak_memfile *memfile);
void jak_int_embedded_table_props_read(jak_table_prop *prop, jak_memfile *memfile);
jak_archive_field_e jak_int_get_value_type_of_char(char c);
jak_archive_field_e jak_int_marker_to_field_type(char symbol);

JAK_END_DECL

#endif
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

#ifndef JAK_IO_CONTEXT_H
#define JAK_IO_CONTEXT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>

JAK_BEGIN_DECL

bool jak_io_context_create(jak_archive_io_context **context, jak_error *err, const char *file_path);
jak_error *jak_io_context_get_error(jak_archive_io_context *context);
FILE *jak_io_context_lock_and_access(jak_archive_io_context *context);
bool jak_io_context_unlock(jak_archive_io_context *context);
bool jak_io_context_drop(jak_archive_io_context *context);

JAK_END_DECL

#endif
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

#ifndef JAK_ARCHIVE_ITER_H
#define JAK_ARCHIVE_ITER_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_archive.h>

JAK_BEGIN_DECL

typedef enum jak_prop_iter_state {
        JAK_PROP_ITER_INIT,
        JAK_PROP_ITER_NULLS,
        JAK_PROP_ITER_BOOLS,
        JAK_PROP_ITER_INT8S,
        JAK_PROP_ITER_INT16S,
        JAK_PROP_ITER_INT32S,
        JAK_PROP_ITER_INT64S,
        JAK_PROP_ITER_UINT8S,
        JAK_PROP_ITER_UINT16S,
        JAK_PROP_ITER_UINT32S,
        JAK_PROP_ITER_UINT64S,
        JAK_PROP_ITER_FLOATS,
        JAK_PROP_ITER_STRINGS,
        JAK_PROP_ITER_OBJECTS,
        JAK_PROP_ITER_NULL_ARRAYS,
        JAK_PROP_ITER_BOOL_ARRAYS,
        JAK_PROP_ITER_INT8_ARRAYS,
        JAK_PROP_ITER_INT16_ARRAYS,
        JAK_PROP_ITER_INT32_ARRAYS,
        JAK_PROP_ITER_INT64_ARRAYS,
        JAK_PROP_ITER_UINT8_ARRAYS,
        JAK_PROP_ITER_UINT16_ARRAYS,
        JAK_PROP_ITER_UINT32_ARRAYS,
        JAK_PROP_ITER_UINT64_ARRAYS,
        JAK_PROP_ITER_FLOAT_ARRAYS,
        JAK_PROP_ITER_STRING_ARRAYS,
        JAK_PROP_ITER_OBJECT_ARRAYS,
        JAK_PROP_ITER_DONE
} jak_prop_iter_state_e;

typedef struct jak_archive_object {
        jak_uid_t object_id;                  /* unique object id */
        jak_offset_t offset;                        /* this objects header offset */
        jak_archive_prop_offs prop_offsets;  /* per-property type offset in the record table byte stream */
        jak_offset_t next_obj_off;                  /* offset to next object in list, or NULL if no such exists */
        jak_memfile memfile;
        jak_error err;
} jak_archive_object;

typedef enum jak_prop_iter_mode {
        JAK_PROP_ITER_MODE_OBJECT,
        JAK_PROP_ITER_MODE_COLLECTION,
} jak_prop_iter_mode_e;

typedef struct jak_object_iter_state {
        jak_fixed_prop prop_group_header;    /* type, num props and keys */
        jak_offset_t current_prop_group_off;
        jak_offset_t prop_data_off;
        const jak_archive_field_sid_t *keys;                /* current property key in this iteration */
        enum jak_archive_field_type type;                   /* property basic value type (e.g., int8, or object) */
        bool is_array;                          /* flag indicating that property is an array type */
} jak_object_iter_state;

typedef struct jak_collection_iter_state {
        jak_offset_t collection_start_off;
        jak_u32 num_column_groups;
        jak_u32 current_column_group_idx;
        const jak_archive_field_sid_t *column_group_keys;
        const jak_offset_t *column_group_offsets;

        struct {
                jak_u32 num_columns;
                jak_u32 num_objects;
                const jak_uid_t *object_ids;
                const jak_offset_t *column_offs;
                struct {
                        jak_u32 idx;
                        jak_archive_field_sid_t name;
                        enum jak_archive_field_type type;
                        jak_u32 num_elem;
                        const jak_offset_t *elem_offsets;
                        const jak_u32 *elem_positions;
                        struct {
                                jak_u32 idx;
                                jak_u32 array_length;
                                const void *array_base;
                        } current_entry;
                } current_column;
        } current_column_group;
} jak_collection_iter_state;

typedef struct jak_archive_value_vector {
        jak_prop_iter *prop_iter;            /* pointer to property iterator that created this iterator */
        jak_memfile record_table_memfile;    /* iterator-local read-only memfile on archive record table */
        enum jak_archive_field_type prop_type;              /* property basic value type (e.g., int8, or object) */
        bool is_array;                          /* flag indicating whether value type is an array or not */
        jak_offset_t data_off;                      /* offset in memfile where type-dependent data begins */
        jak_u32 value_max_idx;                      /* maximum index of a value callable by 'at' functions */
        jak_error err;                         /* JAK_ERROR information */
        jak_uid_t object_id;                  /* current object id */
        const jak_archive_field_sid_t *keys;
        union {
                struct {
                        const jak_offset_t *offsets;
                        jak_archive_object object;
                } object;
                struct {
                        union {
                                const jak_archive_field_i8_t *int8s;
                                const jak_archive_field_i16_t *int16s;
                                const jak_archive_field_i32_t *int32s;
                                const jak_archive_field_i64_t *int64s;
                                const jak_archive_field_u8_t *uint8s;
                                const jak_archive_field_u16_t *uint16s;
                                const jak_archive_field_u32_t *uint32s;
                                const jak_archive_field_u64_t *uint64s;
                                const jak_archive_field_number_t *numbers;
                                const jak_archive_field_sid_t *strings;
                                const jak_archive_field_boolean_t *booleans;
                        } values;
                } basic;
                struct {
                        union {
                                const jak_u32 *array_lengths;
                                const jak_u32 *num_nulls_contained;
                        } meta;

                        union {
                                const jak_archive_field_i8_t *int8s_base;
                                const jak_archive_field_i16_t *int16s_base;
                                const jak_archive_field_i32_t *int32s_base;
                                const jak_archive_field_i64_t *int64s_base;
                                const jak_archive_field_u8_t *uint8s_base;
                                const jak_archive_field_u16_t *uint16s_base;
                                const jak_archive_field_u32_t *uint32s_base;
                                const jak_archive_field_u64_t *uint64s_base;
                                const jak_archive_field_number_t *numbers_base;
                                const jak_archive_field_sid_t *strings_base;
                                const jak_archive_field_boolean_t *booleans_base;
                        } values;
                } arrays;
        } data;
} jak_archive_value_vector;

typedef struct jak_prop_iter {
        jak_archive_object object;                 /* current object */
        jak_memfile record_table_memfile;          /* iterator-local read-only memfile on archive record table */
        jak_u16 mask;                                     /* user-defined mask which properties to include */
        jak_prop_iter_mode_e mode;                     /* determines whether to iterating over object or collection */
        jak_error err;                               /* JAK_ERROR information */
        jak_prop_iter_state_e prop_cursor;             /* current property type in iteration */
        jak_object_iter_state mode_object;
        jak_collection_iter_state mode_collection;
} jak_prop_iter;

typedef struct jak_independent_iter_state {
        jak_memfile record_table_memfile;           /* iterator-local read-only memfile on archive record table */
        jak_collection_iter_state state;            /* iterator-local state */
        jak_error err;                                /* JAK_ERROR information */
} jak_independent_iter_state;

typedef struct jak_column_object_iter {
        jak_memfile memfile;
        jak_collection_iter_state entry_state;
        jak_archive_object obj;
        jak_offset_t next_obj_off;
        jak_error err;
} jak_column_object_iter;

#define JAK_ARCHIVE_ITER_MASK_PRIMITIVES             (1 << 1)
#define JAK_ARCHIVE_ITER_MASK_ARRAYS                 (1 << 2)

#define JAK_ARCHIVE_ITER_MASK_INT8                   (1 << 3)
#define JAK_ARCHIVE_ITER_MASK_INT16                  (1 << 4)
#define JAK_ARCHIVE_ITER_MASK_INT32                  (1 << 5)
#define JAK_ARCHIVE_ITER_MASK_INT64                  (1 << 6)
#define JAK_ARCHIVE_ITER_MASK_UINT8                  (1 << 7)
#define JAK_ARCHIVE_ITER_MASK_UINT16                 (1 << 8)
#define JAK_ARCHIVE_ITER_MASK_UINT32                 (1 << 9)
#define JAK_ARCHIVE_ITER_MASK_UINT64                 (1 << 10)
#define JAK_ARCHIVE_ITER_MASK_NUMBER                 (1 << 11)
#define JAK_ARCHIVE_ITER_MASK_STRING                 (1 << 12)
#define JAK_ARCHIVE_ITER_MASK_BOOLEAN                (1 << 13)
#define JAK_ARCHIVE_ITER_MASK_NULL                   (1 << 14)
#define JAK_ARCHIVE_ITER_MASK_OBJECT                 (1 << 15)

#define JAK_ARCHIVE_ITER_MASK_INTEGER               JAK_ARCHIVE_ITER_MASK_INT8       |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_INT16      |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_INT32      |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_INT64      |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_UINT8      |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_UINT16     |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_UINT32     |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_UINT64

#define JAK_ARCHIVE_ITER_MASK_ANY                   JAK_ARCHIVE_ITER_MASK_PRIMITIVES |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_ARRAYS     |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_INTEGER    |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_NUMBER     |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_STRING     |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_BOOLEAN    |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_NULL       |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_OBJECT

JAK_DEFINE_GET_ERROR_FUNCTION(archive_value_vector, jak_archive_value_vector, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_prop_iter, jak_prop_iter, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_collection_iter, jak_independent_iter_state, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_column_group_iter, jak_independent_iter_state, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_column_iter, jak_independent_iter_state, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_column_entry_iter, jak_independent_iter_state, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_column_entry_object_iter, jak_column_object_iter, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_object, jak_archive_object, obj)

bool jak_archive_prop_iter_from_archive(jak_prop_iter *iter, jak_error *err, jak_u16 mask, jak_archive *archive);
bool jak_archive_prop_iter_from_object(jak_prop_iter *iter, jak_u16 mask, jak_error *err, const jak_archive_object *obj);
bool jak_archive_value_jak_vector_from_prop_iter(jak_archive_value_vector *value, jak_error *err, jak_prop_iter *prop_iter);
bool jak_archive_prop_iter_next(jak_prop_iter_mode_e *type, jak_archive_value_vector *value_vector, jak_independent_iter_state *collection_iter, jak_prop_iter *prop_iter);
const jak_archive_field_sid_t *jak_archive_collection_iter_get_keys(jak_u32 *num_keys, jak_independent_iter_state *iter);
bool jak_archive_collection_next_column_group(jak_independent_iter_state *group_iter, jak_independent_iter_state *iter);
const jak_uid_t *jak_archive_column_group_get_object_ids(jak_u32 *num_objects, jak_independent_iter_state *iter);
bool jak_archive_column_group_next_column(jak_independent_iter_state *column_iter, jak_independent_iter_state *iter);
bool jak_archive_column_get_name(jak_archive_field_sid_t *name, enum jak_archive_field_type *type, jak_independent_iter_state *column_iter);
const jak_u32 * jak_archive_column_get_entry_positions(jak_u32 *num_entry, jak_independent_iter_state *column_iter);
bool jak_archive_column_next_entry(jak_independent_iter_state *entry_iter, jak_independent_iter_state *iter);
bool jak_archive_column_entry_get_type(enum jak_archive_field_type *type, jak_independent_iter_state *entry);

#define JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(built_in_type, name)                                            \
const built_in_type *                                                                                      \
jak_archive_column_entry_get_##name(jak_u32 *array_length, jak_independent_iter_state *entry);

JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_i8_t, int8s);
JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_i16_t, int16s);
JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_i32_t, int32s);
JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_i64_t, int64s);
JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u8_t, uint8s);
JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u16_t, uint16s);
JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u32_t, uint32s);
JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u64_t, uint64s);
JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_sid_t, strings);
JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_number_t, numbers);
JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_boolean_t, booleans);
JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u32_t, nulls);

bool jak_archive_column_entry_get_objects(jak_column_object_iter *iter, jak_independent_iter_state *entry);
const jak_archive_object *jak_archive_column_entry_object_iter_next_object(jak_column_object_iter *iter);
bool jak_archive_object_get_object_id(jak_uid_t *id, const jak_archive_object *object);
bool jak_archive_object_get_prop_iter(jak_prop_iter *iter, const jak_archive_object *object);
bool jak_archive_value_jak_vector_get_object_id(jak_uid_t *id, const jak_archive_value_vector *iter);
const jak_archive_field_sid_t *jak_archive_value_jak_vector_get_keys(jak_u32 *num_keys, jak_archive_value_vector *iter);
bool jak_archive_value_jak_vector_get_basic_type(enum jak_archive_field_type *type, const jak_archive_value_vector *value);
bool jak_archive_value_jak_vector_is_array_type(bool *is_array, const jak_archive_value_vector *value);
bool jak_archive_value_jak_vector_get_length(jak_u32 *length, const jak_archive_value_vector *value);
bool jak_archive_value_jak_vector_is_of_objects(bool *is_object, jak_archive_value_vector *value);
bool jak_archive_value_jak_vector_get_object_at(jak_archive_object *object, jak_u32 idx, jak_archive_value_vector *value);

#define JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(name)                                                            \
bool                                                                                                       \
jak_archive_value_jak_vector_is_##name(bool *type_match, jak_archive_value_vector *value);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int8);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int16);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int32);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int64);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint8);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint16);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint32);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint64);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(string);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(number);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(boolean);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(null);

#define JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(name, built_in_type)                                            \
const built_in_type *                                                                                      \
jak_archive_value_jak_vector_get_##name(jak_u32 *num_values, jak_archive_value_vector *value);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int8s, jak_archive_field_i8_t)
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int16s, jak_archive_field_i16_t)
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int32s, jak_archive_field_i32_t)
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int64s, jak_archive_field_i64_t)
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint8s, jak_archive_field_u8_t)
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint16s, jak_archive_field_u16_t)
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint32s, jak_archive_field_u32_t)
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint64s, jak_archive_field_u64_t)
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(strings, jak_archive_field_sid_t)
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(numbers, jak_archive_field_number_t)
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(booleans, jak_archive_field_boolean_t)

const jak_archive_field_u32_t *jak_archive_value_jak_vector_get_null_arrays(jak_u32 *num_values, jak_archive_value_vector *value);

#define JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(name, built_in_type)                                         \
const built_in_type *                                                                                      \
jak_archive_value_jak_vector_get_##name##_arrays_at(jak_u32 *array_length, jak_u32 idx,                                                \
                                               jak_archive_value_vector *value);                                    \


JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int8, jak_archive_field_i8_t);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int16, jak_archive_field_i16_t);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int32, jak_archive_field_i32_t);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int64, jak_archive_field_i64_t);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint8, jak_archive_field_u8_t);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint16, jak_archive_field_u16_t);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint32, jak_archive_field_u32_t);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint64, jak_archive_field_u64_t);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(string, jak_archive_field_sid_t);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(number, jak_archive_field_number_t);
JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(boolean, jak_archive_field_boolean_t);

void jak_archive_int_reset_carbon_object_mem_file(jak_archive_object *object);

JAK_END_DECL

#endif
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

#ifndef JAK_STRING_PRED_H
#define JAK_STRING_PRED_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>

JAK_BEGIN_DECL

typedef bool (*jak_string_pred_func_t)(size_t *idxs_matching, size_t *num_matching, char **strings, size_t num_strings, void *capture);

typedef struct jak_string_pred {
        jak_string_pred_func_t func;
        jak_i64 limit;
} jak_string_pred;

JAK_BUILT_IN(static bool) jak_string_pred_validate(jak_error *err, const jak_string_pred *pred)
{
        JAK_ERROR_IF_NULL(pred);
        JAK_ERROR_IF_NOT_IMPLEMENTED(err, pred, func)
        return true;
}

JAK_BUILT_IN(static bool) jak_string_pred_eval(const jak_string_pred *pred, size_t *idxs_matching,
                                               size_t *num_matching, char **strings, size_t num_strings, void *capture)
{
        JAK_ASSERT(pred);
        JAK_ASSERT(idxs_matching);
        JAK_ASSERT(num_matching);
        JAK_ASSERT(strings);
        JAK_ASSERT(pred->func);
        return pred->func(idxs_matching, num_matching, strings, num_strings, capture);
}

JAK_BUILT_IN(static bool) jak_string_pred_get_limit(jak_i64 *limit, const jak_string_pred *pred)
{
        JAK_ERROR_IF_NULL(limit);
        JAK_ERROR_IF_NULL(pred);
        *limit = pred->limit;
        return true;
}

JAK_END_DECL

#endif
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

#ifndef JAK_QUERY_H
#define JAK_QUERY_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_archive.h>
#include <jak_archive_strid_it.h>
#include <jak_archive_pred.h>
#include <jak_hash_table.h>

JAK_BEGIN_DECL

typedef struct jak_archive_query {
        jak_archive *archive;
        jak_archive_io_context *context;
        jak_error err;
} jak_archive_query;

JAK_DEFINE_GET_ERROR_FUNCTION(query, jak_archive_query, query)

bool jak_query_create(jak_archive_query *query, jak_archive *archive);
bool jak_query_drop(jak_archive_query *query);
bool jak_query_scan_strids(jak_strid_iter *it, jak_archive_query *query);
bool jak_query_create_index_jak_string_id_to_offset(struct jak_sid_to_offset **index, jak_archive_query *query);
void jak_query_drop_index_jak_string_id_to_offset(struct jak_sid_to_offset *index);
bool jak_query_index_id_to_offset_serialize(FILE *file, jak_error *err, struct jak_sid_to_offset *index);
bool jak_query_index_id_to_offset_deserialize(struct jak_sid_to_offset **index, jak_error *err, const char *file_path, jak_offset_t offset);
char *jak_query_fetch_jak_string_by_id(jak_archive_query *query, jak_archive_field_sid_t id);
char *jak_query_fetch_jak_string_by_id_nocache(jak_archive_query *query, jak_archive_field_sid_t id);
char **jak_query_fetch_strings_by_offset(jak_archive_query *query, jak_offset_t *offs, jak_u32 *strlens, size_t num_offs);
jak_archive_field_sid_t *jak_query_find_ids(size_t *num_found, jak_archive_query *query, const jak_string_pred *pred, void *capture, jak_i64 limit);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_STRID_ITER_H
#define JAK_STRID_ITER_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_types.h>
#include <jak_archive.h>

JAK_BEGIN_DECL

typedef struct jak_strid_info {
        jak_archive_field_sid_t id;
        jak_u32 strlen;
        jak_offset_t offset;
} jak_strid_info;

typedef struct jak_strid_iter {
        FILE *disk_file;
        bool is_open;
        jak_offset_t disk_offset;
        jak_strid_info vector[100000];
} jak_strid_iter;

bool jak_strid_iter_open(jak_strid_iter *it, jak_error *err, jak_archive *archive);
bool jak_strid_iter_next(bool *success, jak_strid_info **info, jak_error *err, size_t *info_length, jak_strid_iter *it);
bool jak_strid_iter_close(jak_strid_iter *it);

JAK_END_DECL

#endif
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

#ifndef JAK_ARCHIVE_VISITOR_H
#define JAK_ARCHIVE_VISITOR_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include "jak_archive_it.h"

typedef struct jak_path_entry {
        jak_archive_field_sid_t key;
        jak_u32 idx;
} jak_path_entry;

typedef struct jak_archive_visitor_desc {
        int visit_mask;                 /** bitmask of 'JAK_ARCHIVE_ITER_MASK_XXX' */
} jak_archive_visitor_desc;

typedef enum jak_visit_policy {
        JAK_VISIT_INCLUDE, JAK_VISIT_EXCLUDE,
} jak_visit_policy_e;

typedef const jak_vector ofType(jak_path_entry) *path_stack_t;

#define JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(name, built_in_type)                                                             \
void (*visit_##name##_pairs) (jak_archive *jak_archive, path_stack_t path, jak_uid_t id,                              \
                              const jak_archive_field_sid_t *keys, const built_in_type *values, jak_u32 num_pairs,                     \
                              void *capture);

#define JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(name, built_in_type)                                                             \
jak_visit_policy_e (*visit_enter_##name##_array_pairs)(jak_archive *jak_archive, path_stack_t path,                      \
                                                        jak_uid_t id, const jak_archive_field_sid_t *keys,                       \
                                                        jak_u32 num_pairs,                                                 \
                                                        void *capture);                                                \
                                                                                                                       \
void (*visit_enter_##name##_array_pair)(jak_archive *jak_archive, path_stack_t path, jak_uid_t id,                    \
                                        jak_archive_field_sid_t key, jak_u32 entry_idx, jak_u32 num_elems,                                 \
                                        void *capture);                                                                \
                                                                                                                       \
void (*visit_##name##_array_pair) (jak_archive *jak_archive, path_stack_t path, jak_uid_t id,                         \
                                   jak_archive_field_sid_t key, jak_u32 entry_idx, jak_u32 max_entries,                                    \
                                   const built_in_type *array, jak_u32 array_length, void *capture);                       \
                                                                                                                       \
void (*visit_leave_##name##_array_pair)(jak_archive *jak_archive, path_stack_t path, jak_uid_t id,                    \
                                        jak_u32 pair_idx, jak_u32 num_pairs, void *capture);                                   \
                                                                                                                       \
void (*visit_leave_##name##_array_pairs)(jak_archive *jak_archive, path_stack_t path, jak_uid_t id,                   \
                                         void *capture);

#define JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(name, built_in_type)                                                     \
    void (*visit_object_array_object_property_##name)(jak_archive *jak_archive, path_stack_t path,                      \
                                               jak_uid_t parent_id,                                                  \
                                               jak_archive_field_sid_t key,                                                        \
                                               jak_uid_t nested_object_id,                                           \
                                               jak_archive_field_sid_t nested_key,                                                 \
                                               const built_in_type *nested_values,                                     \
                                               jak_u32 num_nested_values, void *capture);

typedef struct jak_archive_visitor {
        void (*visit_root_object)(jak_archive *archive, jak_uid_t id, void *capture);
        void (*before_visit_starts)(jak_archive *archive, void *capture);
        void (*after_visit_ends)(jak_archive *archive, void *capture);
        jak_visit_policy_e (*before_object_visit)(jak_archive *archive, path_stack_t path, jak_uid_t parent_id, jak_uid_t value_id, jak_u32 object_idx, jak_u32 num_objects, jak_archive_field_sid_t key, void *capture);
        void (*after_object_visit)(jak_archive *archive, path_stack_t path, jak_uid_t id, jak_u32 object_idx, jak_u32 num_objects, void *capture);
        void (*first_prop_type_group)(jak_archive *archive, path_stack_t path, jak_uid_t id, const jak_archive_field_sid_t *keys, enum jak_archive_field_type type, bool is_array, jak_u32 num_pairs, void *capture);
        void (*next_prop_type_group)(jak_archive *archive, path_stack_t path, jak_uid_t id, const jak_archive_field_sid_t *keys, enum jak_archive_field_type type, bool is_array, jak_u32 num_pairs, void *capture);
        void (*visit_null_pairs)(jak_archive *archive, path_stack_t path, jak_uid_t id, const jak_archive_field_sid_t *keys, jak_u32 num_pairs, void *capture);

        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(int8, jak_archive_field_i8_t);
        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(int16, jak_archive_field_i16_t);
        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(int32, jak_archive_field_i32_t);
        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(int64, jak_archive_field_i64_t);
        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(uint8, jak_archive_field_u8_t);
        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(uint16, jak_archive_field_u16_t);
        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(uint32, jak_archive_field_u32_t);
        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(uint64, jak_archive_field_u64_t);
        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(number, jak_archive_field_number_t);
        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(string, jak_archive_field_sid_t);
        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(boolean, jak_archive_field_boolean_t);
        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(int8, jak_archive_field_i8_t);
        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(int16, jak_archive_field_i16_t);
        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(int32, jak_archive_field_i32_t);
        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(int64, jak_archive_field_i64_t);
        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint8, jak_archive_field_u8_t);
        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint16, jak_archive_field_u16_t);
        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint32, jak_archive_field_u32_t);
        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint64, jak_archive_field_u64_t);
        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(number, jak_archive_field_number_t);
        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(string, jak_archive_field_sid_t);
        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(boolean, jak_archive_field_boolean_t);

        jak_visit_policy_e (*visit_enter_null_array_pairs)(jak_archive *archive, path_stack_t path, jak_uid_t id, const jak_archive_field_sid_t *keys, jak_u32 num_pairs, void *capture);
        void (*visit_enter_null_array_pair)(jak_archive *archive, path_stack_t path, jak_uid_t id, jak_archive_field_sid_t key, jak_u32 entry_idx, jak_u32 num_elems, void *capture);
        void (*visit_null_array_pair)(jak_archive *archive, path_stack_t path, jak_uid_t id, jak_archive_field_sid_t key, jak_u32 entry_idx, jak_u32 max_entries, jak_archive_field_u32_t num_nulls, void *capture);
        void (*visit_leave_null_array_pair)(jak_archive *archive, path_stack_t path, jak_uid_t id, jak_u32 pair_idx, jak_u32 num_pairs, void *capture);
        void (*visit_leave_null_array_pairs)(jak_archive *archive, path_stack_t path, jak_uid_t id, void *capture);
        jak_visit_policy_e (*before_visit_object_array)(jak_archive *archive, path_stack_t path, jak_uid_t parent_id, jak_archive_field_sid_t key, void *capture);
        void (*before_visit_object_array_objects)(bool *skip_group_object_ids, jak_archive *archive, path_stack_t path, jak_uid_t parent_id, jak_archive_field_sid_t key, const jak_uid_t *group_object_ids, jak_u32 num_group_object_ids, void *capture);
        jak_visit_policy_e (*before_visit_object_array_object_property)(jak_archive *archive, path_stack_t path, jak_uid_t parent_id, jak_archive_field_sid_t key, jak_archive_field_sid_t nested_key, enum jak_archive_field_type nested_value_type, void *capture);

        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int8s, jak_archive_field_i8_t);
        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int16s, jak_archive_field_i16_t);
        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int32s, jak_archive_field_i32_t);
        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int64s, jak_archive_field_i64_t);
        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint8s, jak_archive_field_u8_t);
        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint16s, jak_archive_field_u16_t);
        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint32s, jak_archive_field_u32_t);
        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint64s, jak_archive_field_u64_t);
        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(numbers, jak_archive_field_number_t);
        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(strings, jak_archive_field_sid_t);
        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(booleans, jak_archive_field_boolean_t);
        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(nulls, jak_archive_field_u32_t);

        jak_visit_policy_e (*before_object_array_object_property_object)(jak_archive *archive, path_stack_t path, jak_uid_t parent_id, jak_archive_field_sid_t key, jak_uid_t nested_object_id, jak_archive_field_sid_t nested_key, jak_u32 nested_value_object_id, void *capture);
        void (*visit_object_property)(jak_archive *archive, path_stack_t path, jak_uid_t parent_id, jak_archive_field_sid_t key, enum jak_archive_field_type type, bool is_array_type, void *capture);
        void (*visit_object_array_prop)(jak_archive *archive, path_stack_t path, jak_uid_t parent_id, jak_archive_field_sid_t key, enum jak_archive_field_type type, void *capture);
        bool (*get_column_entry_count)(jak_archive *archive, path_stack_t path, jak_archive_field_sid_t key, enum jak_archive_field_type type, jak_u32 count, void *capture);
} jak_archive_visitor;

bool jak_archive_visit_archive(jak_archive *archive, const jak_archive_visitor_desc *desc, jak_archive_visitor *visitor, void *capture);
bool jak_archive_visitor_print_path(FILE *file, jak_archive *archive, const jak_vector ofType(jak_path_entry) *path_stack);
void jak_archive_visitor_path_to_string(char path_buffer[2048], jak_archive *archive, const jak_vector ofType(jak_path_entry) *path_stack);
bool jak_archive_visitor_path_compare(const jak_vector ofType(jak_path_entry) *path, jak_archive_field_sid_t *group_name, const char *path_str, jak_archive *archive);

#endif
/**
 * Copyright 2018 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without ion, including without limitation the
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

#ifndef JAK_ASYNC_H
#define JAK_ASYNC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <stdlib.h>

#include <jak_stdinc.h>
#include <jak_error.h>

JAK_BEGIN_DECL

#define JAK_ASYNC_MSG_UNKNOWN_HINT "Unknown threading hint"

typedef uint_fast16_t jak_thread_id_t;

typedef void (*jak_for_body_func_t)(const void *start, size_t width, size_t len, void *args, jak_thread_id_t tid);
typedef void (*jak_map_body_func_t)(void *dst, const void *src, size_t src_width, size_t dst_width, size_t len, void *args);
typedef void(*jak_pred_func_t)(size_t *matching_positions, size_t *num_matching_positions, const void *src, size_t width, size_t len, void *args, size_t position_offset_to_add);

typedef enum jak_threading_hint {
        JAK_THREADING_HINT_SINGLE, JAK_THREADING_HINT_MULTI
} jak_threading_hint;

typedef struct jak_async_func_proxy {
        jak_for_body_func_t function;
        const void *start;
        size_t width;
        size_t len;
        jak_thread_id_t tid;
        void *args;
} jak_async_func_proxy;

typedef struct jak_filter_arg {
        size_t num_positions;
        size_t *src_positions;
        const void *start;
        size_t len;
        size_t width;
        void *args;
        jak_pred_func_t pred;
        size_t position_offset_to_add;
} jak_filter_arg;

typedef struct jak_map_args {
        jak_map_body_func_t map_func;
        void *dst;
        const void *src;
        size_t dst_width;
        void *args;
} jak_map_args;

typedef struct jak_gather_scatter_args {
        const size_t *idx;
        const void *src;
        void *dst;
} jak_gather_scatter_args;

void *jak_async_for_proxy_function(void *args);

#define JAK_PARALLEL_ERROR(msg, retval)                                                                             \
{                                                                                                                      \
    perror(msg);                                                                                                       \
    return retval;                                                                                                     \
}

#define JAK_ASYNC_MATCH(forSingle, forMulti)                                                                            \
{                                                                                                                      \
    if (JAK_LIKELY(hint == JAK_THREADING_HINT_MULTI)) {                                             \
        return (forMulti);                                                                                             \
    } else if (hint == JAK_THREADING_HINT_SINGLE) {                                                           \
        return (forSingle);                                                                                            \
    } else JAK_PARALLEL_ERROR(JAK_ASYNC_MSG_UNKNOWN_HINT, false);                                                    \
}

bool jak_for(const void *base, size_t width, size_t len, jak_for_body_func_t f, void *args, jak_threading_hint hint, uint_fast16_t num_threads);
bool jak_map(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width, jak_map_body_func_t f, void *args, jak_threading_hint hint, uint_fast16_t num_threads);
bool jak_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len, jak_threading_hint hint, uint_fast16_t num_threads);
bool jak_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num, jak_threading_hint hint, uint_fast16_t num_threads);
bool jak_scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num, jak_threading_hint hint, uint_fast16_t num_threads);
bool jak_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx, const size_t *src_idx, size_t idxLen, jak_threading_hint hint);
bool jak_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len, jak_pred_func_t pred, void *args, jak_threading_hint hint, uint_fast16_t num_threads);
bool jak_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len, jak_pred_func_t pred, void *args, jak_threading_hint hint, size_t num_threads);

bool jak_sync_for(const void *base, size_t width, size_t len, jak_for_body_func_t f, void *args);
bool jak_async_for(const void *base, size_t width, size_t len, jak_for_body_func_t f, void *args, uint_fast16_t num_threads);
bool jak_async_map_exec(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width, jak_map_body_func_t f, void *args, jak_threading_hint hint, uint_fast16_t num_threads);
bool jak_sync_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len);
bool jak_async_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len, uint_fast16_t num_threads);
bool jak_sync_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num);
bool jak_int_async_gather_adr_func(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num, uint_fast16_t num_threads);
bool jak_sync_scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num);
bool jak_sync_scatter_func(void *dst, const void *src, size_t width, const size_t *idx, size_t num, uint_fast16_t num_threads);
bool jak_sync_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx, const size_t *src_idx, size_t idx_len);
bool jak_async_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx, const size_t *src_idx, size_t idx_len);
bool jak_async_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len, jak_pred_func_t pred, void *args);
bool jak_int_async_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len, jak_pred_func_t pred, void *args, uint_fast16_t num_threads);
bool jak_int_sync_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len, jak_pred_func_t pred, void *args);
bool jak_async_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len, jak_pred_func_t pred, void *args, size_t num_threads);

bool jak_for(const void *base, size_t width, size_t len, jak_for_body_func_t f, void *args, jak_threading_hint hint, uint_fast16_t num_threads);
bool jak_map(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width, jak_map_body_func_t f, void *args, jak_threading_hint hint, uint_fast16_t num_threads);
bool jak_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len, jak_threading_hint hint, uint_fast16_t num_threads);
bool jak_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num, jak_threading_hint hint, uint_fast16_t num_threads);
bool jak_scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num, jak_threading_hint hint, uint_fast16_t num_threads);
bool jak_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx, const size_t *src_idx, size_t idx_len, jak_threading_hint hint);
bool jak_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len, jak_pred_func_t pred, void *args, jak_threading_hint hint, uint_fast16_t num_threads);
bool jak_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len, jak_pred_func_t pred, void *args, jak_threading_hint hint, size_t num_threads);
bool jak_sync_for(const void *base, size_t width, size_t len, jak_for_body_func_t f, void *args);
bool jak_async_for(const void *base, size_t width, size_t len, jak_for_body_func_t f, void *args, uint_fast16_t num_threads);

void jak_map_proxy(const void *src, size_t src_width, size_t len, void *args, jak_thread_id_t tid);
bool jak_async_map_exec(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width, jak_map_body_func_t f, void *args, jak_threading_hint hint, uint_fast16_t num_threads);
void jak_int_async_gather(const void *start, size_t width, size_t len, void *args, jak_thread_id_t tid);

bool jak_sync_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len);
bool jak_async_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len, uint_fast16_t num_threads);
bool jak_sync_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num);
void jak_async_gather_adr_func(const void *start, size_t width, size_t len, void *args, jak_thread_id_t tid);
bool jak_int_async_gather_adr_func(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num, uint_fast16_t num_threads);
void jak_async_scatter(const void *start, size_t width, size_t len, void *args, jak_thread_id_t tid);
bool jak_sync_scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num);
bool jak_sync_scatter_func(void *dst, const void *src, size_t width, const size_t *idx, size_t num, uint_fast16_t num_threads);
bool jak_sync_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx, const size_t *src_idx, size_t idx_len);
bool jak_async_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx, const size_t *src_idx, size_t idx_len);

bool jak_int_sync_filter_late(size_t *positions, size_t *num_positions, const void *source, size_t width, size_t length, jak_pred_func_t predicate, void *arguments);
void *jak_int_sync_filter_procy_func(void *args);

bool jak_async_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len, jak_pred_func_t pred, void *args, size_t num_threads);
bool jak_async_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len, jak_pred_func_t pred, void *args);
bool jak_int_async_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len, jak_pred_func_t pred, void *args, uint_fast16_t num_threads);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_BITMAP_H
#define JAK_BITMAP_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>

JAK_BEGIN_DECL

typedef struct jak_bitmap {
        jak_vector ofType(jak_u64) data;
        jak_u16 num_bits;
} jak_bitmap;

bool jak_bitmap_create(jak_bitmap *bitmap, jak_u16 num_bits);
bool jak_bitmap_cpy(jak_bitmap *dst, const jak_bitmap *src);
bool jak_bitmap_drop(jak_bitmap *map);
size_t jak_bitmap_nbits(const jak_bitmap *map);
bool jak_bitmap_clear(jak_bitmap *map);
bool jak_bitmap_set(jak_bitmap *map, jak_u16 bit_position, bool on);
bool jak_bitmap_get(jak_bitmap *map, jak_u16 bit_position);
bool jak_bitmap_lshift(jak_bitmap *map);
bool jak_bitmap_print(FILE *file, const jak_bitmap *map);
bool jak_bitmap_blocks(jak_u32 **blocks, jak_u32 *num_blocks, const jak_bitmap *map);
void jak_bitmap_print_bits(FILE *file, jak_u32 n);
void jak_bitmap_print_bits_in_char(FILE *file, char n);

JAK_END_DECL

#endif/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_BLOOM_H
#define JAK_BLOOM_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_bitmap.h>
#include <jak_hash.h>

JAK_BEGIN_DECL

#define JAK_BLOOM_SET(filter, key, key_size)                                                                           \
({                                                                                                                     \
    size_t nbits = jak_bitmap_nbits(filter);                                                                           \
    size_t b0 = JAK_HASH_ADDITIVE(key_size, key) % nbits;                                                              \
    size_t b1 = JAK_HASH_XOR(key_size, key) % nbits;                                                                   \
    size_t b2 = JAK_HASH_ROT(key_size, key) % nbits;                                                                   \
    size_t b3 = JAK_HASH_SAX(key_size, key) % nbits;                                                                   \
    jak_bitmap_set(filter, b0, true);                                                                                  \
    jak_bitmap_set(filter, b1, true);                                                                                  \
    jak_bitmap_set(filter, b2, true);                                                                                  \
    jak_bitmap_set(filter, b3, true);                                                                                  \
})

#define JAK_BLOOM_TEST(filter, key, key_size)                                                                          \
({                                                                                                                     \
    size_t nbits = jak_bitmap_nbits(filter);                                                                           \
    size_t b0 = JAK_HASH_ADDITIVE(key_size, key) % nbits;                                                              \
    size_t b1 = JAK_HASH_XOR(key_size, key) % nbits;                                                                   \
    size_t b2 = JAK_HASH_ROT(key_size, key) % nbits;                                                                   \
    size_t b3 = JAK_HASH_SAX(key_size, key) % nbits;                                                                   \
    bool b0set = jak_bitmap_get(filter, b0);                                                                           \
    bool b1set = jak_bitmap_get(filter, b1);                                                                           \
    bool b2set = jak_bitmap_get(filter, b2);                                                                           \
    bool b3set = jak_bitmap_get(filter, b3);                                                                           \
    (b0set && b1set && b2set && b3set);                                                                                \
})

#define JAK_BLOOM_TEST_AND_SET(filter, key, key_size)                                                                  \
({                                                                                                                     \
    size_t nbits = jak_bitmap_nbits(filter);                                                                           \
    size_t b0 = JAK_HASH_ADDITIVE(key_size, key) % nbits;                                                              \
    size_t b1 = JAK_HASH_XOR(key_size, key) % nbits;                                                                   \
    size_t b2 = JAK_HASH_ROT(key_size, key) % nbits;         \
    size_t b3 = JAK_HASH_SAX(key_size, key) % nbits;         \
    bool b0set = jak_bitmap_get(filter, b0);                    \
    bool b1set = jak_bitmap_get(filter, b1);                    \
    bool b2set = jak_bitmap_get(filter, b2);                    \
    bool b3set = jak_bitmap_get(filter, b3);                    \
    jak_bitmap_set(filter, b0, true);                           \
    jak_bitmap_set(filter, b1, true);                           \
    jak_bitmap_set(filter, b2, true);                           \
    jak_bitmap_set(filter, b3, true);                           \
    (b0set && b1set && b2set && b3set);                     \
})

bool jak_bloom_create(jak_bitmap *filter, size_t size);
bool jak_bloom_drop(jak_bitmap *filter);
bool jak_bloom_clear(jak_bitmap *filter);
size_t jak_bloom_nbits(jak_bitmap *filter);

unsigned jak_bloom_nhashs();

JAK_END_DECL

#endif/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements an (read-/write) iterator for (JSON) arrays in carbon
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

#ifndef JAK_CARBON_ARRAY_IT_H
#define JAK_CARBON_ARRAY_IT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_vector.h>
#include <jak_spinlock.h>
#include <jak_carbon.h>
#include <jak_carbon_field.h>

JAK_BEGIN_DECL

typedef struct jak_field_access {
        jak_carbon_field_type_e it_field_type;

        const void *it_field_data;
        jak_u64 it_field_len;

        const char *it_mime_type;
        jak_u64 it_mime_type_strlen;

        bool nested_array_it_is_created;
        bool nested_array_it_accessed;

        bool nested_object_it_is_created;
        bool nested_object_it_accessed;

        bool nested_column_it_is_created;

        jak_carbon_array_it *nested_array_it;
        jak_carbon_column_it *nested_column_it;
        jak_carbon_object_it *nested_object_it;
} jak_field_access;

typedef struct jak_carbon_array_it {
        jak_memfile memfile;
        jak_offset_t payload_start;
        jak_spinlock lock;
        jak_error err;

        /* in case of modifications (updates, inserts, deletes), the number of bytes that are added resp. removed */
        jak_i64 mod_size;
        bool array_end_reached;

        jak_vector ofType(jak_offset_t) history;
        jak_field_access field_access;
        jak_offset_t field_offset;
} jak_carbon_array_it;

JAK_DEFINE_ERROR_GETTER(jak_carbon_array_it);

#define DECLARE_IN_PLACE_UPDATE_FUNCTION(type_name)                                                                    \
bool jak_carbon_array_it_update_in_place_##type_name(jak_carbon_array_it *it, jak_##type_name value);

DECLARE_IN_PLACE_UPDATE_FUNCTION(u8)
DECLARE_IN_PLACE_UPDATE_FUNCTION(u16)
DECLARE_IN_PLACE_UPDATE_FUNCTION(u32)
DECLARE_IN_PLACE_UPDATE_FUNCTION(u64)
DECLARE_IN_PLACE_UPDATE_FUNCTION(i8)
DECLARE_IN_PLACE_UPDATE_FUNCTION(i16)
DECLARE_IN_PLACE_UPDATE_FUNCTION(i32)
DECLARE_IN_PLACE_UPDATE_FUNCTION(i64)
DECLARE_IN_PLACE_UPDATE_FUNCTION(float)

bool jak_carbon_array_it_update_in_place_true(jak_carbon_array_it *it);
bool jak_carbon_array_it_update_in_place_false(jak_carbon_array_it *it);
bool jak_carbon_array_it_update_in_place_null(jak_carbon_array_it *it);

/**
 * Constructs a new array iterator in a carbon document, where <code>payload_start</code> is a memory offset
 * that starts with the first (potentially empty) array entry. If there is some data before the array contents
 * (e.g., a header), <code>payload_start</code> must not include this data.
 */
bool jak_carbon_array_it_create(jak_carbon_array_it *it, jak_memfile *memfile, jak_error *err, jak_offset_t payload_start);
bool jak_carbon_array_it_copy(jak_carbon_array_it *dst, jak_carbon_array_it *src);
bool jak_carbon_array_it_clone(jak_carbon_array_it *dst, jak_carbon_array_it *src);
bool jak_carbon_array_it_readonly(jak_carbon_array_it *it);
bool jak_carbon_array_it_length(jak_u64 *len, jak_carbon_array_it *it);
bool jak_carbon_array_it_is_empty(jak_carbon_array_it *it);

/**
 * Drops the iterator.
 */
bool jak_carbon_array_it_drop(jak_carbon_array_it *it);

/**
 * Locks the iterator with a spinlock. A call to <code>jak_carbon_array_it_unlock</code> is required for unlocking.
 */
bool jak_carbon_array_it_lock(jak_carbon_array_it *it);

/**
 * Unlocks the iterator
 */
bool jak_carbon_array_it_unlock(jak_carbon_array_it *it);

/**
 * Positions the iterator at the beginning of this array.
 */
bool jak_carbon_array_it_rewind(jak_carbon_array_it *it);

/**
 * Positions the iterator to the slot after the current element, potentially pointing to next element.
 * The function returns true, if the slot is non-empty, and false otherwise.
 */
bool jak_carbon_array_it_next(jak_carbon_array_it *it);
bool jak_carbon_array_it_has_next(jak_carbon_array_it *it);
bool jak_carbon_array_it_is_unit(jak_carbon_array_it *it);
bool jak_carbon_array_it_prev(jak_carbon_array_it *it);

jak_offset_t jak_carbon_array_it_memfilepos(jak_carbon_array_it *it);
jak_offset_t jak_carbon_array_it_tell(jak_carbon_array_it *it);
bool jak_carbon_int_array_it_offset(jak_offset_t *off, jak_carbon_array_it *it);
bool jak_carbon_array_it_fast_forward(jak_carbon_array_it *it);

bool jak_carbon_array_it_field_type(jak_carbon_field_type_e *type, jak_carbon_array_it *it);
bool jak_carbon_array_it_u8_value(jak_u8 *value, jak_carbon_array_it *it);
bool jak_carbon_array_it_u16_value(jak_u16 *value, jak_carbon_array_it *it);
bool jak_carbon_array_it_u32_value(jak_u32 *value, jak_carbon_array_it *it);
bool jak_carbon_array_it_u64_value(jak_u64 *value, jak_carbon_array_it *it);
bool jak_carbon_array_it_i8_value(jak_i8 *value, jak_carbon_array_it *it);
bool jak_carbon_array_it_i16_value(jak_i16 *value, jak_carbon_array_it *it);
bool jak_carbon_array_it_i32_value(jak_i32 *value, jak_carbon_array_it *it);
bool jak_carbon_array_it_i64_value(jak_i64 *value, jak_carbon_array_it *it);
bool jak_carbon_array_it_float_value(bool *is_null_in, float *value, jak_carbon_array_it *it);
bool jak_carbon_array_it_signed_value(bool *is_null_in, jak_i64 *value, jak_carbon_array_it *it);
bool jak_carbon_array_it_unsigned_value(bool *is_null_in, jak_u64 *value, jak_carbon_array_it *it);
const char *jak_carbon_array_it_jak_string_value(jak_u64 *strlen, jak_carbon_array_it *it);
bool jak_carbon_array_it_binary_value(jak_carbon_binary *out, jak_carbon_array_it *it);
jak_carbon_array_it *jak_carbon_array_it_array_value(jak_carbon_array_it *it_in);
jak_carbon_object_it *jak_carbon_array_it_object_value(jak_carbon_array_it *it_in);
jak_carbon_column_it *jak_carbon_array_it_column_value(jak_carbon_array_it *it_in);

/**
 * Inserts a new element at the current position of the iterator.
 */
bool jak_carbon_array_it_insert_begin(jak_carbon_insert *inserter, jak_carbon_array_it *it);
bool jak_carbon_array_it_insert_end(jak_carbon_insert *inserter);
bool jak_carbon_array_it_remove(jak_carbon_array_it *it);

JAK_END_DECL

#endif
/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements an (read-/write) iterator for (JSON) arrays in carbon
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

#ifndef JAK_CARBON_COLUMN_IT_H
#define JAK_CARBON_COLUMN_IT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_spinlock.h>
#include <jak_carbon_field.h>
#include <jak_carbon.h>

JAK_BEGIN_DECL

typedef struct jak_carbon_column_it {
        jak_memfile memfile;

        jak_offset_t num_and_capacity_start_offset;
        jak_offset_t column_start_offset;

        jak_error err;
        jak_carbon_field_type_e type;

        /* in case of modifications (updates, inserts, deletes), the number of bytes that are added resp. removed */
        jak_i64 mod_size;

        jak_u32 column_capacity;
        jak_u32 column_num_elements;

        jak_spinlock lock;
} jak_carbon_column_it;

bool jak_carbon_column_it_create(jak_carbon_column_it *it, jak_memfile *memfile, jak_error *err, jak_offset_t column_start_offset);
bool jak_carbon_column_it_clone(jak_carbon_column_it *dst, jak_carbon_column_it *src);

bool jak_carbon_column_it_insert(jak_carbon_insert *inserter, jak_carbon_column_it *it);
bool jak_carbon_column_it_fast_forward(jak_carbon_column_it *it);
jak_offset_t jak_carbon_column_it_memfilepos(jak_carbon_column_it *it);
jak_offset_t jak_carbon_column_it_tell(jak_carbon_column_it *it, jak_u32 elem_idx);

const void *jak_carbon_column_it_values(jak_carbon_field_type_e *type, jak_u32 *nvalues, jak_carbon_column_it *it);
bool jak_carbon_column_it_values_info(jak_carbon_field_type_e *type, jak_u32 *nvalues, jak_carbon_column_it *it);

bool jak_carbon_column_it_value_is_null(jak_carbon_column_it *it, jak_u32 pos);

const jak_u8 *jak_carbon_column_it_boolean_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_u8 *jak_carbon_column_it_u8_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_u16 *jak_carbon_column_it_u16_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_u32 *jak_carbon_column_it_u32_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_u64 *jak_carbon_column_it_u64_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_i8 *jak_carbon_column_it_i8_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_i16 *jak_carbon_column_it_i16_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_i32 *jak_carbon_column_it_i32_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_i64 *jak_carbon_column_it_i64_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const float *jak_carbon_column_it_float_values(jak_u32 *nvalues, jak_carbon_column_it *it);

bool jak_carbon_column_it_remove(jak_carbon_column_it *it, jak_u32 pos);

bool jak_carbon_column_it_update_set_null(jak_carbon_column_it *it, jak_u32 pos);
bool jak_carbon_column_it_update_set_true(jak_carbon_column_it *it, jak_u32 pos);
bool jak_carbon_column_it_update_set_false(jak_carbon_column_it *it, jak_u32 pos);
bool jak_carbon_column_it_update_set_u8(jak_carbon_column_it *it, jak_u32 pos, jak_u8 value);
bool jak_carbon_column_it_update_set_u16(jak_carbon_column_it *it, jak_u32 pos, jak_u16 value);
bool jak_carbon_column_it_update_set_u32(jak_carbon_column_it *it, jak_u32 pos, jak_u32 value);
bool jak_carbon_column_it_update_set_u64(jak_carbon_column_it *it, jak_u32 pos, jak_u64 value);
bool jak_carbon_column_it_update_set_i8(jak_carbon_column_it *it, jak_u32 pos, jak_i8 value);
bool jak_carbon_column_it_update_set_i16(jak_carbon_column_it *it, jak_u32 pos, jak_i16 value);
bool jak_carbon_column_it_update_set_i32(jak_carbon_column_it *it, jak_u32 pos, jak_i32 value);
bool jak_carbon_column_it_update_set_i64(jak_carbon_column_it *it, jak_u32 pos, jak_i64 value);
bool jak_carbon_column_it_update_set_float(jak_carbon_column_it *it, jak_u32 pos, float value);

/**
 * Locks the iterator with a spinlock. A call to <code>jak_carbon_column_it_unlock</code> is required for unlocking.
 */
bool jak_carbon_column_it_lock(jak_carbon_column_it *it);

/**
 * Unlocks the iterator
 */
bool jak_carbon_column_it_unlock(jak_carbon_column_it *it);

/**
 * Positions the iterator at the beginning of this array.
 */
bool jak_carbon_column_it_rewind(jak_carbon_column_it *it);

JAK_END_DECL

#endif
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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#ifndef JAK_CARBON_COMMIT_HASH_H
#define JAK_CARBON_COMMIT_HASH_H

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_memfile.h>
#include <jak_string.h>

JAK_BEGIN_DECL

bool jak_carbon_commit_hash_create(jak_memfile *file);
bool jak_carbon_commit_hash_skip(jak_memfile *file);
bool jak_carbon_commit_hash_read(jak_u64 *commit_hash, jak_memfile *file);
bool jak_carbon_commit_hash_peek(jak_u64 *commit_hash, jak_memfile *file);
bool jak_carbon_commit_hash_update(jak_memfile *file, const char *base, jak_u64 len);
bool jak_carbon_commit_hash_compute(jak_u64 *commit_hash, const void *base, jak_u64 len);
const char *jak_carbon_commit_hash_to_str(jak_string *dst, jak_u64 commit_hash);
bool jak_carbon_commit_hash_append_to_str(jak_string *dst, jak_u64 commit_hash);
jak_u64 jak_carbon_commit_hash_from_str(const char *commit_str, jak_error *err);

JAK_END_DECL

#endif
/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements the document format itself
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

#ifndef JAK_CARBON_DOT_H
#define JAK_CARBON_DOT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_types.h>
#include <jak_string.h>
#include <jak_carbon_array_it.h>
#include <jak_carbon_column_it.h>
#include <jak_carbon_object_it.h>

JAK_BEGIN_DECL

typedef enum carbon_dot_node {
        JAK_DOT_NODE_ARRAY_IDX,
        JAK_DOT_NODE_KEY_NAME
} carbon_dot_node_e;

typedef struct jak_carbon_dot_node {
        carbon_dot_node_e type;
        union {
                char *string;
                jak_u32 idx;
        } identifier;
} jak_carbon_dot_node;

typedef struct jak_carbon_dot_path {
        jak_carbon_dot_node nodes[256];
        jak_u32 path_len;
        jak_error err;
} jak_carbon_dot_path;

typedef enum jak_carbon_path_status {
        JAK_CARBON_PATH_RESOLVED,
        JAK_CARBON_PATH_EMPTY_DOC,
        JAK_CARBON_PATH_NOSUCHINDEX,
        JAK_CARBON_PATH_NOSUCHKEY,
        JAK_CARBON_PATH_NOTTRAVERSABLE,
        JAK_CARBON_PATH_NOCONTAINER,
        JAK_CARBON_PATH_NOTANOBJECT,
        JAK_CARBON_PATH_NONESTING,
        JAK_CARBON_PATH_INTERNAL
} jak_carbon_path_status_e;

JAK_DEFINE_ERROR_GETTER(jak_carbon_dot_path)

bool jak_carbon_dot_path_create(jak_carbon_dot_path *path);
bool jak_carbon_dot_path_from_string(jak_carbon_dot_path *path, const char *path_string);
bool jak_carbon_dot_path_drop(jak_carbon_dot_path *path);

bool jak_carbon_dot_path_add_key(jak_carbon_dot_path *dst, const char *key);
bool jak_carbon_dot_path_add_nkey(jak_carbon_dot_path *dst, const char *key, size_t len);
bool jak_carbon_dot_path_add_idx(jak_carbon_dot_path *dst, jak_u32 idx);
bool jak_carbon_dot_path_len(jak_u32 *len, const jak_carbon_dot_path *path);
bool jak_carbon_dot_path_is_empty(const jak_carbon_dot_path *path);
bool jak_carbon_dot_path_type_at(carbon_dot_node_e *type_out, jak_u32 pos, const jak_carbon_dot_path *path);
bool jak_carbon_dot_path_idx_at(jak_u32 *idx, jak_u32 pos, const jak_carbon_dot_path *path);
const char *jak_carbon_dot_path_key_at(jak_u32 pos, const jak_carbon_dot_path *path);

bool jak_carbon_dot_path_to_str(jak_string *sb, jak_carbon_dot_path *path);
bool jak_carbon_dot_path_fprint(FILE *file, jak_carbon_dot_path *path);
bool jak_carbon_dot_path_print(jak_carbon_dot_path *path);

JAK_END_DECL

#endif
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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_carbon.h>

#ifndef JAK_CARBON_JAK_FIELD_H
#define JAK_CARBON_JAK_FIELD_H

typedef enum jak_carbon_field_type {
        /* constants */
        JAK_CARBON_FIELD_TYPE_NULL = JAK_CARBON_MARKER_NULL, /* null */
        JAK_CARBON_FIELD_TYPE_TRUE = JAK_CARBON_MARKER_TRUE, /* true */
        JAK_CARBON_FIELD_TYPE_FALSE = JAK_CARBON_MARKER_FALSE, /* false */

        /* containers */
        JAK_CARBON_FIELD_TYPE_OBJECT = JAK_CARBON_MARKER_OBJECT_BEGIN, /* object */
        JAK_CARBON_FIELD_TYPE_ARRAY = JAK_CARBON_MARKER_ARRAY_BEGIN, /* variable-type array of elements of varying type */
        JAK_CARBON_FIELD_TYPE_COLUMN_U8 = JAK_CARBON_MARKER_COLUMN_U8, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_U16 = JAK_CARBON_MARKER_COLUMN_U16, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_U32 = JAK_CARBON_MARKER_COLUMN_U32, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_U64 = JAK_CARBON_MARKER_COLUMN_U64, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_I8 = JAK_CARBON_MARKER_COLUMN_I8, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_I16 = JAK_CARBON_MARKER_COLUMN_I16, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_I32 = JAK_CARBON_MARKER_COLUMN_I32, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_I64 = JAK_CARBON_MARKER_COLUMN_I64, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT = JAK_CARBON_MARKER_COLUMN_FLOAT, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN = JAK_CARBON_MARKER_COLUMN_BOOLEAN, /* fixed-type array of elements of particular type */

        /* character strings */
        JAK_CARBON_FIELD_TYPE_STRING = JAK_CARBON_MARKER_STRING, /* UTF-8 string */

        /* numbers */
        JAK_CARBON_FIELD_TYPE_NUMBER_U8 = JAK_CARBON_MARKER_U8, /* 8bit unsigned integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_U16 = JAK_CARBON_MARKER_U16, /* 16bit unsigned integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_U32 = JAK_CARBON_MARKER_U32, /* 32bit unsigned integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_U64 = JAK_CARBON_MARKER_U64, /* 64bit unsigned integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_I8 = JAK_CARBON_MARKER_I8, /* 8bit signed integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_I16 = JAK_CARBON_MARKER_I16, /* 16bit signed integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_I32 = JAK_CARBON_MARKER_I32, /* 32bit signed integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_I64 = JAK_CARBON_MARKER_I64, /* 64bit signed integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT = JAK_CARBON_MARKER_FLOAT, /* 32bit float */

        /* binary data */
        JAK_CARBON_FIELD_TYPE_BINARY = JAK_CARBON_MARKER_BINARY, /* arbitrary binary object with known mime type */
        JAK_CARBON_FIELD_TYPE_BINARY_CUSTOM = JAK_CARBON_MARKER_CUSTOM_BINARY, /* arbitrary binary object with unknown mime type*/
} jak_carbon_field_type_e;

typedef enum jak_carbon_column_type {
        JAK_CARBON_COLUMN_TYPE_U8,
        JAK_CARBON_COLUMN_TYPE_U16,
        JAK_CARBON_COLUMN_TYPE_U32,
        JAK_CARBON_COLUMN_TYPE_U64,
        JAK_CARBON_COLUMN_TYPE_I8,
        JAK_CARBON_COLUMN_TYPE_I16,
        JAK_CARBON_COLUMN_TYPE_I32,
        JAK_CARBON_COLUMN_TYPE_I64,
        JAK_CARBON_COLUMN_TYPE_FLOAT,
        JAK_CARBON_COLUMN_TYPE_BOOLEAN
} jak_carbon_column_type_e;

typedef enum jak_carbon_field_class {
        JAK_CARBON_FIELD_CLASS_CONSTANT,
        JAK_CARBON_FIELD_CLASS_NUMBER,
        JAK_CARBON_FIELD_CLASS_CHARACTER_STRING,
        JAK_CARBON_FIELD_CLASS_BINARY_STRING,
        JAK_CARBON_FIELD_CLASS_CONTAINER
} jak_carbon_field_class_e;

typedef enum jak_carbon_constant {
        JAK_CARBON_CONSTANT_TRUE,
        JAK_CARBON_CONSTANT_FALSE,
        JAK_CARBON_CONSTANT_NULL
} jak_carbon_constant_e;

#define JAK_CARBON_FIELD_TYPE_NULL_STR "null"
#define JAK_CARBON_FIELD_TYPE_TRUE_STR "boolean-true"
#define JAK_CARBON_FIELD_TYPE_FALSE_STR "boolean-false"
#define JAK_CARBON_FIELD_TYPE_OBJECT_STR "object"
#define JAK_CARBON_FIELD_TYPE_ARRAY_STR "array"
#define JAK_CARBON_FIELD_TYPE_COLUMN_U8_STR "column-jak_u8"
#define JAK_CARBON_FIELD_TYPE_COLUMN_U16_STR "column-jak_u16"
#define JAK_CARBON_FIELD_TYPE_COLUMN_U32_STR "column-jak_u32"
#define JAK_CARBON_FIELD_TYPE_COLUMN_U64_STR "column-jak_u64"
#define JAK_CARBON_FIELD_TYPE_COLUMN_I8_STR "column-jak_i8"
#define JAK_CARBON_FIELD_TYPE_COLUMN_I16_STR "column-jak_i16"
#define JAK_CARBON_FIELD_TYPE_COLUMN_I32_STR "column-jak_i32"
#define JAK_CARBON_FIELD_TYPE_COLUMN_I64_STR "column-jak_i64"
#define JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT_STR "column-float"
#define JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN_STR "column-boolean"
#define JAK_CARBON_FIELD_TYPE_STRING_STR "string"
#define JAK_CARBON_FIELD_TYPE_BINARY_STR "binary"
#define JAK_CARBON_FIELD_TYPE_NUMBER_U8_STR "number-jak_u8"
#define JAK_CARBON_FIELD_TYPE_NUMBER_U16_STR "number-jak_u16"
#define JAK_CARBON_FIELD_TYPE_NUMBER_U32_STR "number-jak_u32"
#define JAK_CARBON_FIELD_TYPE_NUMBER_U64_STR "number-jak_u64"
#define JAK_CARBON_FIELD_TYPE_NUMBER_I8_STR "number-jak_i8"
#define JAK_CARBON_FIELD_TYPE_NUMBER_I16_STR "number-jak_i16"
#define JAK_CARBON_FIELD_TYPE_NUMBER_I32_STR "number-jak_i32"
#define JAK_CARBON_FIELD_TYPE_NUMBER_I64_STR "number-jak_i64"
#define JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT_STR "number-float"

JAK_BEGIN_DECL

const char *jak_carbon_field_type_str(jak_error *err, jak_carbon_field_type_e type);

bool jak_carbon_field_type_is_traversable(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_signed(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_unsigned(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_floating(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_number(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_integer(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_binary(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_boolean(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_array(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_column(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_object(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_null(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_string(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_constant(jak_carbon_field_type_e type);

jak_carbon_field_class_e jak_carbon_field_type_get_class(jak_carbon_field_type_e type, jak_error *err);

bool jak_carbon_field_skip(jak_memfile *file);
bool jak_carbon_field_skip_object(jak_memfile *file);
bool jak_carbon_field_skip_array(jak_memfile *file);
bool jak_carbon_field_skip_column(jak_memfile *file);
bool jak_carbon_field_skip_binary(jak_memfile *file);
bool jak_carbon_field_skip_custom_binary(jak_memfile *file);
bool jak_carbon_field_skip_string(jak_memfile *file);
bool jak_carbon_field_skip_float(jak_memfile *file);
bool jak_carbon_field_skip_boolean(jak_memfile *file);
bool jak_carbon_field_skip_null(jak_memfile *file);
bool jak_carbon_field_skip_8(jak_memfile *file);
bool jak_carbon_field_skip_16(jak_memfile *file);
bool jak_carbon_field_skip_32(jak_memfile *file);
bool jak_carbon_field_skip_64(jak_memfile *file);

jak_carbon_field_type_e jak_carbon_field_type_for_column(jak_carbon_column_type_e type);
jak_carbon_field_type_e jak_carbon_field_type_column_entry_to_regular_type(jak_carbon_field_type_e type, bool is_null, bool is_true);

JAK_END_DECL

#endif
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

#ifndef JAK_CARBON_FIND_H
#define JAK_CARBON_FIND_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_carbon.h>
#include <jak_carbon_column_it.h>
#include <jak_carbon_array_it.h>
#include <jak_carbon_object_it.h>
#include <jak_carbon_dot.h>
#include <jak_carbon_path.h>

JAK_BEGIN_DECL

typedef struct jak_carbon_find {
        jak_carbon *doc;
        jak_carbon_field_type_e type;
        jak_error err;
        jak_carbon_path_evaluator path_evaluater;

        bool value_is_nulled;

        union {
                jak_carbon_array_it *array_it;
                jak_carbon_column_it *column_it;
                jak_carbon_object_it *object_it;
                bool boolean;
                jak_u64 unsigned_number;
                jak_i64 signed_number;
                float float_number;

                struct {
                        const char *base;
                        jak_u64 len;
                } string;

                jak_carbon_binary binary;
        } value;
} jak_carbon_find;

JAK_DEFINE_ERROR_GETTER(jak_carbon_find)

bool jak_carbon_find_open(jak_carbon_find *out, const char *dot_path, jak_carbon *doc);
bool jak_carbon_find_close(jak_carbon_find *find);
bool jak_carbon_find_create(jak_carbon_find *find, jak_carbon_dot_path *path, jak_carbon *doc);
bool jak_carbon_find_drop(jak_carbon_find *find);

bool jak_carbon_find_has_result(jak_carbon_find *find);
const char *jak_carbon_find_result_to_str(jak_string *dst_str, jak_carbon_printer_impl_e print_type, jak_carbon_find *find);
const char *jak_carbon_find_result_to_json_compact(jak_string *dst_str, jak_carbon_find *find);
char *jak_carbon_find_result_to_json_compact_dup(jak_carbon_find *find);

bool jak_carbon_find_result_type(jak_carbon_field_type_e *type, jak_carbon_find *find);

jak_carbon_array_it *jak_carbon_find_result_array(jak_carbon_find *find);
jak_carbon_object_it *jak_carbon_find_result_object(jak_carbon_find *find);
jak_carbon_column_it *jak_carbon_find_result_column(jak_carbon_find *find);
bool jak_carbon_find_result_boolean(bool *out, jak_carbon_find *find);
bool jak_carbon_find_result_unsigned(jak_u64 *out, jak_carbon_find *find);
bool jak_carbon_find_result_signed(jak_i64 *out, jak_carbon_find *find);
bool jak_carbon_find_result_float(float *out, jak_carbon_find *find);
const char *jak_carbon_find_result_string(jak_u64 *str_len, jak_carbon_find *find);
jak_carbon_binary *jak_carbon_find_result_binary(jak_carbon_find *find);

JAK_END_DECL

#endif
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

#ifndef JAK_CARBON_GET_H
#define JAK_CARBON_GET_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_types.h>
#include <jak_carbon.h>

JAK_BEGIN_DECL

jak_u64 jak_carbon_get_or_default_unsigned(jak_carbon *doc, const char *path, jak_u64 default_val);
jak_i64 jak_carbon_get_or_default_signed(jak_carbon *doc, const char *path, jak_i64 default_val);
float jak_carbon_get_or_default_float(jak_carbon *doc, const char *path, float default_val);
bool jak_carbon_get_or_default_boolean(jak_carbon *doc, const char *path, bool default_val);
const char *jak_carbon_get_or_default_string(jak_u64 *len_out, jak_carbon *doc, const char *path, const char *default_val);
jak_carbon_binary *jak_carbon_get_or_default_binary(jak_carbon *doc, const char *path, jak_carbon_binary *default_val);
jak_carbon_array_it *carbon_get_array_or_null(jak_carbon *doc, const char *path);
jak_carbon_column_it *carbon_get_column_or_null(jak_carbon *doc, const char *path);

JAK_END_DECL

#endif
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

#ifndef JAK_CARBON_H
#define JAK_CARBON_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_memblock.h>
#include <jak_memfile.h>
#include <jak_unique_id.h>
#include <jak_string.h>
#include <jak_spinlock.h>
#include <jak_vector.h>
#include <jak_stdinc.h>
#include <jak_alloc.h>
#include <jak_bitmap.h>
#include <jak_bloom.h>
#include <jak_archive.h>
#include <jak_archive_it.h>
#include <jak_archive_visitor.h>
#include <jak_archive_converter.h>
#include <jak_encoded_doc.h>
#include <jak_stdinc.h>
#include <jak_utils_convert.h>
#include <jak_column_doc.h>
#include <jak_doc.h>
#include <jak_error.h>
#include <jak_hash_table.h>
#include <jak_hash.h>
#include <jak_huffman.h>
#include <jak_json.h>
#include <jak_memblock.h>
#include <jak_memfile.h>
#include <jak_unique_id.h>
#include <jak_utils_sort.h>
#include <jak_async.h>
#include <jak_slicelist.h>
#include <jak_spinlock.h>
#include <jak_string_dict.h>
#include <jak_str_hash.h>
#include <jak_archive_strid_it.h>
#include <jak_archive_cache.h>
#include <jak_time.h>
#include <jak_types.h>
#include <jak_archive_query.h>
#include <jak_vector.h>
#include <jak_alloc_trace.h>
#include <jak_encode_async.h>
#include <jak_encode_sync.h>
#include <jak_str_hash_mem.h>
#include <jak_pred_contains.h>
#include <jak_pred_equals.h>
#include <jak_carbon_printers.h>
#include <jak_uintvar_stream.h>

JAK_BEGIN_DECL

typedef struct jak_carbon {
        jak_memblock *memblock;
        jak_memfile memfile;

        struct {
                jak_spinlock write_lock;
                bool commit_lock;
                bool is_latest;
        } versioning;

        jak_error err;
} jak_carbon;

typedef struct jak_carbon_revise {
        jak_carbon *original;
        jak_carbon *revised_doc;
        jak_error err;
} jak_carbon_revise;

typedef struct jak_carbon_binary {
        const char *mime_type;
        jak_u64 mime_type_strlen;
        const void *blob;
        jak_u64 blob_len;
} jak_carbon_binary;

typedef struct jak_carbon_new {
        jak_error err;
        jak_carbon original;
        jak_carbon_revise revision_context;
        jak_carbon_array_it *content_it;
        jak_carbon_insert *inserter;
        /* options shrink or compact (or both) documents, see
         * CARBON_KEEP, CARBON_SHRINK, CARBON_COMPACT, and CARBON_OPTIMIZE  */
        int mode;
} jak_carbon_new;

typedef enum jak_carbon_container_type {
        JAK_CARBON_OBJECT, JAK_CARBON_ARRAY, JAK_CARBON_COLUMN
} jak_carbon_container_e;

typedef enum jak_carbon_printer_impl {
        JAK_JSON_EXTENDED, JAK_JSON_COMPACT
} jak_carbon_printer_impl_e;

#define JAK_CARBON_MARKER_KEY_NOKEY '?'
#define JAK_CARBON_MARKER_KEY_AUTOKEY '*'
#define JAK_CARBON_MARKER_KEY_UKEY '+'
#define JAK_CARBON_MARKER_KEY_IKEY '-'
#define JAK_CARBON_MARKER_KEY_SKEY '!'

typedef enum jak_carbon_key_type {
        /* no key, no revision number */
        JAK_CARBON_KEY_NOKEY = JAK_CARBON_MARKER_KEY_NOKEY,
        /* auto-generated 64bit unsigned integer key */
        JAK_CARBON_KEY_AUTOKEY = JAK_CARBON_MARKER_KEY_AUTOKEY,
        /* user-defined 64bit unsigned integer key */
        JAK_CARBON_KEY_UKEY = JAK_CARBON_MARKER_KEY_UKEY,
        /* user-defined 64bit signed integer key */
        JAK_CARBON_KEY_IKEY = JAK_CARBON_MARKER_KEY_IKEY,
        /* user-defined n-char string key */
        JAK_CARBON_KEY_SKEY = JAK_CARBON_MARKER_KEY_SKEY
} jak_carbon_key_e;

#define JAK_CARBON_NIL_STR "_nil"
#define JAK_CARBON_MARKER_NULL 'n'
#define JAK_CARBON_MARKER_TRUE 't'
#define JAK_CARBON_MARKER_FALSE 'f'
#define JAK_CARBON_MARKER_STRING 's'
#define JAK_CARBON_MARKER_U8 'c'
#define JAK_CARBON_MARKER_U16 'd'
#define JAK_CARBON_MARKER_U32 'i'
#define JAK_CARBON_MARKER_U64 'l'
#define JAK_CARBON_MARKER_I8 'C'
#define JAK_CARBON_MARKER_I16 'D'
#define JAK_CARBON_MARKER_I32 'I'
#define JAK_CARBON_MARKER_I64 'L'
#define JAK_CARBON_MARKER_FLOAT 'r'
#define JAK_CARBON_MARKER_BINARY 'b'
#define JAK_CARBON_MARKER_CUSTOM_BINARY 'x'
#define JAK_CARBON_MARKER_OBJECT_BEGIN '{'
#define JAK_CARBON_MARKER_OBJECT_END '}'
#define JAK_CARBON_MARKER_ARRAY_BEGIN '['
#define JAK_CARBON_MARKER_ARRAY_END ']'
#define JAK_CARBON_MARKER_COLUMN_U8 '1'
#define JAK_CARBON_MARKER_COLUMN_U16 '2'
#define JAK_CARBON_MARKER_COLUMN_U32 '3'
#define JAK_CARBON_MARKER_COLUMN_U64 '4'
#define JAK_CARBON_MARKER_COLUMN_I8 '5'
#define JAK_CARBON_MARKER_COLUMN_I16 '6'
#define JAK_CARBON_MARKER_COLUMN_I32 '7'
#define JAK_CARBON_MARKER_COLUMN_I64 '8'
#define JAK_CARBON_MARKER_COLUMN_FLOAT 'R'
#define JAK_CARBON_MARKER_COLUMN_BOOLEAN 'B'

JAK_DEFINE_ERROR_GETTER(jak_carbon);
JAK_DEFINE_ERROR_GETTER(jak_carbon_new);

#define JAK_CARBON_KEEP              0x0
#define JAK_CARBON_SHRINK            0x1
#define JAK_CARBON_COMPACT           0x2
#define JAK_CARBON_OPTIMIZE          (JAK_CARBON_SHRINK | JAK_CARBON_COMPACT)

/**
 * Constructs a new context in which a new document can be created. The parameter <b>options</b> controls
 * how reserved spaces should be handled after document creation is done. Set <code>options</code> to
 * <code>CARBON_KEEP</code> for no optimization. With this option, all capacities (i.e., additional ununsed but free
 * space) in containers (objects, arrays, and columns) are kept and tailing free space after the document is
 * kept, too. Use this option to optimize for "insertion-heavy" documents since keeping all capacities lowerst the
 * probability of reallocations and memory movements. Set <b>options</b> to <code>CARBON_COMPACT</code> if capacities in
 * containers should be removed after creation, and <code>CARBON_COMPACT</code> to remove tailing free space. Use
 * <code>CARBON_OPTIMIZE</code> to use both <code>CARBON_SHRINK</code> and <code>CARBON_COMPACT</code>.
 *
 * As a rule of thumb for <b>options</b>. The resulting document...
 * <ul>
 *  <li>...will be updated heavily where updates may change the type-width of fields, will be target of many inserts
 *  containers, use <code>CARBON_KEEP</code>. The document will have a notable portion of reserved memory contained;
 *  insertions or updates will, however, not require immediately reallocation or memory movements.</li>
 *  <li>...will <i>not</i> be target of insertion of strings or blob fields in the near future, use
 *      <code>CARBON_SHRINK</code>. The document will not have padding reserved memory at the end, which means that
 *      a realloction will be required once the document grows (e.g., a container must be englarged). Typically,
 *      document growth is handled with container capacities (see <code>CARBON_COMPACT</code>). However, insertions
 *      of variable-length data (i.e., strings and blobs) may require container enlargement. In this case, having
 *      padding reserved memory at the end of the document lowers the risk of a reallocation.</li>
 *  <li>...will <i>not</i> not be target of insertion operations or update operations that changes a fields type-width
 *      in the near future. In simpler words, if a document is updated and each such update keeps the (byte) size
 *      of the field, use <code>CARBON_COMPACT</code>. This option will remove all capacities in containers.</li>
 *  <li>...is read-mostly, or updates will not change the type or type-width of fields, use <code>CARBON_OPTIMIZE</code>.
 *      The document will have the smallest memory footprint possible.</li>
 * </ul>
 */
jak_carbon_insert *jak_carbon_create_begin(jak_carbon_new *context, jak_carbon *doc, jak_carbon_key_e type, int options);
bool jak_carbon_create_end(jak_carbon_new *context);
bool jak_carbon_create_empty(jak_carbon *doc, jak_carbon_key_e type);
bool jak_carbon_create_empty_ex(jak_carbon *doc, jak_carbon_key_e type, jak_u64 doc_cap, jak_u64 array_cap);
bool jak_carbon_from_json(jak_carbon *doc, const char *json, jak_carbon_key_e type, const void *key, jak_error *err);
bool jak_carbon_drop(jak_carbon *doc);

const void *jak_carbon_raw_data(jak_u64 *len, jak_carbon *doc);

bool jak_carbon_is_up_to_date(jak_carbon *doc);
bool jak_carbon_key_type(jak_carbon_key_e *out, jak_carbon *doc);
const void *jak_carbon_key_raw_value(jak_u64 *len, jak_carbon_key_e *type, jak_carbon *doc);
bool jak_carbon_key_signed_value(jak_i64 *key, jak_carbon *doc);
bool jak_carbon_key_unsigned_value(jak_u64 *key, jak_carbon *doc);
const char *jak_carbon_key_jak_string_value(jak_u64 *len, jak_carbon *doc);
bool jak_carbon_has_key(jak_carbon_key_e type);
bool jak_carbon_key_is_unsigned(jak_carbon_key_e type);
bool jak_carbon_key_is_signed(jak_carbon_key_e type);
bool jak_carbon_key_is_string(jak_carbon_key_e type);
bool jak_carbon_clone(jak_carbon *clone, jak_carbon *doc);
bool jak_carbon_commit_hash(jak_u64 *hash, jak_carbon *doc);

bool jak_carbon_to_str(jak_string *dst, jak_carbon_printer_impl_e printer, jak_carbon *doc);
const char *jak_carbon_to_json_extended(jak_string *dst, jak_carbon *doc);
const char *jak_carbon_to_json_compact(jak_string *dst, jak_carbon *doc);
char *jak_carbon_to_json_extended_dup(jak_carbon *doc);
char *jak_carbon_to_json_compact_dup(jak_carbon *doc);
bool jak_carbon_iterator_open(jak_carbon_array_it *it, jak_carbon *doc);
bool jak_carbon_iterator_close(jak_carbon_array_it *it);
bool jak_carbon_print(FILE *file, jak_carbon_printer_impl_e printer, jak_carbon *doc);
bool jak_carbon_hexdump_print(FILE *file, jak_carbon *doc);

JAK_END_DECL

#endif
/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements the document format itself
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

#ifndef JAK_CARBON_INSERT_H
#define JAK_CARBON_INSERT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_memblock.h>
#include <jak_memfile.h>
#include <jak_spinlock.h>
#include <jak_carbon.h>
#include <jak_carbon_int.h>

JAK_BEGIN_DECL

bool jak_carbon_int_insert_create_for_array(jak_carbon_insert *inserter, jak_carbon_array_it *context);
bool jak_carbon_int_insert_create_for_column(jak_carbon_insert *inserter, jak_carbon_column_it *context);
bool jak_carbon_int_insert_create_for_object(jak_carbon_insert *inserter, jak_carbon_object_it *context);

bool jak_carbon_insert_null(jak_carbon_insert *inserter);
bool jak_carbon_insert_true(jak_carbon_insert *inserter);
bool jak_carbon_insert_false(jak_carbon_insert *inserter);
bool jak_carbon_insert_u8(jak_carbon_insert *inserter, jak_u8 value);
bool jak_carbon_insert_u16(jak_carbon_insert *inserter, jak_u16 value);
bool jak_carbon_insert_u32(jak_carbon_insert *inserter, jak_u32 value);
bool jak_carbon_insert_u64(jak_carbon_insert *inserter, jak_u64 value);
bool jak_carbon_insert_i8(jak_carbon_insert *inserter, jak_i8 value);
bool jak_carbon_insert_i16(jak_carbon_insert *inserter, jak_i16 value);
bool jak_carbon_insert_i32(jak_carbon_insert *inserter, jak_i32 value);
bool jak_carbon_insert_i64(jak_carbon_insert *inserter, jak_i64 value);
bool jak_carbon_insert_unsigned(jak_carbon_insert *inserter, jak_u64 value);
bool jak_carbon_insert_signed(jak_carbon_insert *inserter, jak_i64 value);
bool jak_carbon_insert_float(jak_carbon_insert *inserter, float value);
bool jak_carbon_insert_string(jak_carbon_insert *inserter, const char *value);
bool jak_carbon_insert_nchar(jak_carbon_insert *inserter, const char *value, jak_u64 value_len);
/**
 * Inserts a user-defined binary string <code>value</code> of <code>nbytes</code> bytes along with a (mime) type annotation.
 * The type annotation is automatically found if <code>file_ext</code> is non-null and known to the system. If it is
 * not known or null, the non-empty <code>user_type</code> string is used to encode the mime annotation. In case
 * <code>user_type</code> is null (or empty) and <code>file_ext</code> is null (or not known), the mime type is set to
 * <code>application/octet-stream</code>, which encodes arbitrary binary data.
 */
bool jak_carbon_insert_binary(jak_carbon_insert *inserter, const void *value, size_t nbytes, const char *file_ext, const char *user_type);

jak_carbon_insert *jak_carbon_insert_object_begin(jak_carbon_insert_object_state *out, jak_carbon_insert *inserter, jak_u64 object_capacity);
bool jak_carbon_insert_object_end(jak_carbon_insert_object_state *state);

jak_carbon_insert *jak_carbon_insert_array_begin(jak_carbon_insert_array_state *state_out, jak_carbon_insert *inserter_in, jak_u64 array_capacity);
bool jak_carbon_insert_array_end(jak_carbon_insert_array_state *state_in);

jak_carbon_insert *jak_carbon_insert_column_begin(jak_carbon_insert_column_state *state_out, jak_carbon_insert *inserter_in, jak_carbon_column_type_e type, jak_u64 column_capacity);
bool jak_carbon_insert_column_end(jak_carbon_insert_column_state *state_in);

bool jak_carbon_insert_prop_null(jak_carbon_insert *inserter, const char *key);
bool jak_carbon_insert_prop_true(jak_carbon_insert *inserter, const char *key);
bool jak_carbon_insert_prop_false(jak_carbon_insert *inserter, const char *key);
bool jak_carbon_insert_prop_u8(jak_carbon_insert *inserter, const char *key, jak_u8 value);
bool jak_carbon_insert_prop_u16(jak_carbon_insert *inserter, const char *key, jak_u16 value);
bool jak_carbon_insert_prop_u32(jak_carbon_insert *inserter, const char *key, jak_u32 value);
bool jak_carbon_insert_prop_u64(jak_carbon_insert *inserter, const char *key, jak_u64 value);
bool jak_carbon_insert_prop_i8(jak_carbon_insert *inserter, const char *key, jak_i8 value);
bool jak_carbon_insert_prop_i16(jak_carbon_insert *inserter, const char *key, jak_i16 value);
bool jak_carbon_insert_prop_i32(jak_carbon_insert *inserter, const char *key, jak_i32 value);
bool jak_carbon_insert_prop_i64(jak_carbon_insert *inserter, const char *key, jak_i64 value);
bool jak_carbon_insert_prop_unsigned(jak_carbon_insert *inserter, const char *key, jak_u64 value);
bool jak_carbon_insert_prop_signed(jak_carbon_insert *inserter, const char *key, jak_i64 value);
bool jak_carbon_insert_prop_float(jak_carbon_insert *inserter, const char *key, float value);
bool jak_carbon_insert_prop_string(jak_carbon_insert *inserter, const char *key, const char *value);
bool jak_carbon_insert_prop_nchar(jak_carbon_insert *inserter, const char *key, const char *value, jak_u64 value_len);
bool jak_carbon_insert_prop_binary(jak_carbon_insert *inserter, const char *key, const void *value, size_t nbytes, const char *file_ext, const char *user_type);

jak_carbon_insert *jak_carbon_insert_prop_object_begin(jak_carbon_insert_object_state *out, jak_carbon_insert *inserter, const char *key, jak_u64 object_capacity);
jak_u64 jak_carbon_insert_prop_object_end(jak_carbon_insert_object_state *state);

jak_carbon_insert *jak_carbon_insert_prop_array_begin(jak_carbon_insert_array_state *state, jak_carbon_insert *inserter, const char *key, jak_u64 array_capacity);
jak_u64 jak_carbon_insert_prop_array_end(jak_carbon_insert_array_state *state);

jak_carbon_insert *jak_carbon_insert_prop_column_begin(jak_carbon_insert_column_state *state_out, jak_carbon_insert *inserter_in, const char *key, jak_carbon_column_type_e type, jak_u64 column_capacity);
jak_u64 jak_carbon_insert_prop_column_end(jak_carbon_insert_column_state *state_in);

bool jak_carbon_insert_drop(jak_carbon_insert *inserter);

JAK_END_DECL

#endif
/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file is for internal usage only; do not call these functions from outside
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

#ifndef JAK_CARBON_INT_H
#define JAK_CARBON_INT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_memfile.h>
#include <jak_uintvar_stream.h>
#include <jak_json.h>
#include <jak_carbon_int.h>
#include <jak_carbon_field.h>
#include <jak_carbon_array_it.h>

JAK_BEGIN_DECL

typedef struct jak_carbon_insert {
        jak_carbon_container_e context_type;
        union {
                jak_carbon_array_it *array;
                jak_carbon_column_it *column;
                jak_carbon_object_it *object;
        } context;

        jak_memfile memfile;
        jak_offset_t position;
        jak_error err;
} jak_carbon_insert;

typedef struct jak_carbon_insert_array_state {
        jak_carbon_insert *parent_inserter;
        jak_carbon_array_it *nested_array;
        jak_carbon_insert nested_inserter;
        jak_offset_t array_begin, array_end;
} jak_carbon_insert_array_state;

typedef struct jak_carbon_insert_object_state {
        jak_carbon_insert *parent_inserter;
        jak_carbon_object_it *it;
        jak_carbon_insert inserter;
        jak_offset_t object_begin, object_end;
} jak_carbon_insert_object_state;

typedef struct jak_carbon_insert_column_state {
        jak_carbon_insert *parent_inserter;
        jak_carbon_field_type_e type;
        jak_carbon_column_it *nested_column;
        jak_carbon_insert nested_inserter;
        jak_offset_t column_begin, column_end;
} jak_carbon_insert_column_state;

bool jak_carbon_int_insert_object(jak_memfile *memfile, size_t nbytes);
bool jak_carbon_int_insert_array(jak_memfile *memfile, size_t nbytes);
bool jak_carbon_int_insert_column(jak_memfile *jak_memfile_in, jak_error *err_in, jak_carbon_column_type_e type, size_t capactity);

/**
 * Returns the number of bytes required to store a field type including its type marker in a byte sequence.
 */
size_t jak_carbon_int_get_type_size_encoded(jak_carbon_field_type_e type);

/**
 * Returns the number of bytes required to store a field value of a particular type exclusing its type marker.
 */
size_t jak_carbon_int_get_type_value_size(jak_carbon_field_type_e type);

bool jak_carbon_int_array_it_next(bool *is_empty_slot, bool *is_array_end, jak_carbon_array_it *it);
bool jak_carbon_int_array_it_refresh(bool *is_empty_slot, bool *is_array_end, jak_carbon_array_it *it);
bool jak_carbon_int_array_it_field_type_read(jak_carbon_array_it *it);
bool jak_carbon_int_array_skip_contents(bool *is_empty_slot, bool *is_array_end, jak_carbon_array_it *it);

bool jak_carbon_int_object_it_next(bool *is_empty_slot, bool *is_object_end, jak_carbon_object_it *it);
bool jak_carbon_int_object_it_refresh(bool *is_empty_slot, bool *is_object_end, jak_carbon_object_it *it);
bool jak_carbon_int_object_it_prop_key_access(jak_carbon_object_it *it);
bool jak_carbon_int_object_it_prop_value_skip(jak_carbon_object_it *it);
bool jak_carbon_int_object_it_prop_skip(jak_carbon_object_it *it);
bool jak_carbon_int_object_skip_contents(bool *is_empty_slot, bool *is_array_end, jak_carbon_object_it *it);
bool jak_carbon_int_field_data_access(jak_memfile *file, jak_error *err, jak_field_access *field_access);

jak_offset_t jak_carbon_int_column_get_payload_off(jak_carbon_column_it *it);
jak_offset_t jak_carbon_int_payload_after_header(jak_carbon *doc);

jak_u64 jak_carbon_int_header_get_commit_hash(jak_carbon *doc);

void jak_carbon_int_history_push(jak_vector ofType(jak_offset_t) *vec, jak_offset_t off);
void jak_carbon_int_history_clear(jak_vector ofType(jak_offset_t) *vec);
jak_offset_t jak_carbon_int_history_pop(jak_vector ofType(jak_offset_t) *vec);
jak_offset_t jak_carbon_int_history_peek(jak_vector ofType(jak_offset_t) *vec);
bool jak_carbon_int_history_has(jak_vector ofType(jak_offset_t) *vec);

bool jak_carbon_int_field_access_create(jak_field_access *field);
bool jak_carbon_int_field_access_clone(jak_field_access *dst, jak_field_access *src);
bool jak_carbon_int_field_access_drop(jak_field_access *field);
bool jak_carbon_int_field_auto_close(jak_field_access *it);
bool jak_carbon_int_field_access_object_it_opened(jak_field_access *field);
bool jak_carbon_int_field_access_array_it_opened(jak_field_access *field);
bool jak_carbon_int_field_access_column_it_opened(jak_field_access *field);
bool jak_carbon_int_field_access_field_type(jak_carbon_field_type_e *type, jak_field_access *field);
bool jak_carbon_int_field_access_u8_value(jak_u8 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_u16_value(jak_u16 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_u32_value(jak_u32 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_u64_value(jak_u64 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_i8_value(jak_i8 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_i16_value(jak_i16 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_i32_value(jak_i32 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_i64_value(jak_i64 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_float_value(bool *is_null_in, float *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_signed_value(bool *is_null_in, jak_i64 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_unsigned_value(bool *is_null_in, jak_u64 *value, jak_field_access *field, jak_error *err);
const char *jak_carbon_int_field_access_jak_string_value(jak_u64 *strlen, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_binary_value(jak_carbon_binary *out, jak_field_access *field, jak_error *err);
jak_carbon_array_it *jak_carbon_int_field_access_array_value(jak_field_access *field, jak_error *err);
jak_carbon_object_it *jak_carbon_int_field_access_object_value(jak_field_access *field, jak_error *err);
jak_carbon_column_it *jak_carbon_int_field_access_column_value(jak_field_access *field, jak_error *err);

void jak_carbon_int_auto_close_nested_array_it(jak_field_access *field);
void jak_carbon_int_auto_close_nested_object_it(jak_field_access *field);
void jak_carbon_int_auto_close_nested_column_it(jak_field_access *field);

bool jak_carbon_int_field_remove(jak_memfile *memfile, jak_error *err, jak_carbon_field_type_e type);

/**
 * For <code>mode</code>, see <code>jak_carbon_create_begin</code>
 */
bool jak_carbon_int_from_json(jak_carbon *doc, const jak_json *data, jak_carbon_key_e key_type, const void *primary_key, int mode);

JAK_END_DECL

#endif
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

#ifndef JAK_CARBON_KEY_H
#define JAK_CARBON_KEY_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_carbon.h>
#include <jak_memfile.h>

JAK_BEGIN_DECL

bool jak_carbon_key_create(jak_memfile *file, jak_carbon_key_e type, jak_error *err);
bool jak_carbon_key_skip(jak_carbon_key_e *out, jak_memfile *file);
bool jak_carbon_key_read_type(jak_carbon_key_e *out, jak_memfile *file);
bool jak_carbon_key_write_unsigned(jak_memfile *file, jak_u64 key);
bool jak_carbon_key_write_signed(jak_memfile *file, jak_i64 key);
bool jak_carbon_key_write_string(jak_memfile *file, const char *key);
bool jak_carbon_key_update_string(jak_memfile *file, const char *key);
bool jak_carbon_key_update_jak_string_wnchar(jak_memfile *file, const char *key, size_t length);
const void *jak_carbon_key_read(jak_u64 *len, jak_carbon_key_e *out, jak_memfile *file);
const char *jak_carbon_key_type_str(jak_carbon_key_e type);

JAK_END_DECL

#endif
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

#ifndef JAK_CARBON_MEDIA_H
#define JAK_CARBON_MEDIA_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_memfile.h>
#include <jak_carbon.h>
#include <jak_carbon_field.h>

JAK_BEGIN_DECL

typedef jak_u8 jak_media_type; /* byte to determine type at hand (e.g., JSON array, string, null, ...) */

static struct mime_type {
        const char *type;
        const char *ext;

} jak_global_mime_type_register[] = { /* the entries in this list must be sorted by extension! */
        {"application/vnd.lotus-1-2-3",                                               "123"},
        {"text/vnd.in3d.3dml",                                                        "3dml"},
        {"video/3gpp2",                                                               "3g2"},
        {"video/3gpp",                                                                "3gp"},
        {"application/x-7z-compressed",                                               "7z"},
        {"application/x-authorware-bin",                                              "aab"},
        {"audio/x-aac",                                                               "aac"},
        {"application/x-authorware-map",                                              "aam"},
        {"application/x-authorware-seg",                                              "aas"},
        {"application/x-abiword",                                                     "abw"},
        {"application/pkix-attr-cert",                                                "ac"},
        {"application/vnd.americandynamics.acc",                                      "acc"},
        {"application/x-ace-compressed",                                              "ace"},
        {"application/vnd.acucobol",                                                  "acu"},
        {"audio/adpcm",                                                               "adp"},
        {"application/vnd.audiograph",                                                "aep"},
        {"application/vnd.ibm.modcap",                                                "afp"},
        {"application/vnd.ahead.space",                                               "ahead"},
        {"application/postscript",                                                    "ai"},
        {"audio/x-aiff",                                                              "aif"},
        {"application/vnd.adobe.air-application-installer-package+zip",               "air"},
        {"application/vnd.dvb.ait",                                                   "ait"},
        {"application/vnd.amiga.ami",                                                 "ami"},
        {"application/vnd.android.package-archive",                                   "apk"},
        {"application/x-ms-application",                                              "application"},
        {"application/vnd.lotus-approach",                                            "apr"},
        {"video/x-ms-asf",                                                            "asf"},
        {"application/vnd.accpac.simply.aso",                                         "aso"},
        {"application/vnd.acucorp",                                                   "atc"},
        {"application/atom+xml",                                                      "atom"},
        {"application/atomcat+xml",                                                   "atomcat"},
        {"application/atomsvc+xml",                                                   "atomsvc"},
        {"application/vnd.antix.game-component",                                      "atx"},
        {"audio/basic",                                                               "au"},
        {"video/x-msvideo",                                                           "avi"},
        {"application/applixware",                                                    "aw"},
        {"application/vnd.airzip.filesecure.azf",                                     "azf"},
        {"application/vnd.airzip.filesecure.azs",                                     "azs"},
        {"application/vnd.amazon.ebook",                                              "azw"},
        {"application/x-bcpio",                                                       "bcpio"},
        {"application/x-font-bdf",                                                    "bdf"},
        {"application/vnd.syncml.dm+wbxml",                                           "bdm"},
        {"application/vnd.realvnc.bed",                                               "bed"},
        {"application/vnd.fujitsu.oasysprs",                                          "bh2"},
        {"application/octet-stream",                                                  "bin"},
        {"application/vnd.bmi",                                                       "bmi"},
        {"image/bmp",                                                                 "bmp"},
        {"application/vnd.previewsystems.box",                                        "box"},
        {"image/prs.btif",                                                            "btif"},
        {"application/x-bzip",                                                        "bz"},
        {"application/x-bzip2",                                                       "bz2"},
        {"text/x-c",                                                                  "c"},
        {"application/vnd.cluetrust.cartomobile-config",                              "c11amc"},
        {"application/vnd.cluetrust.cartomobile-config-pkg",                          "c11amz"},
        {"application/vnd.clonk.c4group",                                             "c4g"},
        {"application/vnd.ms-cab-compressed",                                         "cab"},
        {"application/vnd.curl.car",                                                  "car"},
        {"application/vnd.ms-pki.seccat",                                             "cat"},
        {"application/vnd.contact.cmsg",                                              "cdbcmsg"},
        {"application/vnd.mediastation.cdkey",                                        "cdkey"},
        {"application/cdmi-capability",                                               "cdmia"},
        {"application/cdmi-container",                                                "cdmic"},
        {"application/cdmi-domain",                                                   "cdmid"},
        {"application/cdmi-object",                                                   "cdmio"},
        {"application/cdmi-queue",                                                    "cdmiq"},
        {"chemical/x-cdx",                                                            "cdx"},
        {"application/vnd.chemdraw+xml",                                              "cdxml"},
        {"application/vnd.cinderella",                                                "cdy"},
        {"application/pkix-cert",                                                     "cer"},
        {"image/cgm",                                                                 "cgm"},
        {"application/x-chat",                                                        "chat"},
        {"application/vnd.ms-htmlhelp",                                               "chm"},
        {"application/vnd.kde.kchart",                                                "chrt"},
        {"chemical/x-cif",                                                            "cif"},
        {"application/vnd.anser-web-certificate-issue-initiation",                    "cii"},
        {"application/vnd.ms-artgalry",                                               "cil"},
        {"application/vnd.claymore",                                                  "cla"},
        {"application/java-vm",                                                       "class"},
        {"application/vnd.crick.clicker.keyboard",                                    "clkk"},
        {"application/vnd.crick.clicker.palette",                                     "clkp"},
        {"application/vnd.crick.clicker.template",                                    "clkt"},
        {"application/vnd.crick.clicker.wordbank",                                    "clkw"},
        {"application/vnd.crick.clicker",                                             "clkx"},
        {"application/x-msclip",                                                      "clp"},
        {"application/vnd.cosmocaller",                                               "cmc"},
        {"chemical/x-cmdf",                                                           "cmdf"},
        {"chemical/x-cml",                                                            "cml"},
        {"application/vnd.yellowriver-custom-menu",                                   "cmp"},
        {"image/x-cmx",                                                               "cmx"},
        {"application/vnd.rim.cod",                                                   "cod"},
        {"application/x-cpio",                                                        "cpio"},
        {"application/mac-compactpro",                                                "cpt"},
        {"application/x-mscardfile",                                                  "crd"},
        {"application/pkix-crl",                                                      "crl"},
        {"application/vnd.rig.cryptonote",                                            "cryptonote"},
        {"application/x-csh",                                                         "csh"},
        {"chemical/x-csml",                                                           "csml"},
        {"application/vnd.commonspace",                                               "csp"},
        {"text/css",                                                                  "css"},
        {"text/csv",                                                                  "csv"},
        {"application/cu-seeme",                                                      "cu"},
        {"text/vnd.curl",                                                             "curl"},
        {"application/prs.cww",                                                       "cww"},
        {"model/vnd.collada+xml",                                                     "dae"},
        {"application/vnd.mobius.daf",                                                "daf"},
        {"application/davmount+xml",                                                  "davmount"},
        {"text/vnd.curl.dcurl",                                                       "dcurl"},
        {"application/vnd.oma.dd2+xml",                                               "dd2"},
        {"application/vnd.fujixerox.ddd",                                             "ddd"},
        {"application/x-debian-package",                                              "deb"},
        {"application/x-x509-ca-cert",                                                "der"},
        {"application/vnd.dreamfactory",                                              "dfac"},
        {"application/x-director",                                                    "dir"},
        {"application/vnd.mobius.dis",                                                "dis"},
        {"image/vnd.djvu",                                                            "djvu"},
        {"application/x-apple-diskimage",                                             "dmg"},
        {"application/vnd.dna",                                                       "dna"},
        {"application/msword",                                                        "doc"},
        {"application/vnd.ms-word.document.macroenabled.12",                          "docm"},
        {"application/vnd.openxmlformats-officedocument.wordprocessingml.document",   "docx"},
        {"application/vnd.ms-word.template.macroenabled.12",                          "dotm"},
        {"application/vnd.openxmlformats-officedocument.wordprocessingml.template",   "dotx"},
        {"application/vnd.osgi.dp",                                                   "dp"},
        {"application/vnd.dpgraph",                                                   "dpg"},
        {"audio/vnd.dra",                                                             "dra"},
        {"text/prs.lines.tag",                                                        "dsc"},
        {"application/dssc+der",                                                      "dssc"},
        {"application/x-dtbook+xml",                                                  "dtb"},
        {"application/xml-dtd",                                                       "dtd"},
        {"audio/vnd.dts",                                                             "dts"},
        {"audio/vnd.dts.hd",                                                          "dtshd"},
        {"application/x-dvi",                                                         "dvi"},
        {"model/vnd.dwf",                                                             "dwf"},
        {"image/vnd.dwg",                                                             "dwg"},
        {"image/vnd.dxf",                                                             "dxf"},
        {"application/vnd.spotfire.dxp",                                              "dxp"},
        {"audio/vnd.nuera.ecelp4800",                                                 "ecelp4800"},
        {"audio/vnd.nuera.ecelp7470",                                                 "ecelp7470"},
        {"audio/vnd.nuera.ecelp9600",                                                 "ecelp9600"},
        {"application/vnd.novadigm.edm",                                              "edm"},
        {"application/vnd.novadigm.edx",                                              "edx"},
        {"application/vnd.picsel",                                                    "efif"},
        {"application/vnd.pg.osasli",                                                 "ei6"},
        {"message/rfc822",                                                            "eml"},
        {"application/emma+xml",                                                      "emma"},
        {"audio/vnd.digital-winds",                                                   "eol"},
        {"application/vnd.ms-fontobject",                                             "eot"},
        {"application/epub+zip",                                                      "epub"},
        {"application/ecmascript",                                                    "es"},
        {"application/vnd.eszigno3+xml",                                              "es3"},
        {"application/vnd.epson.esf",                                                 "esf"},
        {"text/x-setext",                                                             "etx"},
        {"application/x-msdownload",                                                  "exe"},
        {"application/exi",                                                           "exi"},
        {"application/vnd.novadigm.ext",                                              "ext"},
        {"application/vnd.ezpix-album",                                               "ez2"},
        {"application/vnd.ezpix-package",                                             "ez3"},
        {"text/x-fortran",                                                            "f"},
        {"video/x-f4v",                                                               "f4v"},
        {"image/vnd.fastbidsheet",                                                    "fbs"},
        {"application/vnd.isac.fcs",                                                  "fcs"},
        {"application/vnd.fdf",                                                       "fdf"},
        {"application/vnd.denovo.fcselayout-link",                                    "fe_launch"},
        {"application/vnd.fujitsu.oasysgp",                                           "fg5"},
        {"image/x-freehand",                                                          "fh"},
        {"application/x-xfig",                                                        "fig"},
        {"video/x-fli",                                                               "fli"},
        {"application/vnd.micrografx.flo",                                            "flo"},
        {"video/x-flv",                                                               "flv"},
        {"application/vnd.kde.kivio",                                                 "flw"},
        {"text/vnd.fmi.flexstor",                                                     "flx"},
        {"text/vnd.fly",                                                              "fly"},
        {"application/vnd.framemaker",                                                "fm"},
        {"application/vnd.frogans.fnc",                                               "fnc"},
        {"image/vnd.fpx",                                                             "fpx"},
        {"application/vnd.fsc.weblaunch",                                             "fsc"},
        {"image/vnd.fst",                                                             "fst"},
        {"application/vnd.fluxtime.clip",                                             "ftc"},
        {"application/vnd.anser-web-funds-transfer-initiation",                       "fti"},
        {"video/vnd.fvt",                                                             "fvt"},
        {"application/vnd.adobe.fxp",                                                 "fxp"},
        {"application/vnd.fuzzysheet",                                                "fzs"},
        {"application/vnd.geoplan",                                                   "g2w"},
        {"image/g3fax",                                                               "g3"},
        {"application/vnd.geospace",                                                  "g3w"},
        {"application/vnd.groove-account",                                            "gac"},
        {"model/vnd.gdl",                                                             "gdl"},
        {"application/vnd.dynageo",                                                   "geo"},
        {"application/vnd.geometry-explorer",                                         "gex"},
        {"application/vnd.geogebra.file",                                             "ggb"},
        {"application/vnd.geogebra.tool",                                             "ggt"},
        {"application/vnd.groove-help",                                               "ghf"},
        {"image/gif",                                                                 "gif"},
        {"application/vnd.groove-identity-message",                                   "gim"},
        {"application/vnd.gmx",                                                       "gmx"},
        {"application/x-gnumeric",                                                    "gnumeric"},
        {"application/vnd.flographit",                                                "gph"},
        {"application/vnd.grafeq",                                                    "gqf"},
        {"application/srgs",                                                          "gram"},
        {"application/vnd.groove-injector",                                           "grv"},
        {"application/srgs+xml",                                                      "grxml"},
        {"application/x-font-ghostscript",                                            "gsf"},
        {"application/x-gtar",                                                        "gtar"},
        {"application/vnd.groove-tool-message",                                       "gtm"},
        {"model/vnd.gtw",                                                             "gtw"},
        {"text/vnd.graphviz",                                                         "gv"},
        {"application/vnd.geonext",                                                   "gxt"},
        {"video/h261",                                                                "h261"},
        {"video/h263",                                                                "h263"},
        {"video/h264",                                                                "h264"},
        {"application/vnd.hal+xml",                                                   "hal"},
        {"application/vnd.hbci",                                                      "hbci"},
        {"application/x-hdf",                                                         "hdf"},
        {"application/winhlp",                                                        "hlp"},
        {"application/vnd.hp-hpgl",                                                   "hpgl"},
        {"application/vnd.hp-hpid",                                                   "hpid"},
        {"application/vnd.hp-hps",                                                    "hps"},
        {"application/mac-binhex40",                                                  "hqx"},
        {"application/vnd.kenameaapp",                                                "htke"},
        {"text/html",                                                                 "html"},
        {"application/vnd.yamaha.hv-dic",                                             "hvd"},
        {"application/vnd.yamaha.hv-voice",                                           "hvp"},
        {"application/vnd.yamaha.hv-script",                                          "hvs"},
        {"application/vnd.intergeo",                                                  "i2g"},
        {"application/vnd.iccprofile",                                                "icc"},
        {"x-conference/x-cooltalk",                                                   "ice"},
        {"image/x-icon",                                                              "ico"},
        {"text/calendar",                                                             "ics"},
        {"image/ief",                                                                 "ief"},
        {"application/vnd.shana.informed.formdata",                                   "ifm"},
        {"application/vnd.igloader",                                                  "igl"},
        {"application/vnd.insors.igm",                                                "igm"},
        {"model/iges",                                                                "igs"},
        {"application/vnd.micrografx.igx",                                            "igx"},
        {"application/vnd.shana.informed.interchange",                                "iif"},
        {"application/vnd.accpac.simply.imp",                                         "imp"},
        {"application/vnd.ms-ims",                                                    "ims"},
        {"application/ipfix",                                                         "ipfix"},
        {"application/vnd.shana.informed.package",                                    "ipk"},
        {"application/vnd.ibm.rights-management",                                     "irm"},
        {"application/vnd.irepository.package+xml",                                   "irp"},
        {"application/vnd.shana.informed.formtemplate",                               "itp"},
        {"application/vnd.immervision-ivp",                                           "ivp"},
        {"application/vnd.immervision-ivu",                                           "ivu"},
        {"text/vnd.sun.j2me.app-descriptor",                                          "jad"},
        {"application/vnd.jam",                                                       "jam"},
        {"application/java-archive",                                                  "jar"},
        {"text/x-java-source",                                                        "java"},
        {"application/vnd.jisp",                                                      "jisp"},
        {"application/vnd.hp-jlyt",                                                   "jlt"},
        {"application/x-java-jnlp-file",                                              "jnlp"},
        {"application/vnd.joost.joda-archive",                                        "joda"},
        {"image/jpeg",                                                                "jpeg"},
        {"video/jpeg",                                                                "jpgv"},
        {"video/jpm",                                                                 "jpm"},
        {"application/javascript",                                                    "js"},
        {"application/json",                                                          "json"},
        {"application/vnd.kde.karbon",                                                "karbon"},
        {"application/vnd.kde.kformula",                                              "kfo"},
        {"application/vnd.kidspiration",                                              "kia"},
        {"application/vnd.google-earth.kml+xml",                                      "kml"},
        {"application/vnd.google-earth.kmz",                                          "kmz"},
        {"application/vnd.kinar",                                                     "kne"},
        {"application/vnd.kde.kontour",                                               "kon"},
        {"application/vnd.kde.kpresenter",                                            "kpr"},
        {"application/vnd.kde.kspread",                                               "ksp"},
        {"image/ktx",                                                                 "ktx"},
        {"application/vnd.kahootz",                                                   "ktz"},
        {"application/vnd.kde.kword",                                                 "kwd"},
        {"application/vnd.las.las+xml",                                               "lasxml"},
        {"application/x-latex",                                                       "latex"},
        {"application/vnd.llamagraphics.life-balance.desktop",                        "lbd"},
        {"application/vnd.llamagraphics.life-balance.exchange+xml",                   "lbe"},
        {"application/vnd.hhe.lesson-player",                                         "les"},
        {"application/vnd.route66.link66+xml",                                        "link66"},
        {"application/vnd.ms-lrm",                                                    "lrm"},
        {"application/vnd.frogans.ltf",                                               "ltf"},
        {"audio/vnd.lucent.voice",                                                    "lvp"},
        {"application/vnd.lotus-wordpro",                                             "lwp"},
        {"application/mp21",                                                          "m21"},
        {"audio/x-mpegurl",                                                           "m3u"},
        {"application/vnd.apple.mpegurl",                                             "m3u8"},
        {"video/x-m4v",                                                               "m4v"},
        {"application/mathematica",                                                   "ma"},
        {"application/mads+xml",                                                      "mads"},
        {"application/vnd.ecowin.chart",                                              "mag"},
        {"application/mathml+xml",                                                    "mathml"},
        {"application/vnd.mobius.mbk",                                                "mbk"},
        {"application/mbox",                                                          "mbox"},
        {"application/vnd.medcalcdata",                                               "mc1"},
        {"application/vnd.mcd",                                                       "mcd"},
        {"text/vnd.curl.mcurl",                                                       "mcurl"},
        {"application/x-msaccess",                                                    "mdb"},
        {"image/vnd.ms-modi",                                                         "mdi"},
        {"application/metalink4+xml",                                                 "meta4"},
        {"application/mets+xml",                                                      "mets"},
        {"application/vnd.mfmp",                                                      "mfm"},
        {"application/vnd.osgeo.mapguide.package",                                    "mgp"},
        {"application/vnd.proteus.magazine",                                          "mgz"},
        {"audio/midi",                                                                "mid"},
        {"application/vnd.mif",                                                       "mif"},
        {"video/mj2",                                                                 "mj2"},
        {"application/vnd.dolby.mlp",                                                 "mlp"},
        {"application/vnd.chipnuts.karaoke-mmd",                                      "mmd"},
        {"application/vnd.smaf",                                                      "mmf"},
        {"image/vnd.fujixerox.edmics-mmr",                                            "mmr"},
        {"application/x-msmoney",                                                     "mny"},
        {"application/mods+xml",                                                      "mods"},
        {"video/x-sgi-movie",                                                         "movie"},
        {"application/mp4",                                                           "mp4"},
        {"application/vnd.mophun.certificate",                                        "mpc"},
        {"video/mpeg",                                                                "mpeg"},
        {"audio/mpeg",                                                                "mpga"},
        {"application/vnd.apple.installer+xml",                                       "mpkg"},
        {"application/vnd.blueice.multipass",                                         "mpm"},
        {"application/vnd.mophun.application",                                        "mpn"},
        {"application/vnd.ms-project",                                                "mpp"},
        {"application/vnd.ibm.minipay",                                               "mpy"},
        {"application/vnd.mobius.mqy",                                                "mqy"},
        {"application/marc",                                                          "mrc"},
        {"application/marcxml+xml",                                                   "mrcx"},
        {"application/mediaservercontrol+xml",                                        "mscml"},
        {"application/vnd.mseq",                                                      "mseq"},
        {"application/vnd.epson.msf",                                                 "msf"},
        {"model/mesh",                                                                "msh"},
        {"application/vnd.mobius.msl",                                                "msl"},
        {"application/vnd.muvee.style",                                               "msty"},
        {"model/vnd.mts",                                                             "mts"},
        {"application/vnd.musician",                                                  "mus"},
        {"application/vnd.recordare.musicxml+xml",                                    "musicxml"},
        {"application/x-msmediaview",                                                 "mvb"},
        {"application/vnd.mfer",                                                      "mwf"},
        {"application/mxf",                                                           "mxf"},
        {"application/vnd.recordare.musicxml",                                        "mxl"},
        {"application/xv+xml",                                                        "mxml"},
        {"application/vnd.triscape.mxs",                                              "mxs"},
        {"video/vnd.mpegurl",                                                         "mxu"},
        {"application/vnd.nokia.n-gage.symbian.install",                              "n-gage"},
        {"text/n3",                                                                   "n3"},
        {"application/vnd.wolfram.player",                                            "nbp"},
        {"application/x-netcdf",                                                      "nc"},
        {"application/x-dtbncx+xml",                                                  "ncx"},
        {"application/vnd.nokia.n-gage.data",                                         "ngdat"},
        {"application/vnd.neurolanguage.nlu",                                         "nlu"},
        {"application/vnd.enliven",                                                   "nml"},
        {"application/vnd.noblenet-directory",                                        "nnd"},
        {"application/vnd.noblenet-sealer",                                           "nns"},
        {"application/vnd.noblenet-web",                                              "nnw"},
        {"image/vnd.net-fpx",                                                         "npx"},
        {"application/vnd.lotus-notes",                                               "nsf"},
        {"application/vnd.fujitsu.oasys2",                                            "oa2"},
        {"application/vnd.fujitsu.oasys3",                                            "oa3"},
        {"application/vnd.fujitsu.oasys",                                             "oas"},
        {"application/x-msbinder",                                                    "obd"},
        {"application/oda",                                                           "oda"},
        {"application/vnd.oasis.opendocument.database",                               "odb"},
        {"application/vnd.oasis.opendocument.chart",                                  "odc"},
        {"application/vnd.oasis.opendocument.formula",                                "odf"},
        {"application/vnd.oasis.opendocument.formula-template",                       "odft"},
        {"application/vnd.oasis.opendocument.graphics",                               "odg"},
        {"application/vnd.oasis.opendocument.image",                                  "odi"},
        {"application/vnd.oasis.opendocument.text-master",                            "odm"},
        {"application/vnd.oasis.opendocument.presentation",                           "odp"},
        {"application/vnd.oasis.opendocument.spreadsheet",                            "ods"},
        {"application/vnd.oasis.opendocument.text",                                   "odt"},
        {"audio/ogg",                                                                 "oga"},
        {"video/ogg",                                                                 "ogv"},
        {"application/ogg",                                                           "ogx"},
        {"application/onenote",                                                       "onetoc"},
        {"application/oebps-package+xml",                                             "opf"},
        {"application/vnd.lotus-organizer",                                           "org"},
        {"application/vnd.yamaha.openscoreformat",                                    "osf"},
        {"application/vnd.yamaha.openscoreformat.osfpvg+xml",                         "osfpvg"},
        {"application/vnd.oasis.opendocument.chart-template",                         "otc"},
        {"application/x-font-otf",                                                    "otf"},
        {"application/vnd.oasis.opendocument.graphics-template",                      "otg"},
        {"application/vnd.oasis.opendocument.text-web",                               "oth"},
        {"application/vnd.oasis.opendocument.image-template",                         "oti"},
        {"application/vnd.oasis.opendocument.presentation-template",                  "otp"},
        {"application/vnd.oasis.opendocument.spreadsheet-template",                   "ots"},
        {"application/vnd.oasis.opendocument.text-template",                          "ott"},
        {"application/vnd.openofficeorg.extension",                                   "oxt"},
        {"text/x-pascal",                                                             "p"},
        {"application/pkcs10",                                                        "p10"},
        {"application/x-pkcs12",                                                      "p12"},
        {"application/x-pkcs7-certificates",                                          "p7b"},
        {"application/pkcs7-mime",                                                    "p7m"},
        {"application/x-pkcs7-certreqresp",                                           "p7r"},
        {"application/pkcs7-signature",                                               "p7s"},
        {"application/pkcs8",                                                         "p8"},
        {"text/plain-bas",                                                            "par"},
        {"application/vnd.pawaafile",                                                 "paw"},
        {"application/vnd.powerbuilder6",                                             "pbd"},
        {"image/x-portable-bitmap",                                                   "pbm"},
        {"application/x-font-pcf",                                                    "pcf"},
        {"application/vnd.hp-pcl",                                                    "pcl"},
        {"application/vnd.hp-pclxl",                                                  "pclxl"},
        {"application/vnd.curl.pcurl",                                                "pcurl"},
        {"image/x-pcx",                                                               "pcx"},
        {"application/vnd.palm",                                                      "pdb"},
        {"application/pdf",                                                           "pdf"},
        {"application/x-font-type1",                                                  "pfa"},
        {"application/font-tdpfr",                                                    "pfr"},
        {"image/x-portable-graymap",                                                  "pgm"},
        {"application/x-chess-pgn",                                                   "pgn"},
        {"application/pgp-encrypted",                                                 "pgp"},
        {"image/x-pict",                                                              "pic"},
        {"image/pjpeg",                                                               "pjpeg"},
        {"application/pkixcmp",                                                       "pki"},
        {"application/pkix-pkipath",                                                  "pkipath"},
        {"application/vnd.3gpp.pic-bw-large",                                         "plb"},
        {"application/vnd.mobius.plc",                                                "plc"},
        {"application/vnd.pocketlearn",                                               "plf"},
        {"application/pls+xml",                                                       "pls"},
        {"application/vnd.ctc-posml",                                                 "pml"},
        {"image/png",                                                                 "png"},
        {"image/x-portable-anymap",                                                   "pnm"},
        {"application/vnd.macports.portpkg",                                          "portpkg"},
        {"application/vnd.ms-powerpoint.template.macroenabled.12",                    "potm"},
        {"application/vnd.openxmlformats-officedocument.presentationml.template",     "potx"},
        {"application/vnd.ms-powerpoint.addin.macroenabled.12",                       "ppam"},
        {"application/vnd.cups-ppd",                                                  "ppd"},
        {"image/x-portable-pixmap",                                                   "ppm"},
        {"application/vnd.ms-powerpoint.slideshow.macroenabled.12",                   "ppsm"},
        {"application/vnd.openxmlformats-officedocument.presentationml.slideshow",    "ppsx"},
        {"application/vnd.ms-powerpoint",                                             "ppt"},
        {"application/vnd.ms-powerpoint.presentation.macroenabled.12",                "pptm"},
        {"application/vnd.openxmlformats-officedocument.presentationml.presentation", "pptx"},
        {"application/x-mobipocket-ebook",                                            "prc"},
        {"application/vnd.lotus-freelance",                                           "pre"},
        {"application/pics-rules",                                                    "prf"},
        {"application/vnd.3gpp.pic-bw-small",                                         "psb"},
        {"image/vnd.adobe.photoshop",                                                 "psd"},
        {"application/x-font-linux-psf",                                              "psf"},
        {"application/pskc+xml",                                                      "pskcxml"},
        {"application/vnd.pvi.ptid1",                                                 "ptid"},
        {"application/x-mspublisher",                                                 "pub"},
        {"application/vnd.3gpp.pic-bw-var",                                           "pvb"},
        {"application/vnd.3m.post-it-notes",                                          "pwn"},
        {"audio/vnd.ms-playready.media.pya",                                          "pya"},
        {"video/vnd.ms-playready.media.pyv",                                          "pyv"},
        {"application/vnd.epson.quickanime",                                          "qam"},
        {"application/vnd.intu.qbo",                                                  "qbo"},
        {"application/vnd.intu.qfx",                                                  "qfx"},
        {"application/vnd.publishare-delta-tree",                                     "qps"},
        {"video/quicktime",                                                           "qt"},
        {"application/vnd.quark.quarkxpress",                                         "qxd"},
        {"audio/x-pn-realaudio",                                                      "ram"},
        {"application/x-rar-compressed",                                              "rar"},
        {"image/x-cmu-raster",                                                        "ras"},
        {"application/vnd.ipunplugged.rcprofile",                                     "rcprofile"},
        {"application/rdf+xml",                                                       "rdf"},
        {"application/vnd.data-vision.rdz",                                           "rdz"},
        {"application/vnd.businessobjects",                                           "rep"},
        {"application/x-dtbresource+xml",                                             "res"},
        {"image/x-rgb",                                                               "rgb"},
        {"application/reginfo+xml",                                                   "rif"},
        {"audio/vnd.rip",                                                             "rip"},
        {"application/resource-lists+xml",                                            "rl"},
        {"image/vnd.fujixerox.edmics-rlc",                                            "rlc"},
        {"application/resource-lists-diff+xml",                                       "rld"},
        {"application/vnd.rn-realmedia",                                              "rm"},
        {"audio/x-pn-realaudio-plugin",                                               "rmp"},
        {"application/vnd.jcp.javame.midlet-rms",                                     "rms"},
        {"application/relax-ng-compact-syntax",                                       "rnc"},
        {"application/vnd.cloanto.rp9",                                               "rp9"},
        {"application/vnd.nokia.radio-presets",                                       "rpss"},
        {"application/vnd.nokia.radio-preset",                                        "rpst"},
        {"application/sparql-query",                                                  "rq"},
        {"application/rls-services+xml",                                              "rs"},
        {"application/rsd+xml",                                                       "rsd"},
        {"application/rss+xml",                                                       "rss"},
        {"application/rtf",                                                           "rtf"},
        {"text/richtext",                                                             "rtx"},
        {"text/x-asm",                                                                "s"},
        {"application/vnd.yamaha.smaf-audio",                                         "saf"},
        {"application/sbml+xml",                                                      "sbml"},
        {"application/vnd.ibm.secure-container",                                      "sc"},
        {"application/x-msschedule",                                                  "scd"},
        {"application/vnd.lotus-screencam",                                           "scm"},
        {"application/scvp-cv-request",                                               "scq"},
        {"application/scvp-cv-response",                                              "scs"},
        {"text/vnd.curl.scurl",                                                       "scurl"},
        {"application/vnd.stardivision.draw",                                         "sda"},
        {"application/vnd.stardivision.calc",                                         "sdc"},
        {"application/vnd.stardivision.impress",                                      "sdd"},
        {"application/vnd.solent.sdkm+xml",                                           "sdkm"},
        {"application/sdp",                                                           "sdp"},
        {"application/vnd.stardivision.writer",                                       "sdw"},
        {"application/vnd.seemail",                                                   "see"},
        {"application/vnd.fdsn.seed",                                                 "seed"},
        {"application/vnd.sema",                                                      "sema"},
        {"application/vnd.semd",                                                      "semd"},
        {"application/vnd.semf",                                                      "semf"},
        {"application/java-serialized-object",                                        "ser"},
        {"application/set-payment-initiation",                                        "setpay"},
        {"application/set-registration-initiation",                                   "setreg"},
        {"application/vnd.hydrostatix.sof-data",                                      "sfd-hdstx"},
        {"application/vnd.spotfire.sfs",                                              "sfs"},
        {"application/vnd.stardivision.writer-global",                                "sgl"},
        {"text/sgml",                                                                 "sgml"},
        {"application/x-sh",                                                          "sh"},
        {"application/x-shar",                                                        "shar"},
        {"application/shf+xml",                                                       "shf"},
        {"application/vnd.symbian.install",                                           "sis"},
        {"application/x-stuffit",                                                     "sit"},
        {"application/x-stuffitx",                                                    "sitx"},
        {"application/vnd.koan",                                                      "skp"},
        {"application/vnd.ms-powerpoint.slide.macroenabled.12",                       "sldm"},
        {"application/vnd.openxmlformats-officedocument.presentationml.slide",        "sldx"},
        {"application/vnd.epson.salt",                                                "slt"},
        {"application/vnd.stepmania.stepchart",                                       "sm"},
        {"application/vnd.stardivision.math",                                         "smf"},
        {"application/smil+xml",                                                      "smi"},
        {"application/x-font-snf",                                                    "snf"},
        {"application/vnd.yamaha.smaf-phrase",                                        "spf"},
        {"application/x-futuresplash",                                                "spl"},
        {"text/vnd.in3d.spot",                                                        "spot"},
        {"application/scvp-vp-response",                                              "spp"},
        {"application/scvp-vp-request",                                               "spq"},
        {"application/x-wais-source",                                                 "src"},
        {"application/sru+xml",                                                       "sru"},
        {"application/sparql-results+xml",                                            "srx"},
        {"application/vnd.kodak-descriptor",                                          "sse"},
        {"application/vnd.epson.ssf",                                                 "ssf"},
        {"application/ssml+xml",                                                      "ssml"},
        {"application/vnd.sailingtracker.track",                                      "st"},
        {"application/vnd.sun.xml.calc.template",                                     "stc"},
        {"application/vnd.sun.xml.draw.template",                                     "std"},
        {"application/vnd.wt.stf",                                                    "stf"},
        {"application/vnd.sun.xml.impress.template",                                  "sti"},
        {"application/hyperstudio",                                                   "stk"},
        {"application/vnd.ms-pki.stl",                                                "stl"},
        {"application/vnd.pg.format",                                                 "str"},
        {"application/vnd.sun.xml.writer.template",                                   "stw"},
        {"image/vnd.dvb.subtitle",                                                    "sub"},
        {"application/vnd.sus-calendar",                                              "sus"},
        {"application/x-sv4cpio",                                                     "sv4cpio"},
        {"application/x-sv4crc",                                                      "sv4crc"},
        {"application/vnd.dvb.service",                                               "svc"},
        {"application/vnd.svd",                                                       "svd"},
        {"image/svg+xml",                                                             "svg"},
        {"application/x-shockwave-flash",                                             "swf"},
        {"application/vnd.aristanetworks.swi",                                        "swi"},
        {"application/vnd.sun.xml.calc",                                              "sxc"},
        {"application/vnd.sun.xml.draw",                                              "sxd"},
        {"application/vnd.sun.xml.writer.global",                                     "sxg"},
        {"application/vnd.sun.xml.impress",                                           "sxi"},
        {"application/vnd.sun.xml.math",                                              "sxm"},
        {"application/vnd.sun.xml.writer",                                            "sxw"},
        {"text/troff",                                                                "t"},
        {"application/vnd.tao.intent-module-archive",                                 "tao"},
        {"application/x-tar",                                                         "tar"},
        {"application/vnd.3gpp2.tcap",                                                "tcap"},
        {"application/x-tcl",                                                         "tcl"},
        {"application/vnd.smart.teacher",                                             "teacher"},
        {"application/tei+xml",                                                       "tei"},
        {"application/x-tex",                                                         "tex"},
        {"application/x-texinfo",                                                     "texinfo"},
        {"application/thraud+xml",                                                    "tfi"},
        {"application/x-tex-tfm",                                                     "tfm"},
        {"application/vnd.ms-officetheme",                                            "thmx"},
        {"image/tiff",                                                                "tiff"},
        {"application/vnd.tmobile-livetv",                                            "tmo"},
        {"application/x-bittorrent",                                                  "torrent"},
        {"application/vnd.groove-tool-template",                                      "tpl"},
        {"application/vnd.trid.tpt",                                                  "tpt"},
        {"application/vnd.trueapp",                                                   "tra"},
        {"application/x-msterminal",                                                  "trm"},
        {"application/timestamped-data",                                              "tsd"},
        {"text/tab-separated-values",                                                 "tsv"},
        {"application/x-font-ttf",                                                    "ttf"},
        {"text/turtle",                                                               "ttl"},
        {"application/vnd.simtech-mindmapper",                                        "twd"},
        {"application/vnd.genomatix.tuxedo",                                          "txd"},
        {"application/vnd.mobius.txf",                                                "txf"},
        {"text/plain",                                                                "txt"},
        {"application/vnd.ufdl",                                                      "ufd"},
        {"application/vnd.umajin",                                                    "umj"},
        {"application/vnd.unity",                                                     "unityweb"},
        {"application/vnd.uoml+xml",                                                  "uoml"},
        {"text/uri-list",                                                             "uri"},
        {"application/x-ustar",                                                       "ustar"},
        {"application/vnd.uiq.theme",                                                 "utz"},
        {"text/x-uuencode",                                                           "uu"},
        {"audio/vnd.dece.audio",                                                      "uva"},
        {"video/vnd.dece.hd",                                                         "uvh"},
        {"image/vnd.dece.graphic",                                                    "uvi"},
        {"video/vnd.dece.mobile",                                                     "uvm"},
        {"video/vnd.dece.pd",                                                         "uvp"},
        {"video/vnd.dece.sd",                                                         "uvs"},
        {"video/vnd.uvvu.mp4",                                                        "uvu"},
        {"video/vnd.dece.video",                                                      "uvv"},
        {"application/x-cdlink",                                                      "vcd"},
        {"text/x-vcard",                                                              "vcf"},
        {"application/vnd.groove-vcard",                                              "vcg"},
        {"text/x-vcalendar",                                                          "vcs"},
        {"application/vnd.vcx",                                                       "vcx"},
        {"application/vnd.visionary",                                                 "vis"},
        {"video/vnd.vivo",                                                            "viv"},
        {"application/vnd.visio",                                                     "vsd"},
        {"application/vnd.visio2013",                                                 "vsdx"},
        {"application/vnd.vsf",                                                       "vsf"},
        {"model/vnd.vtu",                                                             "vtu"},
        {"application/voicexml+xml",                                                  "vxml"},
        {"application/x-doom",                                                        "wad"},
        {"audio/x-wav",                                                               "wav"},
        {"audio/x-ms-wax",                                                            "wax"},
        {"image/vnd.wap.wbmp",                                                        "wbmp"},
        {"application/vnd.criticaltools.wbs+xml",                                     "wbs"},
        {"application/vnd.wap.wbxml",                                                 "wbxml"},
        {"audio/webm",                                                                "weba"},
        {"video/webm",                                                                "webm"},
        {"image/webp",                                                                "webp"},
        {"application/vnd.pmi.widget",                                                "wg"},
        {"application/widget",                                                        "wgt"},
        {"video/x-ms-wm",                                                             "wm"},
        {"audio/x-ms-wma",                                                            "wma"},
        {"application/x-ms-wmd",                                                      "wmd"},
        {"application/x-msmetafile",                                                  "wmf"},
        {"text/vnd.wap.wml",                                                          "wml"},
        {"application/vnd.wap.wmlc",                                                  "wmlc"},
        {"text/vnd.wap.wmlscript",                                                    "wmls"},
        {"application/vnd.wap.wmlscriptc",                                            "wmlsc"},
        {"video/x-ms-wmv",                                                            "wmv"},
        {"video/x-ms-wmx",                                                            "wmx"},
        {"application/x-ms-wmz",                                                      "wmz"},
        {"application/x-font-woff",                                                   "woff"},
        {"application/vnd.wordperfect",                                               "wpd"},
        {"application/vnd.ms-wpl",                                                    "wpl"},
        {"application/vnd.ms-works",                                                  "wps"},
        {"application/vnd.wqd",                                                       "wqd"},
        {"application/x-mswrite",                                                     "wri"},
        {"model/vrml",                                                                "wrl"},
        {"application/wsdl+xml",                                                      "wsdl"},
        {"application/wspolicy+xml",                                                  "wspolicy"},
        {"application/vnd.webturbo",                                                  "wtb"},
        {"video/x-ms-wvx",                                                            "wvx"},
        {"application/vnd.hzn-3d-crossword",                                          "x3d"},
        {"application/x-silverlight-app",                                             "xap"},
        {"application/vnd.xara",                                                      "xar"},
        {"application/x-ms-xbap",                                                     "xbap"},
        {"application/vnd.fujixerox.docuworks.binder",                                "xbd"},
        {"image/x-xbitmap",                                                           "xbm"},
        {"application/xcap-diff+xml",                                                 "xdf"},
        {"application/vnd.syncml.dm+xml",                                             "xdm"},
        {"application/vnd.adobe.xdp+xml",                                             "xdp"},
        {"application/dssc+xml",                                                      "xdssc"},
        {"application/vnd.fujixerox.docuworks",                                       "xdw"},
        {"application/xenc+xml",                                                      "xenc"},
        {"application/patch-ops-JAK_ERROR+xml",                                           "xer"},
        {"application/vnd.adobe.xfdf",                                                "xfdf"},
        {"application/vnd.xfdl",                                                      "xfdl"},
        {"application/xhtml+xml",                                                     "xhtml"},
        {"image/vnd.xiff",                                                            "xif"},
        {"application/vnd.ms-excel.addin.macroenabled.12",                            "xlam"},
        {"application/vnd.ms-excel",                                                  "xls"},
        {"application/vnd.ms-excel.sheet.binary.macroenabled.12",                     "xlsb"},
        {"application/vnd.ms-excel.sheet.macroenabled.12",                            "xlsm"},
        {"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",         "xlsx"},
        {"application/vnd.ms-excel.template.macroenabled.12",                         "xltm"},
        {"application/vnd.openxmlformats-officedocument.spreadsheetml.template",      "xltx"},
        {"application/xml",                                                           "xml"},
        {"application/vnd.olpc-sugar",                                                "xo"},
        {"application/xop+xml",                                                       "xop"},
        {"application/x-xpinstall",                                                   "xpi"},
        {"image/x-xpixmap",                                                           "xpm"},
        {"application/vnd.is-xpr",                                                    "xpr"},
        {"application/vnd.ms-xpsdocument",                                            "xps"},
        {"application/vnd.intercon.formnet",                                          "xpw"},
        {"application/xslt+xml",                                                      "xslt"},
        {"application/vnd.syncml+xml",                                                "xsm"},
        {"application/xspf+xml",                                                      "xspf"},
        {"application/vnd.mozilla.xul+xml",                                           "xul"},
        {"image/x-xwindowdump",                                                       "xwd"},
        {"chemical/x-xyz",                                                            "xyz"},
        {"text/yaml",                                                                 "yaml"},
        {"application/yang",                                                          "yang"},
        {"application/yin+xml",                                                       "yin"},
        {"application/vnd.zzazz.deck+xml",                                            "zaz"},
        {"application/zip",                                                           "zip"},
        {"application/vnd.zul",                                                       "zir"},
        {"application/vnd.handheld-entertainment+xml",                                "zmm"},
};

static const jak_u32 _jak_global_mime_type_register = (jak_u32) JAK_ARRAY_LENGTH(jak_global_mime_type_register);

bool jak_carbon_media_write(jak_memfile *dst, jak_carbon_field_type_e type);

/**
 * Returns the mime type identifier for a file extension <code>ext</code>. If <code>ext</code> is not known,
 * the mime type application/octet-stream (.bin) is returned.
 */
jak_u32 jak_carbon_media_mime_type_by_ext(const char *ext);

/**
 * Returns a human readable string representing the mime type for the mime type identifier <code>id</code>.
 * In case <code>id</code> is invalid, the mime type application/octet-stream is returned.
 */
const char *jak_carbon_media_mime_type_by_id(jak_u32 id);

/**
 * Returns the file extension for the mime type identifier <code>id</code>.
 * In case <code>id</code> is invalid, the file extension "bin" is returned.
 */
const char *jak_carbon_media_mime_ext_by_id(jak_u32 id);

JAK_END_DECL

#endif
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

#ifndef JAK_CARBON_OBJECT_IT_H
#define JAK_CARBON_OBJECT_IT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_memfile.h>
#include <jak_carbon_field.h>
#include <jak_carbon_array_it.h>

JAK_BEGIN_DECL

typedef struct jak_carbon_object_it {
        jak_memfile memfile;
        jak_error err;

        jak_offset_t object_contents_off;
        bool object_end_reached;

        jak_vector ofType(jak_offset_t) history;

        struct {
                struct {
                        jak_offset_t offset;
                        const char *name;
                        jak_u64 name_len;
                } key;
                struct {
                        jak_offset_t offset;
                        jak_field_access data;
                } value;
        } field;

        jak_spinlock lock;
        /* in case of modifications (updates, inserts, deletes), the number of bytes that are added resp. removed */
        jak_i64 mod_size;
} jak_carbon_object_it;

bool jak_carbon_object_it_create(jak_carbon_object_it *it, jak_memfile *memfile, jak_error *err, jak_offset_t payload_start);
bool jak_carbon_object_it_copy(jak_carbon_object_it *dst, jak_carbon_object_it *src);
bool jak_carbon_object_it_clone(jak_carbon_object_it *dst, jak_carbon_object_it *src);
bool jak_carbon_object_it_drop(jak_carbon_object_it *it);

bool jak_carbon_object_it_rewind(jak_carbon_object_it *it);
bool jak_carbon_object_it_next(jak_carbon_object_it *it);
bool jak_carbon_object_it_has_next(jak_carbon_object_it *it);
bool jak_carbon_object_it_fast_forward(jak_carbon_object_it *it);
bool jak_carbon_object_it_prev(jak_carbon_object_it *it);

jak_offset_t jak_carbon_object_it_jak_memfile_pos(jak_carbon_object_it *it);
bool jak_carbon_object_it_tell(jak_offset_t *key_off, jak_offset_t *value_off, jak_carbon_object_it *it);

const char *jak_carbon_object_it_prop_name(jak_u64 *key_len, jak_carbon_object_it *it);
bool jak_carbon_object_it_remove(jak_carbon_object_it *it);
bool jak_carbon_object_it_prop_type(jak_carbon_field_type_e *type, jak_carbon_object_it *it);

bool jak_carbon_object_it_insert_begin(jak_carbon_insert *inserter, jak_carbon_object_it *it);
bool jak_carbon_object_it_insert_end(jak_carbon_insert *inserter);

bool jak_carbon_object_it_lock(jak_carbon_object_it *it);
bool jak_carbon_object_it_unlock(jak_carbon_object_it *it);

bool jak_carbon_object_it_u8_value(jak_u8 *value, jak_carbon_object_it *it);
bool jak_carbon_object_it_u16_value(jak_u16 *value, jak_carbon_object_it *it);
bool jak_carbon_object_it_u32_value(jak_u32 *value, jak_carbon_object_it *it);
bool jak_carbon_object_it_u64_value(jak_u64 *value, jak_carbon_object_it *it);
bool jak_carbon_object_it_i8_value(jak_i8 *value, jak_carbon_object_it *it);
bool jak_carbon_object_it_i16_value(jak_i16 *value, jak_carbon_object_it *it);
bool jak_carbon_object_it_i32_value(jak_i32 *value, jak_carbon_object_it *it);
bool jak_carbon_object_it_i64_value(jak_i64 *value, jak_carbon_object_it *it);
bool jak_carbon_object_it_float_value(bool *is_null_in, float *value, jak_carbon_object_it *it);
bool jak_carbon_object_it_signed_value(bool *is_null_in, jak_i64 *value, jak_carbon_object_it *it);
bool jak_carbon_object_it_unsigned_value(bool *is_null_in, jak_u64 *value, jak_carbon_object_it *it);
const char *jak_carbon_object_it_jak_string_value(jak_u64 *strlen, jak_carbon_object_it *it);
bool jak_carbon_object_it_binary_value(jak_carbon_binary *out, jak_carbon_object_it *it);
jak_carbon_array_it *jak_carbon_object_it_array_value(jak_carbon_object_it *it_in);
jak_carbon_object_it *jak_carbon_object_it_object_value(jak_carbon_object_it *it_in);
jak_carbon_column_it *jak_carbon_object_it_column_value(jak_carbon_object_it *it_in);

JAK_END_DECL

#endif
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

#ifndef JAK_CARBON_PATH_H
#define JAK_CARBON_PATH_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_carbon_dot.h>

JAK_BEGIN_DECL

typedef struct jak_carbon_path_evaluator {
        jak_carbon *doc;
        jak_carbon_array_it root_it;
        jak_carbon_path_status_e status;
        jak_error err;
        struct {
                jak_carbon_container_e container_type;
                union {
                        struct {
                                jak_carbon_array_it it;
                        } array;

                        struct {
                                jak_carbon_object_it it;
                        } object;

                        struct {
                                jak_carbon_column_it it;
                                jak_u32 elem_pos;
                        } column;

                } containers;
        } result;
} jak_carbon_path_evaluator;

bool jak_carbon_path_evaluator_begin(jak_carbon_path_evaluator *eval, jak_carbon_dot_path *path, jak_carbon *doc);
bool jak_carbon_path_evaluator_begin_mutable(jak_carbon_path_evaluator *eval, const jak_carbon_dot_path *path, jak_carbon_revise *context);
bool jak_carbon_path_evaluator_end(jak_carbon_path_evaluator *state);

bool jak_carbon_path_evaluator_status(jak_carbon_path_status_e *status, jak_carbon_path_evaluator *state);
bool jak_carbon_path_evaluator_has_result(jak_carbon_path_evaluator *state);
bool jak_carbon_path_exists(jak_carbon *doc, const char *path);

bool jak_carbon_path_is_array(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_column(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_object(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_container(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_null(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_number(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_boolean(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_string(jak_carbon *doc, const char *path);

JAK_END_DECL

#endif
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

#ifndef JAK_CARBON_PATH_INDEX_H
#define JAK_CARBON_PATH_INDEX_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_memfile.h>
#include <jak_memblock.h>
#include <jak_error.h>
#include <jak_carbon.h>
#include <jak_carbon_find.h>
#include <jak_carbon_dot.h>

JAK_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//  types
// ---------------------------------------------------------------------------------------------------------------------

typedef struct jak_carbon_path_index {
        jak_memblock *memblock;
        jak_memfile memfile;
        jak_error err;
} jak_carbon_path_index;

typedef struct jak_carbon_path_index_it {
        jak_carbon *doc;
        jak_memfile memfile;
        jak_error err;

        jak_carbon_container_e container_type;
        jak_u64 pos;
} jak_carbon_path_index_it;

typedef enum jak_path_index_node {
        JAK_PATH_ROOT, JAK_PATH_INDEX_PROP_KEY, JAK_PATH_INDEX_ARRAY_INDEX, JAK_PATH_INDEX_COLUMN_INDEX
} jak_path_index_node_e;

// ---------------------------------------------------------------------------------------------------------------------
//  construction and deconstruction
// ---------------------------------------------------------------------------------------------------------------------

bool jak_carbon_path_index_create(jak_carbon_path_index *index, jak_carbon *doc);
bool jak_carbon_path_index_drop(jak_carbon_path_index *index);

// ---------------------------------------------------------------------------------------------------------------------
//  index data access and meta information
// ---------------------------------------------------------------------------------------------------------------------

const void *jak_carbon_path_index_raw_data(jak_u64 *size, jak_carbon_path_index *index);
bool jak_carbon_path_index_commit_hash(jak_u64 *commit_hash, jak_carbon_path_index *index);
bool jak_carbon_path_index_key_type(jak_carbon_key_e *key_type, jak_carbon_path_index *index);
bool jak_carbon_path_index_key_unsigned_value(jak_u64 *key, jak_carbon_path_index *index);
bool jak_carbon_path_index_key_signed_value(jak_i64 *key, jak_carbon_path_index *index);
const char *jak_carbon_path_index_key_jak_string_value(jak_u64 *str_len, jak_carbon_path_index *index);
bool jak_carbon_path_index_indexes_doc(jak_carbon_path_index *index, jak_carbon *doc);

JAK_DEFINE_ERROR_GETTER(jak_carbon_path_index);

// ---------------------------------------------------------------------------------------------------------------------
//  index access and type information
// ---------------------------------------------------------------------------------------------------------------------

bool jak_carbon_path_index_it_open(jak_carbon_path_index_it *it, jak_carbon_path_index *index, jak_carbon *doc);
bool jak_carbon_path_index_it_type(jak_carbon_container_e *type, jak_carbon_path_index_it *it);

// ---------------------------------------------------------------------------------------------------------------------
//  array and column container functions
// ---------------------------------------------------------------------------------------------------------------------

bool jak_carbon_path_index_it_list_length(jak_u64 *key_len, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_list_goto(jak_u64 pos, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_list_pos(jak_u64 *pos, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_list_can_enter(jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_list_enter(jak_carbon_path_index_it *it);

// ---------------------------------------------------------------------------------------------------------------------
//  object container functions
// ---------------------------------------------------------------------------------------------------------------------

bool jak_carbon_path_index_it_obj_num_props(jak_u64 *num_props, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_obj_goto(const char *key_name, jak_carbon_path_index_it *it);
const char *jak_carbon_path_index_it_key_name(jak_u64 *name_len, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_obj_can_enter(jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_obj_enter(jak_carbon_path_index_it *it);

// ---------------------------------------------------------------------------------------------------------------------
//  field access
// ---------------------------------------------------------------------------------------------------------------------

bool jak_carbon_path_index_it_field_type(jak_carbon_field_type_e *type, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_u8_value(jak_u8 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_u16_value(jak_u16 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_u32_value(jak_u32 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_u64_value(jak_u64 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_i8_value(jak_i8 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_i16_value(jak_i16 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_i32_value(jak_i32 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_i64_value(jak_i64 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_float_value(bool *is_null_in, float *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_signed_value(bool *is_null_in, jak_i64 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_unsigned_value(bool *is_null_in, jak_u64 *value, jak_carbon_path_index_it *it);
const char *jak_carbon_path_index_it_field_jak_string_value(jak_u64 *strlen, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_binary_value(jak_carbon_binary *out, jak_carbon_array_it *it);
bool jak_carbon_path_index_it_field_array_value(jak_carbon_array_it *it_out, jak_carbon_path_index_it *it_in);
bool jak_carbon_path_index_it_field_object_value(jak_carbon_object_it *it_out, jak_carbon_path_index_it *it_in);
bool jak_carbon_path_index_it_field_column_value(jak_carbon_column_it *it_out, jak_carbon_path_index_it *it_in);

// ---------------------------------------------------------------------------------------------------------------------
//  diagnostics
// ---------------------------------------------------------------------------------------------------------------------

bool jak_carbon_path_index_hexdump(FILE *file, jak_carbon_path_index *index);
bool jak_carbon_path_index_to_carbon(jak_carbon *doc, jak_carbon_path_index *index);
const char *jak_carbon_path_index_to_str(jak_string *str, jak_carbon_path_index *index);
bool jak_carbon_path_index_print(FILE *file, jak_carbon_path_index *index);

JAK_END_DECL

#endif
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

#ifndef JAK_CARBON_PRINTERS_H
#define JAK_CARBON_PRINTERS_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_types.h>
#include <jak_string.h>
#include <jak_unique_id.h>

JAK_BEGIN_DECL

typedef struct jak_carbon_printer
{
        void *extra;

        void (*drop)(jak_carbon_printer *self);
        void (*record_begin)(jak_carbon_printer *self, jak_string *builder);
        void (*record_end)(jak_carbon_printer *self, jak_string *builder);
        void (*meta_begin)(jak_carbon_printer *self, jak_string *builder);
        /* type is of jak_carbon_key_e */
        void (*meta_data)(jak_carbon_printer *self, jak_string *builder, int key_type, const void *key, jak_u64 key_length, jak_u64 commit_hash);
        void (*meta_end)(jak_carbon_printer *self, jak_string *builder);
        void (*doc_begin)(jak_carbon_printer *self, jak_string *builder);
        void (*doc_end)(jak_carbon_printer *self, jak_string *builder);
        void (*empty_record)(jak_carbon_printer *self, jak_string *builder);
        void (*unit_array_begin)(jak_carbon_printer *self, jak_string *builder);
        void (*unit_array_end)(jak_carbon_printer *self, jak_string *builder);
        void (*array_begin)(jak_carbon_printer *self, jak_string *builder);
        void (*array_end)(jak_carbon_printer *self, jak_string *builder);
        void (*const_null)(jak_carbon_printer *self, jak_string *builder);
        void (*const_true)(jak_carbon_printer *self, bool is_null, jak_string *builder);
        void (*const_false)(jak_carbon_printer *self, bool is_null, jak_string *builder);
        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*val_signed)(jak_carbon_printer *self, jak_string *builder, const jak_i64 *value);
        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*val_unsigned)(jak_carbon_printer *self, jak_string *builder, const jak_u64 *value);
        /* if <code>value</code> is NULL, <code>value</code> is interpreted as null-value'd entry */
        void (*val_float)(jak_carbon_printer *self, jak_string *builder, const float *value);
        void (*val_string)(jak_carbon_printer *self, jak_string *builder, const char *value, jak_u64 strlen);
        void (*val_binary)(jak_carbon_printer *self, jak_string *builder, const jak_carbon_binary *binary);
        void (*comma)(jak_carbon_printer *self, jak_string *builder);
        void (*obj_begin)(jak_carbon_printer *self, jak_string *builder);
        void (*obj_end)(jak_carbon_printer *self, jak_string *builder);
        void (*prop_null)(jak_carbon_printer *self, jak_string *builder, const char *key_name, jak_u64 key_len);
        void (*prop_true)(jak_carbon_printer *self, jak_string *builder, const char *key_name, jak_u64 key_len);
        void (*prop_false)(jak_carbon_printer *self, jak_string *builder, const char *key_name, jak_u64 key_len);
        void (*prop_signed)(jak_carbon_printer *self, jak_string *builder, const char *key_name, jak_u64 key_len, const jak_i64 *value);
        void (*prop_unsigned)(jak_carbon_printer *self, jak_string *builder, const char *key_name, jak_u64 key_len, const jak_u64 *value);
        void (*prop_float)(jak_carbon_printer *self, jak_string *builder, const char *key_name, jak_u64 key_len, const float *value);
        void (*prop_string)(jak_carbon_printer *self, jak_string *builder, const char *key_name, jak_u64 key_len, const char *value, jak_u64 strlen);
        void (*prop_binary)(jak_carbon_printer *self, jak_string *builder, const char *key_name, jak_u64 key_len, const jak_carbon_binary *binary);
        void (*array_prop_name)(jak_carbon_printer *self, jak_string *builder, const char *key_name, jak_u64 key_len);
        void (*column_prop_name)(jak_carbon_printer *self, jak_string *builder, const char *key_name, jak_u64 key_len);
        void (*obj_prop_name)(jak_carbon_printer *self, jak_string *builder, const char *key_name, jak_u64 key_len);
} jak_carbon_printer;

/* 'impl' is of jak_carbon_printer_impl_e */
bool jak_carbon_printer_drop(jak_carbon_printer *printer);
bool jak_carbon_printer_by_type(jak_carbon_printer *printer, int impl);

bool jak_carbon_printer_begin(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_end(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_header_begin(jak_carbon_printer *printer, jak_string *str);
/* 'key_type' is of jak_carbon_key_e */
bool jak_carbon_printer_header_contents(jak_carbon_printer *printer, jak_string *str, int key_type, const void *key, jak_u64 key_length, jak_u64 rev);
bool jak_carbon_printer_header_end(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_payload_begin(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_payload_end(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_empty_record(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_array_begin(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_array_end(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_unit_array_begin(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_unit_array_end(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_object_begin(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_object_end(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_null(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_true(jak_carbon_printer *printer, bool is_null, jak_string *str);
bool jak_carbon_printer_false(jak_carbon_printer *printer, bool is_null, jak_string *str);
bool jak_carbon_printer_comma(jak_carbon_printer *printer, jak_string *str);
bool jak_carbon_printer_signed_nonull(jak_carbon_printer *printer, jak_string *str, const jak_i64 *value);
bool jak_carbon_printer_unsigned_nonull(jak_carbon_printer *printer, jak_string *str, const jak_u64 *value);
bool jak_carbon_printer_u8_or_null(jak_carbon_printer *printer, jak_string *str, jak_u8 value);
bool jak_carbon_printer_u16_or_null(jak_carbon_printer *printer, jak_string *str, jak_u16 value);
bool jak_carbon_printer_u32_or_null(jak_carbon_printer *printer, jak_string *str, jak_u32 value);
bool jak_carbon_printer_u64_or_null(jak_carbon_printer *printer, jak_string *str, jak_u64 value);
bool jak_carbon_printer_i8_or_null(jak_carbon_printer *printer, jak_string *str, jak_i8 value);
bool jak_carbon_printer_i16_or_null(jak_carbon_printer *printer, jak_string *str, jak_i16 value);
bool jak_carbon_printer_i32_or_null(jak_carbon_printer *printer, jak_string *str, jak_i32 value);
bool jak_carbon_printer_i64_or_null(jak_carbon_printer *printer, jak_string *str, jak_i64 value);
bool jak_carbon_printer_float(jak_carbon_printer *printer, jak_string *str, const float *value);
bool jak_carbon_printer_string(jak_carbon_printer *printer, jak_string *str, const char *value, jak_u64 strlen);
bool jak_carbon_printer_binary(jak_carbon_printer *printer, jak_string *str, const jak_carbon_binary *binary);
bool jak_carbon_printer_prop_null(jak_carbon_printer *printer, jak_string *str, const char *key_name, jak_u64 key_len);
bool jak_carbon_printer_prop_true(jak_carbon_printer *printer, jak_string *str, const char *key_name, jak_u64 key_len);
bool jak_carbon_printer_prop_false(jak_carbon_printer *printer, jak_string *str, const char *key_name, jak_u64 key_len);
bool jak_carbon_printer_prop_signed(jak_carbon_printer *printer, jak_string *str, const char *key_name, jak_u64 key_len, const jak_i64 *value);
bool jak_carbon_printer_prop_unsigned(jak_carbon_printer *printer, jak_string *str, const char *key_name, jak_u64 key_len, const jak_u64 *value);
bool jak_carbon_printer_prop_float(jak_carbon_printer *printer, jak_string *str, const char *key_name, jak_u64 key_len, const float *value);
bool jak_carbon_printer_prop_string(jak_carbon_printer *printer, jak_string *str, const char *key_name, jak_u64 key_len, const char *value, jak_u64 strlen);
bool jak_carbon_printer_prop_binary(jak_carbon_printer *printer, jak_string *str, const char *key_name, jak_u64 key_len, const jak_carbon_binary *binary);
bool jak_carbon_printer_array_prop_name(jak_carbon_printer *printer, jak_string *str, const char *key_name, jak_u64 key_len);
bool jak_carbon_printer_column_prop_name(jak_carbon_printer *printer, jak_string *str, const char *key_name, jak_u64 key_len);
bool jak_carbon_printer_object_prop_name(jak_carbon_printer *printer, jak_string *str, const char *key_name, jak_u64 key_len);
bool jak_carbon_printer_print_object(jak_carbon_object_it *it, jak_carbon_printer *printer, jak_string *builder);
bool jak_carbon_printer_print_array(jak_carbon_array_it *it, jak_carbon_printer *printer, jak_string *builder, bool is_record_container);
bool jak_carbon_printer_print_column(jak_carbon_column_it *it, jak_carbon_printer *printer, jak_string *builder);

JAK_END_DECL

#endif
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

#ifndef JAK_CARBON_PROP_H
#define JAK_CARBON_PROP_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_carbon_field.h>

JAK_BEGIN_DECL

jak_u64 jak_carbon_prop_size(jak_memfile *file);

JAK_END_DECL

#endif
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

#ifndef JAK_CARBON_REVISE_H
#define JAK_CARBON_REVISE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_unique_id.h>
#include <jak_carbon.h>

JAK_BEGIN_DECL

JAK_DEFINE_ERROR_GETTER(jak_carbon_revise)

/**
 * Acquires a new revision context for the carbon document.
 *
 * In case of an already running revision, the function returns <code>false</code> without blocking.
 * Otherwise, <code>jak_carbon_revise_begin</code> is called internally.
 *
 * @param context non-null pointer to revision context
 * @param doc document that should be revised
 * @return <code>false</code> in case of an already running revision. Otherwise returns value of
 *                            <code>jak_carbon_revise_begin</code>
 */
bool jak_carbon_revise_try_begin(jak_carbon_revise *context, jak_carbon *revised_doc, jak_carbon *doc);
bool jak_carbon_revise_begin(jak_carbon_revise *context, jak_carbon *revised_doc, jak_carbon *original);
const jak_carbon *jak_carbon_revise_end(jak_carbon_revise *context);

bool jak_carbon_revise_key_generate(jak_uid_t *out, jak_carbon_revise *context);

bool jak_carbon_revise_key_set_unsigned(jak_carbon_revise *context, jak_u64 key_value);
bool jak_carbon_revise_key_set_signed(jak_carbon_revise *context, jak_i64 key_value);
bool jak_carbon_revise_key_set_string(jak_carbon_revise *context, const char *key_value);

bool jak_carbon_revise_iterator_open(jak_carbon_array_it *it, jak_carbon_revise *context);
bool jak_carbon_revise_iterator_close(jak_carbon_array_it *it);

bool jak_carbon_revise_find_open(jak_carbon_find *out, const char *dot_path, jak_carbon_revise *context);
bool jak_carbon_revise_find_close(jak_carbon_find *find);

bool jak_carbon_revise_remove(const char *dot_path, jak_carbon_revise *context);
bool jak_carbon_revise_remove_one(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc);

bool jak_carbon_revise_pack(jak_carbon_revise *context);
bool jak_carbon_revise_shrink(jak_carbon_revise *context);

bool jak_carbon_revise_abort(jak_carbon_revise *context);

JAK_END_DECL

#endif
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

#ifndef JAK_CARBON_STRING_H
#define JAK_CARBON_STRING_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_memfile.h>

JAK_BEGIN_DECL

bool jak_carbon_jak_string_write(jak_memfile *file, const char *string);
bool jak_carbon_jak_string_nchar_write(jak_memfile *file, const char *string, jak_u64 str_len);
bool jak_carbon_jak_string_nomarker_write(jak_memfile *file, const char *string);
bool jak_carbon_jak_string_nomarker_nchar_write(jak_memfile *file, const char *string, jak_u64 str_len);
bool jak_carbon_jak_string_nomarker_remove(jak_memfile *file);
bool jak_carbon_jak_string_remove(jak_memfile *file);
bool jak_carbon_jak_string_update(jak_memfile *file, const char *string);
bool jak_carbon_jak_string_update_wnchar(jak_memfile *file, const char *string, size_t str_len);
bool jak_carbon_jak_string_skip(jak_memfile *file);
bool jak_carbon_jak_string_nomarker_skip(jak_memfile *file);
const char *jak_carbon_jak_string_read(jak_u64 *len, jak_memfile *file);
const char *jak_carbon_jak_string_nomarker_read(jak_u64 *len, jak_memfile *file);

JAK_END_DECL

#endif
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

#ifndef JAK_CARBON_UPDATE_H
#define JAK_CARBON_UPDATE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_memblock.h>
#include <jak_memfile.h>
#include <jak_spinlock.h>
#include <jak_carbon.h>
#include <jak_carbon_dot.h>
#include <jak_carbon_path.h>
#include <jak_carbon_int.h>

JAK_BEGIN_DECL

typedef struct jak_carbon_update {
        jak_carbon_revise *context;
        jak_carbon_path_evaluator path_evaluater;
        const jak_carbon_dot_path *path;
        jak_error err;
        bool is_found;
} jak_carbon_update;

JAK_DEFINE_ERROR_GETTER(jak_carbon_update)

bool jak_carbon_update_set_null(jak_carbon_revise *context, const char *path);
bool jak_carbon_update_set_true(jak_carbon_revise *context, const char *path);
bool jak_carbon_update_set_false(jak_carbon_revise *context, const char *path);
bool jak_carbon_update_set_u8(jak_carbon_revise *context, const char *path, jak_u8 value);
bool jak_carbon_update_set_u16(jak_carbon_revise *context, const char *path, jak_u16 value);
bool jak_carbon_update_set_u32(jak_carbon_revise *context, const char *path, jak_u32 value);
bool jak_carbon_update_set_u64(jak_carbon_revise *context, const char *path, jak_u64 value);
bool jak_carbon_update_set_i8(jak_carbon_revise *context, const char *path, jak_i8 value);
bool jak_carbon_update_set_i16(jak_carbon_revise *context, const char *path, jak_i16 value);
bool jak_carbon_update_set_i32(jak_carbon_revise *context, const char *path, jak_i32 value);
bool jak_carbon_update_set_i64(jak_carbon_revise *context, const char *path, jak_i64 value);
bool jak_carbon_update_set_float(jak_carbon_revise *context, const char *path, float value);
bool jak_carbon_update_set_unsigned(jak_carbon_revise *context, const char *path, jak_u64 value);
bool jak_carbon_update_set_signed(jak_carbon_revise *context, const char *path, jak_i64 value);
bool jak_carbon_update_set_string(jak_carbon_revise *context, const char *path, const char *value);
bool jak_carbon_update_set_binary(jak_carbon_revise *context, const char *path, const void *value, size_t nbytes, const char *file_ext, const char *user_type);

jak_carbon_insert *jak_carbon_update_set_array_begin(jak_carbon_revise *context, const char *path, jak_carbon_insert_array_state *state_out, jak_u64 array_capacity);
bool jak_carbon_update_set_array_end(jak_carbon_insert_array_state *state_in);

jak_carbon_insert *jak_carbon_update_set_column_begin(jak_carbon_revise *context, const char *path, jak_carbon_insert_column_state *state_out, jak_carbon_field_type_e type, jak_u64 column_capacity);
bool jak_carbon_update_set_column_end(jak_carbon_insert_column_state *state_in);

bool jak_carbon_update_set_null_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path);
bool jak_carbon_update_set_true_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path);
bool jak_carbon_update_set_false_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path);

bool jak_carbon_update_set_u8_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_u8 value);
bool jak_carbon_update_set_u16_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_u16 value);
bool jak_carbon_update_set_u32_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_u32 value);
bool jak_carbon_update_set_u64_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_u64 value);
bool jak_carbon_update_set_i8_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_i8 value);
bool jak_carbon_update_set_i16_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_i16 value);
bool jak_carbon_update_set_i32_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_i32 value);
bool jak_carbon_update_set_i64_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_i64 value);
bool jak_carbon_update_set_float_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, float value);
bool jak_carbon_update_set_unsigned_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_u64 value);
bool jak_carbon_update_set_signed_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_i64 value);
bool jak_carbon_update_set_jak_string_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, const char *value);
bool jak_carbon_update_set_binary_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, const void *value, size_t nbytes, const char *file_ext, const char *user_type);
jak_carbon_insert * jak_carbon_update_set_array_begin_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_carbon_insert_array_state *state_out, jak_u64 array_capacity);
bool jak_carbon_update_set_array_end_compiled(jak_carbon_insert_array_state *state_in);
jak_carbon_insert *jak_carbon_update_set_column_begin_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_carbon_insert_column_state *state_out, jak_carbon_field_type_e type, jak_u64 column_capacity);
bool jak_carbon_update_set_column_end_compiled(jak_carbon_insert_column_state *state_in);

bool carbon_update_one_set_null(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc);
bool carbon_update_one_set_true(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc);
bool carbon_update_one_set_false(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc);
bool carbon_update_one_set_u8(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_u8 value);
bool carbon_update_one_set_u16(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_u16 value);
bool carbon_update_one_set_u32(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_u32 value);
bool carbon_update_one_set_u64(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_u64 value);
bool carbon_update_one_set_i8(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_i8 value);
bool carbon_update_one_set_i16(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_i16 value);
bool carbon_update_one_set_i32(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_i32 value);
bool carbon_update_one_set_i64(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_i64 value);
bool carbon_update_one_set_float(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, float value);
bool carbon_update_one_set_unsigned(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_u64 value);
bool carbon_update_one_set_signed(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_i64 value);
bool carbon_update_one_set_string(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, const char *value);
bool carbon_update_one_set_binary(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, const void *value, size_t nbytes, const char *file_ext, const char *user_type);
jak_carbon_insert *carbon_update_one_set_array_begin(jak_carbon_insert_array_state *state_out, const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_u64 array_capacity);
bool carbon_update_one_set_array_end(jak_carbon_insert_array_state *state_in);

jak_carbon_insert *carbon_update_one_set_column_begin(jak_carbon_insert_column_state *state_out, const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_carbon_field_type_e type, jak_u64 column_capacity);
bool carbon_update_one_set_column_end(jak_carbon_insert_column_state *state_in);

bool carbon_update_one_set_null_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc);
bool carbon_update_one_set_true_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc);
bool carbon_update_one_set_false_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc);
bool carbon_update_one_set_u8_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_u8 value);
bool carbon_update_one_set_u16_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_u16 value);
bool carbon_update_one_set_u32_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_u32 value);
bool carbon_update_one_set_u64_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_u64 value);
bool carbon_update_one_set_i8_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_i8 value);
bool carbon_update_one_set_i16_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_i16 value);
bool carbon_update_one_set_i32_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_i32 value);
bool carbon_update_one_set_i64_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_i64 value);
bool carbon_update_one_set_float_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, float value);
bool carbon_update_one_set_unsigned_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_u64 value);
bool carbon_update_one_set_signed_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_i64 value);
bool carbon_update_one_set_jak_string_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, const char *value);
bool carbon_update_one_set_binary_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, const void *value, size_t nbytes, const char *file_ext, const char *user_type);
jak_carbon_insert *carbon_update_one_set_array_begin_compiled(jak_carbon_insert_array_state *state_out, const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_u64 array_capacity);
bool carbon_update_one_set_array_end_compiled(jak_carbon_insert_array_state *state_in);

jak_carbon_insert *carbon_update_one_set_column_begin_compiled(jak_carbon_insert_column_state *state_out, const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_carbon_field_type_e type, jak_u64 column_capacity);
bool carbon_update_one_set_column_end_compiled(jak_carbon_insert_column_state *state_in);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_COLUMNDOC_H
#define JAK_COLUMNDOC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>
#include <jak_bloom.h>
#include <jak_string_dict.h>
#include <jak_doc.h>

JAK_BEGIN_DECL

/**
 * Transformation of an JSON-like array of objects to a columnar representation of key values.
 *
 * The assumption is that an array of objects is JAK_LIKELY to have elements of the same (well yet unknown) schema. The
 * following structure captures a particular key (with its data type) found in one or more elements inside this array,
 * and stores the value assigned to this key along with with the object position in the array in which this mapping
 * occurs. Note that a keys name might have different data types inside objects embedded in arrays, and a key is not
 * guaranteed to occur in all objects embedded in the array.
 */
typedef struct jak_column_doc_column {
        /** Key name */
        jak_archive_field_sid_t key_name;
        /** Particular key type */
        jak_archive_field_e type;
        /** Positions of objects in the parent array that has this particular key name with this particular value type */
        jak_vector ofType(jak_u32) array_positions;
        /** Values stored in objects assigned to this key-type mapping. The i-th element in `values` (which hold the
         * i-th value) is associated to the i-th element in `arrayPosition` which holds the position of the object inside
         * the array from which this pair was taken. */
        jak_vector ofType(Vector ofType( < T >)) values;
} jak_column_doc_column;

typedef struct jak_column_doc_group {
        /** Key name */
        jak_archive_field_sid_t key;
        /** Key columns as a decomposition of objects stored in that JSON-like array */
        jak_vector ofType(jak_column_doc_column) columns;
} jak_column_doc_group;

typedef struct jak_column_doc_obj {
        /** Parent document meta doc */
        jak_column_doc *parent;
        /** Key in parent document meta doc that maps to this one, or "/" if this is the top-level meta doc */
        jak_archive_field_sid_t parent_key;
        /** Index inside the array of this doc in its parents property, or 0 if this is not an array type or top-level */
        size_t index;

        /** Inverted index of keys mapping to primitive boolean types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) bool_prop_keys;
        /** Inverted index of keys mapping to primitive int8 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) int8_prop_keys;
        /** Inverted index of keys mapping to primitive int16 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) int16_prop_keys;
        /** Inverted index of keys mapping to primitive int32 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) int32_prop_keys;
        /** Inverted index of keys mapping to primitive int64 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) int64_prop_keys;
        /** Inverted index of keys mapping to primitive uint8 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) uint8_prop_keys;
        /** Inverted index of keys mapping to primitive uint16 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) uint16_prop_keys;
        /** Inverted index of keys mapping to primitive uint32 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) uin32_prop_keys;
        /** Inverted index of keys mapping to primitive uint64 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) uint64_prop_keys;
        /** Inverted index of keys mapping to primitive string types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) jak_string_prop_keys;
        /** Inverted index of keys mapping to primitive real types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) float_prop_keys;
        /** Inverted index of keys mapping to primitive null values (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) null_prop_keys;
        /** Inverted index of keys mapping to exactly one nested object value (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) obj_prop_keys;

        /** Inverted index of keys mapping to array of boolean types (sorted by key)*/
        jak_vector ofType(jak_archive_field_sid_t) bool_array_prop_keys;
        /** Inverted index of keys mapping to array of int8 number types (sorted by key)*/
        jak_vector ofType(jak_archive_field_sid_t) int8_array_prop_keys;
        /** Inverted index of keys mapping to array of int16 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) int16_array_prop_keys;
        /** Inverted index of keys mapping to array of int32 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) int32_array_prop_keys;
        /** Inverted index of keys mapping to array of int64 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) int64_array_prop_keys;
        /** Inverted index of keys mapping to array of uint8 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) uint8_array_prop_keys;
        /** Inverted index of keys mapping to array of uint16 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) uint16_array_prop_keys;
        /** Inverted index of keys mapping to array of uint32 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) uint32_array_prop_keys;
        /** Inverted index of keys mapping to array of uint64 number types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) uint64_array_prop_keys;
        /** Inverted index of keys mapping array of string types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) jak_string_array_prop_keys;
        /** Inverted index of keys mapping array of real types (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) float_array_prop_keys;
        /** Inverted index of keys mapping array of null value (sorted by key)s */
        jak_vector ofType(jak_archive_field_sid_t) null_array_prop_keys;

        /** Primitive boolean values associated to keys stored above (sorted by key) */
        jak_vector ofType(JAK_FIELD_BOOLEANean_t) bool_prop_vals;
        /** Primitive int8 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(jak_archive_field_i8_t) int8_prop_vals;
        /** Primitive int16 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(jak_archive_field_i16_t) int16_prop_vals;
        /** Primitive int32 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(jak_archive_field_i32_t) int32_prop_vals;
        /** Primitive int64 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(jak_archive_field_i64_t) int64_prop_vals;
        /** Primitive uint8 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(jak_archive_field_u8_t) uint8_prop_vals;
        /** Primitive uint16 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(jak_archive_field_u16_t) uint16_prop_vals;
        /** Primitive uint32 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(jak_archive_field_u32_t) uint32_prop_vals;
        /** Primitive uint64 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(jak_archive_field_u64_t) uint64_prop_vals;
        /** Primitive real number values associated to keys stored above (sorted by key) */
        jak_vector ofType(jak_archive_field_number_t) float_prop_vals;
        /** Primitive string number values associated to keys stored above (sorted by key) */
        jak_vector ofType(jak_archive_field_sid_t) jak_string_prop_vals;

        /** Array of boolean values associated to keys stored above (sorted by key) */
        jak_vector ofType(Vector) bool_array_prop_vals;
        /** Array of int8 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(Vector) int8_array_prop_vals;
        /** Array of int16 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(Vector) int16_array_prop_vals;
        /** Array of int32 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(Vector) int32_array_prop_vals;
        /** Array of int64 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(Vector) int64_array_prop_vals;
        /** Array of uint8 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(Vector) uint8_array_prop_vals;
        /** Array of uint16 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(Vector) uint16_array_prop_vals;
        /** Array of uint32 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(Vector) uint32_array_prop_vals;
        /** Array of uint64 number values associated to keys stored above (sorted by key) */
        jak_vector ofType(Vector) ui64_array_prop_vals;
        /** Array of real number values associated to keys stored above (sorted by key) */
        jak_vector ofType(Vector) float_array_prop_vals;
        /** Array of string number values associated to keys stored above (sorted by key) */
        jak_vector ofType(Vector) jak_string_array_prop_vals;
        /** Array of null values associated to keys stored above (sorted by key). The number represents the
         * multiplicity of nulls for the associated key. */
        jak_vector ofType(jak_u16) null_array_prop_vals;
        /** Primitive objects associated to keys stored above (sorted by key) */
        jak_vector ofType(jak_column_doc_obj) obj_prop_vals;

        /** Index of primitive boolean values associated to keys stored above (sorted by value) */
        jak_vector ofType(jak_u32) bool_val_idxs;
        /** Index of primitive int8 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(jak_u32) int8_val_idxs;
        /** Index of primitive int16 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(jak_u32) int16_val_idxs;
        /** Index of primitive int32 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(jak_u32) int32_val_idxs;
        /** Index of primitive int64 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(jak_u32) int64_val_idxs;
        /** Index of primitive uint8 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(jak_u32) uint8_val_idxs;
        /** Index of primitive uint16 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(jak_u32) uint16_val_idxs;
        /** Index of primitive uint32 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(jak_u32) uint32_val_idxs;
        /** Index of primitive uint64 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(jak_u32) uint64_val_idxs;
        /** Index of primitive real number values associated to keys stored above (sorted by value) */
        jak_vector ofType(jak_u32) float_val_idxs;
        /** Index of primitive string number values associated to keys stored above (sorted by value) */
        jak_vector ofType(jak_u32) jak_string_val_idxs;

        /** Index of array of boolean values associated to keys stored above (sorted by value) */
        jak_vector ofType(Vector) bool_array_idxs;
        /** Index of array of int8 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(Vector) int8_array_idxs;
        /** Index of array of int16 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(Vector) int16_array_idxs;
        /** Index of array of int32 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(Vector) int32_array_idxs;
        /** Index of array of int64 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(Vector) int64_array_idxs;
        /** Index of array of uint8 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(Vector) uint8_array_idxs;
        /** Index of array of uint16 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(Vector) uint16_array_idxs;
        /** Index of array of uint32 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(Vector) uint32_array_idxs;
        /** Index of array of uint64 number values associated to keys stored above (sorted by value) */
        jak_vector ofType(Vector) uint64_array_idxs;
        /** Index of array of real number values associated to keys stored above (sorted by value) */
        jak_vector ofType(Vector) float_array_idxs;
        /** Index of array of string number values associated to keys stored above (sorted by value) */
        jak_vector ofType(Vector) jak_string_array_idxs;

        /** Array of objects associated to keys stored above (sorted by key) */
        jak_vector ofType(jak_column_doc_group) obj_array_props;
} jak_column_doc_obj;

typedef struct jak_column_doc {
        const jak_doc *doc;
        jak_string_dict *dic;
        jak_column_doc_obj columndoc;
        const jak_doc_bulk *bulk;
        bool read_optimized;
        jak_error err;
} jak_column_doc;

JAK_DEFINE_GET_ERROR_FUNCTION(columndoc, jak_column_doc, doc)

bool jak_columndoc_create(jak_column_doc *columndoc, jak_error *err, const jak_doc *doc,  const jak_doc_bulk *bulk, const jak_doc_entries *entries, jak_string_dict *dic);
bool jak_columndoc_drop(jak_column_doc *doc);

bool jak_columndoc_free(jak_column_doc *doc);

bool jak_columndoc_print(FILE *file, jak_column_doc *doc);

JAK_END_DECL

#endif/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_DOC_H
#define JAK_DOC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>
#include <jak_string_dict.h>
#include <jak_json.h>

JAK_BEGIN_DECL

typedef struct jak_doc_entries {
        jak_doc_obj *context;
        const char *key;
        jak_archive_field_e type;
        jak_vector ofType(<T>) values;
} jak_doc_entries;

typedef struct jak_doc_bulk {
        jak_string_dict *dic;
        jak_vector ofType(char *) keys, values;
        jak_vector ofType(jak_doc) models;
} jak_doc_bulk;

typedef struct jak_doc {
        jak_doc_bulk *context;
        jak_vector ofType(jak_doc_obj) obj_model;
        jak_archive_field_e type;
} jak_doc;

typedef struct jak_doc_obj {
        jak_vector ofType(jak_doc_entries) entries;
        jak_doc *doc;
} jak_doc_obj;

bool jak_doc_bulk_create(jak_doc_bulk *bulk, jak_string_dict *dic);
bool jak_doc_bulk_drop(jak_doc_bulk *bulk);

bool jak_doc_bulk_shrink(jak_doc_bulk *bulk);
bool jak_doc_bulk_print(FILE *file, jak_doc_bulk *bulk);

jak_doc *jak_doc_bulk_new_doc(jak_doc_bulk *context, jak_archive_field_e type);
jak_doc_obj *jak_doc_bulk_new_obj(jak_doc *model);
bool jak_doc_bulk_get_dic_contents(jak_vector ofType (const char *) **strings, jak_vector ofType(jak_archive_field_sid_t) **jak_string_ids, const jak_doc_bulk *context);

bool jak_doc_print(FILE *file, const jak_doc *doc);
const jak_vector ofType(jak_doc_entries) *jak_doc_get_entries(const jak_doc_obj *model);
void jak_doc_print_entries(FILE *file, const jak_doc_entries *entries);
void jak_doc_drop(jak_doc_obj *model);

bool jak_doc_obj_add_key(jak_doc_entries **out, jak_doc_obj *obj, const char *key, jak_archive_field_e type);
bool jak_doc_obj_push_primtive(jak_doc_entries *entry, const void *value);
bool jak_doc_obj_push_object(jak_doc_obj **out, jak_doc_entries *entry);

jak_doc_entries *jak_doc_bulk_new_entries(jak_doc_bulk *dst);
jak_doc_obj *jak_doc_bulk_add_json(jak_doc_entries *partition, jak_json *jak_json);
jak_doc_obj *jak_doc_entries_get_root(const jak_doc_entries *partition);
jak_column_doc *jak_doc_entries_columndoc(const jak_doc_bulk *bulk, const jak_doc_entries *partition, bool read_optimized);
bool jak_doc_entries_drop(jak_doc_entries *partition);

JAK_END_DECL

#endif/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_STRDIC_ASYNC_H
#define JAK_STRDIC_ASYNC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_string_dict.h>

JAK_BEGIN_DECL

int jak_encode_async_create(jak_string_dict *dic, size_t capacity, size_t num_index_buckets, size_t approx_num_unique_strs, size_t num_threads, const jak_allocator *alloc);

JAK_END_DECL

#endif/**
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

#ifndef JAK_ENCODED_DOC_H
#define JAK_ENCODED_DOC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_hash_table.h>
#include <jak_unique_id.h>
#include <jak_types.h>
#include <jak_archive.h>

JAK_BEGIN_DECL

typedef union jak_encoded_doc_value {
        jak_archive_field_i8_t int8;
        jak_archive_field_i16_t int16;
        jak_archive_field_i32_t int32;
        jak_archive_field_i64_t int64;
        jak_archive_field_u8_t uint8;
        jak_archive_field_u16_t uint16;
        jak_archive_field_u32_t uint32;
        jak_archive_field_u64_t uint64;
        jak_archive_field_number_t number;
        jak_archive_field_boolean_t boolean;
        jak_archive_field_sid_t string;
        jak_uid_t object;
        jak_u32 null;
} jak_encoded_doc_value_u;

typedef enum jak_encoded_doc_string {
        JAK_STRING_ENCODED, JAK_STRING_DECODED,
} jak_encoded_doc_jak_string_e;

typedef enum jak_encoded_doc_value_e {
        JAK_VALUE_BUILTIN, JAK_VALUE_DECODED_STRING,
} jak_encoded_doc_value_e;

typedef struct jak_encoded_doc_prop_header {
        jak_encoded_doc *context;

        jak_encoded_doc_jak_string_e key_type;
        union {
                jak_archive_field_sid_t key_id;
                char *key_str;
        } key;

        jak_encoded_doc_value_e value_type;
        enum jak_archive_field_type type;
} jak_encoded_doc_prop_header;

typedef struct jak_encoded_doc_prop {
        jak_encoded_doc_prop_header header;
        union {
                jak_encoded_doc_value_u builtin;
                char *string;
        } value;
} jak_encoded_doc_prop;

typedef struct jak_encoded_doc_prop_array {
        jak_encoded_doc_prop_header header;
        jak_vector ofType(jak_encoded_doc_value_u) values;
} jak_encoded_doc_prop_array;

typedef struct jak_encoded_doc {
        jak_encoded_doc_list *context;
        jak_uid_t object_id;
        jak_vector ofType(jak_encoded_doc_prop) props;
        jak_vector ofType(jak_encoded_doc_prop_array) props_arrays;
        jak_hashtable ofMapping(jak_archive_field_sid_t,
                                   jak_u32) prop_array_index; /* maps key to index in prop arrays */
        jak_error err;
} jak_encoded_doc;

typedef struct jak_encoded_doc_list {
        jak_archive *archive;
        jak_vector ofType(
                jak_encoded_doc) flat_object_collection;   /* list of objects; also nested ones */
        jak_hashtable ofMapping(object_id_t, jak_u32) index;   /* maps oid to index in collection */
        jak_error err;
} jak_encoded_doc_list;

bool jak_encoded_doc_collection_create(jak_encoded_doc_list *collection, jak_error *err, jak_archive *archive);
bool jak_encoded_doc_collection_drop(jak_encoded_doc_list *collection);
jak_encoded_doc *jak_encoded_doc_collection_get_or_append(jak_encoded_doc_list *collection, jak_uid_t id);
bool jak_encoded_doc_collection_print(FILE *file, jak_encoded_doc_list *collection);

bool jak_encoded_doc_drop(jak_encoded_doc *doc);

#define JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(name, built_in_type)                                                  \
bool jak_encoded_doc_add_prop_##name(jak_encoded_doc *doc, jak_archive_field_sid_t key, built_in_type value);

#define JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(name, built_in_type)                                          \
bool jak_encoded_doc_add_prop_##name##_decoded(jak_encoded_doc *doc, const char *key, built_in_type value);

JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(int8, jak_archive_field_i8_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(int16, jak_archive_field_i16_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(int32, jak_archive_field_i32_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(int64, jak_archive_field_i64_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(uint8, jak_archive_field_u8_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(uint16, jak_archive_field_u16_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(uint32, jak_archive_field_u32_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(uint64, jak_archive_field_u64_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(number, jak_archive_field_number_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(boolean, jak_archive_field_boolean_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(string, jak_archive_field_sid_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int8, jak_archive_field_i8_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int16, jak_archive_field_i16_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int32, jak_archive_field_i32_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int64, jak_archive_field_i64_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint8, jak_archive_field_u8_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint16, jak_archive_field_u16_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint32, jak_archive_field_u32_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint64, jak_archive_field_u64_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(number, jak_archive_field_number_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(boolean, jak_archive_field_boolean_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(string, jak_archive_field_sid_t)

bool jak_encoded_doc_add_prop_jak_string_decoded_jak_string_value_decoded(jak_encoded_doc *doc, const char *key, const char *value);
bool jak_encoded_doc_add_prop_null(jak_encoded_doc *doc, jak_archive_field_sid_t key);
bool jak_encoded_doc_add_prop_null_decoded(jak_encoded_doc *doc, const char *key);
bool jak_encoded_doc_add_prop_object(jak_encoded_doc *doc, jak_archive_field_sid_t key, jak_encoded_doc *value);
bool jak_encoded_doc_add_prop_object_decoded(jak_encoded_doc *doc, const char *key, jak_encoded_doc *value);

#define JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(name)                                                            \
bool jak_encoded_doc_add_prop_array_##name(jak_encoded_doc *doc, jak_archive_field_sid_t key);

#define JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(name)                                                    \
bool jak_encoded_doc_add_prop_array_##name##_decoded(jak_encoded_doc *doc, const char *key);

JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int8)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int16)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int32)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int64)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint8)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint16)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint32)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint64)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(number)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(boolean)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(string)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(null)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(object)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int8)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int16)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int32)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int64)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint8)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint16)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint32)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint64)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(number)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(boolean)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(string)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(null)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(object)

#define JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(name, built_in_type)                                                    \
bool jak_encoded_doc_array_push_##name(jak_encoded_doc *doc, jak_archive_field_sid_t key, const built_in_type *array, jak_u32 array_length);

#define JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(name, built_in_type)                                            \
bool jak_encoded_doc_array_push_##name##_decoded(jak_encoded_doc *doc, const char *key, const built_in_type *array, jak_u32 array_length);

JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(int8, jak_archive_field_i8_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(int16, jak_archive_field_i16_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(int32, jak_archive_field_i32_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(int64, jak_archive_field_i64_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint8, jak_archive_field_u8_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint16, jak_archive_field_u16_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint32, jak_archive_field_u32_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint64, jak_archive_field_u64_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(number, jak_archive_field_number_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(boolean, jak_archive_field_boolean_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(string, jak_archive_field_sid_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(null, jak_archive_field_u32_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int8, jak_archive_field_i8_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int16, jak_archive_field_i16_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int32, jak_archive_field_i32_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int64, jak_archive_field_i64_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint8, jak_archive_field_u8_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint16, jak_archive_field_u16_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint32, jak_archive_field_u32_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint64, jak_archive_field_u64_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(number, jak_archive_field_number_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(boolean, jak_archive_field_boolean_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(string, jak_archive_field_sid_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(null, jak_archive_field_u32_t)

bool jak_encoded_doc_array_push_object(jak_encoded_doc *doc, jak_archive_field_sid_t key, jak_uid_t id);
bool jak_encoded_doc_array_push_object_decoded(jak_encoded_doc *doc, const char *key, jak_uid_t id);
bool jak_encoded_doc_print(FILE *file, jak_encoded_doc *doc);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_STRDIC_SYNC_H
#define JAK_STRDIC_SYNC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_string_dict.h>

JAK_BEGIN_DECL

int jak_encode_sync_create(jak_string_dict *dic, size_t capacity, size_t num_indx_buckets, size_t num_index_bucket_cap, size_t num_threads, const jak_allocator *alloc);

JAK_END_DECL

#endif/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_ERROR_H
#define JAK_ERROR_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <stdint.h>

#include <jak_stdinc.h>
#include <jak_types.h>

JAK_BEGIN_DECL

#define JAK_ERR_NOERR 0                    /** No JAK_ERROR */
#define JAK_ERR_NULLPTR 1                  /** Null pointer detected */
#define JAK_ERR_NOTIMPL 2                  /** Function not implemented */
#define JAK_ERR_OUTOFBOUNDS 3              /** Index is out of bounds */
#define JAK_ERR_MALLOCERR 4                /** Memory allocation failed */
#define JAK_ERR_ILLEGALARG 5               /** Illegal arguments */
#define JAK_ERR_INTERNALERR 6              /** Internal JAK_ERROR */
#define JAK_ERR_ILLEGALIMPL 7              /** Illegal implementation */
#define JAK_ERR_NOTFOUND 8                 /** Not found */
#define JAK_ERR_NIL 9                      /** Element not in list */
#define JAK_ERR_ARRAYOFARRAYS 10           /** Array index out of bounds */
#define JAK_ERR_ARRAYOFMIXEDTYPES 11       /** Illegal JSON array: mixed types */
#define JAK_ERR_FOPEN_FAILED 12            /** Reading from file failed */
#define JAK_ERR_IO 13                      /** I/O JAK_ERROR */
#define JAK_ERR_FORMATVERERR 14            /** Unsupported archive format version */
#define JAK_ERR_CORRUPTED 15               /** Format is corrupted */
#define JAK_ERR_NOCARBONSTREAM 16          /** Stream is not a carbon archive */
#define JAK_ERR_NOBITMODE 17               /** Not in bit writing mode */
#define JAK_ERR_NOTIMPLEMENTED 18          /** Function is not yet implemented */
#define JAK_ERR_NOTYPE 19                  /** Unsupported type found */
#define JAK_ERR_NOCOMPRESSOR 20            /** Unsupported compressor strategy requested */
#define JAK_ERR_NOVALUESTR 21              /** No string representation for type available */
#define JAK_ERR_MARKERMAPPING 22           /** Marker type cannot be mapped to value type */
#define JAK_ERR_PARSETYPE 23               /** Parsing stopped; unknown data type requested */
#define JAK_ERR_NOJSONTOKEN 24             /** Unknown token during parsing JSON detected */
#define JAK_ERR_NOJSONNUMBERT 25           /** Unknown value type for number in JSON property */
#define JAK_ERR_NOARCHIVEFILE 26           /** Stream is not a valid archive file */
#define JAK_ERR_UNSUPFINDSTRAT 27          /** Unsupported strategy requested for key lookup */
#define JAK_ERR_ERRINTERNAL 28             /** Internal JAK_ERROR */
#define JAK_ERR_HUFFERR 29                 /** No huffman code table entry found for character */
#define JAK_ERR_MEMSTATE 30                /** Memory file was opened as read-only but requested a modification */
#define JAK_ERR_JSONTYPE 31                /** Unable to import jak_json file: unsupported type */
#define JAK_ERR_WRITEPROT 32               /** Mode set to read-only but modification was requested */
#define JAK_ERR_READOUTOFBOUNDS 33         /** Read outside of memory range bounds */
#define JAK_ERR_SLOTBROKEN 34              /** Slot management broken */
#define JAK_ERR_THREADOOOBJIDS 35          /** Thread run out of object ids: start another one */
#define JAK_ERR_JSONPARSEERR 36            /** JSON parsing JAK_ERROR */
#define JAK_ERR_BULKCREATEFAILED 37        /** Document insertion bulk creation failed */
#define JAK_ERR_FOPENWRITE 38              /** File cannot be opened for writing */
#define JAK_ERR_WRITEARCHIVE 39            /** Archive cannot be serialized into file */
#define JAK_ERR_ARCHIVEOPEN 40             /** Archive cannot be deserialized form file */
#define JAK_ERR_FREAD_FAILED 41            /** Unable to read from file */
#define JAK_ERR_SCAN_FAILED 42             /** Unable to perform full scan in archive file */
#define JAK_ERR_DECOMPRESSFAILED 43        /** String decompression from archive failed */
#define JAK_ERR_ITERATORNOTCLOSED 44       /** Closing iterator failed */
#define JAK_ERR_HARDCOPYFAILED 45          /** Unable to construct a hard copy of the source object */
#define JAK_ERR_REALLOCERR 46              /** Memory reallocation failed */
#define JAK_ERR_PREDEVAL_FAILED 47         /** Predicate evaluation failed */
#define JAK_ERR_INITFAILED 48              /** Initialization failed */
#define JAK_ERR_DROPFAILED 49              /** Resource release failed: potentially a memory leak occurred */
#define JAK_ERR_OPPFAILED 50               /** Operation failed */
#define JAK_ERR_REHASH_NOROLLBACK 51       /** Rehashing hash table failed; rollback is not performed */
#define JAK_ERR_MEMFILEOPEN_FAILED 52      /** Unable to open memory file */
#define JAK_ERR_VITEROPEN_FAILED 53        /** Value iterator cannot be initialized */
#define JAK_ERR_MEMFILESKIP_FAILED 54      /** Memfile cannot skip desired amount of bytes */
#define JAK_ERR_MEMFILESEEK_FAILED 55      /** Unable to seek in memory file */
#define JAK_ERR_ITER_NOOBJ 56              /** Unable to get value: type is not non-array object */
#define JAK_ERR_ITER_NOBOOL 57             /** Unable to get value: type is not non-array boolean */
#define JAK_ERR_ITER_NOINT8 58             /** Unable to get value: type is not non-array int8 */
#define JAK_ERR_ITER_NOINT16 59            /** Unable to get value: type is not non-array int16 */
#define JAK_ERR_ITER_NOINT32 60            /** Unable to get value: type is not non-array int32 */
#define JAK_ERR_ITER_NOINT64 61            /** Unable to get value: type is not non-array int64 */
#define JAK_ERR_ITER_NOUINT8 62            /** Unable to get value: type is not non-array uint8 */
#define JAK_ERR_ITER_NOUINT16 63           /** Unable to get value: type is not non-array uint16 */
#define JAK_ERR_ITER_NOUINT32 64           /** Unable to get value: type is not non-array uint32 */
#define JAK_ERR_ITER_NOUINT64 65           /** Unable to get value: type is not non-array uint64 */
#define JAK_ERR_ITER_NONUMBER 66           /** Unable to get value: type is not non-array number */
#define JAK_ERR_ITER_NOSTRING 67           /** Unable to get value: type is not non-array string */
#define JAK_ERR_ITER_OBJECT_NEEDED 68      /** Illegal state: iteration over object issued, but collection found */
#define JAK_ERR_ITER_COLLECTION_NEEDED 69  /** Illegal state: iteration over collection issued, but object found */
#define JAK_ERR_TYPEMISMATCH 70            /** Type mismatch detected */
#define JAK_ERR_INDEXCORRUPTED_OFFSET 71   /** Index is corrupted: requested offset is outside file bounds */
#define JAK_ERR_TMP_FOPENWRITE 72          /** Temporary file cannot be opened for writing */
#define JAK_ERR_FWRITE_FAILED 73           /** Unable to write to file */
#define JAK_ERR_HASTABLE_DESERIALERR 74    /** Unable to deserialize hash table from file */
#define JAK_ERR_UNKNOWN_DIC_TYPE 75        /** Unknown string dictionary implementation requested */
#define JAK_ERR_STACK_OVERFLOW 76          /** Stack overflow */
#define JAK_ERR_STACK_UNDERFLOW 77         /** Stack underflow */
#define JAK_ERR_OUTDATED 78                /** Object was modified but is out of date */
#define JAK_ERR_NOTREADABLE 79             /** Object is currently being updated; no read allowed */
#define JAK_ERR_ILLEGALOP 80               /** Illegal operation */
#define JAK_ERR_BADTYPE 81                 /** Unsupported type */
#define JAK_ERR_UNSUPPCONTAINER 82         /** Unsupported container for data type */
#define JAK_ERR_INSERT_TOO_DANGEROUS 83    /** Adding integers with this function will perform an auto casting to
                                             * the smallest type required to store the integer value. Since you push
                                             * integers with this function into an column container that is bound
                                             * to a specific type, any insertion function call will fail once the
                                             * integer value requires a larger (or smaller) type than the fist value
                                             * added to the container. Use '*_insert_X' instead, where X is jak_u8, jak_u16,...
                                             * , jak_u32 resp. jak_i8, jak_i16,..., jak_i32. */
#define JAK_ERR_PARSE_DOT_EXPECTED 84       /** parsing JAK_ERROR: dot ('.') expected */
#define JAK_ERR_PARSE_ENTRY_EXPECTED 85     /** parsing JAK_ERROR: key name or array index expected */
#define JAK_ERR_PARSE_UNKNOWN_TOKEN 86      /** parsing JAK_ERROR: unknown token */
#define JAK_ERR_DOT_PATH_PARSERR 87         /** dot-notated path could not be parsed */
#define JAK_ERR_ILLEGALSTATE 88             /** Illegal state */
#define JAK_ERR_UNSUPPORTEDTYPE 89          /** Unsupported data type */
#define JAK_ERR_FAILED 90                   /** Operation failed */
#define JAK_ERR_CLEANUP 91                  /** Cleanup operation failed; potentially a memory leak occurred */
#define JAK_ERR_DOT_PATH_COMPILEERR 92      /** dot-notated path could not be compiled */
#define JAK_ERR_NONUMBER 93                 /** not a number */
#define JAK_ERR_BUFFERTOOTINY 94            /** buffer capacity exceeded */
#define JAK_ERR_TAILINGJUNK 95              /** tailing junk was detected in a stream */
#define JAK_ERR_NOTINDEXED 96               /** not indexed */

static const char *const jak_global_err_str[] =
        {"No JAK_ERROR", "Null pointer detected", "Function not implemented", "Index is out of bounds",
         "Memory allocation failed", "Illegal arguments", "Internal JAK_ERROR", "Illegal implementation", "Not found",
         "Element not in list", "Array index out of bounds", "Illegal JSON array: mixed types",
         "Reading from file failed", "I/O JAK_ERROR", "Unsupported archive format version", "Format is corrupted",
         "Stream is not a types archive", "Not in bit writing mode", "Function is not yet implemented",
         "Unsupported type found", "Unsupported pack strategy requested", "No string representation for type available",
         "Marker type cannot be mapped to value type", "Parsing stopped; unknown data type requested",
         "Unknown token during parsing JSON detected", "Unknown value type for number in JSON property",
         "Stream is not a valid archive file", "Unsupported strategy requested for key lookup", "Internal JAK_ERROR",
         "No huffman code table entry found for character",
         "Memory file was opened as read-only but requested a modification",
         "Unable to import jak_json file: unsupported type", "Mode set to read-only but modification was requested",
         "Read outside of memory range bounds", "Slot management broken",
         "Thread run out of object ids: start another one", "JSON parsing JAK_ERROR",
         "Document insertion bulk creation failed", "File cannot be opened for writing",
         "Archive cannot be serialized into file", "Archive cannot be deserialized form file",
         "Unable to read from file", "Unable to perform full scan in archive file",
         "String decompression from archive failed", "Closing iterator failed",
         "Unable to construct a hard copy of the source object", "Memory reallocation failed",
         "Predicate evaluation failed", "Initialization failed",
         "Resource release failed: potentially a memory leak occurred", "Operation failed",
         "Rehashing hash table failed; rollback is not performed", "Unable to open memory file",
         "Value iterator cannot be initialized", "Memfile cannot skip desired amount of bytes",
         "Unable to seek in memory file", "Unable to get value: type is not non-array object",
         "Unable to get value: type is not non-array boolean", "Unable to get value: type is not non-array int8",
         "Unable to get value: type is not non-array int16", "Unable to get value: type is not non-array int32",
         "Unable to get value: type is not non-array int64", "Unable to get value: type is not non-array uint8",
         "Unable to get value: type is not non-array uint16", "Unable to get value: type is not non-array uint32",
         "Unable to get value: type is not non-array uint64", "Unable to get value: type is not non-array number",
         "Unable to get value: type is not non-array string",
         "Illegal state: iteration over object issued, but collection found",
         "Illegal state: iteration over collection issued, but object found", "Type mismatch detected",
         "Index is corrupted: requested offset is outside file bounds", "Temporary file cannot be opened for writing",
         "Unable to write to file", "Unable to deserialize hash table from file",
         "Unknown string dictionary implementation requested", "Stack overflow", "Stack underflow",
         "Object was modified but is out of date", "Object is currently being updated; no read allowed",
         "Illegal operation", "Unsupported type", "Unsupported container for data type",
         "Adding integers with this function will perform an auto casting to the smallest type required to store "
         "the integer value. Since you push integers with this function into an column container that is bound "
         "to a specific type, any insertion function call will fail once the integer value requires a larger "
         "(or smaller) type than the fist value added to the container. Use '*_insert_X' instead, where X is "
         "jak_u8, jak_u16,..., jak_u32 resp. jak_i8, jak_i16,..., jak_i32. ", "parsing JAK_ERROR dot ('.') expected",
         "parsing JAK_ERROR key name or array index expected", "parsing JAK_ERROR: unknown token",
         "dot-notated path could not be parsed", "Illegal state", "Unsupported data type", "Operation failed",
         "Cleanup operation failed; potentially a memory leak occurred", "dot-notated path could not be compiled",
         "not a number", "buffer capacity exceeded", "tailing junk was detected in a stream", "not indexed"};

#define JAK_ERRSTR_ILLEGAL_CODE "illegal JAK_ERROR code"

static const int jak_global_nerr_str = JAK_ARRAY_LENGTH(jak_global_err_str);

typedef struct jak_error {
        int code;
        const char *file;
        jak_u32 line;
        char *details;
} jak_error;

bool jak_error_init(jak_error *err);

bool jak_error_cpy(jak_error *dst, const jak_error *src);

bool jak_error_drop(jak_error *err);

bool jak_error_set(jak_error *err, int code, const char *file, jak_u32 line);

bool jak_error_set_wdetails(jak_error *err, int code, const char *file, jak_u32 line, const char *details);

bool jak_error_set_no_abort(jak_error *err, int code, const char *file, jak_u32 line);

bool jak_error_set_wdetails_no_abort(jak_error *err, int code, const char *file, jak_u32 line, const char *details);

bool jak_error_str(const char **errstr, const char **file, jak_u32 *line, bool *details, const char **detailsstr,
               const jak_error *err);

bool jak_error_print_to_stderr(const jak_error *err);

bool jak_error_print_and_abort(const jak_error *err);

#define JAK_ERROR_OCCURED(x)                   ((x)->err.code != JAK_ERR_NOERR)

#define JAK_SUCCESS_ELSE_RETURN(expr, err, code, retval)                                                                   \
{                                                                                                                      \
        bool result = expr;                                                                                            \
        JAK_ERROR_IF(!(result), err, code);                                                                                \
        if (!(result)) { return retval; }                                                                              \
}

#define JAK_SUCCESS_ELSE_NULL(expr, err)           JAK_SUCCESS_ELSE_RETURN(expr, err, JAK_ERR_FAILED, NULL)
#define JAK_SUCCESS_ELSE_FAIL(expr, err)           JAK_SUCCESS_ELSE_RETURN(expr, err, JAK_ERR_FAILED, false)


#define JAK_ERROR(err, code)                     JAK_ERROR_IF (true, err, code)
#define JAK_ERROR_NO_ABORT(err, code)            JAK_ERROR_IF (true, err, code)
#define JAK_ERROR_IF(expr, err, code)            { if (expr) { jak_error_set(err, code, __FILE__, __LINE__); } }
#define JAK_ERROR_IF_AND_RETURN(expr, err, code, retval) \
                                                    { if (expr) { jak_error_set(err, code, __FILE__, __LINE__);            \
                                                                  return retval; } }

#define JAK_ERROR_IF_WDETAILS(expr, err, code, msg)            { if (expr) { JAK_ERROR_WDETAILS(err, code, msg); } }
#define JAK_ERROR_WDETAILS(err, code, msg)                     jak_error_set_wdetails(err, code, __FILE__, __LINE__, msg);

#define JAK_ERROR_PRINT(code)                    JAK_ERROR_PRINT_IF(true, code)
#define JAK_ERROR_PRINT_AND_DIE(code)            { JAK_ERROR_PRINT(code); abort(); }
#define JAK_ERROR_PRINT_AND_DIE_IF(expr, code)   { if(expr) { JAK_ERROR_PRINT_AND_DIE(code) } }
#define JAK_ERROR_PRINT_IF(expr, code)                                                                                     \
{                                                                                                                      \
    if (expr) {                                                                                                        \
        jak_error err;                                                                                                \
        jak_error_init(&err);                                                                                              \
        JAK_ERROR(&err, code);                                                                                             \
        jak_error_print_to_stderr(&err);                                                                                   \
    }                                                                                                                  \
}

#define JAK_DEFINE_ERROR_GETTER(type_tag_name)  JAK_DEFINE_GET_ERROR_FUNCTION(type_tag_name, struct type_tag_name, e)

#define JAK_DEFINE_GET_ERROR_FUNCTION(type_name, type, arg)                                                            \
JAK_FUNC_UNUSED static bool                                                                                            \
jak_##type_name##_get_error(jak_error *err, const type *arg)                                                                \
{                                                                                                                      \
    JAK_ERROR_IF_NULL(err)                                                                                                 \
    JAK_ERROR_IF_NULL(arg)                                                                                                 \
    jak_error_cpy(err, &arg->err);                                                                                         \
    return true;                                                                                                       \
}

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_FORWDECL_H
#define JAK_FORWDECL_H

typedef struct jak_allocator jak_allocator;

typedef struct jak_archive jak_archive;
typedef struct jak_archive_callback jak_archive_callback;
typedef struct jak_sid_cache_stats jak_sid_cache_stats;
typedef struct jak_archive_header jak_archive_header;
typedef struct jak_record_header jak_record_header;
typedef struct jak_object_header jak_object_header;
typedef struct jak_prop_header jak_prop_header;
typedef union jak_string_tab_flags jak_string_tab_flags_u;
typedef struct jak_string_table_header jak_string_table_header;
typedef struct jak_object_array_header jak_object_array_header;
typedef struct jak_column_group_header jak_column_group_header;
typedef struct jak_column_header jak_column_header;
typedef union jak_object_flags jak_object_flags_u;
typedef struct jak_archive_prop_offs jak_archive_prop_offs;
typedef struct jak_fixed_prop jak_fixed_prop;
typedef struct jak_table_prop jak_table_prop;
typedef struct jak_var_prop jak_var_prop;
typedef struct jak_array_prop jak_array_prop;
typedef struct jak_null_prop jak_null_prop;
typedef struct jak_record_flags jak_record_flags;
typedef struct jak_string_table jak_string_table;
typedef struct jak_record_table jak_record_table;
typedef struct jak_archive_info jak_archive_info;
typedef struct jak_string_entry_header jak_string_entry_header;
typedef struct jak_archive_io_context jak_archive_io_context;
typedef struct jak_archive_object jak_archive_object;
typedef struct jak_collection_iter_state jak_collection_iter_state;
typedef struct jak_archive_value_vector jak_archive_value_vector;
typedef struct jak_prop_iter jak_prop_iter;
typedef struct jak_independent_iter_state jak_independent_iter_state;
typedef struct jak_column_object_iter jak_column_object_iter;
typedef struct jak_string_pred jak_string_pred;
typedef struct jak_archive_query jak_archive_query;
typedef struct jak_strid_info jak_strid_info;
typedef struct jak_strid_iter jak_strid_iter;
typedef struct jak_path_entry jak_path_entry;
typedef struct jak_archive_visitor_desc jak_archive_visitor_desc;
typedef struct jak_archive_visitor jak_archive_visitor;
typedef struct jak_column_doc_column jak_column_doc_column;
typedef struct jak_column_doc_group jak_column_doc_group;
typedef struct jak_column_doc_obj jak_column_doc_obj;
typedef struct jak_column_doc jak_column_doc;
typedef struct jak_doc_entries jak_doc_entries;
typedef struct jak_doc_bulk jak_doc_bulk;
typedef struct jak_doc jak_doc;
typedef struct jak_doc_obj jak_doc_obj;
typedef union jak_encoded_doc_value jak_encoded_doc_value_u;
typedef struct jak_encoded_doc_prop_header jak_encoded_doc_prop_header;
typedef struct jak_encoded_doc_prop jak_encoded_doc_prop;
typedef struct jak_encoded_doc_prop_array jak_encoded_doc_prop_array;
typedef struct jak_encoded_doc jak_encoded_doc;
typedef struct jak_encoded_doc_list jak_encoded_doc_list;

typedef struct jak_error jak_error;

typedef struct jak_async_func_proxy jak_async_func_proxy;
typedef struct jak_filter_arg jak_filter_arg;
typedef struct jak_map_args jak_map_args;
typedef struct jak_gather_scatter_args jak_gather_scatter_args;

typedef struct jak_bitmap jak_bitmap;

typedef struct jak_carbon jak_carbon;
typedef struct jak_carbon_insert jak_carbon_insert;
typedef struct jak_carbon_new jak_carbon_new;
typedef struct jak_field_access jak_field_access;
typedef struct jak_carbon_array_it jak_carbon_array_it;
typedef struct jak_carbon_column_it jak_carbon_column_it;
typedef struct jak_carbon_dot_node jak_carbon_dot_node;
typedef struct jak_carbon_dot_path jak_carbon_dot_path;
typedef struct jak_carbon_find jak_carbon_find;
typedef struct jak_carbon_insert_array_state jak_carbon_insert_array_state;
typedef struct jak_carbon_insert_object_state jak_carbon_insert_object_state;
typedef struct jak_carbon_insert_column_state jak_carbon_insert_column_state;
typedef struct jak_carbon_object_it jak_carbon_object_it;
typedef struct jak_carbon_path_evaluator jak_carbon_path_evaluator;
typedef struct jak_carbon_path_index jak_carbon_path_index;
typedef struct jak_carbon_path_index_it jak_carbon_path_index_it;
typedef struct jak_carbon_printer jak_carbon_printer;
typedef struct jak_carbon_revise jak_carbon_revise;
typedef struct jak_carbon_binary jak_carbon_binary;
typedef struct jak_carbon_update jak_carbon_update;
typedef struct jak_packer jak_packer;

typedef struct jak_hashset_bucket jak_hashset_bucket;
typedef struct jak_hashset jak_hashset;
typedef struct jak_hashtable_bucket jak_hashtable_bucket;
typedef struct jak_hashtable jak_hashtable;
typedef struct jak_huffman jak_huffman;
typedef struct jak_pack_huffman_entry jak_pack_huffman_entry;
typedef struct jak_pack_huffman_info jak_pack_huffman_info;
typedef struct jak_pack_huffman_str_info jak_pack_huffman_str_info;

typedef struct jak_json_token jak_json_token;
typedef struct jak_json_err jak_json_err;
typedef struct jak_json_tokenizer jak_json_tokenizer;
typedef struct jak_json_parser jak_json_parser;
typedef struct jak_json jak_json;
typedef struct jak_json_node_value jak_json_node_value;
typedef struct jak_json_object jak_json_object;
typedef struct jak_json_element jak_json_element;
typedef struct jak_json_string jak_json_string;
typedef struct jak_json_prop jak_json_prop;
typedef struct jak_json_members jak_json_members;
typedef struct jak_json_elements jak_json_elements;
typedef struct jak_json_array jak_json_array;
typedef struct jak_json_number jak_json_number;

typedef struct jak_memblock jak_memblock;
typedef struct jak_memfile jak_memfile;

typedef struct jak_command_opt jak_command_opt;
typedef struct jak_command_opt_group jak_command_opt_group;
typedef struct jak_command_opt_mgr jak_command_opt_mgr;

typedef struct jak_priority_queue_element_info jak_priority_queue_element_info;
typedef struct jak_priority_queue jak_priority_queue;

typedef struct jak_slice jak_slice;
typedef struct jak_hash_bounds jak_hash_bounds;
typedef struct jak_slice_descriptor jak_slice_descriptor;
typedef struct jak_slice_list jak_slice_list;
typedef struct jak_slice_handle jak_slice_handle;

typedef struct jak_spinlock jak_spinlock;

typedef struct jak_vector jak_vector;

typedef struct jak_str_hash jak_str_hash;
typedef struct jak_str_hash_counters jak_str_hash_counters;

typedef struct jak_string jak_string;

typedef struct jak_string_dict jak_string_dict;

typedef struct jak_thread_task jak_thread_task;
typedef struct jak_task_state jak_task_state;
typedef struct jak_task_handle jak_task_handle;
typedef struct jak_thread_pool jak_thread_pool;
typedef struct jak_thread_info jak_thread_info;
typedef struct jak_thread_pool_stats jak_thread_pool_stats;
typedef struct jak_thread_stats jak_thread_stats;
typedef struct jak_task_stats jak_task_stats;

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_HASH_H
#define JAK_HASH_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_types.h>

JAK_BEGIN_DECL

typedef jak_u8 hash8_t;
typedef jak_u16 hash16_t;
typedef jak_u32 hash32_t;
typedef jak_u64 hash64_t;

#define JAK_HASH_ADDITIVE(key_size, key)                                                                               \
({                                                                                                                     \
    hash32_t hash = 0;                                                                                                 \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash += ((unsigned char* )key)[i];                                                                             \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define JAK_HASH_BERNSTEIN(key_size, key)        JAK_HASH_BERNSTEIN_WTYPE(key_size, key, hash32_t)
#define JAK_HASH8_BERNSTEIN(key_size, key)       JAK_HASH_BERNSTEIN_WTYPE(key_size, key, hash8_t)
#define JAK_HASH16_BERNSTEIN(key_size, key)      JAK_HASH_BERNSTEIN_WTYPE(key_size, key, hash16_t)
#define JAK_HASH64_BERNSTEIN(key_size, key)        JAK_HASH_BERNSTEIN_WTYPE(key_size, key, hash64_t)

#define JAK_HASH_BERNSTEIN_WTYPE(key_size, key, hash_type)                                                             \
({                                                                                                                     \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash_type hash = 0;                                                                                                \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash ^= 33 * hash + ((unsigned char* )key)[i];                                                                 \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define JAK_HASH64_BERNSTEIN_WSEED(key_size, key, seed)                                                                \
({                                                                                                                     \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash64_t hash = seed;                                                                                              \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash ^= 33 * hash + ((unsigned char* )key)[i];                                                                 \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define JAK_HASH_BERNSTEIN2(key_size, key)                                                                             \
({                                                                                                                     \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash32_t hash = 0;                                                                                                 \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash ^= 33 * hash ^ ((unsigned char* )key)[i];                                                                 \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define JAK_HASH_ELF(key_size, key)                                                                                    \
({                                                                                                                     \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash32_t hash = 0, g;                                                                                              \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash = (hash << 4) + ((unsigned char* )key)[i];                                                                \
        if ((g = hash & 0xf0000000L) != 0) {                                                                           \
            hash ^= g >> 24;                                                                                           \
        }                                                                                                              \
        hash &= ~g;                                                                                                    \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define JAK_HASH8_FNV(key_size, key)             JAK_HASH_FNV_WTYPE(key_size, key, hash8_t)
#define JAK_HASH16_FNV(key_size, key)            JAK_HASH_FNV_WTYPE(key_size, key, hash16_t)
#define JAK_HASH_FNV(key_size, key)              JAK_HASH_FNV_WTYPE(key_size, key, hash32_t)
#define JAK_HASH64_FNV(key_size, key)              JAK_HASH_FNV_WTYPE(key_size, key, hash64_t)

#define JAK_HASH_FNV_WTYPE(key_size, key, hash_type)                                                                   \
({                                                                                                                     \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash_type hash = (hash_type) 2166136261;                                                                           \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash = (hash * 16777619) ^ ((unsigned char* )key)[i];                                                          \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define JAK_JENKINS_MIX(a, b, c)                                                                                       \
{                                                                                                                      \
    a -= b; a -= c; a ^= (c >> 13);                                                                                    \
    b -= c; b -= a; b ^= (a << 8);                                                                                     \
    c -= a; c -= b; c ^= (b >> 13);                                                                                    \
    a -= b; a -= c; a ^= (c >> 12);                                                                                    \
    b -= c; b -= a; b ^= (a << 16);                                                                                    \
    c -= a; c -= b; c ^= (b >> 5);                                                                                     \
    a -= b; a -= c; a ^= (c >> 3);                                                                                     \
    b -= c; b -= a; b ^= (a << 10);                                                                                    \
    c -= a; c -= b; c ^= (b >> 15);                                                                                    \
}

/** implements: hash32_t hash_jenkins(size_t key_size, const void *key) */
#define JAK_HASH_JENKINS(keySizeIn, key)                                                                               \
({                                                                                                                     \
    size_t key_size = keySizeIn;                                                                                       \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    unsigned a, b;                                                                                                     \
    unsigned c = 0;                                                                                                    \
    unsigned char *k = (unsigned char *) key;                                                                          \
                                                                                                                       \
    a = b = 0x9e3779b9;                                                                                                \
                                                                                                                       \
    while (key_size >= 12) {                                                                                           \
        a += (k[0] + ((unsigned)k[1] << 8) + ((unsigned)k[2] << 16) + ((unsigned)k[3] << 24));                         \
        b += (k[4] + ((unsigned)k[5] << 8) + ((unsigned)k[6] << 16) + ((unsigned)k[7] << 24));                         \
        c += (k[8] + ((unsigned)k[9] << 8) + ((unsigned)k[10] << 16) + ((unsigned)k[11] << 24));                       \
        JAK_JENKINS_MIX(a, b, c);                                                                                      \
        k += 12;                                                                                                       \
        key_size -= 12;                                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    c += key_size;                                                                                                     \
                                                                                                                       \
    switch (key_size) {                                                                                                \
        case 11: c += ((unsigned)k[10] << 24); break;                                                                  \
        case 10: c += ((unsigned)k[9] << 16); break;                                                                   \
        case 9: c += ((unsigned)k[8] << 8); break;                                                                     \
        case 8: b += ((unsigned)k[7] << 24); break;                                                                    \
        case 7: b += ((unsigned)k[6] << 16); break;                                                                    \
        case 6: b += ((unsigned)k[5] << 8); break;                                                                     \
        case 5: b += k[4]; break;                                                                                      \
        case 4: a += ((unsigned)k[3] << 24); break;                                                                    \
        case 3: a += ((unsigned)k[2] << 16); break;                                                                    \
        case 2: a += ((unsigned)k[1] << 8); break;                                                                     \
        case 1: a += k[0]; break;                                                                                      \
    }                                                                                                                  \
    JAK_JENKINS_MIX(a, b, c);                                                                                          \
    c;                                                                                                                 \
})

#define JAK_HASH_OAT(key_size, key)                                                                                    \
({                                                                                                                     \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash32_t hash = 0;                                                                                                 \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash += ((unsigned char* )key)[i];                                                                             \
        hash += (hash << 10);                                                                                          \
        hash ^= (hash >> 6);                                                                                           \
    }                                                                                                                  \
                                                                                                                       \
    hash += (hash << 3);                                                                                               \
    hash ^= (hash >> 11);                                                                                              \
    hash += (hash << 15);                                                                                              \
                                                                                                                       \
    hash;                                                                                                              \
})

#define JAK_HASH_ROT(key_size, key)                                                                                    \
({                                                                                                                     \
    hash32_t hash = 0;                                                                                                 \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash ^= (hash << 4) ^ (hash >> 28) ^ ((unsigned char* )key)[i];                                                \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define JAK_HASH_SAX(key_size, key)                                                                                    \
({                                                                                                                     \
    hash32_t hash = 0;                                                                                                 \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash ^= (hash << 5) + (hash >> 2) + ((unsigned char* )key)[i];                                                 \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define JAK_HASH_XOR(key_size, key)                                                                                    \
({                                                                                                                     \
    hash32_t hash = 0;                                                                                                 \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash ^= ((unsigned char* )key)[i];                                                                             \
    }                                                                                                                  \
    hash;                                                                                                              \
})

JAK_END_DECL

#endif/**
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

#ifndef JAK_HASHTEST_H
#define JAK_HASHTEST_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>
#include <jak_spinlock.h>

JAK_BEGIN_DECL

typedef struct jak_hashset_bucket {
        bool in_use_flag;  /* flag indicating if bucket is in use */
        jak_i32 displacement; /* difference between intended position during insert, and actual position in table */
        jak_u64 key_idx;      /* position of key element in owning jak_hashset structure */
} jak_hashset_bucket;

/**
 * Hashset implementation specialized for key of fixed-length size, and where comparision
 * for equals is byte-compare. With this, calling a (type-dependent) compare function becomes obsolete.
 *
 * Example: jak_u64.
 *
 * This hashset is optimized to reduce access time to elements. Internally, a robin-hood hashing technique is used.
 *
 * Note: this implementation does not support string or pointer types. The structure is thread-safe by a spinlock
 * lock implementation.
 */
typedef struct jak_hashset {
        jak_vector key_data;
        jak_vector ofType(jak_hashset_bucket) table;
        jak_spinlock lock;
        jak_u32 size;
        jak_error err;
} jak_hashset;

JAK_DEFINE_GET_ERROR_FUNCTION(jak_hashset, jak_hashset, set);

bool jak_hashset_create(jak_hashset *map, jak_error *err, size_t key_size, size_t capacity);
jak_hashset *jak_hashset_cpy(jak_hashset *src);
bool jak_hashset_drop(jak_hashset *map);

jak_vector *jak_hashset_keys(jak_hashset *map);
bool jak_hashset_clear(jak_hashset *map);
bool jak_hashset_avg_displace(float *displace, const jak_hashset *map);
bool jak_hashset_lock(jak_hashset *map);
bool jak_hashset_unlock(jak_hashset *map);
bool jak_hashset_insert_or_update(jak_hashset *map, const void *keys, uint_fast32_t num_pairs);
bool jak_hashset_remove_if_contained(jak_hashset *map, const void *keys, size_t num_pairs);
bool jak_hashset_contains_key(jak_hashset *map, const void *key);
bool jak_hashset_get_load_factor(float *factor, jak_hashset *map);
bool jak_hashset_rehash(jak_hashset *map);

JAK_END_DECL

#endif
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

#ifndef JAK_HAHSTABLE_H
#define JAK_HAHSTABLE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>
#include <jak_spinlock.h>

JAK_BEGIN_DECL

typedef struct jak_hashtable_bucket {
        bool in_use_flag;  /* flag indicating if bucket is in use */
        jak_i32 displacement; /* difference between intended position during insert, and actual position in table */
        jak_u32 num_probs;    /* number of probe calls to this bucket */
        jak_u64 data_idx;      /* position of key element in owning jak_hashtable structure */
} jak_hashtable_bucket;

/**
 * Hash table implementation specialized for key and value types of fixed-length size, and where comparision
 * for equals is byte-compare. With this, calling a (type-dependent) compare function becomes obsolete.
 *
 * Example: mapping of jak_u64 to jak_u32.
 *
 * This hash table is optimized to reduce access time to elements. Internally, a robin-hood hashing technique is used.
 *
 * Note: this implementation does not support string or pointer types. The structure is thread-safe by a spinlock
 * lock implementation.
 */
typedef struct jak_hashtable {
        jak_vector key_data;
        jak_vector value_data;
        jak_vector ofType(jak_hashtable_bucket) table;
        jak_spinlock lock;
        jak_u32 size;
        jak_error err;
} jak_hashtable;

JAK_DEFINE_GET_ERROR_FUNCTION(jak_hashtable, jak_hashtable, table);

bool jak_hashtable_create(jak_hashtable *map, jak_error *err, size_t key_size, size_t value_size, size_t capacity);
jak_hashtable *jak_hashtable_cpy(jak_hashtable *src);
bool jak_hashtable_drop(jak_hashtable *map);

bool jak_hashtable_clear(jak_hashtable *map);
bool jak_hashtable_avg_displace(float *displace, const jak_hashtable *map);
bool jak_hashtable_lock(jak_hashtable *map);
bool jak_hashtable_unlock(jak_hashtable *map);
bool jak_hashtable_insert_or_update(jak_hashtable *map, const void *keys, const void *values, uint_fast32_t num_pairs);
bool jak_hashtable_serialize(FILE *file, jak_hashtable *table);
bool jak_hashtable_deserialize(jak_hashtable *table, jak_error *err, FILE *file);
bool jak_hashtable_remove_if_contained(jak_hashtable *map, const void *keys, size_t num_pairs);
const void *jak_hashtable_get_value(jak_hashtable *map, const void *key);
bool jak_hashtable_get_load_factor(float *factor, jak_hashtable *map);
bool jak_hashtable_rehash(jak_hashtable *map);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_HUFFMAN_H
#define JAK_HUFFMAN_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>
#include <jak_memfile.h>
#include <jak_types.h>

JAK_BEGIN_DECL

typedef struct jak_huffman {
        jak_vector ofType(jak_pack_huffman_entry) table;
        jak_error err;
} jak_huffman;

typedef struct jak_pack_huffman_entry {
        unsigned char letter;
        jak_u32 *blocks;
        jak_u16 nblocks;
} jak_pack_huffman_entry;

typedef struct jak_pack_huffman_info {
        unsigned char letter;
        jak_u8 nbytes_prefix;
        char *prefix_code;
} jak_pack_huffman_info;

typedef struct jak_pack_huffman_str_info {
        jak_u32 nbytes_encoded;
        const char *encoded_bytes;
} jak_pack_huffman_str_info;

bool jak_coding_huffman_create(jak_huffman *dic);
bool jak_coding_huffman_cpy(jak_huffman *dst, jak_huffman *src);
bool jak_coding_huffman_drop(jak_huffman *dic);

bool jak_coding_huffman_build(jak_huffman *encoder, const jak_string_jak_vector_t *strings);
bool jak_coding_huffman_get_error(jak_error *err, const jak_huffman *dic);
bool jak_coding_huffman_encode(jak_memfile *file, jak_huffman *dic, const char *string);
bool jak_coding_huffman_read_string(jak_pack_huffman_str_info *info, jak_memfile *src);
bool jak_coding_huffman_serialize(jak_memfile *file, const jak_huffman *dic, char marker_symbol);
bool jak_coding_huffman_read_entry(jak_pack_huffman_info *info, jak_memfile *file, char marker_symbol);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_JSON_H
#define JAK_JSON_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>

JAK_BEGIN_DECL

typedef enum jak_json_token_type {
        JAK_OBJECT_OPEN,
        JAK_OBJECT_CLOSE,
        JAK_LITERAL_STRING,
        JAK_LITERAL_INT,
        JAK_LITERAL_FLOAT,
        JAK_LITERAL_TRUE,
        JAK_LITERAL_FALSE,
        JAK_LITERAL_NULL,
        JAK_COMMA,
        JAK_ASSIGN,
        JAK_ARRAY_OPEN,
        JAK_ARRAY_CLOSE,
        JAK_JSON_UNKNOWN
} jak_json_token_e;

typedef struct jak_json_token {
        jak_json_token_e type;
        const char *string;
        unsigned line;
        unsigned column;
        unsigned length;
} jak_json_token;

typedef struct jak_json_err {
        const jak_json_token *token;
        const char *token_type_str;
        const char *msg;
} jak_json_err;

typedef struct jak_json_tokenizer {
        const char *cursor;
        jak_json_token token;
        jak_error err;
} jak_json_tokenizer;

typedef struct jak_json_parser {
        jak_json_tokenizer tokenizer;
        jak_error err;
} jak_json_parser;

typedef enum json_parent {
        JAK_JSON_PARENT_OBJECT, JAK_JSON_PARENT_MEMBER, JAK_JSON_PARENT_ELEMENTS
} json_parent_e;

typedef enum jak_json_value_type_e {
        JAK_JSON_VALUE_OBJECT,
        JAK_JSON_VALUE_ARRAY,
        JAK_JSON_VALUE_STRING,
        JAK_JSON_VALUE_NUMBER,
        JAK_JSON_VALUE_TRUE,
        JAK_JSON_VALUE_FALSE,
        JAK_JSON_VALUE_NULL
} jak_json_value_type_e;

typedef enum jak_json_list_type_e {
        JAK_JSON_LIST_EMPTY,
        JAK_JSON_LIST_VARIABLE_OR_NESTED,
        JAK_JSON_LIST_FIXED_U8,
        JAK_JSON_LIST_FIXED_U16,
        JAK_JSON_LIST_FIXED_U32,
        JAK_JSON_LIST_FIXED_U64,
        JAK_JSON_LIST_FIXED_I8,
        JAK_JSON_LIST_FIXED_I16,
        JAK_JSON_LIST_FIXED_I32,
        JAK_JSON_LIST_FIXED_I64,
        JAK_JSON_LIST_FIXED_FLOAT,
        JAK_JSON_LIST_FIXED_NULL,
        JAK_JSON_LIST_FIXED_BOOLEAN
} jak_json_list_type_e;

typedef struct jak_json {
        jak_json_element *element;
        jak_error err;
} jak_json;

typedef struct jak_json_node_value {
        jak_json_element *parent;
        jak_json_value_type_e value_type;
        union {
                jak_json_object *object;
                jak_json_array *array;
                jak_json_string *string;
                jak_json_number *number;
                void *ptr;
        } value;
} jak_json_node_value;

typedef struct jak_json_object {
        jak_json_node_value *parent;
        jak_json_members *value;
} jak_json_object;

typedef struct jak_json_element {
        json_parent_e parent_type;
        union {
                jak_json *jak_json;
                jak_json_prop *member;
                jak_json_elements *elements;
                void *ptr;
        } parent;
        jak_json_node_value value;
} jak_json_element;

typedef struct jak_json_string {
        jak_json_prop *parent;
        char *value;
} jak_json_string;

typedef struct jak_json_prop {
        jak_json_members *parent;
        jak_json_string key;
        jak_json_element value;
} jak_json_prop;

typedef struct jak_json_members {
        jak_json_object *parent;
        jak_vector ofType(jak_json_prop) members;
} jak_json_members;

typedef struct jak_json_elements {
        jak_json_array *parent;
        jak_vector ofType(jak_json_element) elements;
} jak_json_elements;

typedef struct jak_json_array {
        jak_json_node_value *parent;
        jak_json_elements elements;
} jak_json_array;

typedef enum json_number_type {
        JAK_JSON_NUMBER_FLOAT, JAK_JSON_NUMBER_UNSIGNED, JAK_JSON_NUMBER_SIGNED
} json_number_type_e;

typedef struct jak_json_number {
        jak_json_node_value *parent;
        json_number_type_e value_type;
        union {
                float float_number;
                jak_i64 signed_integer;
                jak_u64 unsigned_integer;
        } value;
} jak_json_number;

bool jak_json_tokenizer_init(jak_json_tokenizer *tokenizer, const char *input);
const jak_json_token *json_tokenizer_next(jak_json_tokenizer *tokenizer);
void jak_json_token_dup(jak_json_token *dst, const jak_json_token *src);
void jak_json_token_print(FILE *file, const jak_json_token *token);
bool jak_json_parser_create(jak_json_parser *parser);
bool jak_json_parse(jak_json *json, jak_json_err *error_desc, jak_json_parser *parser, const char *input);
bool jak_json_test(jak_error *err, jak_json *jak_json);
bool jak_json_drop(jak_json *jak_json);
bool jak_json_print(FILE *file, jak_json *jak_json);
bool jak_json_list_is_empty(const jak_json_elements *elements);
bool jak_json_list_length(jak_u32 *len, const jak_json_elements *elements);
jak_json_list_type_e jak_json_fitting_type(jak_json_list_type_e current, jak_json_list_type_e to_add);
bool jak_json_array_get_type(jak_json_list_type_e *type, const jak_json_array *array);

JAK_DEFINE_GET_ERROR_FUNCTION(jak_json, jak_json, jak_json);

JAK_END_DECL

#endif/**
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

#ifndef JSON_COMPACT_PRINTER_H
#define JSON_COMPACT_PRINTER_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_carbon_printers.h>

bool jak_json_compact_printer_create(jak_carbon_printer *printer);

#endif
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

#ifndef JSON_EXTENDED_PRINTER_H
#define JSON_EXTENDED_PRINTER_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_carbon_printers.h>

bool jak_json_extended_printer_create(jak_carbon_printer *printer);

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_MEMBLOCK_H
#define JAK_MEMBLOCK_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>

JAK_BEGIN_DECL

bool jak_memblock_create(jak_memblock **block, size_t size);
bool jak_memblock_drop(jak_memblock *block);

bool jak_memblock_from_file(jak_memblock **block, FILE *file, size_t nbytes);

bool jak_memblock_get_error(jak_error *out, jak_memblock *block);

bool jak_memblock_zero_out(jak_memblock *block);
bool jak_memblock_size(jak_offset_t *size, const jak_memblock *block);
jak_offset_t jak_memblock_last_used_byte(const jak_memblock *block);
bool jak_memblock_write_to_file(FILE *file, const jak_memblock *block);
const char *jak_memblock_raw_data(const jak_memblock *block);
bool jak_memblock_resize(jak_memblock *block, size_t size);
bool jak_memblock_write(jak_memblock *block, jak_offset_t position, const char *data, jak_offset_t nbytes);
bool jak_memblock_cpy(jak_memblock **dst, jak_memblock *src);
bool jak_memblock_shrink(jak_memblock *block);
bool jak_memblock_move_right(jak_memblock *block, jak_offset_t where, size_t nbytes);
bool jak_memblock_move_left(jak_memblock *block, jak_offset_t where, size_t nbytes);
bool jak_memblock_move_ex(jak_memblock *block, jak_offset_t where, size_t nbytes, bool zero_out);
void *jak_memblock_move_contents_and_drop(jak_memblock *block);
bool jak_memfile_update_last_byte(jak_memblock *block, size_t where);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_MEMFILE_H
#define JAK_MEMFILE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_string.h>
#include <jak_utils_hexdump.h>
#include <jak_memblock.h>

JAK_BEGIN_DECL

typedef struct jak_memfile {
        jak_memblock *memblock;
        jak_offset_t pos;
        jak_offset_t saved_pos[10];
        jak_i8 saved_pos_ptr;
        bool bit_mode;
        size_t current_read_bit, current_write_bit, bytes_completed;
        jak_access_mode_e mode;
        jak_error err;
} jak_memfile;

#define JAK_MEMFILE_PEEK(file, type)                                                                                   \
({                                                                                                                     \
    JAK_ASSERT (jak_memfile_remain_size(file) >= sizeof(type));                                                                \
    (type*) jak_memfile_peek(file, sizeof(type));                                                                          \
})

#define JAK_MEMFILE_READ_TYPE(file, type)                                                                              \
({                                                                                                                     \
    JAK_ASSERT (jak_memfile_remain_size(file) >= sizeof(type));                                                                \
    (type*) jak_memfile_read(file, sizeof(type));                                                                          \
})

#define JAK_MEMFILE_READ_TYPE_LIST(file, type, how_many)                                                               \
    (const type *) JAK_MEMFILE_READ(file, how_many * sizeof(type))

#define JAK_MEMFILE_READ(file, nbytes)                                                                                 \
({                                                                                                                     \
    JAK_ASSERT (jak_memfile_remain_size(file) >= nbytes);                                                                      \
    jak_memfile_read(file, nbytes);                                                                                        \
})

#define jak_memfile_tell(file)                                                                                             \
({                                                                                                                     \
    jak_offset_t offset = 0;                                                                                               \
    jak_memfile_get_offset(&offset, file);                                                                                 \
    offset;                                                                                                            \
})

bool jak_memfile_open(jak_memfile *file, jak_memblock *block, jak_access_mode_e mode);
bool jak_memfile_clone(jak_memfile *dst, jak_memfile *src);

bool jak_memfile_seek(jak_memfile *file, jak_offset_t pos);
bool jak_memfile_seek_from_here(jak_memfile *file, signed_offset_t where);
bool jak_memfile_rewind(jak_memfile *file);
bool jak_memfile_grow(jak_memfile *file_in, size_t grow_by_bytes);
bool jak_memfile_get_offset(jak_offset_t *pos, const jak_memfile *file);
size_t jak_memfile_size(jak_memfile *file);
bool jak_memfile_cut(jak_memfile *file, size_t how_many_bytes);
size_t jak_memfile_remain_size(jak_memfile *file);
bool jak_memfile_shrink(jak_memfile *file);
const char *jak_memfile_read(jak_memfile *file, jak_offset_t nbytes);
jak_u8 jak_memfile_read_byte(jak_memfile *file);
jak_u8 jak_memfile_peek_byte(jak_memfile *file);
jak_u64 jak_memfile_read_u64(jak_memfile *file);
jak_i64 jak_memfile_read_i64(jak_memfile *file);
bool jak_memfile_skip(jak_memfile *file, signed_offset_t nbytes);
#define JAK_MEMFILE_SKIP_BYTE(file) jak_memfile_skip(file, sizeof(jak_u8))
const char *jak_memfile_peek(jak_memfile *file, jak_offset_t nbytes);
bool jak_memfile_write_byte(jak_memfile *file, jak_u8 data);
bool jak_memfile_write(jak_memfile *file, const void *data, jak_offset_t nbytes);
bool jak_memfile_write_zero(jak_memfile *file, size_t how_many);
bool jak_memfile_begin_bit_mode(jak_memfile *file);
bool jak_memfile_write_bit(jak_memfile *file, bool flag);
bool jak_memfile_read_bit(jak_memfile *file);
jak_offset_t jak_memfile_save_position(jak_memfile *file);
bool jak_memfile_restore_position(jak_memfile *file);
signed_offset_t jak_memfile_ensure_space(jak_memfile *memfile, jak_u64 nbytes);
jak_u64 jak_memfile_read_uintvar_stream(jak_u8 *nbytes, jak_memfile *memfile);
bool jak_memfile_skip_uintvar_stream(jak_memfile *memfile);
jak_u64 jak_memfile_peek_uintvar_stream(jak_u8 *nbytes, jak_memfile *memfile);
jak_u64 jak_memfile_write_uintvar_stream(jak_u64 *nbytes_moved, jak_memfile *memfile, jak_u64 value);
signed_offset_t jak_memfile_update_uintvar_stream(jak_memfile *memfile, jak_u64 value);
bool jak_memfile_seek_to_start(jak_memfile *file);
bool jak_memfile_seek_to_end(jak_memfile *file);

/**
 * Moves the contents of the underlying memory block <code>nbytes</code> towards the end of the file.
 * The offset in the memory block from where this move is done is the current position stored in this file.
 * In case of not enough space, the underlying memory block is resized.
 */
bool jak_memfile_inplace_insert(jak_memfile *file, size_t nbytes);
bool jak_memfile_inplace_remove(jak_memfile *file, size_t nbytes_from_here);
bool jak_memfile_end_bit_mode(size_t *num_bytes_written, jak_memfile *file);
void *jak_memfile_current_pos(jak_memfile *file, jak_offset_t nbytes);
bool jak_memfile_hexdump(jak_string *sb, jak_memfile *file);
bool jak_memfile_hexdump_printf(FILE *file, jak_memfile *memfile);
bool jak_memfile_hexdump_print(jak_memfile *memfile);

JAK_END_DECL

#endif/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_OPT_H
#define JAK_OPT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>

typedef struct jak_command_opt {
        char *opt_name;
        char *opt_desc;
        char *opt_manfile;
        int (*callback)(int argc, char **argv, FILE *file);
} jak_command_opt;

typedef struct jak_command_opt_group {
        jak_vector ofType(jak_command_opt) cmd_options;
        char *desc;
} jak_command_opt_group;

typedef enum jak_module_arg_policy {
        JAK_MOD_ARG_REQUIRED, JAK_MOD_ARG_NOT_REQUIRED, JAK_MOD_ARG_MAYBE_REQUIRED,
} jak_module_arg_policy;

typedef struct jak_command_opt_mgr {
        jak_vector ofType(jak_command_opt_group) groups;
        jak_module_arg_policy policy;
        bool (*fallback)(int argc, char **argv, FILE *file, jak_command_opt_mgr *manager);
        char *module_name;
        char *module_desc;
} jak_command_opt_mgr;

bool jak_opt_manager_create(jak_command_opt_mgr *manager, char *module_name, char *module_desc, jak_module_arg_policy policy, bool (*fallback)(int argc, char **argv, FILE *file, jak_command_opt_mgr *manager));
bool jak_opt_manager_drop(jak_command_opt_mgr *manager);

bool jak_opt_manager_process(jak_command_opt_mgr *manager, int argc, char **argv, FILE *file);
bool jak_opt_manager_create_group(jak_command_opt_group **group, const char *desc, jak_command_opt_mgr *manager);
bool opt_group_add_cmd(jak_command_opt_group *group, const char *opt_name, char *opt_desc, char *opt_manfile, int (*callback)(int argc, char **argv, FILE *file));
bool jak_opt_manager_show_help(FILE *file, jak_command_opt_mgr *manager);

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_PACK_H
#define JAK_PACK_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_pack_none.h>
#include <jak_pack_huffman.h>
#include <jak_huffman.h>
#include <jak_stdinc.h>
#include <jak_types.h>

JAK_BEGIN_DECL

/**
 * Unique tag identifying a specific implementation for compressing/decompressing string in a CARBON archives
 * string table.
 */
typedef enum jak_packer_e {
        JAK_PACK_NONE, JAK_PACK_HUFFMAN
} jak_packer_e;

/**
 * Main interface for the compressor framework. A compressor is used to encode/decode strings stored in a
 * CARBON archive.
 */
typedef struct jak_packer {
        /** Tag identifying the implementation of this compressor */
        jak_packer_e tag;

        /** Implementation-specific storage */
        void *extra;

        /**
         * Constructor for implementation-dependent initialization of the compressor at hand.
         *
         * Depending on the implementation, the compressor might allocate dynamic memory for
         * <code>extra</code> for book-keeping purposes.
         *
         * @param self A pointer to itself
         * @return <b>true</b> in case of success, or <b>false</b> otherwise.
         *
         * @author Marcus Pinnecke
         * @since 0.1.00.05
         */
        bool (*create)(jak_packer *self);

        /**
         * Destructor for implementation-dependent deinitialization of the compressor at hand.
         *
         * If the implementation acquired dynamic memory during a call to <code>create</code>,
         * a call to this function frees up this memory.
         *
         * @param self A pointer to itself
         * @return <b>true</b> in case of success, or <b>false</b> otherwise.
         *
         * @author Marcus Pinnecke
         * @since 0.1.00.05
         */
        bool (*drop)(jak_packer *self);

        /**
         * Perform a hard-copy of this compressor to dst
         *
         * @param self  A pointer to itself
         * @param dst   A pointer to the copy target
         *
         * @return <b>true</b> in case of success, or <b>false</b> otherwise.
         *
         * @author Marcus Pinnecke
         * @since 0.1.00.05
         */
        bool (*cpy)(const jak_packer *self, jak_packer *dst);

        /**
         * Function to construct and serialize an implementation-specific dictionary, book-keeping data, or extra data
         * (e.g., a code table)
         *
         * Depending on the implementation, a set of book-keeping data must be managed for a compressor. For instance,
         * in case of a huffman encoded this book-keeping is the code table that maps letters to prefix codes. This
         * function is invoked before any string gets encoded, and implements a compressor-specific management and
         * serialization of that book-keeping data. After internal construction of this book-keeping data,
         * this data is serialized into the <code>dst</code> parameter.
         *
         * Reverse function of <code>read_extra</code>.
         *
         * @note single strings must not be encoded; this is is done when the framework invokes <code>encode_string</code>
         *
         * @param self A pointer to the compressor that is used; maybe accesses <code>extra</code>
         * @param dst A memory file in which the book-keeping data (<code>extra</code>) for this compressor is serialized
         * @param strings The set of all strings that should be encoded. Used for tweaking the compressor;
         *                 <b>not</b> for serialization into <code>dst</code>
         *
         * @note strings in <code>strings</code> are unique (but not sorted)
         *
         * @author Marcus Pinnecke
         * @since 0.1.00.05
         * */
        bool (*write_extra)(jak_packer *self, jak_memfile *dst,
                            const jak_vector ofType (const char *) *strings);

        /**
         * Function to reconstruct implementation-specific dictionary, book-keeping or extra data by deserialization (
         * e.g., a code table)
         *
         * Reverse function of <code>write_extra</code>.
         *
         * @param self A pointer to the compressor that is used; maybe accesses <code>extra</code>
         * @param src A file where the cursor is moved to the first byte of the extra field previously serialized with 'write_extra'
         * @param nbytes Number of bytes written when 'write_extra' was called. Intended to read read to restore the extra field.
         * @return The implementer must return <code>true</code> on success, and <code>false</code> otherwise.
         */
        bool (*read_extra)(jak_packer *self, FILE *src, size_t nbytes);

        /**
         * Encodes an input string and writes its encoded version into a memory file.
         *
         * @param self A pointer to the compressor that is used; maybe accesses <code>extra</code>
         * @param dst A memory file in which the encoded string should be stored
         * @param err An JAK_ERROR information
         * @param string The string that should be encoded
         *
         * @return <b>true</b> in case of success, or <b>false</b> otherwise.
         *
         * @author Marcus Pinnecke
         * @since 0.1.00.05
         */
        bool
        (*encode_string)(jak_packer *self, jak_memfile *dst, jak_error *err, const char *string);

        bool (*decode_string)(jak_packer *self, char *dst, size_t strlen, FILE *src);

        /**
         * Reads implementation-specific book-keeping, meta or extra data from the input memory file and
         * prints its contents in a human-readable version to <code>file</code>
         *
         * @param self A pointer to the compressor that is used; potentially accessing <code>extra</code>
         * @param file A file to which a human-readable version of <code>extra</code> is printed (if any)
         * @param src A memory file which cursor is positioned at the begin of the serialized extra field. After
         *            a call to this function, the memory file cursor must be positioned after the serialized extra
         *            field (i.e., the entire entry must be read (if any))
         *
         * @return <b>true</b> in case of success, or <b>false</b> otherwise.
         *
         * @author Marcus Pinnecke
         * @since 0.1.00.05
         */
        bool (*print_extra)(jak_packer *self, FILE *file, jak_memfile *src);

        /**
         * Reads an implementation-specific encoded string from a memory file <code>src</code>, and prints
         * the encoded string in a human-readable version to <code>file</code>
         *
         * @param self A pointer to the compressor that is used; potentially accessing <code>extra</code>
         * @param file A file to which a human-readable version of the encoded string is printed.
         * @param src A memory file which cursor is positioned at the begin of the encoded string. After a call
         *            to this function, the memory file cursor must be positioned after the encoded string (i.e.,
         *            the entire encoded string must be read)
         * @param decompressed_strlen The length of the decoded string in number of characters
         *
         * @return <b>true</b> in case of success, or <b>false</b> otherwise.
         *
         * @author Marcus Pinnecke
         * @since 0.1.00.05
         */
        bool
        (*print_encoded)(jak_packer *self, FILE *file, jak_memfile *src, jak_u32 decompressed_strlen);
} jak_packer;

static void jak_pack_none_create(jak_packer *strategy)
{
        strategy->tag = JAK_PACK_NONE;
        strategy->create = jak_pack_none_init;
        strategy->cpy = jak_pack_none_cpy;
        strategy->drop = jak_pack_none_drop;
        strategy->write_extra = jak_pack_none_write_extra;
        strategy->read_extra = jak_pack_none_read_extra;
        strategy->encode_string = jak_pack_none_encode_string;
        strategy->decode_string = jak_pack_none_decode_string;
        strategy->print_extra = jak_pack_none_print_extra;
        strategy->print_encoded = jak_pack_none_print_encoded_string;
}

static void jak_pack_huffman_create(jak_packer *strategy)
{
        strategy->tag = JAK_PACK_HUFFMAN;
        strategy->create = jak_pack_huffman_init;
        strategy->cpy = jak_pack_jak_coding_huffman_cpy;
        strategy->drop = jak_pack_jak_coding_huffman_drop;
        strategy->write_extra = jak_pack_huffman_write_extra;
        strategy->read_extra = jak_pack_huffman_read_extra;
        strategy->encode_string = jak_pack_huffman_encode_string;
        strategy->decode_string = jak_pack_huffman_decode_string;
        strategy->print_extra = jak_pack_huffman_print_extra;
        strategy->print_encoded = jak_pack_huffman_print_encoded;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static struct {
        jak_packer_e type;
        const char *name;
        void (*create)(jak_packer *strategy);
        jak_u8 flag_bit;
} jak_global_pack_strategy_register[] =
        {{.type = JAK_PACK_NONE, .name = "none", .create = jak_pack_none_create, .flag_bit = 1 << 0},
         {.type = JAK_PACK_HUFFMAN, .name = "huffman", .create = jak_pack_huffman_create, .flag_bit = 1 << 1}};

#pragma GCC diagnostic pop

bool jak_pack_cpy(jak_error *err, jak_packer *dst, const jak_packer *src);
bool jak_pack_drop(jak_error *err, jak_packer *self);

bool jak_pack_by_type(jak_error *err, jak_packer *strategy, jak_packer_e type);
jak_u8 jak_pack_flagbit_by_type(jak_packer_e type);
bool jak_pack_by_flags(jak_packer *strategy, jak_u8 flags);
bool jak_pack_by_name(jak_packer_e *type, const char *name);

bool jak_pack_write_extra(jak_error *err, jak_packer *self, jak_memfile *dst, const jak_vector ofType (const char *) *strings);
bool jak_pack_read_extra(jak_error *err, jak_packer *self, FILE *src, size_t nbytes);
bool jak_pack_encode(jak_error *err, jak_packer *self, jak_memfile *dst, const char *string);
bool jak_pack_decode(jak_error *err, jak_packer *self, char *dst, size_t strlen, FILE *src);
bool jak_pack_print_extra(jak_error *err, jak_packer *self, FILE *file, jak_memfile *src);
bool jak_pack_print_encoded(jak_error *err, jak_packer *self, FILE *file, jak_memfile *src, jak_u32 decompressed_strlen);

JAK_END_DECL

#endif
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

#ifndef JAK_COMPRESSOR_HUFFMAN_H
#define JAK_COMPRESSOR_HUFFMAN_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>
#include <jak_memfile.h>

JAK_BEGIN_DECL

bool jak_pack_huffman_init(jak_packer *self);
bool jak_pack_jak_coding_huffman_cpy(const jak_packer *self, jak_packer *dst);
bool jak_pack_jak_coding_huffman_drop(jak_packer *self);
bool jak_pack_huffman_write_extra(jak_packer *self, jak_memfile *dst, const jak_vector ofType (const char *) *strings);
bool jak_pack_huffman_read_extra(jak_packer *self, FILE *src, size_t nbytes);
bool jak_pack_huffman_print_extra(jak_packer *self, FILE *file, jak_memfile *src);
bool jak_pack_huffman_print_encoded(jak_packer *self, FILE *file, jak_memfile *src, jak_u32 decompressed_strlen);
bool jak_pack_huffman_encode_string(jak_packer *self, jak_memfile *dst, jak_error *err, const char *string);
bool jak_pack_huffman_decode_string(jak_packer *self, char *dst, size_t strlen, FILE *src);

JAK_END_DECL

#endif
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

#ifndef JAK_COMPRESSOR_NONE_H
#define JAK_COMPRESSOR_NONE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_types.h>
#include <jak_stdinc.h>
#include <jak_vector.h>
#include <jak_memfile.h>

JAK_BEGIN_DECL

bool jak_pack_none_init(jak_packer *self);
bool jak_pack_none_cpy(const jak_packer *self, jak_packer *dst);
bool jak_pack_none_drop(jak_packer *self);
bool jak_pack_none_write_extra(jak_packer *self, jak_memfile *dst, const jak_vector ofType (const char *) *strings);
bool jak_pack_none_read_extra(jak_packer *self, FILE *src, size_t nbytes);
bool jak_pack_none_print_extra(jak_packer *self, FILE *file, jak_memfile *src);
bool jak_pack_none_print_encoded_string(jak_packer *self, FILE *file, jak_memfile *src, jak_u32 decompressed_strlen);
bool jak_pack_none_encode_string(jak_packer *self, jak_memfile *dst, jak_error *err, const char *string);
bool jak_pack_none_decode_string(jak_packer *self, char *dst, size_t strlen, FILE *src);

JAK_END_DECL

#endif
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

#ifndef JAK_STRING_PRED_CONTAINS_H
#define JAK_STRING_PRED_CONTAINS_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_archive_pred.h>

JAK_BEGIN_DECL

JAK_BUILT_IN(static bool) __jak_string_pred_contains_func(size_t *idxs_matching, size_t *num_matching, char **strings, size_t num_strings, void *capture)
{
        size_t result_size = 0;
        const char *needle = (const char *) capture;

        for (size_t i = 0; i < num_strings; i++) {
                if (strstr(strings[i], needle) != NULL) {
                        idxs_matching[result_size++] = i;
                }
        }

        *num_matching = result_size;

        return true;
}

JAK_BUILT_IN(static bool)

jak_string_pred_contains_init(jak_string_pred *pred)
{
        JAK_ERROR_IF_NULL(pred);
        pred->limit = JAK_QUERY_LIMIT_NONE;
        pred->func = __jak_string_pred_contains_func;
        return true;
}

JAK_END_DECL

#endif
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

#ifndef JAK_STRING_PRED_EQUALS_H
#define JAK_STRING_PRED_EQUALS_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_archive_pred.h>

JAK_BEGIN_DECL

JAK_BUILT_IN(static bool) __jak_string_pred_equals_func(size_t *idxs_matching, size_t *num_matching, char **strings, size_t num_strings, void *capture)
{
        size_t result_size = 0;
        const char *needle = (const char *) capture;

        for (size_t i = 0; i < num_strings; i++) {
                if (strstr(strings[i], needle) != NULL) {
                        idxs_matching[result_size++] = i;
                }
        }

        *num_matching = result_size;
        return true;
}

JAK_BUILT_IN(static bool)

jak_string_pred_equals_init(jak_string_pred *pred)
{
        JAK_ERROR_IF_NULL(pred);
        pred->limit = JAK_QUERY_LIMIT_1;
        pred->func = __jak_string_pred_equals_func;
        return true;
}

JAK_END_DECL

#endif
/**
 * Copyright 2019 Marcus Pinnecke, Robert Jendersie, Johannes Wuensche, Johann Wagner, and Marten Wallewein-Eising
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without ion, including without limitation the
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

#ifndef JAK_PRIORITY_QUEUE_H
#define JAK_PRIORITY_QUEUE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>

JAK_BEGIN_DECL

typedef struct jak_priority_queue_element_info {
        size_t priority;
        void *element;
} jak_priority_queue_element_info;

typedef struct jak_priority_queue {
        jak_priority_queue_element_info *data;
        size_t num_elements;
        size_t capacity;
        pthread_mutex_t mutex;
} jak_priority_queue;

void jak_priority_queue_init(jak_priority_queue *queue);
void jak_priority_queue_free(jak_priority_queue *queue);
void jak_priority_queue_push(jak_priority_queue *queue, void *data, size_t priority);
void *jak_priority_queue_pop(jak_priority_queue *queue);
int jak_priority_queue_is_empty(jak_priority_queue *queue);

JAK_END_DECL

#endif/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_SLICELIST_H
#define JAK_SLICELIST_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>
#include <jak_bitmap.h>
#include <jak_spinlock.h>
#include <jak_bloom.h>
#include <jak_hash.h>
#include <jak_types.h>

JAK_BEGIN_DECL

JAK_FORWARD_STRUCT_DECL(jak_slice)

#ifndef JAK_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME
#define JAK_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME "1 of 100 in CPU L1"
#endif
#ifndef JAK_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE
#define JAK_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE (32768/100)
#endif

#ifndef JAK_SLICE_LIST_TARGET_MEMORY_NAME
#define JAK_SLICE_LIST_TARGET_MEMORY_NAME "10 of 100 in CPU L1"
#endif
#ifndef JAK_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE
#define JAK_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE (32768/10)
#endif

#define SLICE_DATA_SIZE (JAK_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE - sizeof(jak_slice_lookup_strat_e) - sizeof(jak_u32))

#define SLICE_KEY_COLUMN_MAX_ELEMS (SLICE_DATA_SIZE / 8 / 3) /** one array with elements of 64 bits each, 3 of them */

typedef enum jak_slice_lookup_strat_e {
        JAK_SLICE_LOOKUP_SCAN, JAK_SLICE_LOOKUP_BESEARCH,
} jak_slice_lookup_strat_e;

typedef struct jak_slice {
        /** Enumeration to determine which strategy for 'find' is currently applied */
        jak_slice_lookup_strat_e strat;

        /** Data stored inside this slice. By setting 'JAK_SLICE_LIST_CPU_L3_SIZE_IN_BYTE' statically to the target
         * CPU L3 size, it is intended that one entire 'JAK_slice_t' structure fits into the L3 cache of the CPU.
         * It is assumed that at least one element can be inserted into a 'JAK_slice_t' object (which means that
         * the type of elements to be inserted must be less or equal to SLICE_DATA_SIZE. In case an element is
         * removed from this list, data is physically moved to avoid a "sparse" list, i.e., it is alwalys
         * guaranteeed that 'data' contains continously elements without any gabs until 'num_elems' limit. This
         * avoids to lookup in a struct bitmap or other structure whether a particular element is removed or not; also
         * this does not steal an element from the domain of the used data type to encode 'not present' with a
         * particular values. However, a remove operation is expensive. */
        const char *key_column[SLICE_KEY_COLUMN_MAX_ELEMS];
        hash32_t key_hash_column[SLICE_KEY_COLUMN_MAX_ELEMS];
        jak_archive_field_sid_t jak_string_id_column[SLICE_KEY_COLUMN_MAX_ELEMS];

        /** The number of elements stored in 'key_colum', 'key_hash_column', and 'jak_string_id_column' */
        jak_u32 num_elems;

        jak_u32 cache_idx;
} jak_slice;

typedef struct jak_hash_bounds {
        /** Min and max values inside this slice. Used to skip the lookup in the per-slice jak_bitmap during search */
        hash32_t min_hash, max_hash;
} jak_hash_bounds;

typedef struct jak_slice_descriptor {
        /** The number of reads to this slice including misses and hits. Along with 'num_reads_hit' used to determine
         * the order of this element w.r.t. to other elements in the list */
        size_t num_reads_all;

        /** The number of reads to this slice that lead to a search hit. See 'num_reads_all' for the purpose. */
        size_t num_reads_hit;

} jak_slice_descriptor;

typedef struct jak_slice_list {
        jak_allocator alloc;
        jak_spinlock lock;

        jak_vector ofType(jak_slice) slices;
        jak_vector ofType(jak_slice_descriptor) descriptors;
        jak_vector ofType(jak_bloomfilter) filters;
        jak_vector ofType(jak_hash_bounds) bounds;

        jak_u32 appender_idx;
        jak_error err;
} jak_slice_list_t;

typedef struct jak_slice_handle {
        jak_slice *container;
        const char *key;
        jak_archive_field_sid_t value;
        bool is_contained;
} jak_slice_handle;

bool jak_slice_list_create(jak_slice_list_t *list, const jak_allocator *alloc, size_t slice_capacity);
bool jak_slice_list_drop(jak_slice_list_t *list);
bool jak_slice_list_lookup(jak_slice_handle *handle, jak_slice_list_t *list, const char *needle);
bool jak_slice_list_is_empty(const jak_slice_list_t *list);
bool jak_slice_list_insert(jak_slice_list_t *list, char **strings, jak_archive_field_sid_t *ids, size_t npairs);
bool jak_slice_list_remove(jak_slice_list_t *list, jak_slice_handle *handle);

JAK_END_DECL

#endif/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_SPINLOCK_H
#define JAK_SPINLOCK_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <stdatomic.h>

#include <jak_stdinc.h>
#include <jak_vector.h>

JAK_BEGIN_DECL

typedef struct jak_spinlock {
        atomic_flag lock;
        pthread_t owner;
} jak_spinlock;

bool jak_spinlock_init(jak_spinlock *spinlock);
bool jak_spinlock_acquire(jak_spinlock *spinlock);
bool jak_spinlock_release(jak_spinlock *spinlock);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_COMMON_H
#define JAK_COMMON_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#include "jak_forwdecl.h"

#ifndef __cplusplus

# include <stdatomic.h>

#else
# include <atomic>
# define _Atomic(X) std::atomic< X >
#endif

#ifdef __cplusplus
#define JAK_BEGIN_DECL  extern "C" {
#define JAK_END_DECL    }
#else
#define JAK_BEGIN_DECL
#define JAK_END_DECL
#endif

#define JAK_MALLOC(size)                \
({                                      \
        void *ptr = malloc(size);       \
        JAK_ZERO_MEMORY(ptr, size);     \
        ptr;                            \
})

#define JAK_QUERY_LIMIT_NONE -1
#define JAK_QUERY_LIMIT_1     1

#define JAK_ARRAY_LENGTH(x)                                                                                            \
    sizeof(x)/sizeof(x[0])

typedef uint64_t jak_offset_t;
typedef int64_t signed_offset_t;

typedef unsigned char u_char;

typedef enum jak_archive_field_type {
        JAK_FIELD_NULL = 0,
        JAK_FIELD_BOOLEAN = 1,
        JAK_FIELD_INT8 = 2,
        JAK_FIELD_INT16 = 3,
        JAK_FIELD_INT32 = 4,
        JAK_FIELD_INT64 = 5,
        JAK_FIELD_UINT8 = 6,
        JAK_FIELD_UINT16 = 7,
        JAK_FIELD_UINT32 = 8,
        JAK_FIELD_UINT64 = 9,
        JAK_FIELD_FLOAT = 10,
        JAK_FIELD_STRING = 11,
        JAK_FIELD_OBJECT = 12
} jak_archive_field_e;

typedef enum jak_access_mode_e {
        JAK_READ_WRITE,
        JAK_READ_ONLY
} jak_access_mode_e;

#define JAK_FUNC_UNUSED __attribute__((unused))

JAK_FUNC_UNUSED static const char *jak_basic_type_to_json_type_str(enum jak_archive_field_type t)
{
        switch (t) {
                case JAK_FIELD_INT8:
                case JAK_FIELD_INT16:
                case JAK_FIELD_INT32:
                case JAK_FIELD_INT64:
                case JAK_FIELD_UINT8:
                case JAK_FIELD_UINT16:
                case JAK_FIELD_UINT32:
                case JAK_FIELD_UINT64:
                        return "integer";
                case JAK_FIELD_FLOAT:
                        return "float";
                case JAK_FIELD_STRING:
                        return "string";
                case JAK_FIELD_BOOLEAN:
                        return "boolean";
                case JAK_FIELD_NULL:
                        return "null";
                case JAK_FIELD_OBJECT:
                        return "object";
                default:
                        return "(unknown)";
        }
}

JAK_FUNC_UNUSED static const char *jak_basic_type_to_system_type_str(enum jak_archive_field_type t)
{
        switch (t) {
                case JAK_FIELD_INT8:
                        return "int8";
                case JAK_FIELD_INT16:
                        return "int16";
                case JAK_FIELD_INT32:
                        return "int32";
                case JAK_FIELD_INT64:
                        return "int64";
                case JAK_FIELD_UINT8:
                        return "uint8";
                case JAK_FIELD_UINT16:
                        return "uint16";
                case JAK_FIELD_UINT32:
                        return "uint32";
                case JAK_FIELD_UINT64:
                        return "uint64";
                case JAK_FIELD_FLOAT:
                        return "float32";
                case JAK_FIELD_STRING:
                        return "string64";
                case JAK_FIELD_BOOLEAN:
                        return "bool8";
                case JAK_FIELD_NULL:
                        return "void";
                case JAK_FIELD_OBJECT:
                        return "variable";
                default:
                        return "(unknown)";
        }
}

#define JAK_NOT_IMPLEMENTED                                                                                            \
{                                                                                                                      \
    jak_error err;                                                                                                    \
    jak_error_init(&err);                                                                                                  \
    JAK_ERROR(&err, JAK_ERR_NOTIMPLEMENTED)                                                                                \
    jak_error_print_and_abort(&err);                                                                                       \
    return false;                                                                                                      \
};

#ifndef NDEBUG
#define JAK_CHECK_TAG(is, expected)                                                                                 \
{                                                                                                                      \
    if (is != expected) {                                                                                              \
        JAK_ERROR_PRINT(JAK_ERR_ERRINTERNAL)                                                                     \
        return false;                                                                                                  \
    }                                                                                                                  \
}
#else
#define JAK_CHECK_TAG(is, expected) { }
#endif

#if !defined(JAK_LOG_TRACE) || defined(NDEBUG)
#define JAK_TRACE(tag, msg, ...) { }
#else
#define JAK_TRACE(tag, msg, ...)                                                                                    \
{                                                                                                                      \
    char buffer[1024];                                                                                                 \
    sprintf(buffer, "--%d-- [TRACE   : %-10s] %s\n", getpid(), tag, msg);                                              \
    fprintf(stderr, buffer, __VA_ARGS__);                                                                              \
    fflush(stderr);                                                                                                    \
}
#endif

#if !defined(JAK_LOG_INFO) || defined(NDEBUG)
#define JAK_INFO(tag, msg, ...) { }
#else
#define JAK_INFO(tag, msg, ...)                                                                                     \
{                                                                                                                      \
    char buffer[1024];                                                                                                 \
    sprintf(buffer, "--%d-- [INFO    : %-10s] %s\n", getpid(), tag, msg);                                              \
    fprintf(stderr, buffer, __VA_ARGS__);                                                                              \
    fflush(stderr);                                                                                                    \
}
#endif

#if !defined(JAK_LOG_DEBUG) || defined(NDEBUG)
#define JAK_DEBUG(tag, msg, ...)                                                                                       \
{ }
#else
#define JAK_DEBUG(tag, msg, ...)                                                                                    \
{                                                                                                                      \
    char buffer[1024];                                                                                                 \
    sprintf(buffer, "--%d-- [DEBUG   : %-10s] %s\n", getpid(), tag, msg);                                              \
    fprintf(stderr, buffer, __VA_ARGS__);                                                                              \
    fflush(stderr);                                                                                                    \
}
#endif

#if !defined(JAK_LOG_WARN) || defined(NDEBUG)
#define JAK_WARN(tag, msg, ...) { }
#else
#define JAK_WARN(tag, msg, ...)                                                                                     \
    {                                                                                                                  \
        char buffer[1024];                                                                                             \
        sprintf(buffer, "--%d-- [WARNING: %-10s] %s\n", getpid(), tag, msg);                                           \
        fprintf(stderr, buffer, __VA_ARGS__);                                                                          \
        fflush(stderr);                                                                                                \
    }
#endif

#define JAK_ASSERT(x) assert(x);

#define JAK_CARBON_ARCHIVE_MAGIC                "MP/CARBON"
#define JAK_CARBON_ARCHIVE_VERSION               1

#define  JAK_MARKER_SYMBOL_OBJECT_BEGIN        '{'
#define  JAK_MARKER_SYMBOL_OBJECT_END          '}'
#define  JAK_MARKER_SYMBOL_PROP_NULL           'n'
#define  JAK_MARKER_SYMBOL_PROP_BOOLEAN        'b'
#define  JAK_MARKER_SYMBOL_PROP_INT8           'c'
#define  JAK_MARKER_SYMBOL_PROP_INT16          's'
#define  JAK_MARKER_SYMBOL_PROP_INT32          'i'
#define  JAK_MARKER_SYMBOL_PROP_INT64          'l'
#define  JAK_MARKER_SYMBOL_PROP_UINT8          'r'
#define  JAK_MARKER_SYMBOL_PROP_UINT16         'h'
#define  JAK_MARKER_SYMBOL_PROP_UINT32         'e'
#define  JAK_MARKER_SYMBOL_PROP_UINT64         'g'
#define  JAK_MARKER_SYMBOL_PROP_REAL           'f'
#define  JAK_MARKER_SYMBOL_PROP_TEXT           't'
#define  JAK_MARKER_SYMBOL_PROP_OBJECT         'o'
#define  JAK_MARKER_SYMBOL_PROP_NULL_ARRAY     'N'
#define  JAK_MARKER_SYMBOL_PROP_BOOLEAN_ARRAY  'B'
#define  JAK_MARKER_SYMBOL_PROP_INT8_ARRAY     'C'
#define  JAK_MARKER_SYMBOL_PROP_INT16_ARRAY    'S'
#define  JAK_MARKER_SYMBOL_PROP_INT32_ARRAY    'I'
#define  JAK_MARKER_SYMBOL_PROP_INT64_ARRAY    'L'
#define  JAK_MARKER_SYMBOL_PROP_UINT8_ARRAY    'R'
#define  JAK_MARKER_SYMBOL_PROP_UINT16_ARRAY   'H'
#define  JAK_MARKER_SYMBOL_PROP_UINT32_ARRAY   'E'
#define  JAK_MARKER_SYMBOL_PROP_UINT64_ARRAY   'G'
#define  JAK_MARKER_SYMBOL_PROP_REAL_ARRAY     'F'
#define  JAK_MARKER_SYMBOL_PROP_TEXT_ARRAY     'T'
#define  JAK_MARKER_SYMBOL_PROP_OBJECT_ARRAY   'O'
#define  JAK_MARKER_SYMBOL_EMBEDDED_STR_DIC    'D'
#define  JAK_MARKER_SYMBOL_EMBEDDED_STR        '-'
#define  JAK_MARKER_SYMBOL_COLUMN_GROUP        'X'
#define  JAK_MARKER_SYMBOL_COLUMN              'x'
#define  JAK_MARKER_SYMBOL_HUFFMAN_DIC_ENTRY   'd'
#define  JAK_MARKER_SYMBOL_RECORD_HEADER       'r'
#define  JAK_MARKER_SYMBOL_HASHTABLE_HEADER    '#'
#define  JAK_MARKER_SYMBOL_VECTOR_HEADER       '|'

#define JAK_DECLARE_AND_INIT(type, name)                                                                               \
        type name;                                                                                                     \
        JAK_ZERO_MEMORY(&name, sizeof(type));

#define JAK_ZERO_MEMORY(dst, len)                                                                                      \
    memset((void *) dst, 0, len);

#define JAK_cast(type, name, src)                                                                                      \
      type name = (type) src

#define JAK_UNUSED(x)   (void)(x);

#define JAK_BUILT_IN(x)   JAK_FUNC_UNUSED x

#define ofType(x) /** a convenience way to write types for generic containers; no effect than just a visual one */
#define ofMapping(x, y) /** a convenience way to write types for generic containers; no effect than just a visual one */

#define JAK_OPTIONAL_CALL(x, func, ...) if((x) && (x)->func) { (x)->func(__VA_ARGS__); }

#define JAK_MAX(a, b)                                                                                                  \
    ((b) > (a) ? (b) : (a))

#define JAK_MIN(a, b)                                                                                                  \
    ((a) < (b) ? (a) : (b))

#define JAK_ERROR_IF_NULL(x)                                                                                               \
{                                                                                                                      \
    if (!(x)) {                                                                                                        \
        jak_error err;                                                                                                \
        jak_error_init(&err);                                                                                              \
        JAK_ERROR(&err, JAK_ERR_NULLPTR);                                                                                  \
        jak_error_print_to_stderr(&err);                                                                                   \
        return false;                                                                                                  \
    }                                                                                                                  \
}

#define JAK_CHECK_SUCCESS(x)                                                                                           \
{                                                                                                                      \
    if (JAK_UNLIKELY(!x)) {                                                                                                \
        return x;                                                                                                      \
    }                                                                                                                  \
}

#define JAK_SUCCESS_OR_JUMP(expr, label)                                                                               \
{                                                                                                                      \
    if (JAK_UNLIKELY(!expr)) {                                                                                             \
        goto label;                                                                                                    \
    }                                                                                                                  \
}

#define JAK_LIKELY(x)                                                                                                      \
    __builtin_expect((x), 1)
#define JAK_UNLIKELY(x)                                                                                                    \
    __builtin_expect((x), 0)

#define JAK_PREFETCH_READ(adr)                                                                                             \
    __builtin_prefetch(adr, 0, 3)

#define JAK_PREFETCH_WRITE(adr)                                                                                            \
    __builtin_prefetch(adr, 1, 3)

#define JAK_FORWARD_STRUCT_DECL(x) struct x;

#define JAK_BIT_NUM_OF(x)             (sizeof(x) * 8)
#define JAK_SET_BIT(n)                ( ((jak_u32) 1) << (n) )
#define JAK_SET_BITS(x, mask)         ( x |=  (mask) )
#define JAK_UNSET_BITS(x, mask)       ( x &= ~(mask) )
#define JAK_ARE_BITS_SET(mask, bit)   (((bit) & mask ) == (bit))

#define JAK_ERROR_IF_NOT_IMPLEMENTED(err, x, func)                                                                         \
    JAK_OPTIONAL(x->func == NULL, JAK_ERROR(err, JAK_ERR_NOTIMPLEMENTED))

#define JAK_OPTIONAL(expr, stmt)                                                                                       \
    if (expr) { stmt; }

#define JAK_OPTIONAL_SET(x, y)                                                                                         \
     JAK_OPTIONAL(x, *x = y)

#define JAK_OPTIONAL_SET_OR_ELSE(x, y, stmt)                                                                           \
    if (x) {                                                                                                           \
        *x = y;                                                                                                        \
    } else { stmt; }

bool jak_global_console_enable_output;

#define JAK_CONSOLE_OUTPUT_ON()                                                                                        \
    jak_global_console_enable_output = true;

#define JAK_CONSOLE_OUTPUT_OFF()                                                                                       \
    jak_global_console_enable_output = false;

#define JAK_CONSOLE_WRITE(file, msg, ...)                                                                              \
{                                                                                                                      \
    if (jak_global_console_enable_output) {                                                                                   \
        pid_t pid = getpid();                                                                                          \
        char timeBuffer[2048];                                                                                         \
        char formatBuffer[2048];                                                                                       \
        time_t now = time (0);                                                                                         \
        fflush(file);                                                                                                  \
        strftime (timeBuffer, 2048, "%Y-%m-%d %H:%M:%S", localtime (&now));                                            \
        sprintf (formatBuffer, msg, __VA_ARGS__);                                                                      \
        fprintf(file, "[%d] %s   %-70s", pid, timeBuffer, formatBuffer);                                               \
        fflush(file);                                                                                                  \
    }                                                                                                                  \
}

#define JAK_CONSOLE_WRITE_ENDL(file)                                                                                   \
{                                                                                                                      \
    if (jak_global_console_enable_output) {                                                                                   \
        fprintf(file, "\n");                                                                                           \
    }                                                                                                                  \
}

#define JAK_CONSOLE_WRITE_CONT(file, msg, ...)                                                                         \
{                                                                                                                      \
    if (jak_global_console_enable_output) {                                                                                   \
        fprintf(file, msg, __VA_ARGS__);                                                                               \
    }                                                                                                                  \
}

#define JAK_CONSOLE_WRITELN(file, msg, ...)                                                                            \
{                                                                                                                      \
    if (jak_global_console_enable_output) {                                                                                   \
        JAK_CONSOLE_WRITE(file, msg, __VA_ARGS__)                                                                      \
        JAK_CONSOLE_WRITE_ENDL(file)                                                                                   \
        fflush(file);                                                                                                  \
    }                                                                                                                  \
}

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

// ---------------------------------------------------------------------------------------------------------------------
//
//  S U M M A R Y
//
// ---------------------------------------------------------------------------------------------------------------------

/**
 * SUMMARY
 *
 * A specialized hash table that uses strings as keys, and 64bit values. The specialization is to avoid some
 * indirection cost to compare keys, e.g., by calling a function pointer to compare two objects like clib suggest it,
 * and to ensure that the value is embedded in continuous memory rather than a pointer to another distant memory block.
 *
 * Internally, the str_hash is organized by paritions where each partition is assigned exclusively to
 * one thread. Each such parition contains of as a vector of length 'num_buckets' which contains of elements of a bucket
 * type. A bucket type is an fixed-size array of entries, each containing a key and a value. In case of no collisions,
 * this array contains of exactly one element. In case of collisions, colliding keys are stored in this array of entries
 * in the same bucket. To speedup lookups, this entry vector may be additionally sorted and a (specialized) binary
 * search is invoked to find the bucket entry associated to a particular key (if any). Other lookup strategies includes
 * both single and multi-threaded forward scans. Which strategy to use in which case is decided by the
 * str_hash itself, however. To satisfy user-specific memory limitations, some per-bucket elements may
 * be swapped out to a didicated swap space.
 *
 * The underlying hashing function is the Jenkins hash function.
 */

#ifndef JAK_STRHASH_H
#define JAK_STRHASH_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_alloc.h>
#include <jak_vector.h>
#include <jak_hash.h>
#include <jak_types.h>

JAK_BEGIN_DECL

/**
 * Enables or disabled packing of entries inside a bucket. By default, packing is disabled.
 * To turn on packing, set 'JAK_CONFIG_JAK_PACK_BUCKETS' symbol
 */
#ifdef JAK_CONFIG_JAK_PACK_BUCKETS
#define
#define JAK_BUCKET_PACKING __attribute__((__packed__))
#else
#define JAK_BUCKET_PACKING
#endif

/**
 * Number of elements stored in the per-bucket cache (Tier 2, see below)
 */
#ifndef JAK_CONFIG_BUCKET_CACHE_SIZE
#define JAK_CONFIG_BUCKET_CACHE_SIZE  16
#endif

/**
 * Maximum number of elements stored per bucket (Tier 3, see below)
 */
#ifndef JAK_CONFIG_BUCKET_CAPACITY
#define JAK_CONFIG_BUCKET_CAPACITY  1024
#endif

typedef enum jak_str_hash_tag {
        JAK_MEMORY_RESIDENT
} jak_str_hash_tag_e;

typedef struct jak_str_hash_counters {
        size_t num_bucket_search_miss;
        size_t num_bucket_search_hit;
        size_t num_bucket_cache_search_miss;
        size_t num_bucket_cache_search_hit;
} jak_str_hash_counters;

typedef struct jak_str_hash {
        /**
         * Implementation-specific values
         */
        void *extra;

        /**
         * Implementation tag
         */
        jak_str_hash_tag_e tag;

        /**
         * Statistics to lookup misses and hits
         *
         * <b>Note</b>: Implementation must maintain counters by itself
         */
        jak_str_hash_counters counters;

        /**
        *  Memory allocator that is used to get memory for user data
        */
        jak_allocator allocator;

        /**
         *  Frees resources bound to <code>self</code> via the allocator specified by the constructor
         */
        int (*drop)(jak_str_hash *self);

        /**
         * Put <code>num_pair</code> objects into this str_hash maybe updating old objects with the same key.
         */
        int (*put_bulk_safe)(jak_str_hash *self, char *const *keys, const jak_archive_field_sid_t *values, size_t npairs);

        /**
         * Put <code>num_pair</code> objects into this str_hash maybe without checking for updates.
         */
        int (*put_bulk_fast)(jak_str_hash *self, char *const *keys, const jak_archive_field_sid_t *values, size_t npairs);

        /**
         * Same as 'put_safe_bulk' but specialized for a single element
         */
        int (*put_exact_safe)(jak_str_hash *self, const char *key, jak_archive_field_sid_t value);

        /**
         * Same as 'put_fast_bulk' but specialized for a single element
         */
        int (*put_exact_fast)(jak_str_hash *self, const char *key, jak_archive_field_sid_t value);

        /**
         * Get the values associated with <code>keys</code> in this str_hash (if any).
         */
        int (*get_bulk_safe)(jak_str_hash *self, jak_archive_field_sid_t **out, bool **found_mask, size_t *nnot_found, char *const *keys, size_t nkeys);

        /**
         * The same as 'get_safe_bulk' but optimized for a single element
         */
        int (*get_exact_safe)(jak_str_hash *self, jak_archive_field_sid_t *out, bool *found_mask, const char *key);

        /**
         * Get the values associated with <code>keys</code> in this str_hash. All keys <u>must</u> exist.
         */
        int (*get_fast)(jak_str_hash *self, jak_archive_field_sid_t **out, char *const *keys, size_t nkeys);

        /**
         * Updates keys associated with <code>values</code> in this str_hash. All values <u>must</u> exist, and the
         * mapping between keys and values must be bidirectional.
         */
        int (*update_key_fast)(jak_str_hash *self, const jak_archive_field_sid_t *values, char *const *keys, size_t nkeys);

        /**
         * Removes the objects with the gives keys from this str_hash
         */
        int (*remove)(jak_str_hash *self, char *const *keys, size_t nkeys);

        /**
         * Frees up allocated memory for <code>ptr</code> via the allocator in <code>str_hash</code> that was specified
         * by the call to <code>jak_string_id_map_create</code>
         */
        int (*free)(jak_str_hash *self, void *ptr);

        /**
         *  Error information
         */
        jak_error err;
} jak_str_hash;

JAK_DEFINE_GET_ERROR_FUNCTION(jak_str_hash, jak_str_hash, table);

/**
 * Frees resources bound to <code>str_hash</code> via the allocator specified by the call to <code>jak_string_id_map_create</code>.
 *
 * @param str_hash a non-null pointer to the str_hash
 * @return <code>true</code> in case of success, otherwise a value indiciating the JAK_ERROR.
 */
inline static int jak_str_hash_drop(jak_str_hash *str_hash)
{
        JAK_ERROR_IF_NULL(str_hash);
        JAK_ASSERT(str_hash->drop);
        return str_hash->drop(str_hash);
}

/**
 * Resets statistics counters
 *
 * @param str_hash a non-null pointer to the str_hash
 * @return <code>true</code> in case of success, otherwise a value indicating the JAK_ERROR.
 */
inline static bool jak_str_hash_reset_counters(jak_str_hash *str_hash)
{
        JAK_ERROR_IF_NULL(str_hash);
        memset(&str_hash->counters, 0, sizeof(jak_str_hash_counters));
        return true;
}

/**
 * Returns statistics counters
 * @param out non-null pointer to destination counter
 * @param str_hash non-null pointer to the str_hash
 * @return <code>true</code> in case of success, otherwise a value indicating the JAK_ERROR.
 */
inline static int jak_str_hash_get_counters(jak_str_hash_counters *out, const jak_str_hash *str_hash)
{
        JAK_ERROR_IF_NULL(str_hash);
        JAK_ERROR_IF_NULL(out);
        *out = str_hash->counters;
        return true;
}

/**
 * Put <code>num_pair</code> objects into this str_hash maybe updating old objects with the same key. If it is
 * guaranteed that the key is not yet inserted into this table, use <code>jak_string_jak_hashtable_put_blind</code>
 * instead.
 *
 * @param str_hash a non-null pointer to the str_hash
 * @param keys a non-null constant pointer to a list of at least <code>num_pairs</code> length of constant strings
 * @param values a non-null constant pointer to a list of at least <code>num_pairs</code> length of 64bit values
 * @param num_pairs the number of pairs that are read via <code>keys</code> and <code>values</code>
 * @return <code>true</code> in case of success, otherwise a value indicating the JAK_ERROR.
 */
inline static int jak_str_hash_put_safe(jak_str_hash *str_hash, char *const *keys, const jak_archive_field_sid_t *values, size_t npairs)
{
        JAK_ERROR_IF_NULL(str_hash);
        JAK_ERROR_IF_NULL(keys);
        JAK_ERROR_IF_NULL(values);
        JAK_ASSERT(str_hash->put_bulk_safe);
        return str_hash->put_bulk_safe(str_hash, keys, values, npairs);
}

/**
 * Put <code>num_pair</code> objects into this str_hash, ingoring whether the key exists or not. This function is
 * useful for insert operations of pairs where it is guaranteed that the keys are not yet inserted into this hashtable.
 * In case this guarantee is broken, the behavior is undefined. Depending on the implementation, this specialized
 * <code>put</code> function may have a better performance.
 *
 * If a check for existence is required, use <code>jak_string_jak_hashtable_put_test</code>
 * instead.
 *
 * @param str_hash a non-null pointer to the str_hash
 * @param keys a non-null constant pointer to a list of at least <code>num_pairs</code> length of constant strings
 * @param values a non-null constant pointer to a list of at least <code>num_pairs</code> length of 64bit values
 * @param num_pairs the number of pairs that are read via <code>keys</code> and <code>values</code>
 * @return <code>true</code> in case of success, otherwise a value indiciating the JAK_ERROR.
 */
inline static int jak_str_hash_put_bulk_fast(jak_str_hash *str_hash, char *const *keys, const jak_archive_field_sid_t *values, size_t npairs)
{
        JAK_ERROR_IF_NULL(str_hash);
        JAK_ERROR_IF_NULL(keys);
        JAK_ERROR_IF_NULL(values);
        JAK_ASSERT(str_hash->put_bulk_fast);
        return str_hash->put_bulk_fast(str_hash, keys, values, npairs);
}

/**
 * Same as 'jak_string_lookup_put_bulk' but specialized for a single pair
 */
inline static int jak_str_hash_put_exact(jak_str_hash *str_hash, const char *key, jak_archive_field_sid_t value)
{
        JAK_ERROR_IF_NULL(str_hash);
        JAK_ERROR_IF_NULL(key);
        JAK_ASSERT(str_hash->put_exact_safe);
        return str_hash->put_exact_safe(str_hash, key, value);
}

/**
 * Same as 'jak_string_lookup_put_fast_bulk' but specialized for a single pair
 */
inline static int jak_str_hash_put_exact_fast(jak_str_hash *str_hash, const char *key, jak_archive_field_sid_t value)
{
        JAK_ERROR_IF_NULL(str_hash);
        JAK_ERROR_IF_NULL(key);
        JAK_ASSERT(str_hash->put_exact_fast);
        return str_hash->put_exact_fast(str_hash, key, value);
}

/**
 * Get the values associated with <code>keys</code> in this str_hash (if any). In case one <code>key</code> does not
 * exists, the function will return this information via the parameters <code>found_mask</code> and
 * <code>num_not_found</code>. However, in case it is guaranteed that all keys exist, consider to use
 * <code>jak_string_id_map_get_blind</code> instead. *
 *
 * @param out A non-null pointer to an unallocated memory address. The str_hash will allocate enough memory to store the
 *            result. There are <code>num_keys</code> elements returned, but not all of them are guaranteed to
 *            contain a particular value. That an entry does not contain a particular value happens if the
 *            associated key is not stored in this str_hash. Whether or not one particular entry is a valid value,
 *            can be determined by the caller via the <code>found_mask</code>.
 *            <b>Important</b> <code>out</code> must be freed manually by calling <code>jak_string_id_map_free</code>.
 * @param found_mask A non-null pointer to an unallocated memory address. The str_hash will allocate enough memory to store
 *            the result. There are <code>num_keys</code> boolean values returned. This mask is used to determine
 *            if the i-th key has a mapping in this str_hash. If this is the case, the i-th entry in <code>found_mask</code>
 *            is <b>true</b> and the i-th entry in <code>out</code> holds the value. Otherwise, in case the i-th
 *            value in <code>found_mask</code> is <b>false</b>, there is no value stored to the i-th key in
 *            <code>keys</code>, and reading <code>out</code> for the i-th position is undefined.
 * @param num_not_found A non-null pointer to a value that will store the number of keys in <code>keys</code> for
 *                      which no value is stored in this str_hash.
 * @param num_out A non-null pointer to an unsigned integer that will contain the number of values return by the
 *                call to this function.
 * @param str_hash a non-null pointer to the str_hash
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>true</code> in case of success, otherwise a value indicating the JAK_ERROR.
 */
inline static int jak_str_hash_get_bulk_safe(jak_archive_field_sid_t **out, bool **found_mask, size_t *num_not_found, jak_str_hash *str_hash, char *const *keys, size_t nkeys)
{
        JAK_ERROR_IF_NULL(out);
        JAK_ERROR_IF_NULL(found_mask);
        JAK_ERROR_IF_NULL(num_not_found);
        JAK_ERROR_IF_NULL(str_hash);
        JAK_ERROR_IF_NULL(keys);
        JAK_ASSERT(str_hash->get_bulk_safe);
        int result = str_hash->get_bulk_safe(str_hash, out, found_mask, num_not_found, keys, nkeys);
        JAK_ASSERT (out != NULL);
        JAK_ASSERT (found_mask != NULL);
        return result;
}

inline static int jak_str_hash_get_bulk_safe_exact(jak_archive_field_sid_t *out, bool *found, jak_str_hash *str_hash, const char *key)
{
        JAK_ERROR_IF_NULL(out);
        JAK_ERROR_IF_NULL(found);
        JAK_ERROR_IF_NULL(str_hash);
        JAK_ERROR_IF_NULL(key);
        JAK_ASSERT(str_hash->get_exact_safe);
        int result = str_hash->get_exact_safe(str_hash, out, found, key);
        JAK_ASSERT (out != NULL);
        JAK_ASSERT (found != NULL);
        return result;
}

/**
 * Get the values associated with <code>keys</code> in this str_hash. In case one <code>key</code> does not
 * exists, the behavior is undefined.
 *
 * However, if it cannot be guaranteed that all keys are known, use
 * <code>jak_string_id_map_get_test</code> instead.
 *
 * @param out A non-null pointer to an unallocated memory address. The str_hash will allocate <code>num_keys</code>
 *            times <code>sizeof(jak_archive_field_sid_t)</code> bytes memory to store the result. There are <code>num_keys</code>
 *            elements returned, and all of them are guaranteed to contain a particular value.
 * @param str_hash a non-null pointer to the str_hash
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>true</code> in case of success, otherwise a value indicating the JAK_ERROR.
 */
inline static int jak_str_hash_get_bulk_fast(jak_archive_field_sid_t **out, jak_str_hash *str_hash, char *const *keys, size_t nkeys)
{
        JAK_ERROR_IF_NULL(out);
        JAK_ERROR_IF_NULL(str_hash);
        JAK_ERROR_IF_NULL(keys);
        JAK_ASSERT(str_hash->get_fast);
        return str_hash->get_fast(str_hash, out, keys, nkeys);
}

/**
 * Update keys for a given list of values. It must be guaranteed that the mapping between a key and its value is
 * bidirectional, and that all values exists.
 *
 * If you want to update a value given its key, use <code>jak_string_jak_hashtable_put_test</code> or
 * <code>jak_string_jak_hashtable_put_blind</code> instead.
 *
 * @param out A non-null pointer to an unallocated memory address. The str_hash will allocate <code>num_keys</code>
 *            times <code>sizeof(jak_archive_field_sid_t)</code> bytes memory to store the result. There are <code>num_keys</code>
 *            elements returned, and all of them are guaranteed to contain a particular value.
 * @param str_hash a non-null pointer to the str_hash
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>true</code> in case of success, otherwise a value indicating the JAK_ERROR.
 */
inline static int jak_str_hash_update_fast(jak_str_hash *str_hash, const jak_archive_field_sid_t *values, char *const *keys, size_t nkeys)
{
        JAK_ERROR_IF_NULL(str_hash);
        JAK_ERROR_IF_NULL(keys);
        JAK_ASSERT(str_hash->update_key_fast);
        return str_hash->update_key_fast(str_hash, values, keys, nkeys);
}

/**
 * Removes the objects with the gives keys from this str_hash
 *
 * @param str_hash a non-null pointer to the str_hash
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return
 */
inline static int jak_str_hash_remove(jak_str_hash *str_hash, char *const *keys, size_t nkeys)
{
        JAK_ERROR_IF_NULL(str_hash);
        JAK_ERROR_IF_NULL(keys);
        JAK_ASSERT(str_hash->remove);
        return str_hash->remove(str_hash, keys, nkeys);
}

/**
 * Frees up allocated memory for <code>values</code> via the allocator in <code>str_hash</code> that was specified
 * by the call to <code>jak_string_id_map_create</code>
 *
 * @param values A non-null pointer (potentially resulting from a call to <code>jak_string_id_map_get</code>)
 * @return <code>true</code> in case of success, otherwise a value indiciating the JAK_ERROR.
 */
inline static int jak_str_hash_free(void *ptr, jak_str_hash *str_hash)
{
        JAK_ERROR_IF_NULL(ptr);
        JAK_ERROR_IF_NULL(str_hash);
        JAK_ASSERT(str_hash->free);
        return str_hash->free(str_hash, ptr);
}

/**
 * Resets the counter <code>counters</code> by setting all members to zero.
 *
 * @param counters non-null pointer to counter object
 * @return true if everything went normal, otherwise an value indicating the JAK_ERROR
 */
inline static int jak_str_hash_counters_init(jak_str_hash_counters *counters)
{
        JAK_ERROR_IF_NULL(counters);
        memset(counters, 0, sizeof(jak_str_hash_counters));
        return true;
}

/**
 * Adds members of both input parameters and stores the result in <code>dstLhs</code>.
 *
 * @param dstLhs non-null pointer to counter (will contain the result)
 * @param rhs non-null pointer to counter
 * @return true if everything went normal, otherwise an value indicating the JAK_ERROR
 */
inline static int jak_str_hash_counters_add(jak_str_hash_counters *dst_lhs, const jak_str_hash_counters *rhs)
{
        JAK_ERROR_IF_NULL(dst_lhs);
        JAK_ERROR_IF_NULL(rhs);
        dst_lhs->num_bucket_search_miss += rhs->num_bucket_search_miss;
        dst_lhs->num_bucket_search_hit += rhs->num_bucket_search_hit;
        dst_lhs->num_bucket_cache_search_hit += rhs->num_bucket_cache_search_hit;
        dst_lhs->num_bucket_cache_search_miss += rhs->num_bucket_cache_search_miss;
        return true;
}

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_STRHASH_MEM_H
#define JAK_STRHASH_MEM_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_alloc.h>
#include <jak_str_hash.h>

JAK_BEGIN_DECL

bool jak_str_hash_create_inmemory(jak_str_hash *str_hash, const jak_allocator *alloc, size_t num_buckets, size_t cap_buckets);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_STRDIC_H
#define JAK_STRDIC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_alloc.h>
#include <jak_types.h>
#include <jak_hash.h>
#include <jak_vector.h>

JAK_BEGIN_DECL

JAK_FORWARD_STRUCT_DECL(StringDictionary)
JAK_FORWARD_STRUCT_DECL(Vector)

typedef enum jak_str_dict_tag_e {
        JAK_SYNC, JAK_ASYNC
} jak_str_dict_tag_e;

/**
 * Thread-safe string pool implementation
 */
typedef struct jak_string_dict {
        /**
         * Implementation-specific fields
         */
        void *extra;

        /**
         * Tag determining the current implementation
         */
        jak_str_dict_tag_e tag;

        /**
         * Memory allocator that is used to get memory for user data
         */
        jak_allocator alloc;

        /**
         * Frees up implementation-specific resources.
         *
         * Note: Implementation must ensure thread-safeness
         */
        bool (*drop)(jak_string_dict *self);

        /**
         * Inserts a particular number of strings into this dictionary and returns associated string identifiers.
         *
         * Note: Implementation must ensure thread-safeness
        */
        bool (*insert)(jak_string_dict *self, jak_archive_field_sid_t **out, char *const *strings, size_t nstrings, size_t nthreads);

        /**
         * Removes a particular number of strings from this dictionary by their ids. The caller must ensure that
         * all string identifiers in <code>strings</code> are valid.
         *
         * Note: Implementation must ensure thread-safeness
         */
        bool (*remove)(jak_string_dict *self, jak_archive_field_sid_t *strings, size_t nstrings);

        /**
         * Get the string ids associated with <code>keys</code> in this jak_async_map_exec (if any).
         *
         * Note: Implementation must ensure thread-safeness
         */
        bool (*locate_safe)(jak_string_dict *self, jak_archive_field_sid_t **out, bool **found_mask, size_t *num_not_found, char *const *keys, size_t num_keys);

        /**
         * Get the string ids associated with <code>keys</code> in this dic. All keys <u>must</u> exist.
         *
         * Note: Implementation must ensure thread-safeness
        */
        bool (*locate_fast)(jak_string_dict *self, jak_archive_field_sid_t **out, char *const *keys, size_t num_keys);

        /**
         * Extracts strings given their string identifier. All <code>ids</code> must be known.
         *
         * Note: Implementation must ensure thread-safeness
         */
        char **(*extract)(jak_string_dict *self, const jak_archive_field_sid_t *ids, size_t num_ids);

        /**
         * Frees up memory allocated inside a function call via the allocator given in the constructor
         *
         * Note: Implementation must ensure thread-safeness
         */
        bool (*free)(jak_string_dict *self, void *ptr);

        /**
         * Reset internal statistic counters
         */
        bool (*resetCounters)(jak_string_dict *self);

        /**
         * Get internal statistic counters
         */
        bool (*counters)(jak_string_dict *self, jak_str_hash_counters *counters);

        /**
         * Returns number of distinct strings stored in the dictionary
         */
        bool (*num_distinct)(jak_string_dict *self, size_t *num);

        /**
         * Returns all contained (unique) strings and their mapped (unique) ids
         */
        bool (*get_contents)(jak_string_dict *self, jak_vector ofType (char *) *strings, jak_vector ofType(jak_archive_field_sid_t) *jak_string_ids);
} jak_string_dict;

/**
 *
 * @param dic
 * @return
 */
static JAK_BUILT_IN(bool) jak_string_dict_drop(jak_string_dict *dic)
{
        JAK_ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->drop);
        return dic->drop(dic);
}

static JAK_BUILT_IN(bool)
jak_string_dict_insert(jak_string_dict *dic, jak_archive_field_sid_t **out, char *const *strings, size_t nstrings, size_t nthreads)
{
        JAK_ERROR_IF_NULL(dic);
        JAK_ERROR_IF_NULL(strings);
        JAK_ASSERT(dic->insert);
        return dic->insert(dic, out, strings, nstrings, nthreads);
}

static JAK_BUILT_IN(bool)  jak_string_dict_reset_counters(jak_string_dict *dic)
{
        JAK_ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->resetCounters);
        return dic->resetCounters(dic);
}

static JAK_BUILT_IN(bool)  jak_string_dict_get_counters(jak_str_hash_counters *counters, jak_string_dict *dic)
{
        JAK_ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->counters);
        return dic->counters(dic, counters);
}

static JAK_BUILT_IN(bool)  jak_string_dict_remove(jak_string_dict *dic, jak_archive_field_sid_t *strings, size_t num_strings)
{
        JAK_ERROR_IF_NULL(dic);
        JAK_ERROR_IF_NULL(strings);
        JAK_ASSERT(dic->remove);
        return dic->remove(dic, strings, num_strings);
}

static JAK_BUILT_IN(bool) jak_string_dict_locate_safe(jak_archive_field_sid_t **out, bool **found_mask, size_t *num_not_found, jak_string_dict *dic, char *const *keys, size_t num_keys)
{
        JAK_ERROR_IF_NULL(out);
        JAK_ERROR_IF_NULL(found_mask);
        JAK_ERROR_IF_NULL(num_not_found);
        JAK_ERROR_IF_NULL(dic);
        JAK_ERROR_IF_NULL(keys);
        JAK_ASSERT(dic->locate_safe);
        return dic->locate_safe(dic, out, found_mask, num_not_found, keys, num_keys);
}

static JAK_BUILT_IN(bool) jak_string_dict_locate_fast(jak_archive_field_sid_t **out, jak_string_dict *dic, char *const *keys, size_t nkeys)
{
        JAK_ERROR_IF_NULL(out);
        JAK_ERROR_IF_NULL(dic);
        JAK_ERROR_IF_NULL(keys);
        JAK_ASSERT(dic->locate_fast);
        return dic->locate_fast(dic, out, keys, nkeys);
}

static JAK_BUILT_IN(char **)jak_string_dict_extract(jak_string_dict *dic, const jak_archive_field_sid_t *ids, size_t nids)
{
        return dic->extract(dic, ids, nids);
}

static JAK_BUILT_IN(bool) jak_string_dict_free(jak_string_dict *dic, void *ptr)
{
        JAK_ERROR_IF_NULL(dic);
        if (ptr) {
                JAK_ASSERT(dic->free);
                return dic->free(dic, ptr);
        } else {
                return true;
        }
}

static JAK_BUILT_IN(bool) jak_string_dict_num_distinct(size_t *num, jak_string_dict *dic)
{
        JAK_ERROR_IF_NULL(num);
        JAK_ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->num_distinct);
        return dic->num_distinct(dic, num);
}

static JAK_BUILT_IN(bool) jak_string_dict_get_contents(jak_vector ofType (char *) *strings, jak_vector ofType(jak_archive_field_sid_t) *jak_string_ids, jak_string_dict *dic)
{
        JAK_ERROR_IF_NULL(strings)
        JAK_ERROR_IF_NULL(jak_string_ids)
        JAK_ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->get_contents);
        return dic->get_contents(dic, strings, jak_string_ids);
}

JAK_END_DECL

#endif
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

#ifndef JAK_STRING_H
#define JAK_STRING_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>

JAK_BEGIN_DECL

typedef struct jak_string {
        char *data;
        size_t cap;
        size_t end;
        jak_error err;
} jak_string;

JAK_DEFINE_GET_ERROR_FUNCTION(jak_string, jak_string, builder);

bool jak_string_create(jak_string *builder);
bool jak_string_create_ex(jak_string *builder, size_t capacity);
bool jak_string_drop(jak_string *builder);

bool jak_string_add(jak_string *builder, const char *str);
bool jak_string_add_nchar(jak_string *builder, const char *str, jak_u64 strlen);
bool jak_string_add_char(jak_string *builder, char c);
bool jak_string_add_u8(jak_string *builder, jak_u8 value);
bool jak_string_add_u16(jak_string *builder, jak_u16 value);
bool jak_string_add_u32(jak_string *builder, jak_u32 value);
bool jak_string_add_u64(jak_string *builder, jak_u64 value);
bool jak_string_add_i8(jak_string *builder, jak_i8 value);
bool jak_string_add_i16(jak_string *builder, jak_i16 value);
bool jak_string_add_i32(jak_string *builder, jak_i32 value);
bool jak_string_add_i64(jak_string *builder, jak_i64 value);
bool jak_string_add_u64_as_hex(jak_string *builder, jak_u64 value);
bool jak_string_add_u64_as_hex_0x_prefix_compact(jak_string *builder, jak_u64 value);
bool jak_string_add_float(jak_string *builder, float value);
bool jak_string_clear(jak_string *builder);
bool jak_string_ensure_capacity(jak_string *builder, jak_u64 cap);
size_t jak_string_len(jak_string *builder);

const char *jak_string_cstr(jak_string *builder);

bool jak_string_print(jak_string *builder);
bool jak_string_fprint(FILE *file, jak_string *builder);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_STRING_UTILS_H
#define JAK_STRING_UTILS_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>

JAK_BEGIN_DECL

bool jak_strings_contains_blank_char(const char *str);
bool jak_strings_is_enquoted(const char *str);
bool jak_strings_is_enquoted_wlen(const char *str, size_t len);
const char *jak_strings_skip_blanks(const char *str);
char *jak_strings_remove_tailing_blanks(char *str_in);

JAK_END_DECL

#endif
/**
 * Copyright 2019 Marcus Pinnecke, Robert Jendersie, Johannes Wuensche, Johann Wagner, and Marten Wallewein-Eising
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without ion, including without limitation the
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

#ifndef JAK_THREAD_POOL_H
#define JAK_THREAD_POOL_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_priority_queue.h>
#include <jak_thread_pool_status.h>
#include <jak_thread_pool_stats.h>

JAK_BEGIN_DECL

#ifndef JAK_NOOP
#define JAK_NOOP (void)0
#endif

#include <stdlib.h>
#include <pthread.h>

// Required due to bug in gcc, see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60932
// stdatomic.h must no be included in GTest
#ifndef CPP_TEST

#include <stdatomic.h>

#endif

#define JAK_THREAD_POOL_MAX_TASKS 4097

typedef void (*jak_task_routine)(void *routine);

typedef struct jak_thread_task {
        void *args;
        pthread_attr_t *attr;
        jak_task_routine routine;
        size_t group_id;
        size_t priority;
        jak_task_stats statistics;
} jak_thread_task;

typedef struct jak_task_state {
        atomic_int task_count; // remaining tasks in this group
        unsigned generation;
} jak_task_state;

typedef struct jak_task_handle {
        size_t index;
        unsigned generation;
} jak_task_handle;

typedef struct jak_thread_pool {
        char *name;
        pthread_t *pool;
        jak_priority_queue waiting_tasks;
        jak_task_state *task_group_states;
        size_t task_state_capacity; // number of tasks that can be tracked
        size_t size;
        size_t capacity;
        jak_thread_info **thread_infos;
        jak_thread_task **thread_tasks;
        jak_thread_pool_stats *statistics;
        int enable_monitoring;
} jak_thread_pool;

typedef struct jak_thread_info {
        char name[12];
        jak_thread_pool *pool;
        size_t id;
        atomic_int status;
        jak_thread_stats *statistics;
} jak_thread_info;

jak_thread_pool *jak_thread_pool_create(size_t num_threads, int enable_monitoring);
jak_thread_pool *jak_thread_pool_create_named(size_t num_threads, const char *name, int enable_monitoring);

// Releases all resources hold by the threadpool. 
// Currently working threads may finish but tasks left in the queue will be discarded.
void jak_thread_pool_free(jak_thread_pool *pool);
void jak_thread_pool_set_name(jak_thread_pool *pool, const char *name);

// Sets the number of active threads to num_threads.
// Currently working threads are terminated after there task is completed.
bool jak_thread_pool_resize(jak_thread_pool *pool, size_t num_threads);

// Add multiple tasks to be executed. Their progress is tracked by a single handle.
// hndl can be a nullptr.
bool jak_thread_pool_enqueue_tasks(jak_thread_task *task, jak_thread_pool *pool, size_t num_tasks, jak_task_handle *hndl);

bool jak_thread_pool_enqueue_task(jak_thread_task *task, jak_thread_pool *pool, jak_task_handle *hndl);

// Add multiple tasks to be executed. Waits until all passed tasks are finished. 
// The main thread also participates in task execution
bool jak_thread_pool_enqueue_tasks_wait(jak_thread_task *task, jak_thread_pool *pool, size_t num_tasks);

// Waits until the tasks referenced by hndl are completed.
bool jak_thread_pool_wait_for_task(jak_thread_pool *pool, jak_task_handle *hndl);

// Waits until all tasks currently in the queue are executed.
// The main thread also participates in task execution.
bool jak_thread_pool_wait_for_all(jak_thread_pool *pool);

void *__jak_thread_main(void *args);
jak_thread_task *__jak_get_next_task(jak_thread_pool *pool);
bool __jak_create_thread(jak_thread_info *thread_info, pthread_t *pp);
void __jak_sig_seg(int sig);

JAK_END_DECL

#endif
/**
 * Copyright 2019 Marcus Pinnecke, Robert Jendersie, Johannes Wuensche, Johann Wagner, and Marten Wallewein-Eising
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without ion, including without limitation the
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

#ifndef JAK_THREAD_POOL_MONITORING_H
#define JAK_THREAD_POOL_MONITORING_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>

JAK_BEGIN_DECL

#include "jak_thread_pool.h"

// Returns the average fraction of time the active threads have been working.
double jak_thread_pool_get_time_working(jak_thread_pool *pool);

// Fill all stats of the passed thread pool instance
jak_thread_pool_stats jak_thread_pool_get_stats(jak_thread_pool *pool);

// Fill all stats of the thread matching the given id in the thread pool
jak_thread_stats jak_thread_pool_get_thread_stats(jak_thread_pool *pool, size_t id);

JAK_END_DECL

#endif
/**
 * Copyright 2019 Marcus Pinnecke, Robert Jendersie, Johannes Wuensche, Johann Wagner, and Marten Wallewein-Eising
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without ion, including without limitation the
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

#ifndef JAK_THREAD_POOL_STATISTICS_H
#define JAK_THREAD_POOL_STATISTICS_H

#include <jak_stdinc.h>

JAK_BEGIN_DECL

#include <jak_time.h>
#include <stdio.h>

typedef struct jak_thread_pool_stats {
        struct timespec creation_time;
        unsigned int task_enqueued_count;
        unsigned int task_complete_count;
        long long complete_time;
        long long wait_time;
        long long avg_complete_time;
        long long avg_wait_time;
} jak_thread_pool_stats;

typedef struct jak_thread_stats {
        struct timespec creation_time;
        long long idle_time;
        long long busy_time;
        size_t task_count;
} jak_thread_stats;

typedef struct jak_task_stats {
        struct timespec enqueue_time;
        struct timespec execution_time;
        struct timespec complete_time;
} jak_task_stats;

static inline long long __jak_get_time_diff(struct timespec *begin, struct timespec *end)
{
        return (end->tv_sec - begin->tv_sec) * 1000000000L + (end->tv_nsec - begin->tv_nsec); /// 1000000000.0;
}

JAK_END_DECL

#endif
/**
 * Copyright 2019 Marcus Pinnecke, Robert Jendersie, Johannes Wuensche, Johann Wagner, and Marten Wallewein-Eising
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without ion, including without limitation the
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

#ifndef JAK_THREAD_POOL_STATUS_H
#define JAK_THREAD_POOL_STATUS_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>

JAK_BEGIN_DECL

typedef enum jak_thread_status_e {
        JAK_THREAD_STATUS_IDLE = 0,
        JAK_THREAD_STATUS_WORKING = 1,
        JAK_THREAD_STATUS_ABORTED = 2,
        JAK_THREAD_STATUS_FINISHED = 3,
        JAK_THREAD_STATUS_KILLED = 4,
        JAK_THREAD_STATUS_CREATED = 5,
        JAK_THREAD_STATUS_WILL_TERMINATE = 6,
        JAK_THREAD_STATUS_COMPLETED = 7,
        JAK_THREAD_STATUS_EMPTY = 99
} jak_thread_status_e;

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_TIME_H
#define JAK_TIME_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_types.h>

JAK_BEGIN_DECL

typedef jak_u64 jak_timestamp;

jak_timestamp jak_wallclock();

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_TYPES_H
#define JAK_TYPES_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>

JAK_BEGIN_DECL

typedef uint8_t jak_u8;
typedef uint16_t jak_u16;
typedef uint32_t jak_u32;
typedef uint64_t jak_u64;
typedef int8_t jak_i8;
typedef int16_t jak_i16;
typedef int32_t jak_i32;
typedef int64_t jak_i64;
typedef float jak_float;

#define JAK_U8_NULL         UINT8_MAX
#define JAK_U16_NULL        UINT16_MAX
#define JAK_U32_NULL        UINT32_MAX
#define JAK_U64_NULL        UINT64_MAX
#define JAK_I8_NULL         INT8_MIN
#define JAK_I16_NULL        INT16_MIN
#define JAK_I32_NULL        INT32_MIN
#define JAK_I64_NULL        INT64_MIN
#define JAK_FLOAT_NULL      NAN

#define JAK_CARBON_U8_MIN    UINT8_MIN
#define JAK_CARBON_U16_MIN   UINT16_MIN
#define JAK_CARBON_U32_MIN   UINT32_MIN
#define JAK_CARBON_U64_MIN   UINT64_MIN
#define JAK_CARBON_I8_MIN    (INT8_MIN + 1)
#define JAK_CARBON_I16_MIN   (INT16_MIN + 1)
#define JAK_CARBON_I32_MIN   (INT32_MIN + 1)
#define JAK_CARBON_I64_MIN   (INT64_MIN + 1)
#define JAK_CARBON_U8_MAX    (JAK_U8_NULL - 1)
#define JAK_CARBON_U16_MAX   (JAK_U16_NULL - 1)
#define JAK_CARBON_U32_MAX   (JAK_U32_NULL - 1)
#define JAK_CARBON_U64_MAX   (JAK_U64_NULL - 1)
#define JAK_CARBON_I8_MAX    INT8_MAX
#define JAK_CARBON_I16_MAX   INT16_MAX
#define JAK_CARBON_I32_MAX   INT32_MAX
#define JAK_CARBON_I64_MAX   INT64_MAX

#define JAK_CARBON_BOOLEAN_COLUMN_FALSE     0
#define JAK_CARBON_BOOLEAN_COLUMN_TRUE      1
#define JAK_CARBON_BOOLEAN_COLUMN_NULL      2

#define JAK_IS_NULL_BOOLEAN(x)      (x == JAK_CARBON_BOOLEAN_COLUMN_NULL)
#define JAK_IS_NULL_U8(x)           (x == JAK_U8_NULL)
#define JAK_IS_NULL_U16(x)          (x == JAK_U16_NULL)
#define JAK_IS_NULL_U32(x)          (x == JAK_U32_NULL)
#define JAK_IS_NULL_U64(x)          (x == JAK_U64_NULL)
#define JAK_IS_NULL_I8(x)           (x == JAK_I8_NULL)
#define JAK_IS_NULL_I16(x)          (x == JAK_I16_NULL)
#define JAK_IS_NULL_I32(x)          (x == JAK_I32_NULL)
#define JAK_IS_NULL_I64(x)          (x == JAK_I64_NULL)
#define JAK_IS_NULL_FLOAT(x)        (isnan(x))

typedef jak_u64 jak_archive_field_sid_t;  /* string identifier, resolvable by a string dictionary */
typedef char field_null_t;
typedef jak_i8 jak_archive_field_boolean_t;
typedef jak_i8 jak_archive_field_i8_t;
typedef jak_i16 jak_archive_field_i16_t;
typedef jak_i32 jak_archive_field_i32_t;
typedef jak_i64 jak_archive_field_i64_t;
typedef jak_u8 jak_archive_field_u8_t;
typedef jak_u16 jak_archive_field_u16_t;
typedef jak_u32 jak_archive_field_u32_t;
typedef jak_u64 jak_archive_field_u64_t;
typedef float jak_archive_field_number_t;
typedef const char *field_jak_string_t;

#define JAK_NULL_ENCODED_STRING            0
#define JAK_NULL_BOOLEAN                   INT8_MAX
#define JAK_NULL_INT8                      INT8_MAX
#define JAK_NULL_INT16                     INT16_MAX
#define JAK_NULL_INT32                     INT32_MAX
#define JAK_NULL_INT64                     INT64_MAX
#define JAK_NULL_UINT8                     UINT8_MAX
#define JAK_NULL_UINT16                    UINT16_MAX
#define JAK_NULL_UINT32                    UINT32_MAX
#define JAK_NULL_UINT64                    UINT64_MAX
#define JAK_NULL_FLOAT                     NAN
#define JAK_NULL_OBJECT_MODEL(objectModel) (objectModel->entries.num_elems == 0)

#define JAK_IS_NULL_STRING(str)   (str == JAK_NULL_ENCODED_STRING)
#define JAK_IS_NULL_BOOL(val)     (val == JAK_NULL_BOOLEAN)
#define JAK_IS_NULL_INT8(val)     (val == JAK_NULL_INT8)
#define JAK_IS_NULL_INT16(val)    (val == JAK_NULL_INT16)
#define JAK_IS_NULL_INT32(val)    (val == JAK_NULL_INT32)
#define JAK_IS_NULL_INT64(val)    (val == JAK_NULL_INT64)
#define JAK_IS_NULL_UINT8(val)    (val == JAK_NULL_UINT8)
#define JAK_IS_NULL_UINT16(val)   (val == JAK_NULL_UINT16)
#define JAK_IS_NULL_UINT32(val)   (val == JAK_NULL_UINT32)
#define JAK_IS_NULL_UINT64(val)   (val == JAK_NULL_UINT64)
#define JAK_IS_NULL_NUMBER(val)   (val == JAK_NULL_FLOAT)

#define JAK_LIMITS_INT8_MAX                (JAK_NULL_INT8 - 1)
#define JAK_LIMITS_INT16_MAX               (JAK_NULL_INT16 - 1)
#define JAK_LIMITS_INT32_MAX               (JAK_NULL_INT32 - 1)
#define JAK_LIMITS_INT64_MAX               (JAK_NULL_INT64 - 1)
#define JAK_LIMITS_UINT8_MAX               (JAK_NULL_UINT8 - 1)
#define JAK_LIMITS_UINT16_MAX              (JAK_NULL_UINT16 - 1)
#define JAK_LIMITS_UINT32_MAX              (JAK_NULL_UINT32 - 1)
#define JAK_LIMITS_UINT64_MAX              (JAK_NULL_UINT64 - 1)

#define JAK_LIMITS_INT8_MIN                INT8_MIN
#define JAK_LIMITS_INT16_MIN               INT16_MIN
#define JAK_LIMITS_INT32_MIN               INT32_MIN
#define JAK_LIMITS_INT64_MIN               INT64_MIN
#define JAK_LIMITS_UINT8_MIN               0
#define JAK_LIMITS_UINT16_MIN              0
#define JAK_LIMITS_UINT32_MIN              0
#define JAK_LIMITS_UINT64_MIN              0

#define JAK_NULL_TEXT "null"

#define JAK_BOOLEAN_FALSE 0
#define JAK_BOOLEAN_TRUE  1

#define GET_TYPE_SIZE(value_type)                                                                                      \
({                                                                                                                     \
    size_t value_size;                                                                                                 \
    switch (value_type) {                                                                                              \
        case JAK_FIELD_NULL:                                                                                           \
            value_size = sizeof(jak_u16);                                                                              \
            break;                                                                                                     \
        case JAK_FIELD_BOOLEAN:                                                                                        \
            value_size = sizeof(jak_archive_field_boolean_t);                                                          \
            break;                                                                                                     \
        case JAK_FIELD_INT8:                                                                                           \
            value_size = sizeof(jak_archive_field_i8_t);                                                               \
            break;                                                                                                     \
        case JAK_FIELD_INT16:                                                                                          \
            value_size = sizeof(jak_archive_field_i16_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_INT32:                                                                                          \
            value_size = sizeof(jak_archive_field_i32_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_INT64:                                                                                          \
            value_size = sizeof(jak_archive_field_i64_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_UINT8:                                                                                          \
            value_size = sizeof(jak_archive_field_u8_t);                                                               \
            break;                                                                                                     \
        case JAK_FIELD_UINT16:                                                                                         \
            value_size = sizeof(jak_archive_field_u16_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_UINT32:                                                                                         \
            value_size = sizeof(jak_archive_field_u32_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_UINT64:                                                                                         \
            value_size = sizeof(jak_archive_field_u64_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_FLOAT:                                                                                          \
            value_size = sizeof(jak_archive_field_number_t);                                                           \
            break;                                                                                                     \
        case JAK_FIELD_STRING:                                                                                         \
            value_size = sizeof(jak_archive_field_sid_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_OBJECT:                                                                                         \
            value_size = sizeof(jak_column_doc_obj);                                                                   \
            break;                                                                                                     \
        default:                                                                                                       \
        JAK_ERROR_PRINT_AND_DIE(JAK_ERR_NOTYPE);                                                                       \
    }                                                                                                                  \
    value_size;                                                                                                        \
})

JAK_END_DECL

#endif/**
 * A variable-length unsigned integer type that encodes the number of used bytes by a preceding marker byte
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

#ifndef JAK_UINTVAR_MARKER_H
#define JAK_UINTVAR_MARKER_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include "jak_stdinc.h"
#include "stdbool.h"
#include "jak_types.h"

/**
 * This type is for variable-length unsigned integer types.
 *
 * The encoding uses a dedicated byte (called marker) to identify the number of subsequent bytes that holding the
 * actual value: if the first byte read is...
 *      - ... 'c', then the next byte contains an unsigned integer value of 8bit.
 *      - ... 'd', then the next 2 bytes contain an unsigned integer value of 16bit.
 *      - ... 'i', then the next 4 bytes contain an unsigned integer value of 32bit.
 *      - ... 'l', then the next 8 bytes contain an unsigned integer value of 64bit.
 *
 * This implementation supports variable-length encoding to the maximum of unsigned integers of
 * 64bit (fixed-sized) requiring constant 1 byte more than the standard C type.
 *
 * Note that size requirements for this kind of variable-length encoding is (relatively) huge;
 * the encoding requires as least 12.5% additional storage (to encode 64bit integers) and at most
 * 100.0% (!) additional storage (to encode 8bit integers). The benefit of marker-based variable-length encoding is that
 * read-/write performance is superior to byte-stream based variable-length encoding (see <code>uintvar_stream</code>),
 * and that size requirements payoff for values larger than 65536. Faster read/write performance compared to byte-stream
 * based variable-length encoding comes by the fact that after determination of actual number of bytes to reads
 * (i.e., the marker), there is no interpretation overhead to read the actual value while in byte-stream based encoding
 * each subsequent byte must be inspect (on whether it is the last byte in the stream) before reading its contained
 * value (after some byte shift operations).
 *
 * Rule of thumb:
 *      - if fixed-length types are a good choice, and...
 *          - ... if speed matters, use fast-types of the C library (e.g., <code>uint_fast32_t</code>)
 *          - ... if space matters, use fix-types of the C library (e.g., <code>uint32_t</code>)
 *      - if variable-length types are a good choice, and...
 *          - ... if space shall be minimized in exchange of read/write performance, use <code>jak_uintvar_stream_t</code>
 *          - ... if read/write performance shall be maximized in exchange of space, use <code>jak_uintvar_marker_t</code>
 */

JAK_BEGIN_DECL

#define UINT_VAR_MARKER_8 'c'
#define UINT_VAR_MARKER_16 'd'
#define UINT_VAR_MARKER_32 'i'
#define UINT_VAR_MARKER_64 'l'

typedef void *jak_uintvar_marker_t;

typedef enum jak_uintvar_marker {
        JAK_UINTVAR_8,
        JAK_UINTVAR_16,
        JAK_UINTVAR_32,
        JAK_UINTVAR_64
} jak_uintvar_marker_e;

bool jak_uintvar_marker_write(jak_uintvar_marker_t dst, jak_u64 value);
jak_u64 jak_uintvar_marker_read(jak_u8 *nbytes_read, jak_uintvar_marker_t src);
jak_uintvar_marker_e jak_uintvar_marker_type_for(jak_u64 value);
bool jak_uintvar_marker_type(const void *data);
size_t jak_uintvar_marker_sizeof(jak_uintvar_marker_t value);
size_t jak_uintvar_marker_required_size(jak_u64 value);

JAK_END_DECL

#endif
/**
 * A variable-length unsigned integer type that encodes the number of used bytes by a flag bit in the byte stream
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

#ifndef JAK_UINTVAR_STREAM_H
#define JAK_UINTVAR_STREAM_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_types.h>
#include <jak_memfile.h>

JAK_BEGIN_DECL

/**
 * This type is for variable-length unsigned integer types.
 *
 * The encoding uses the most significant bit (MSB) for each byte in sequence of bytes (called blocks) to determine the
 * number of bytes required to express the unsigned integer value. The MSB is 1 if there is at least one further byte to
 * be read, and 0 if the end of the sequence is reached. The remaining 7 bits per block contain the actual bits for
 * integer value encoding.
 *
 * This implementation supports variable-length encoding of the maximum value up to unsigned integer of 64bit (fixed-
 * length) in at most 10 blocks.
 *
 * Example: Given the unsigned integer 16389, its fixed-length representation is
 *                  01000000 00000101
 *          Using the varuint type, the representation is
 *                  (1)0000001 (1)0000000 (0)0000101
 *
 *
 *      # required |      min value      |      max value
 *        blocks   |       (incl.)       |       (incl.)
 *      -----------+---------------------+----------------------
 *               1 |                   0 |                  127
 *               2 |                 128 |                16383
 *               3 |               16384 |              2097151
 *               4 |             2097152 |            268435455
 *               5 |           268435456 |          34359738367
 *               6 |         34359738368 |        4398046511103
 *               7 |       4398046511104 |      562949953421311
 *               8 |     562949953421312 |    72057594037927935
 *               9 |   72057594037927936 |  9223372036854775807
 *              10 | 9223372036854775808 | 18446744073709551615
 *
  * Rule of thumb:
 *      - if fixed-length types are a good choice, and...
 *          - ... if speed matters, use fast-types of the C library (e.g., <code>uint_fast32_t</code>)
 *          - ... if space matters, use fix-types of the C library (e.g., <code>uint32_t</code>)
 *      - if variable-length types are a good choice, and...
 *          - ... if space shall be minimized in exchange of read/write performance, use <code>jak_uintvar_stream_t</code>
 *          - ... if read/write performance shall be maximized in exchange of space, use <code>jak_uintvar_marker_t</code>
 */

typedef void *jak_uintvar_stream_t;

#define JAK_UINTVAR_STREAM_MAX_BLOCKS()    (4)

jak_u8 jak_uintvar_stream_write(jak_uintvar_stream_t dst, jak_u64 value);

#define JAK_UINTVAR_STREAM_SIZEOF(value)                                                                               \
({                                                                                                                     \
        size_t num_blocks_strlen = JAK_UINTVAR_STREAM_REQUIRED_BLOCKS(value);                                          \
        num_blocks_strlen = num_blocks_strlen < sizeof(jak_uintvar_stream_t) ? sizeof(jak_uintvar_stream_t):num_blocks_strlen; \
        num_blocks_strlen;                                                                                             \
})

#define JAK_UINTVAR_STREAM_REQUIRED_BLOCKS(value)                                                                      \
({                                                                                                                     \
        jak_u8 num_blocks_required;                                                                                    \
        if (value < 128u) {                                                                                            \
                num_blocks_required = 1;                                                                               \
        } else if (value < 16384u) {                                                                                   \
                num_blocks_required = 2;                                                                               \
        } else if (value < 2097152u) {                                                                                 \
                num_blocks_required = 3;                                                                               \
        } else if (value < 268435456u) {                                                                               \
                num_blocks_required = 4;                                                                               \
        } else if (value < 34359738368u) {                                                                             \
                num_blocks_required = 5;                                                                               \
        } else if (value < 4398046511104u) {                                                                           \
                num_blocks_required = 6;                                                                               \
        } else if (value < 562949953421312u) {                                                                         \
                num_blocks_required = 7;                                                                               \
        } else if (value < 72057594037927936u) {                                                                       \
                num_blocks_required = 8;                                                                               \
        } else if (value < 9223372036854775808u) {                                                                     \
                num_blocks_required = 9;                                                                               \
        } else {                                                                                                       \
                num_blocks_required = 10;                                                                              \
        }                                                                                                              \
        num_blocks_required;                                                                                           \
})

jak_u64 jak_uintvar_stream_read(jak_u8 *nbytes, jak_uintvar_stream_t src);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_UNIQUE_ID_H
#define JAK_UNIQUE_ID_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_types.h>

JAK_BEGIN_DECL

typedef jak_u64 jak_uid_t;

bool jak_unique_id_create(jak_uid_t *out);

bool jak_unique_id_get_global_wallclocktime(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_global_build_path_bit(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_global_build_time_bit(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_process_id(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_process_magic(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_process_counter(uint_fast16_t *out, jak_uid_t id);
bool jak_unique_id_get_thread_id(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_thread_magic(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_thread_counter(uint_fast32_t *out, jak_uid_t id);
bool jak_unique_id_get_call_random(uint_fast8_t *out, jak_uid_t id);

JAK_END_DECL

#endif/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_CONVERT_H
#define JAK_CONVERT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_types.h>

JAK_BEGIN_DECL

jak_i64 jak_convert_atoi64(const char *string);
jak_u64 jak_convert_atoiu64(const char *string);

JAK_END_DECL

#endif/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_HEXDUMP_H
#define JAK_HEXDUMP_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_string.h>

JAK_BEGIN_DECL

bool jak_hexdump(jak_string *dst, const void *base, jak_u64 nbytes);
bool jak_hexdump_print(FILE *file, const void *base, jak_u64 nbytes);

JAK_END_DECL

#endif
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

#ifndef AKR_NUMBERS_H
#define AKR_NUMBERS_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_types.h>

typedef enum jak_number_min_type_e {
        JAK_NUMBER_U8,
        JAK_NUMBER_U16,
        JAK_NUMBER_U32,
        JAK_NUMBER_U64,
        JAK_NUMBER_I8,
        JAK_NUMBER_I16,
        JAK_NUMBER_I32,
        JAK_NUMBER_I64,
        JAK_NUMBER_UNKNOWN
} jak_number_min_type_e;

jak_number_min_type_e jak_number_min_type_unsigned(jak_u64 value);
jak_number_min_type_e jak_number_min_type_signed(jak_i64 value);

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_SORT_H
#define JAK_SORT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_alloc.h>

#include "stdlib.h"

JAK_BEGIN_DECL

typedef bool (*jak_less_eq_func_t)(const void *lhs, const void *rhs);

typedef bool (*jak_less_eq_wargs_func_t)(const void *lhs, const void *rhs, void *args);

typedef bool (*jak_eq_func_t)(const void *lhs, const void *rhs);

typedef bool (*jak_less_func_t)(const void *lhs, const void *rhs);

#define JAK_QSORT_INDICES_SWAP(x, y)                                                                                   \
{                                                                                                                      \
    size_t *a = x;                                                                                                     \
    size_t *b = y;                                                                                                     \
    size_t tmp = *a;                                                                                                   \
    *a = *b;                                                                                                           \
    *b = tmp;                                                                                                          \
}

#define JAK_QSORT_INDICIES_PARTITION(indices, base, width, comp, l, h)                                                 \
({                                                                                                                     \
    const void   *x       = base + indices[h] * width;                                                                 \
    jak_i64        i       = (l - 1);                                                                                  \
                                                                                                                       \
    for (jak_i64 j = l; j <= h - 1; j++)                                                                               \
    {                                                                                                                  \
        if (comp(base + indices[j] * width, x))                                                                        \
        {                                                                                                              \
            i++;                                                                                                       \
            JAK_QSORT_INDICES_SWAP (indices + i, indices + j);                                                         \
        }                                                                                                              \
    }                                                                                                                  \
    JAK_QSORT_INDICES_SWAP (indices + (i + 1), indices + h);                                                           \
    (i + 1);                                                                                                           \
})

#define JAK_QSORT_INDICIES_PARTITION_WARGS(indices, base, width, comp, l, h, args)                                     \
({                                                                                                                     \
    const void   *x       = base + indices[h] * width;                                                                 \
    jak_i64        i       = (l - 1);                                                                                  \
                                                                                                                       \
    for (jak_i64 j = l; j <= h - 1; j++)                                                                               \
    {                                                                                                                  \
        if (comp(base + indices[j] * width, x, args))                                                                  \
        {                                                                                                              \
            i++;                                                                                                       \
            JAK_QSORT_INDICES_SWAP (indices + i, indices + j);                                                         \
        }                                                                                                              \
    }                                                                                                                  \
    JAK_QSORT_INDICES_SWAP (indices + (i + 1), indices + h);                                                           \
    (i + 1);                                                                                                           \
})

bool jak_sort_qsort_indicies(size_t *indices, const void *base, size_t width, jak_less_eq_func_t comp, size_t nelemns, jak_allocator *alloc);
int jak_sort_qsort_indicies_wargs(size_t *indices, const void *base, size_t width, jak_less_eq_wargs_func_t comp, size_t nelemens, jak_allocator *alloc, void *args);
size_t jak_sort_bsearch_indicies(const size_t *indicies, const void *base, size_t width, size_t nelemens, const void *neelde, jak_eq_func_t compEq, jak_less_func_t compLess);

size_t jak_sort_get_min(const size_t *elements, size_t nelemens);
size_t jak_sort_get_max(const size_t *elements, size_t nelemens);
double jak_sort_get_sum(const size_t *elements, size_t nelemens);
double jak_sort_get_avg(const size_t *elements, size_t nelemens);

JAK_END_DECL

#endif
/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_VECTOR_H
#define JAK_VECTOR_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <sys/mman.h>

#include <jak_stdinc.h>
#include <jak_alloc.h>
#include <jak_memfile.h>

JAK_BEGIN_DECL

#define DECLARE_PRINTER_FUNC(type)                                                                                     \
    void jak_vector_##type##_printer_func(jak_memfile *dst, void ofType(T) *values, size_t num_elems);

DECLARE_PRINTER_FUNC(u_char)

DECLARE_PRINTER_FUNC(jak_i8)

DECLARE_PRINTER_FUNC(jak_i16)

DECLARE_PRINTER_FUNC(jak_i32)

DECLARE_PRINTER_FUNC(jak_i64)

DECLARE_PRINTER_FUNC(jak_u8)

DECLARE_PRINTER_FUNC(jak_u16)

DECLARE_PRINTER_FUNC(jak_u32)

DECLARE_PRINTER_FUNC(jak_u64)

DECLARE_PRINTER_FUNC(size_t)

#define VECTOR_PRINT_UCHAR  jak_vector_u_char_printer_func
#define VECTOR_PRINT_UINT8  jak_vector_u8_printer_func
#define VECTOR_PRINT_UINT16 jak_vector_u16_printer_func
#define VECTOR_PRINT_UINT32 jak_vector_u32_printer_func
#define VECTOR_PRINT_UINT64 jak_vector_u64_printer_func
#define VECTOR_PRINT_INT8   jak_vector_i8_printer_func
#define VECTOR_PRINT_INT16  jak_vector_i16_printer_func
#define VECTOR_PRINT_INT32  jak_vector_i32_printer_func
#define VECTOR_PRINT_INT64  jak_vector_i64_printer_func
#define VECTOR_PRINT_SIZE_T jak_vector_size_t_printer_func

/**
 * An implementation of the concrete data type Vector, a resizeable dynamic array.
 */
typedef struct jak_vector {
        /**
        *  Memory allocator that is used to get memory for user data
        */
        jak_allocator *allocator;

        /**
         *  Fixed number of bytes for a single element that should be stored in the vector
         */
        size_t elem_size;

        /**
         *  The number of elements currently stored in the vector
         */
        jak_u32 num_elems;

        /**
         *  The number of elements for which currently memory is reserved
         */
        jak_u32 cap_elems;

        /**
        * The grow factor considered for resize operations
        */
        float grow_factor;

        /**
         * A pointer to a memory address managed by 'allocator' that contains the user data
         */
        void *base;

        /**
         *  Error information
         */
        jak_error err;
} jak_vector;

/**
 * Utility implementation of generic vector to specialize for type of 'char *'
 */
typedef jak_vector ofType(const char *) jak_string_jak_vector_t;

/**
 * Constructs a new vector for elements of size 'elem_size', reserving memory for 'cap_elems' elements using
 * the allocator 'alloc'.
 *
 * @param out non-null vector that should be constructed
 * @param alloc an allocator
 * @param elem_size fixed-length element size
 * @param cap_elems number of elements for which memory should be reserved
 * @return STATUS_OK if success, and STATUS_NULLPTR in case of NULL pointer parameters
 */
bool jak_vector_create(jak_vector *out, const jak_allocator *alloc, size_t elem_size, size_t cap_elems);

bool jak_vector_serialize(FILE *file, jak_vector *vec);

bool jak_vector_deserialize(jak_vector *vec, jak_error *err, FILE *file);

/**
 * Provides hints on the OS kernel how to deal with memory inside this vector.
 *
 * @param vec non-null vector
 * @param madviseAdvice value to give underlying <code>madvise</code> syscall and advice, see man page
 * of <code>madvise</code>
 * @return STATUS_OK if success, otherwise a value indicating the JAK_ERROR
 */
bool jak_vector_memadvice(jak_vector *vec, int madviseAdvice);

/**
 * Sets the factor for determining the reallocation size in case of a resizing operation.
 *
 * Note that <code>factor</code> must be larger than one.
 *
 * @param vec non-null vector for which the grow factor should be changed
 * @param factor a positive real number larger than 1
 * @return STATUS_OK if success, otherwise a value indicating the JAK_ERROR
 */
bool jak_vector_set_grow_factor(jak_vector *vec, float factor);

/**
 * Frees up memory requested via the allocator.
 *
 * Depending on the allocator implementation, dropping the reserved memory might not take immediately effect.
 * The pointer 'vec' itself gets not freed.
 *
 * @param vec vector to be freed
 * @return STATUS_OK if success, and STATUS_NULL_PTR in case of NULL pointer to 'vec'
 */
bool jak_vector_drop(jak_vector *vec);

/**
 * Returns information on whether elements are stored in this vector or not.
 * @param vec non-null pointer to the vector
 * @return Returns <code>STATUS_TRUE</code> if <code>vec</code> is empty. Otherwise <code>STATUS_FALSE</code> unless
 *         an JAK_ERROR occurs. In case an JAK_ERROR is occured, the return value is neither <code>STATUS_TRUE</code> nor
 *         <code>STATUS_FALSE</code> but an value indicating that JAK_ERROR.
 */
bool jak_vector_is_empty(const jak_vector *vec);

/**
 * Appends 'num_elems' elements stored in 'data' into the vector by copying num_elems * vec->elem_size into the
 * vectors memory block.
 *
 * In case the capacity is not sufficient, the vector gets automatically resized.
 *
 * @param vec the vector in which the data should be pushed
 * @param data non-null pointer to data that should be appended. Must be at least size of 'num_elems' * vec->elem_size.
 * @param num_elems number of elements stored in data
 * @return STATUS_OK if success, and STATUS_NULLPTR in case of NULL pointer parameters
 */
bool jak_vector_push(jak_vector *vec, const void *data, size_t num_elems);

const void *jak_vector_peek(jak_vector *vec);

#define JAK_VECTOR_PEEK(vec, type) (type *)(jak_vector_peek(vec))

/**
 * Appends 'how_many' elements of the same source stored in 'data' into the vector by copying how_many * vec->elem_size
 * into the vectors memory block.
 *
 * In case the capacity is not sufficient, the vector gets automatically resized.
 *
 * @param vec the vector in which the data should be pushed
 * @param data non-null pointer to data that should be appended. Must be at least size of one vec->elem_size.
 * @param num_elems number of elements stored in data
 * @return STATUS_OK if success, and STATUS_NULLPTR in case of NULL pointer parameters
 */
bool jak_vector_repeated_push(jak_vector *vec, const void *data, size_t how_often);

/**
 * Returns a pointer to the last element in this vector, or <code>NULL</code> is the vector is already empty.
 * The number of elements contained in that vector is decreased, too.
 *
 * @param vec non-null pointer to the vector
 * @return Pointer to last element, or <code>NULL</code> if vector is empty
 */
const void *jak_vector_pop(jak_vector *vec);

bool jak_vector_clear(jak_vector *vec);

/**
 * Shinks the vector's internal data block to fits its real size, i.e., remove reserved memory
 *
 * @param vec
 * @return
 */
bool jak_vector_shrink(jak_vector *vec);

/**
 * Increases the capacity of that vector according the internal grow factor
 * @param numNewSlots a pointer to a value that will store the number of newly created slots in that vector if
 *                      <code>num_new_slots</code> is non-null. If this parameter is <code>NULL</code>, it is ignored.
 * @param vec non-null pointer to the vector that should be grown
 * @return STATUS_OK in case of success, and another value indicating an JAK_ERROR otherwise.
 */
bool jak_vector_grow(size_t *numNewSlots, jak_vector *vec);

bool jak_vector_grow_to(jak_vector *vec, size_t capacity);

/**
 * Returns the number of elements currently stored in the vector
 *
 * @param vec the vector for which the operation is started
 * @return 0 in case of NULL pointer to 'vec', or the number of elements otherwise.
 */
size_t jak_vector_length(const jak_vector *vec);

#define JAK_VECTOR_GET(vec, pos, type) (type *) jak_vector_at(vec, pos)

#define JAK_VECTOR_NEW_AND_GET(vec, type)                                                                              \
({                                                                                                                     \
    type obj;                                                                                                          \
    size_t vectorLength = jak_vector_length(vec);                                                                      \
    jak_vector_push(vec, &obj, 1);                                                                                     \
    JAK_VECTOR_GET(vec, vectorLength, type);                                                                           \
})

const void *jak_vector_at(const jak_vector *vec, size_t pos);

/**
 * Returns the number of elements for which memory is currently reserved in the vector
 *
 * @param vec the vector for which the operation is started
 * @return 0 in case of NULL pointer to 'vec', or the number of elements otherwise.
 */
size_t jak_vector_capacity(const jak_vector *vec);

/**
 * Set the internal size of <code>vec</code> to its capacity.
 */
bool jak_vector_enlarge_size_to_capacity(jak_vector *vec);

bool jak_vector_zero_memory(jak_vector *vec);

bool jak_vector_zero_memory_in_range(jak_vector *vec, size_t from, size_t to);

bool jak_vector_set(jak_vector *vec, size_t pos, const void *data);

bool jak_vector_cpy(jak_vector *dst, const jak_vector *src);

bool jak_vector_cpy_to(jak_vector *dst, jak_vector *src);

/**
 * Gives raw data access to data stored in the vector; do not manipulate this data since otherwise the vector
 * might get corrupted.
 *
 * @param vec the vector for which the operation is started
 * @return pointer to user-data managed by this vector
 */
const void *jak_vector_data(const jak_vector *vec);

char *jak_vector_string(const jak_vector ofType(T) *vec,
                    void (*printerFunc)(jak_memfile *dst, void ofType(T) *values, size_t num_elems));

#define JAK_VECTOR_ALL(vec, type) (type *) jak_vector_data(vec)

JAK_END_DECL

#endif
