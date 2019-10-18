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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/hexdump.h>
#include <jakson/carbon/path/index.h>
#include <jakson/carbon/key.h>
#include <jakson/carbon/internal.h>
#include <jakson/carbon/string.h>
#include <jakson/carbon/insert.h>
#include <jakson/carbon/commit.h>

// ---------------------------------------------------------------------------------------------------------------------
//  config
// ---------------------------------------------------------------------------------------------------------------------

#define PATH_INDEX_CAPACITY 1024

#define PATH_MARKER_PROP_NODE 'P'
#define PATH_MARKER_ARRAY_NODE 'a'
#define PATH_MARKER_COLUMN_NODE 'A'

// ---------------------------------------------------------------------------------------------------------------------
//  types
// ---------------------------------------------------------------------------------------------------------------------

struct path_index_node {
        path_index_node_e type;

        union {
                u64 pos;
                struct {
                        const char *name;
                        u64 name_len;
                        offset_t offset;
                } key;
        } entry;

        carbon_field_type_e field_type;
        offset_t field_offset;

        vector ofType(struct path_index_node) sub_entries;
};

// ---------------------------------------------------------------------------------------------------------------------
//  helper prototypes
// ---------------------------------------------------------------------------------------------------------------------

static void
array_to_str(string_buffer *str, carbon_path_index *index, bool is_root, unsigned intent_level);

static void array_into_carbon(carbon_insert *ins, carbon_path_index *index, bool is_root);

static void prop_to_str(string_buffer *str, carbon_path_index *index, unsigned intent_level);

static void prop_into_carbon(carbon_insert *ins, carbon_path_index *index);

static void column_to_str(string_buffer *str, carbon_path_index *index, unsigned intent_level);

static void column_into_carbon(carbon_insert *ins, carbon_path_index *index);

static void object_build_index(struct path_index_node *parent, carbon_object_it *elem_it);

static void array_build_index(struct path_index_node *parent, carbon_array_it *elem_it);

static void node_flat(memfile *file, struct path_index_node *node);

// ---------------------------------------------------------------------------------------------------------------------
//  helper
// ---------------------------------------------------------------------------------------------------------------------

static void intent(string_buffer *str, unsigned intent)
{
        string_buffer_add_char(str, '\n');
        for (unsigned i = 0; i < intent; i++) {
                string_buffer_add(str, "    ");
        }
}

static void path_index_node_init(struct path_index_node *node)
{
        ZERO_MEMORY(node, sizeof(struct path_index_node));
        vector_create(&node->sub_entries, NULL, sizeof(struct path_index_node), 10);
        node->type = PATH_ROOT;
}

static void path_index_node_drop(struct path_index_node *node)
{
        for (u32 i = 0; i < node->sub_entries.num_elems; i++) {
                struct path_index_node *sub = VECTOR_GET(&node->sub_entries, i, struct path_index_node);
                path_index_node_drop(sub);
        }
        vector_drop(&node->sub_entries);
}

static void path_index_node_new_array_element(struct path_index_node *node, u64 pos, offset_t value_off)
{
        path_index_node_init(node);
        node->type = PATH_INDEX_ARRAY_INDEX;
        node->entry.pos = pos;
        node->field_offset = value_off;
}

static void path_index_node_new_column_element(struct path_index_node *node, u64 pos, offset_t value_off)
{
        path_index_node_init(node);
        node->type = PATH_INDEX_COLUMN_INDEX;
        node->entry.pos = pos;
        node->field_offset = value_off;
}

static void path_index_node_new_object_prop(struct path_index_node *node, offset_t key_off, const char *name,
                                            u64 name_len, offset_t value_off)
{
        path_index_node_init(node);
        node->type = PATH_INDEX_PROP_KEY;
        node->entry.key.offset = key_off;
        node->entry.key.name = name;
        node->entry.key.name_len = name_len;
        node->field_offset = value_off;
}

static void path_index_node_set_field_type(struct path_index_node *node, carbon_field_type_e field_type)
{
        node->field_type = field_type;
}

static struct path_index_node *
path_index_node_add_array_elem(struct path_index_node *parent, u64 pos, offset_t value_off)
{
        /** For elements in array, the type marker (e.g., [c]) is contained. That is needed since the element might
         * be a container */
        struct path_index_node *sub = VECTOR_NEW_AND_GET(&parent->sub_entries, struct path_index_node);
        path_index_node_new_array_element(sub, pos, value_off);
        return sub;
}

static struct path_index_node *
path_index_node_add_column_elem(struct path_index_node *parent, u64 pos, offset_t value_off)
{
        /** For elements in column, there is no type marker since no value is allowed to be a container */
        struct path_index_node *sub = VECTOR_NEW_AND_GET(&parent->sub_entries, struct path_index_node);
        path_index_node_new_column_element(sub, pos, value_off);
        return sub;
}

static struct path_index_node *path_index_node_add_key_elem(struct path_index_node *parent, offset_t key_off,
                                                            const char *name, u64 name_len, offset_t value_off)
{
        struct path_index_node *sub = VECTOR_NEW_AND_GET(&parent->sub_entries, struct path_index_node);
        path_index_node_new_object_prop(sub, key_off, name, name_len, value_off);
        return sub;
}

static void path_index_node_print_level(FILE *file, struct path_index_node *node, unsigned level)
{
        for (unsigned i = 0; i < level; i++) {
                fprintf(file, " ");
        }
        if (node->type == PATH_ROOT) {
                fprintf(file, "root");
        } else if (node->type == PATH_INDEX_ARRAY_INDEX) {
                fprintf(file, "array_idx(%"PRIu64"), ", node->entry.pos);
        } else if (node->type == PATH_INDEX_COLUMN_INDEX) {
                fprintf(file, "column_idx(%"PRIu64"), ", node->entry.pos);
        } else {
                fprintf(file, "key('%*.*s', offset: 0x%x), ", 0, (int) node->entry.key.name_len, node->entry.key.name,
                        (unsigned) node->entry.key.offset);
        }
        if (node->type != PATH_ROOT) {
                fprintf(file, "field(type: %s, offset: 0x%x)\n", carbon_field_type_str(NULL, node->field_type),
                        (unsigned) node->field_offset);
        } else {
                fprintf(file, "\n");
        }

        for (u32 i = 0; i < node->sub_entries.num_elems; i++) {
                struct path_index_node *sub = VECTOR_GET(&node->sub_entries, i, struct path_index_node);
                path_index_node_print_level(file, sub, level + 1);
        }
}

static const void *
record_ref_read(carbon_key_e *key_type, u64 *key_length, u64 *commit_hash, memfile *memfile)
{
        memfile_save_position(memfile);
        memfile_seek(memfile, 0);
        const void *ret = carbon_key_read(key_length, key_type, memfile);
        u64 *hash = MEMFILE_READ_TYPE(memfile, u64);
        OPTIONAL_SET(commit_hash, *hash);
        memfile_restore_position(memfile);
        return ret;
}

static void record_ref_create(memfile *memfile, carbon *doc)
{
        carbon_key_e key_type;
        u64 commit_hash;
        carbon_key_type(&key_type, doc);
        carbon_commit_hash(&commit_hash, doc);

        /** write record key */
        memfile_seek(memfile, 0);
        carbon_key_create(memfile, key_type, &doc->err);
        switch (key_type) {
                case CARBON_KEY_NOKEY: {
                        /** nothing to do */
                }
                        break;
                case CARBON_KEY_AUTOKEY:
                case CARBON_KEY_UKEY: {
                        u64 key;
                        carbon_key_unsigned_value(&key, doc);
                        memfile_seek(memfile, 0);
                        carbon_key_write_unsigned(memfile, key);
                }
                        break;
                case CARBON_KEY_IKEY: {
                        DECLARE_AND_INIT(i64, key)
                        carbon_key_signed_value(&key, doc);
                        memfile_seek(memfile, 0);
                        carbon_key_write_signed(memfile, key);
                }
                        break;
                case CARBON_KEY_SKEY: {
                        u64 len;
                        const char *key = carbon_key_string_value(&len, doc);
                        memfile_seek(memfile, 0);
                        carbon_key_update_string_wnchar(memfile, key, len);
                }
                        break;
                default: ERROR(&doc->err, ERR_TYPEMISMATCH)
        }

        /** write record version */
        memfile_write(memfile, &commit_hash, sizeof(u64));
}

static void array_traverse(struct path_index_node *parent, carbon_array_it *it)
{
        u64 sub_elem_pos = 0;
        while (carbon_array_it_next(it)) {
                offset_t sub_elem_off = carbon_array_it_tell(it);
                struct path_index_node *elem_node = path_index_node_add_array_elem(parent, sub_elem_pos, sub_elem_off);
                array_build_index(elem_node, it);

                sub_elem_pos++;
        }
}

static void column_traverse(struct path_index_node *parent, carbon_column_it *it)
{
        carbon_field_type_e column_type;
        carbon_field_type_e entry_type;
        u32 nvalues = 0;

        carbon_column_it_values_info(&column_type, &nvalues, it);

        for (u32 i = 0; i < nvalues; i++) {
                bool is_null = carbon_column_it_value_is_null(it, i);
                bool is_true = false;
                if (carbon_field_type_is_column_bool_or_subtype(column_type)) {
                        is_true = carbon_column_it_boolean_values(NULL, it)[i];
                }
                entry_type = carbon_field_type_column_entry_to_regular_type(column_type, is_null, is_true);
                offset_t sub_elem_off = carbon_column_it_tell(it, i);

                struct path_index_node *node = path_index_node_add_column_elem(parent, i, sub_elem_off);
                path_index_node_set_field_type(node, entry_type);
        }
}

static void object_traverse(struct path_index_node *parent, carbon_object_it *it)
{
        while (carbon_object_it_next(it)) {
                u64 prop_name_len = 0;
                offset_t key_off = 0, value_off = 0;
                carbon_object_it_tell(&key_off, &value_off, it);
                const char *prop_name = carbon_object_it_prop_name(&prop_name_len, it);
                struct path_index_node *elem_node = path_index_node_add_key_elem(parent, key_off,
                                                                                 prop_name, prop_name_len, value_off);
                object_build_index(elem_node, it);
        }
}

static void object_build_index(struct path_index_node *parent, carbon_object_it *elem_it)
{
        carbon_field_type_e field_type = 0;;
        carbon_object_it_prop_type(&field_type, elem_it);
        path_index_node_set_field_type(parent, field_type);

        switch (field_type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                case CARBON_FIELD_STRING:
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I8:
                case CARBON_FIELD_NUMBER_I16:
                case CARBON_FIELD_NUMBER_I32:
                case CARBON_FIELD_NUMBER_I64:
                case CARBON_FIELD_NUMBER_FLOAT:
                case CARBON_FIELD_BINARY:
                case CARBON_FIELD_BINARY_CUSTOM:
                        /** path ends here */
                        break;
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET: {
                        carbon_column_it *it = carbon_object_it_column_value(elem_it);
                        column_traverse(parent, it);

                }
                        break;
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET: {
                        carbon_array_it *it = carbon_object_it_array_value(elem_it);
                        array_traverse(parent, it);
                        carbon_array_it_drop(it);
                }
                        break;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP: {
                        carbon_object_it *it = carbon_object_it_object_value(elem_it);
                        object_traverse(parent, it);
                        carbon_object_it_drop(it);
                }
                        break;
                default: ERROR(&elem_it->err, ERR_INTERNALERR);
        }
}

static void array_build_index(struct path_index_node *parent, carbon_array_it *elem_it)
{
        carbon_field_type_e field_type;
        carbon_array_it_field_type(&field_type, elem_it);
        path_index_node_set_field_type(parent, field_type);

        switch (field_type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                case CARBON_FIELD_STRING:
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I8:
                case CARBON_FIELD_NUMBER_I16:
                case CARBON_FIELD_NUMBER_I32:
                case CARBON_FIELD_NUMBER_I64:
                case CARBON_FIELD_NUMBER_FLOAT:
                case CARBON_FIELD_BINARY:
                case CARBON_FIELD_BINARY_CUSTOM:
                        /** path ends here */
                        break;
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET: {
                        carbon_column_it *it = carbon_array_it_column_value(elem_it);
                        column_traverse(parent, it);

                }
                        break;
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET: {
                        carbon_array_it *it = carbon_array_it_array_value(elem_it);
                        array_traverse(parent, it);
                        carbon_array_it_drop(it);
                }
                        break;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP: {
                        carbon_object_it *it = carbon_array_it_object_value(elem_it);
                        object_traverse(parent, it);
                        carbon_object_it_drop(it);
                }
                        break;
                default: ERROR(&elem_it->err, ERR_INTERNALERR);
        }
}

static void field_ref_write(memfile *file, struct path_index_node *node)
{
        memfile_write_byte(file, node->field_type);
        if (node->field_type != CARBON_FIELD_NULL && node->field_type != CARBON_FIELD_TRUE &&
            node->field_type != CARBON_FIELD_FALSE) {
                /** only in case of field type that is not null, true, or false, there is more information behind
                 * the field offset */
                memfile_write_uintvar_stream(NULL, file, node->field_offset);
        }
}

static void container_contents_flat(memfile *file, struct path_index_node *node)
{
        memfile_write_uintvar_stream(NULL, file, node->sub_entries.num_elems);

        /** write position offsets */
        offset_t position_off_latest = memfile_tell(file);
        for (u32 i = 0; i < node->sub_entries.num_elems; i++) {
                memfile_write_uintvar_stream(NULL, file, 0);
        }

        for (u32 i = 0; i < node->sub_entries.num_elems; i++) {
                offset_t node_off = memfile_tell(file);
                struct path_index_node *sub = VECTOR_GET(&node->sub_entries, i, struct path_index_node);
                node_flat(file, sub);
                memfile_save_position(file);
                memfile_seek(file, position_off_latest);
                signed_offset_t shift = memfile_update_uintvar_stream(file, node_off);
                position_off_latest = memfile_tell(file);
                memfile_restore_position(file);
                memfile_seek_from_here(file, shift);
        }
}

static void container_field_flat(memfile *file, struct path_index_node *node)
{
        switch (node->field_type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                case CARBON_FIELD_STRING:
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I8:
                case CARBON_FIELD_NUMBER_I16:
                case CARBON_FIELD_NUMBER_I32:
                case CARBON_FIELD_NUMBER_I64:
                case CARBON_FIELD_NUMBER_FLOAT:
                case CARBON_FIELD_BINARY:
                case CARBON_FIELD_BINARY_CUSTOM:
                        /** any path will end with this kind of field, and therefore no subsequent elements exists */
                        JAK_ASSERT(node->sub_entries.num_elems == 0);
                        break;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP:
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET:
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                        /** each of these field types allows for further path traversals, and therefore at least one
                         * subsequent path element must exist */
                        container_contents_flat(file, node);
                        break;
                default: ERROR(&file->err, ERR_INTERNALERR);
        }
}

static void prop_flat(memfile *file, struct path_index_node *node)
{
        memfile_write_byte(file, PATH_MARKER_PROP_NODE);
        field_ref_write(file, node);
        memfile_write_uintvar_stream(NULL, file, node->entry.key.offset);
        container_field_flat(file, node);
}

static void array_flat(memfile *file, struct path_index_node *node)
{
        memfile_write_byte(file, PATH_MARKER_ARRAY_NODE);
        field_ref_write(file, node);
        if (UNLIKELY(node->type == PATH_ROOT)) {
                container_contents_flat(file, node);
        } else {
                container_field_flat(file, node);
        }
}

FUNC_UNUSED
static void node_into_carbon(carbon_insert *ins, carbon_path_index *index)
{
        u8 next = memfile_peek_byte(&index->memfile);
        switch (next) {
                case PATH_MARKER_PROP_NODE:
                        prop_into_carbon(ins, index);
                        break;
                case PATH_MARKER_ARRAY_NODE:
                        array_into_carbon(ins, index, false);
                        break;
                case PATH_MARKER_COLUMN_NODE:
                        column_into_carbon(ins, index);
                        break;
                default: ERROR(&index->err, ERR_CORRUPTED)
        }
}

static void node_to_str(string_buffer *str, carbon_path_index *index, unsigned intent_level)
{
        u8 next = memfile_peek_byte(&index->memfile);
        intent_level++;

        switch (next) {
                case PATH_MARKER_PROP_NODE:
                        prop_to_str(str, index, intent_level);
                        break;
                case PATH_MARKER_ARRAY_NODE:
                        array_to_str(str, index, false, intent_level);
                        break;
                case PATH_MARKER_COLUMN_NODE:
                        column_to_str(str, index, intent_level);
                        break;
                default: ERROR(&index->err, ERR_CORRUPTED)
        }
}

static u8 field_ref_into_carbon(carbon_insert *ins, carbon_path_index *index, bool is_root)
{
        u8 field_type = memfile_read_byte(&index->memfile);

        if (is_root) {
                carbon_insert_prop_null(ins, "container");
        } else {
                carbon_insert_prop_string(ins, "container", carbon_field_type_str(NULL, field_type));
        }


        if (field_type != CARBON_FIELD_NULL && field_type != CARBON_FIELD_TRUE &&
            field_type != CARBON_FIELD_FALSE) {
                /** only in case of field type that is not null, true, or false, there is more information behind
                 * the field offset */
                u64 field_offset = memfile_read_uintvar_stream(NULL, &index->memfile);
                if (is_root) {
                        carbon_insert_prop_null(ins, "offset");
                } else {
                        string_buffer str;
                        string_buffer_create(&str);
                        string_buffer_add_u64_as_hex_0x_prefix_compact(&str, field_offset);
                        carbon_insert_prop_string(ins, "offset", string_cstr(&str));
                        string_buffer_drop(&str);
                }
        } else {
                carbon_insert_prop_null(ins, "offset");
        }
        return field_type;
}

static u8 field_ref_to_str(string_buffer *str, carbon_path_index *index)
{
        u8 field_type = memfile_read_byte(&index->memfile);

        string_buffer_add_char(str, '[');
        string_buffer_add_char(str, field_type);
        string_buffer_add_char(str, ']');

        if (field_type != CARBON_FIELD_NULL && field_type != CARBON_FIELD_TRUE &&
            field_type != CARBON_FIELD_FALSE) {
                /** only in case of field type that is not null, true, or false, there is more information behind
                 * the field offset */
                u64 field_offset = memfile_read_uintvar_stream(NULL, &index->memfile);
                string_buffer_add_char(str, '(');
                string_buffer_add_u64_as_hex_0x_prefix_compact(str, field_offset);
                string_buffer_add_char(str, ')');
        }

        return field_type;
}

static void column_to_str(string_buffer *str, carbon_path_index *index, unsigned intent_level)
{
        intent(str, intent_level);
        u8 marker = memfile_read_byte(&index->memfile);
        string_buffer_add_char(str, '[');
        string_buffer_add_char(str, marker);
        string_buffer_add_char(str, ']');

        field_ref_to_str(str, index);
}

static u8 _insert_field_ref(carbon_insert *ins, carbon_path_index *index, bool is_root)
{
        carbon_insert_object_state object;
        carbon_insert *oins = carbon_insert_prop_object_begin(&object, ins, "record-reference", 1024);
        u8 ret = field_ref_into_carbon(oins, index, is_root);
        carbon_insert_prop_object_end(&object);
        return ret;
}

FUNC_UNUSED
static void column_into_carbon(carbon_insert *ins, carbon_path_index *index)
{
        MEMFILE_SKIP_BYTE(&index->memfile);
        carbon_insert_prop_string(ins, "type", "column");
        _insert_field_ref(ins, index, false);
}

static void container_contents_into_carbon(carbon_insert *ins, carbon_path_index *index)
{
        u64 num_elems = memfile_read_uintvar_stream(NULL, &index->memfile);
        carbon_insert_prop_unsigned(ins, "element-count", num_elems);

        carbon_insert_array_state array;
        carbon_insert *ains = carbon_insert_prop_array_begin(&array, ins, "element-offsets", 1024);

        string_buffer str;
        string_buffer_create(&str);
        for (u32 i = 0; i < num_elems; i++) {
                u64 pos_offs = memfile_read_uintvar_stream(NULL, &index->memfile);
                string_buffer_clear(&str);
                string_buffer_add_u64_as_hex_0x_prefix_compact(&str, pos_offs);
                carbon_insert_string(ains, string_cstr(&str));
        }
        string_buffer_drop(&str);

        carbon_insert_prop_array_end(&array);

        ains = carbon_insert_prop_array_begin(&array, ins, "elements", 1024);
        UNUSED(ains)
        for (u32 i = 0; i < num_elems; i++) {
                carbon_insert_object_state node_obj;
                carbon_insert *node_obj_ins = carbon_insert_object_begin(&node_obj, ains, 1024);
                node_into_carbon(node_obj_ins, index);
                carbon_insert_object_end(&node_obj);
        }
        carbon_insert_prop_array_end(&array);

}

static void
container_contents_to_str(string_buffer *str, carbon_path_index *index, unsigned intent_level)
{
        u64 num_elems = memfile_read_uintvar_stream(NULL, &index->memfile);
        string_buffer_add_char(str, '(');
        string_buffer_add_u64(str, num_elems);
        string_buffer_add_char(str, ')');

        for (u32 i = 0; i < num_elems; i++) {
                u64 pos_offs = memfile_read_uintvar_stream(NULL, &index->memfile);
                string_buffer_add_char(str, '(');
                string_buffer_add_u64_as_hex_0x_prefix_compact(str, pos_offs);
                string_buffer_add_char(str, ')');
        }

        for (u32 i = 0; i < num_elems; i++) {
                node_to_str(str, index, intent_level);
        }
}

static void
container_to_str(string_buffer *str, carbon_path_index *index, u8 field_type, unsigned intent_level)
{
        switch (field_type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                case CARBON_FIELD_STRING:
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I8:
                case CARBON_FIELD_NUMBER_I16:
                case CARBON_FIELD_NUMBER_I32:
                case CARBON_FIELD_NUMBER_I64:
                case CARBON_FIELD_NUMBER_FLOAT:
                case CARBON_FIELD_BINARY:
                case CARBON_FIELD_BINARY_CUSTOM:
                        /** nothing to do */
                        break;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP:
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET:
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET: {
                        /** subsequent path elements to be printed */
                        container_contents_to_str(str, index, ++intent_level);
                }
                        break;
                default: ERROR(&index->err, ERR_INTERNALERR);
        }
}

static void container_into_carbon(carbon_insert *ins, carbon_path_index *index, u8 field_type)
{
        switch (field_type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                case CARBON_FIELD_STRING:
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I8:
                case CARBON_FIELD_NUMBER_I16:
                case CARBON_FIELD_NUMBER_I32:
                case CARBON_FIELD_NUMBER_I64:
                case CARBON_FIELD_NUMBER_FLOAT:
                case CARBON_FIELD_BINARY:
                case CARBON_FIELD_BINARY_CUSTOM:
                        /** nothing to do */
                        break;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP:
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET:
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET: {
                        /** subsequent path elements to be printed */
                        container_contents_into_carbon(ins, index);
                }
                        break;
                default: ERROR(&index->err, ERR_INTERNALERR);
        }
}

static void prop_to_str(string_buffer *str, carbon_path_index *index, unsigned intent_level)
{
        intent(str, intent_level++);

        u8 marker = memfile_read_byte(&index->memfile);
        string_buffer_add_char(str, '[');
        string_buffer_add_char(str, marker);
        string_buffer_add_char(str, ']');

        u8 field_type = field_ref_to_str(str, index);

        u64 key_offset = memfile_read_uintvar_stream(NULL, &index->memfile);

        string_buffer_add_char(str, '(');
        string_buffer_add_u64_as_hex_0x_prefix_compact(str, key_offset);
        string_buffer_add_char(str, ')');

        container_to_str(str, index, field_type, intent_level);
}

FUNC_UNUSED
static void prop_into_carbon(carbon_insert *ins, carbon_path_index *index)
{
        MEMFILE_SKIP_BYTE(&index->memfile);
        carbon_insert_prop_string(ins, "type", "key");
        u8 field_type = _insert_field_ref(ins, index, false);

        string_buffer str;
        string_buffer_create(&str);

        u64 key_offset = memfile_read_uintvar_stream(NULL, &index->memfile);
        string_buffer_add_u64_as_hex_0x_prefix_compact(&str, key_offset);
        carbon_insert_prop_string(ins, "key", string_cstr(&str));
        string_buffer_drop(&str);

        container_into_carbon(ins, index, field_type);
}

static void array_into_carbon(carbon_insert *ins, carbon_path_index *index, bool is_root)
{
        MEMFILE_SKIP_BYTE(&index->memfile);
        u8 field_type;

        carbon_insert_prop_string(ins, "parent", is_root ? "record" : "array");
        field_type = _insert_field_ref(ins, index, is_root);

        carbon_insert_object_state object;
        carbon_insert *oins = carbon_insert_prop_object_begin(&object, ins, "nodes", 1024);
        if (UNLIKELY(is_root)) {
                container_contents_into_carbon(oins, index);
        } else {
                container_into_carbon(oins, index, field_type);
        }
        carbon_insert_prop_object_end(&object);
}

static void
array_to_str(string_buffer *str, carbon_path_index *index, bool is_root, unsigned intent_level)
{
        intent(str, intent_level++);

        u8 marker = memfile_read_byte(&index->memfile);
        string_buffer_add_char(str, '[');
        string_buffer_add_char(str, marker);
        string_buffer_add_char(str, ']');

        u8 field_type = field_ref_to_str(str, index);

        if (UNLIKELY(is_root)) {
                container_contents_to_str(str, index, intent_level);
        } else {
                container_to_str(str, index, field_type, intent_level);
        }
}

static void column_flat(memfile *file, struct path_index_node *node)
{
        memfile_write_byte(file, PATH_MARKER_COLUMN_NODE);
        field_ref_write(file, node);
        JAK_ASSERT(node->sub_entries.num_elems == 0);
}

static void node_flat(memfile *file, struct path_index_node *node)
{
        switch (node->type) {
                case PATH_INDEX_PROP_KEY:
                        prop_flat(file, node);
                        break;
                case PATH_INDEX_ARRAY_INDEX:
                        array_flat(file, node);
                        break;
                case PATH_INDEX_COLUMN_INDEX:
                        column_flat(file, node);
                        break;
                default: ERROR(&file->err, ERR_INTERNALERR);
                        return;
        }
}

static void index_flat(memfile *file, struct path_index_node *root_array)
{
        array_flat(file, root_array);
}

static void index_build(memfile *file, carbon *doc)
{
        struct path_index_node root_array;

        /** init */
        path_index_node_init(&root_array);

        carbon_array_it it;
        u64 array_pos = 0;
        carbon_read_begin(&it, doc);

        /** build index as tree structure */
        while (carbon_array_it_next(&it)) {
                offset_t entry_offset = carbon_array_it_tell(&it);
                struct path_index_node *node = path_index_node_add_array_elem(&root_array, array_pos, entry_offset);
                array_build_index(node, &it);
                array_pos++;
        }
        carbon_read_end(&it);

        /** for debug */
        path_index_node_print_level(stdout, &root_array, 0); // TODO: Debug remove

        index_flat(file, &root_array);
        memfile_shrink(file);

        /** cleanup */
        path_index_node_drop(&root_array);
}

static void record_ref_to_str(string_buffer *str, carbon_path_index *index)
{
        u8 key_type = memfile_read_byte(&index->memfile);
        string_buffer_add_char(str, '[');
        string_buffer_add_char(str, key_type);
        string_buffer_add_char(str, ']');

        switch (key_type) {
                case CARBON_KEY_NOKEY:
                        /** nothing to do */
                        break;
                case CARBON_KEY_AUTOKEY:
                case CARBON_KEY_UKEY: {
                        u64 key = memfile_read_u64(&index->memfile);
                        string_buffer_add_char(str, '[');
                        string_buffer_add_u64(str, key);
                        string_buffer_add_char(str, ']');
                }
                        break;
                case CARBON_KEY_IKEY: {
                        i64 key = memfile_read_i64(&index->memfile);
                        string_buffer_add_char(str, '[');;
                        string_buffer_add_i64(str, key);
                        string_buffer_add_char(str, ']');
                }
                        break;
                case CARBON_KEY_SKEY: {
                        u64 key_len;
                        const char *key = carbon_string_read(&key_len, &index->memfile);
                        string_buffer_add_char(str, '(');
                        string_buffer_add_nchar(str, key, key_len);
                        string_buffer_add_char(str, ')');
                }
                        break;
                default: ERROR(&index->err, ERR_INTERNALERR);
        }
        u64 commit_hash = memfile_read_u64(&index->memfile);
        string_buffer_add_char(str, '[');
        string_buffer_add_u64(str, commit_hash);
        string_buffer_add_char(str, ']');
}

static void record_ref_to_carbon(carbon_insert *roins, carbon_path_index *index)
{
        char key_type = memfile_read_byte(&index->memfile);
        carbon_insert_prop_string(roins, "key-type", carbon_key_type_str(key_type));

        switch (key_type) {
                case CARBON_KEY_NOKEY:
                        /** nothing to do */
                        break;
                case CARBON_KEY_AUTOKEY:
                case CARBON_KEY_UKEY: {
                        u64 key = memfile_read_u64(&index->memfile);
                        carbon_insert_prop_unsigned(roins, "key-value", key);
                }
                        break;
                case CARBON_KEY_IKEY: {
                        i64 key = memfile_read_i64(&index->memfile);
                        carbon_insert_prop_signed(roins, "key-value", key);
                }
                        break;
                case CARBON_KEY_SKEY: {
                        u64 key_len;
                        const char *key = carbon_string_read(&key_len, &index->memfile);
                        carbon_insert_prop_nchar(roins, "key-value", key, key_len);
                }
                        break;
                default: ERROR(&index->err, ERR_INTERNALERR);
        }
        u64 commit_hash = memfile_read_u64(&index->memfile);
        string_buffer str;
        string_buffer_create(&str);
        carbon_commit_hash_to_str(&str, commit_hash);
        carbon_insert_prop_string(roins, "commit-hash", string_cstr(&str));
        string_buffer_drop(&str);
}

// ---------------------------------------------------------------------------------------------------------------------
//  construction and deconstruction
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_create(carbon_path_index *index, carbon *doc)
{
        ERROR_IF_NULL(index);
        ERROR_IF_NULL(doc);
        memblock_create(&index->memblock, PATH_INDEX_CAPACITY);
        memfile_open(&index->memfile, index->memblock, READ_WRITE);
        error_init(&index->err);
        record_ref_create(&index->memfile, doc);
        index_build(&index->memfile, doc);
        return true;
}

bool carbon_path_index_drop(carbon_path_index *index)
{
        UNUSED(index)
        return false;
}

// ---------------------------------------------------------------------------------------------------------------------
//  index data access and meta information
// ---------------------------------------------------------------------------------------------------------------------

const void *carbon_path_index_raw_data(u64 *size, carbon_path_index *index)
{
        if (size && index) {
                const char *raw = memblock_raw_data(index->memfile.memblock);
                memblock_size(size, index->memfile.memblock);
                return raw;
        } else {
                return NULL;
        }
}

bool carbon_path_index_commit_hash(u64 *commit_hash, carbon_path_index *index)
{
        ERROR_IF_NULL(commit_hash)
        ERROR_IF_NULL(index)
        record_ref_read(NULL, NULL, commit_hash, &index->memfile);
        return true;
}

bool carbon_path_index_key_type(carbon_key_e *key_type, carbon_path_index *index)
{
        ERROR_IF_NULL(key_type)
        ERROR_IF_NULL(index)
        record_ref_read(key_type, NULL, NULL, &index->memfile);
        return true;
}

bool carbon_path_index_key_unsigned_value(u64 *key, carbon_path_index *index)
{
        ERROR_IF_NULL(key)
        ERROR_IF_NULL(index)
        carbon_key_e key_type;
        u64 ret = *(u64 *) record_ref_read(&key_type, NULL, NULL, &index->memfile);
        ERROR_IF(key_type != CARBON_KEY_AUTOKEY && key_type != CARBON_KEY_UKEY, &index->err, ERR_TYPEMISMATCH);
        *key = ret;
        return true;
}

bool carbon_path_index_key_signed_value(i64 *key, carbon_path_index *index)
{
        ERROR_IF_NULL(key)
        ERROR_IF_NULL(index)
        carbon_key_e key_type;
        i64 ret = *(i64 *) record_ref_read(&key_type, NULL, NULL, &index->memfile);
        ERROR_IF(key_type != CARBON_KEY_IKEY, &index->err, ERR_TYPEMISMATCH);
        *key = ret;
        return true;
}

const char *carbon_path_index_key_string_value(u64 *str_len, carbon_path_index *index)
{
        if (str_len && index) {
                carbon_key_e key_type;
                const char *ret = (const char *) record_ref_read(&key_type, str_len, NULL, &index->memfile);
                ERROR_IF(key_type != CARBON_KEY_SKEY, &index->err, ERR_TYPEMISMATCH);
                return ret;
        } else {
                ERROR(&index->err, ERR_NULLPTR);
                return NULL;
        }
}

bool carbon_path_index_indexes_doc(carbon_path_index *index, carbon *doc)
{
        ERROR_IF_NULL(doc);

        u64 index_hash = 0, doc_hash = 0;
        carbon_path_index_commit_hash(&index_hash, index);
        carbon_commit_hash(&doc_hash, doc);
        if (LIKELY(index_hash == doc_hash)) {
                carbon_key_e index_key_type, doc_key_type;
                carbon_path_index_key_type(&index_key_type, index);
                carbon_key_type(&doc_key_type, doc);
                if (LIKELY(index_key_type == doc_key_type)) {
                        switch (index_key_type) {
                                case CARBON_KEY_NOKEY:
                                        return true;
                                case CARBON_KEY_AUTOKEY:
                                case CARBON_KEY_UKEY: {
                                        u64 index_key, doc_key;
                                        carbon_path_index_key_unsigned_value(&index_key, index);
                                        carbon_key_unsigned_value(&doc_key, doc);
                                        return index_key == doc_key;
                                }
                                case CARBON_KEY_IKEY: {
                                        i64 index_key, doc_key;
                                        carbon_path_index_key_signed_value(&index_key, index);
                                        carbon_key_signed_value(&doc_key, doc);
                                        return index_key == doc_key;
                                }
                                case CARBON_KEY_SKEY: {
                                        u64 index_key_len, doc_key_len;
                                        const char *index_key = carbon_path_index_key_string_value(&index_key_len,
                                                                                                   index);
                                        const char *doc_key = carbon_key_string_value(&doc_key_len, doc);
                                        return (index_key_len == doc_key_len) && (strcmp(index_key, doc_key) == 0);
                                }
                                default: ERROR(&doc->err, ERR_TYPEMISMATCH)
                                        return false;
                        }
                } else {
                        return false;
                }
        } else {
                return false;
        }
}

// ---------------------------------------------------------------------------------------------------------------------
//  index access and type information
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_it_open(carbon_path_index_it *it, carbon_path_index *index,
                               carbon *doc)
{
        ERROR_IF_NULL(it)
        ERROR_IF_NULL(index)
        ERROR_IF_NULL(doc)
        if (carbon_path_index_indexes_doc(index, doc)) {
                ZERO_MEMORY(it, sizeof(carbon_path_index_it));
                error_init(&it->err);
                memfile_open(&it->memfile, index->memfile.memblock, READ_ONLY);
                it->doc = doc;
                it->container_type = CARBON_ARRAY;
                return true;
        } else {
                ERROR(&index->err, ERR_NOTINDEXED)
                return false;
        }
}

//bool carbon_path_index_it_type(carbon_container_e *type, carbon_path_index_it *it)
//{
//
//}
//
//// ---------------------------------------------------------------------------------------------------------------------
////  array and column container functions
//// ---------------------------------------------------------------------------------------------------------------------
//
//bool carbon_path_index_it_list_length(u64 *key_len, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_list_goto(u64 pos, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_list_pos(u64 *pos, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_list_can_enter(carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_list_enter(carbon_path_index_it *it)
//{
//
//}
//
//// ---------------------------------------------------------------------------------------------------------------------
////  object container functions
//// ---------------------------------------------------------------------------------------------------------------------
//
//bool carbon_path_index_it_obj_num_props(u64 *num_props, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_obj_goto(const char *key_name, carbon_path_index_it *it)
//{
//
//}
//
//const char *carbon_path_index_it_key_name(u64 *name_len, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_obj_can_enter(carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_obj_enter(carbon_path_index_it *it)
//{
//
//}
//
//// ---------------------------------------------------------------------------------------------------------------------
////  field access
//// ---------------------------------------------------------------------------------------------------------------------
//
//bool carbon_path_index_it_field_type(carbon_field_type_e *type, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_u8_value(u8 *value, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_u16_value(u16 *value, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_u32_value(u32 *value, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_u64_value(u64 *value, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_i8_value(i8 *value, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_i16_value(i16 *value, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_i32_value(i32 *value, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_i64_value(i64 *value, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_float_value(bool *is_null_in, float *value, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_signed_value(bool *is_null_in, i64 *value, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_unsigned_value(bool *is_null_in, u64 *value, carbon_path_index_it *it)
//{
//
//}
//
//const char *carbon_path_index_it_field_string_value(u64 *strlen, carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_binary_value(carbon_binary *out, carbon_array_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_array_value(carbon_array_it *it_out, carbon_path_index_it *it_in)
//{
//
//}
//
//bool carbon_path_index_it_field_object_value(carbon_object_it *it_out, carbon_path_index_it *it_in)
//{
//
//}
//
//bool carbon_path_index_it_field_column_value(carbon_column_it *it_out, carbon_path_index_it *it_in)
//{
//
//}

// ---------------------------------------------------------------------------------------------------------------------
//  diagnostics
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_hexdump(FILE *file, carbon_path_index *index)
{
        return memfile_hexdump_printf(file, &index->memfile);
}

fn_result carbon_path_index_to_carbon(carbon *doc, carbon_path_index *index)
{
        carbon_new context;
        carbon_insert_object_state object;

        memfile_seek_to_start(&index->memfile);

        carbon_insert *ins = carbon_create_begin(&context, doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
        carbon_insert *oins = carbon_insert_object_begin(&object, ins, 1024);

        {
                carbon_insert_object_state ref_object;
                carbon_insert *roins = carbon_insert_prop_object_begin(&ref_object, oins,
                                                                                  "record-association", 1024);
                record_ref_to_carbon(roins, index);
                carbon_insert_prop_object_end(&ref_object);
        }
        {
                carbon_insert_object_state root_object;
                carbon_insert *roins = carbon_insert_prop_object_begin(&root_object, oins, "index", 1024);
                array_into_carbon(roins, index, true);
                carbon_insert_prop_object_end(&root_object);
        }

        carbon_insert_object_end(&object);
        carbon_create_end(&context);
        return FN_OK();
}

const char *carbon_path_index_to_str(string_buffer *str, carbon_path_index *index)
{
        memfile_seek_to_start(&index->memfile);
        record_ref_to_str(str, index);
        array_to_str(str, index, true, 0);
        return string_cstr(str);
}

bool carbon_path_index_print(FILE *file, carbon_path_index *index)
{
        string_buffer str;
        string_buffer_create(&str);
        memfile_save_position(&index->memfile);
        memfile_seek_to_start(&index->memfile);
        fprintf(file, "%s", carbon_path_index_to_str(&str, index));
        memfile_restore_position(&index->memfile);
        string_buffer_drop(&str);
        return true;
}
