#include <inttypes.h>
#include <assert.h>
#include <carbon/carbon-compressor.h>
#include <carbon/carbon-doc.h>
#include <carbon/carbon-io-device.h>

#include <carbon/compressor/compressor-utils.h>
#include <carbon/compressor/auto/selector.h>
#include <carbon/compressor/carbon-compressor-auto.h>

typedef struct {
    char const *key;
    uint32_t length;

    carbon_compressor_t *compressor;
} carbon_compressor_auto_range_t;

typedef struct {
    // Maps key -> compressor
    carbon_hashmap_t ofType(carbon_compressor_t *) compressors;
    carbon_vec_t ofType(carbon_compressor_auto_range_t) ranges;
    carbon_vec_t ofType(carbon_off_t) range_begin_table;
    carbon_doc_bulk_t const *context;

    size_t current_range_idx;
    size_t current_position;
    carbon_off_t range_begin_table_offset;
    carbon_compressor_t *current_range_compressor;
} carbon_compressor_auto_extra_t;


carbon_compressor_t *this_compressor_for(
        carbon_doc_bulk_t const *context,
        carbon_vec_t ofType(char *) *values
    ) {
    return carbon_compressor_find_by_strings(values, context);
}

int this_sort_strdic_entries_by_key_id(void const *a, void const *b) {
    carbon_strdic_entry_t const *entry_a = (carbon_strdic_entry_t const *)a;
    carbon_strdic_entry_t const *entry_b = (carbon_strdic_entry_t const *)b;

    return entry_a->grouping_key == entry_b->grouping_key ? 0 : (entry_a->grouping_key < entry_b->grouping_key ? -1 : 1);
}

int this_sort_strings(void const *a, void const *b) {
    char const * const str_a = (char const * const)a;
    char const * const str_b = (char const * const)b;

    return strcmp(str_a, str_b);
}

typedef bool (*carbon_compressor_auto_extra_processor_t)(carbon_compressor_t *compressor, carbon_io_device_t *src, void *callback_extra);

bool this_auto_read_extra(
        carbon_io_device_t *src,
        carbon_compressor_auto_extra_t *extra,
        carbon_compressor_auto_extra_processor_t callback,
        void *callback_extra
    ) {
    bool ok;
    size_t num_compressors = carbon_vlq_decode_from_io(src, &ok);

    if(!ok)
        return false;

    carbon_err_t err;
    for(size_t i = 0; i < num_compressors;++i) {
        carbon_off_t range_begin;
        carbon_io_device_read(src, &range_begin, sizeof(range_begin), 1);
        carbon_vec_push(&extra->range_begin_table, &range_begin, 1);
    }


    for(size_t i = 0; i < num_compressors; ++i) {
        carbon_compressor_t *compressor = malloc(sizeof(carbon_compressor_t));
        compressor->options = carbon_hashmap_new();

        uint8_t compressor_type;
        carbon_io_device_read(src, &compressor_type, 1, 1);

        if(!carbon_compressor_by_type(&err, compressor, NULL, (carbon_compressor_type_e)compressor_type))
            return false;

        size_t num_entries = carbon_vlq_decode_from_io(src, &ok);
        if(!ok)
            return false;

        carbon_compressor_auto_range_t range = {
            .key = NULL, .length = (uint32_t)num_entries, .compressor = compressor
        };
        carbon_vec_push(&extra->ranges, &range, 1);
    }

    for(size_t i = 0; i < extra->ranges.num_elems;++i) {
        carbon_compressor_auto_range_t const *range =
                (carbon_compressor_auto_range_t const *)carbon_vec_at(&extra->ranges, i);

        callback(range->compressor, src, callback_extra);
    }

    return true;
}


carbon_compressor_t *this_find_compressor_for_position(carbon_compressor_auto_extra_t *extra, carbon_off_t position)
{
    size_t range_idx = 0;
    while(
        range_idx + 1 < extra->ranges.num_elems &&
        *CARBON_VECTOR_GET(&extra->range_begin_table, range_idx + 1, carbon_off_t const) <= position
    )
        ++range_idx;

    carbon_compressor_auto_range_t const *range =
            (carbon_compressor_auto_range_t const *)carbon_vec_at(&extra->ranges, range_idx);
    return range->compressor;
}


CARBON_EXPORT(bool)
carbon_compressor_auto_init(carbon_compressor_t *self, carbon_doc_bulk_t const *context)
{
    CARBON_UNUSED(self);
    CARBON_UNUSED(context);

    self->extra = malloc(sizeof(carbon_compressor_auto_extra_t));

    carbon_compressor_auto_extra_t *extra =
            (carbon_compressor_auto_extra_t *)self->extra;
    extra->compressors = carbon_hashmap_new();


    if(context) {
        extra->context = context;

        carbon_vec_t keys_with_duplicates;
        carbon_vec_cpy(&keys_with_duplicates, &context->keys);

        carbon_vec_t *keys_unique = malloc(sizeof(carbon_vec_t));
        carbon_vec_create(keys_unique, NULL, sizeof(char *), 10);

        qsort(keys_with_duplicates.base, keys_with_duplicates.num_elems, sizeof(char *), this_sort_strings);

        char const * previous_key = NULL;
        for(size_t i = 0; i < keys_with_duplicates.num_elems;++i) {
            char const * const key = *(char const * const*)carbon_vec_at(&keys_with_duplicates, i);
            if(previous_key != NULL && strcmp(previous_key, key) == 0)
                continue;

            char *cpy = strdup(key);
            carbon_vec_push(keys_unique, &cpy, 1);
            previous_key = key;
        }

        carbon_hashmap_put(extra->context->values, "", keys_unique);


        for(carbon_hashmap_iterator_t it = carbon_hashmap_begin(context->values);it.valid;carbon_hashmap_next(&it)) {
            CARBON_CONSOLE_WRITELN(stdout, "      - Setting up compressor for %s", it.key);
            carbon_hashmap_put(extra->compressors, it.key, this_compressor_for(context, (carbon_vec_t *)it.value));
        }
    }

    extra->current_position = 0;
    extra->current_range_idx = 0;
    extra->current_range_compressor = NULL;
    carbon_vec_create(&extra->ranges, NULL, sizeof(carbon_compressor_auto_range_t), context ? carbon_hashmap_length(context->values) : 10);
    carbon_vec_create(&extra->range_begin_table, NULL, sizeof(carbon_off_t), 10);

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_auto_cpy(const carbon_compressor_t *self, carbon_compressor_t *dst)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_AUTO);

    /* nothing to hard copy but the function pointers */
    *dst = *self;
    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_auto_drop(carbon_compressor_t *self)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_AUTO);
    carbon_compressor_auto_extra_t *extra =
            (carbon_compressor_auto_extra_t *)self->extra;

    carbon_err_t err;


    for(size_t i = 0; i < extra->ranges.num_elems;++i) {
        carbon_compressor_drop(&err, ((carbon_compressor_auto_range_t const *)carbon_vec_at(&extra->ranges, i))->compressor);
    }

    carbon_hashmap_drop(extra->compressors);
    carbon_vec_drop(&extra->ranges);
    carbon_vec_drop(&extra->range_begin_table);

    free(extra);
    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_auto_write_extra(carbon_compressor_t *self, carbon_memfile_t *dst,
                                        const carbon_vec_t ofType (const char *) *strings)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_AUTO);
    CARBON_UNUSED(strings);

    carbon_err_t err;
    carbon_io_device_t io;
    carbon_io_device_from_memfile(&io, dst);

    carbon_compressor_auto_extra_t *extra =
            (carbon_compressor_auto_extra_t *)self->extra;

    carbon_vlq_encode_to_io(extra->ranges.num_elems, &io);
    extra->range_begin_table_offset = carbon_io_device_tell(&io);

    for(size_t i = 0; i < extra->ranges.num_elems;++i) {
        carbon_string_id_t id = 0;
        carbon_io_device_write(&io, &id, sizeof(carbon_string_id_t), 1);
    }

    for(size_t i = 0; i < extra->ranges.num_elems;++i) {
        carbon_compressor_auto_range_t const *range = carbon_vec_at(&extra->ranges, i);
        carbon_compressor_t *compressor;
        carbon_hashmap_get(extra->compressors, range->key, (void **)&compressor);

        uint8_t compressor_type = (uint8_t)compressor->tag;
        carbon_io_device_write(&io, &compressor_type, 1, 1);

        carbon_vec_t * values;
        carbon_hashmap_get(extra->context->values, range->key, (void **)&values);
        carbon_vlq_encode_to_io(values->num_elems, &io);
    }

    uint32_t offset = 0;
    for(size_t i = 0; i < extra->ranges.num_elems;++i) {
        carbon_compressor_auto_range_t const *range = carbon_vec_at(&extra->ranges, i);

        carbon_compressor_t *compressor;
        carbon_hashmap_get(extra->compressors, range->key, (void **)&compressor);

        carbon_vec_t current = *strings;
        current.base = (void *)carbon_vec_at(strings, offset);
        current.num_elems = range->length;
        current.cap_elems = strings->cap_elems - offset;

        offset += range->length;
        carbon_compressor_write_extra(&err, compressor, dst, &current);
    }
    return true;
}

bool this_compressor_auto_fwd_args_to_read_extra(
        carbon_compressor_t *compressor, carbon_io_device_t *src, void *extra
        ) {
    carbon_err_t err;

    CARBON_UNUSED(extra);

    // uses knowledge about internals of carbon_io_device_t
    return carbon_compressor_read_extra(&err, compressor, (FILE *)src->extra, 0);
}

CARBON_EXPORT(bool)
carbon_compressor_auto_read_extra(carbon_compressor_t *self, FILE *src, size_t nbytes)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_AUTO);

    CARBON_UNUSED(nbytes);

    carbon_io_device_t io;
    carbon_io_device_from_file(&io, src);

    return this_auto_read_extra(&io, (carbon_compressor_auto_extra_t *)self->extra, this_compressor_auto_fwd_args_to_read_extra, NULL);
}

bool this_compressor_auto_fwd_args_to_print_extra(
        carbon_compressor_t *compressor, carbon_io_device_t *src, void *extra
        ) {
    carbon_err_t err;

    // uses knowledge about internals of carbon_io_device_t
    return carbon_compressor_print_extra(&err, compressor, (FILE *)extra, (carbon_memfile_t *)src->extra, 0);
}

bool carbon_compressor_auto_print_extra(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src, size_t nbytes)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_AUTO);

    CARBON_UNUSED(self);
    CARBON_UNUSED(file);
    CARBON_UNUSED(src);
    CARBON_UNUSED(nbytes);

    carbon_io_device_t io;
    carbon_io_device_from_memfile(&io, src);

    return this_auto_read_extra(&io, (carbon_compressor_auto_extra_t *)self->extra, this_compressor_auto_fwd_args_to_print_extra, file);
}

CARBON_EXPORT(bool)
carbon_compressor_auto_print_encoded_string(carbon_compressor_t *self,
                                                 FILE *file,
                                                 carbon_memfile_t *src,
                                                 uint32_t decompressed_strlen)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_AUTO);

    carbon_compressor_t *compressor =
            this_find_compressor_for_position((carbon_compressor_auto_extra_t *)self->extra, CARBON_MEMFILE_TELL(src));

    carbon_err_t err;
    return carbon_compressor_print_encoded(&err, compressor, file, src, decompressed_strlen);
}

CARBON_EXPORT(bool)
carbon_compressor_auto_prepare_entries(carbon_compressor_t *self,
                                       carbon_vec_t ofType(carbon_strdic_entry_t) *entries)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_AUTO);

    carbon_compressor_auto_extra_t *extra =
            (carbon_compressor_auto_extra_t *)self->extra;

    carbon_err_t err;

    qsort(entries->base, entries->num_elems, sizeof(carbon_strdic_entry_t), this_sort_strdic_entries_by_key_id);


    carbon_vec_t ofType(carbon_strdic_entry_t) keys;
    carbon_vec_create(&keys, NULL, sizeof(carbon_strdic_entry_t), carbon_hashmap_length(extra->context->values));

    // Build a vector of keys and sort it so they are in the same order as the entries
    {
        for(carbon_hashmap_iterator_t it = carbon_hashmap_begin(extra->compressors);it.valid;carbon_hashmap_next(&it)) {
            if(strcmp(it.key, "") == 0) {
                carbon_strdic_entry_t entry = { .string = it.key, .grouping_key = 0, .id = 0 };
                carbon_vec_push(&keys, &entry, 1);
            } else {
                carbon_string_id_t *id_ptr;

                carbon_strdic_locate_fast(&id_ptr, extra->context->dic, (char * const *)&it.key, 1);

                carbon_strdic_entry_t entry = { .string = it.key, .grouping_key = *id_ptr, .id = *id_ptr };
                carbon_vec_push(&keys, &entry, 1);

                carbon_strdic_free(extra->context->dic, id_ptr);
            }
        }

        qsort(keys.base, keys.num_elems, sizeof(carbon_strdic_entry_t), this_sort_strdic_entries_by_key_id);
    }


    uint32_t offset = 0;

    for(size_t i = 0; i < keys.num_elems; ++i) {
        carbon_strdic_entry_t const *key = carbon_vec_at(&keys, i);

        for(; offset < entries->num_elems && ((carbon_strdic_entry_t const *)carbon_vec_at(entries, offset))->grouping_key < key->id;++offset);

        uint32_t num_values = 0;
        for(; offset + num_values < entries->num_elems && ((carbon_strdic_entry_t const *)carbon_vec_at(entries, offset + num_values))->grouping_key == key->grouping_key;++num_values);

        if(num_values == 0)
            continue;

        carbon_compressor_t *compressor;
        carbon_hashmap_get(extra->compressors, key->string, (void **)&compressor);

        carbon_compressor_auto_range_t range = { .key = key->string, .length = num_values, .compressor = compressor };
        carbon_vec_push(&extra->ranges, &range, 1);

        carbon_vec_t values_for_key = *entries;
        values_for_key.base = (carbon_strdic_entry_t *)entries->base + offset;
        values_for_key.num_elems = num_values;
        values_for_key.cap_elems = entries->cap_elems - offset;

        carbon_compressor_prepare_entries(&err, compressor, &values_for_key);

        offset += num_values;
    }

    carbon_vec_drop(&keys);
    CARBON_UNUSED(entries);

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_auto_encode_string(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err,
                                          const char *string, carbon_string_id_t grouping_key)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_AUTO);
    carbon_compressor_auto_extra_t *extra =
            (carbon_compressor_auto_extra_t *)self->extra;

    carbon_compressor_auto_range_t const *range =
            (carbon_compressor_auto_range_t const *)carbon_vec_at(&extra->ranges, extra->current_range_idx);

    // Jump to the next range if the end was reached
    if(extra->current_position == range->length) {
        extra->current_position = 0;
        extra->current_range_idx++;

        range = (carbon_compressor_auto_range_t const *)carbon_vec_at(&extra->ranges, extra->current_range_idx);
        extra->current_range_compressor = NULL;
    }

    if(extra->current_range_compressor == NULL) {
        carbon_off_t current_file_pos = CARBON_MEMFILE_TELL(dst);
        carbon_memfile_seek(dst, extra->range_begin_table_offset + extra->current_range_idx * sizeof(carbon_off_t));
        carbon_memfile_write(dst, &current_file_pos, sizeof(current_file_pos));
        carbon_memfile_seek(dst, current_file_pos);

        carbon_hashmap_get(extra->compressors, range->key, (void **)&extra->current_range_compressor);
    }

    CARBON_SUCCESS_OR_JUMP(carbon_compressor_encode(err, extra->current_range_compressor, dst, string, grouping_key), error_handling);

    extra->current_position++;
    return true;

error_handling:
    CARBON_ERROR(err, CARBON_ERR_IO)
    return false;
}

CARBON_EXPORT(bool)
carbon_compressor_auto_decode_string(carbon_compressor_t *self, char *dst, size_t strlen, FILE *src)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_AUTO);

    carbon_compressor_t *compressor =
            this_find_compressor_for_position((carbon_compressor_auto_extra_t *)self->extra, (carbon_off_t)ftell(src));

    carbon_err_t err;
    return carbon_compressor_decode(&err, compressor, dst, strlen, src);
}
