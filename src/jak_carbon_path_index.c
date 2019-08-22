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

#include <jak_utils_hexdump.h>
#include <jak_carbon_path_index.h>
#include <jak_carbon_key.h>
#include "jak_carbon_int.h"
#include "jak_carbon_string.h"
#include "jak_carbon_insert.h"
#include "jak_carbon_commit.h"

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
        enum path_index_node_type type;

        union {
                jak_u64 pos;
                struct {
                        const char *name;
                        jak_u64 name_len;
                        jak_offset_t offset;
                } key;
        } entry;

        carbon_field_type_e field_type;
        jak_offset_t field_offset;

        struct jak_vector ofType(struct path_index_node) sub_entries;
};

// ---------------------------------------------------------------------------------------------------------------------
//  helper prototypes
// ---------------------------------------------------------------------------------------------------------------------

static void
array_to_str(struct jak_string *str, struct jak_carbon_path_index *index, bool is_root, unsigned intent_level);

static void array_into_carbon(jak_carbon_insert *ins, struct jak_carbon_path_index *index, bool is_root);

static void prop_to_str(struct jak_string *str, struct jak_carbon_path_index *index, unsigned intent_level);

static void prop_into_carbon(jak_carbon_insert *ins, struct jak_carbon_path_index *index);

static void column_to_str(struct jak_string *str, struct jak_carbon_path_index *index, unsigned intent_level);

static void column_into_carbon(jak_carbon_insert *ins, struct jak_carbon_path_index *index);

static void object_build_index(struct path_index_node *parent, struct jak_carbon_object_it *elem_it);

static void array_build_index(struct path_index_node *parent, jak_carbon_array_it *elem_it);

static void node_flat(struct jak_memfile *file, struct path_index_node *node);

// ---------------------------------------------------------------------------------------------------------------------
//  helper
// ---------------------------------------------------------------------------------------------------------------------

static void intent(struct jak_string *str, unsigned intent)
{
        string_add_char(str, '\n');
        for (unsigned i = 0; i < intent; i++) {
                string_add(str, "    ");
        }
}

static void path_index_node_init(struct path_index_node *node)
{
        JAK_zero_memory(node, sizeof(struct path_index_node));
        vec_create(&node->sub_entries, NULL, sizeof(struct path_index_node), 10);
        node->type = PATH_ROOT;
}

static void path_index_node_drop(struct path_index_node *node)
{
        for (jak_u32 i = 0; i < node->sub_entries.num_elems; i++) {
                struct path_index_node *sub = vec_get(&node->sub_entries, i, struct path_index_node);
                path_index_node_drop(sub);
        }
        vec_drop(&node->sub_entries);
}

static void path_index_node_new_array_element(struct path_index_node *node, jak_u64 pos, jak_offset_t value_off)
{
        path_index_node_init(node);
        node->type = PATH_INDEX_ARRAY_INDEX;
        node->entry.pos = pos;
        node->field_offset = value_off;
}

static void path_index_node_new_column_element(struct path_index_node *node, jak_u64 pos, jak_offset_t value_off)
{
        path_index_node_init(node);
        node->type = PATH_INDEX_COLUMN_INDEX;
        node->entry.pos = pos;
        node->field_offset = value_off;
}

static void path_index_node_new_object_prop(struct path_index_node *node, jak_offset_t key_off, const char *name,
                                            jak_u64 name_len, jak_offset_t value_off)
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
path_index_node_add_array_elem(struct path_index_node *parent, jak_u64 pos, jak_offset_t value_off)
{
        /* For elements in array, the type marker (e.g., [c]) is contained. That is needed since the element might
         * be a container */
        struct path_index_node *sub = vec_new_and_get(&parent->sub_entries, struct path_index_node);
        path_index_node_new_array_element(sub, pos, value_off);
        return sub;
}

static struct path_index_node *
path_index_node_add_column_elem(struct path_index_node *parent, jak_u64 pos, jak_offset_t value_off)
{
        /* For elements in column, there is no type marker since no value is allowed to be a container */
        struct path_index_node *sub = vec_new_and_get(&parent->sub_entries, struct path_index_node);
        path_index_node_new_column_element(sub, pos, value_off);
        return sub;
}

static struct path_index_node *path_index_node_add_key_elem(struct path_index_node *parent, jak_offset_t key_off,
                                                            const char *name, jak_u64 name_len, jak_offset_t value_off)
{
        struct path_index_node *sub = vec_new_and_get(&parent->sub_entries, struct path_index_node);
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

        for (jak_u32 i = 0; i < node->sub_entries.num_elems; i++) {
                struct path_index_node *sub = vec_get(&node->sub_entries, i, struct path_index_node);
                path_index_node_print_level(file, sub, level + 1);
        }
}

static const void *
record_ref_read(jak_carbon_key_e *key_type, jak_u64 *key_length, jak_u64 *commit_hash, struct jak_memfile *memfile)
{
        memfile_save_position(memfile);
        memfile_seek(memfile, 0);
        const void *ret = carbon_key_read(key_length, key_type, memfile);
        jak_u64 *hash = JAK_MEMFILE_READ_TYPE(memfile, jak_u64);
        JAK_optional_set(commit_hash, *hash);
        memfile_restore_position(memfile);
        return ret;
}

static void record_ref_create(struct jak_memfile *memfile, jak_carbon *doc)
{
        jak_carbon_key_e key_type;
        jak_u64 commit_hash;
        jak_carbon_key_type(&key_type, doc);
        jak_carbon_commit_hash(&commit_hash, doc);

        /* write record key */
        memfile_seek(memfile, 0);
        carbon_key_create(memfile, key_type, &doc->err);
        switch (key_type) {
                case JAK_CARBON_KEY_NOKEY: {
                        /* nothing to do */
                }
                        break;
                case JAK_CARBON_KEY_AUTOKEY:
                case JAK_CARBON_KEY_UKEY: {
                        jak_u64 key;
                        jak_carbon_key_unsigned_value(&key, doc);
                        memfile_seek(memfile, 0);
                        carbon_key_write_unsigned(memfile, key);
                }
                        break;
                case JAK_CARBON_KEY_IKEY: {
                        jak_i64 key;
                        jak_carbon_key_signed_value(&key, doc);
                        memfile_seek(memfile, 0);
                        carbon_key_write_signed(memfile, key);
                }
                        break;
                case JAK_CARBON_KEY_SKEY: {
                        jak_u64 len;
                        const char *key = jak_carbon_key_string_value(&len, doc);
                        memfile_seek(memfile, 0);
                        carbon_key_update_string_wnchar(memfile, key, len);
                }
                        break;
                default: error(&doc->err, JAK_ERR_TYPEMISMATCH)
        }

        /* write record version */
        memfile_write(memfile, &commit_hash, sizeof(jak_u64));
}

static void array_traverse(struct path_index_node *parent, jak_carbon_array_it *it)
{
        jak_u64 sub_elem_pos = 0;
        while (jak_carbon_array_it_next(it)) {
                jak_offset_t sub_elem_off = jak_carbon_array_it_tell(it);
                struct path_index_node *elem_node = path_index_node_add_array_elem(parent, sub_elem_pos, sub_elem_off);
                array_build_index(elem_node, it);

                sub_elem_pos++;
        }
}

static void column_traverse(struct path_index_node *parent, jak_carbon_column_it *it)
{
        carbon_field_type_e column_type, entry_type;
        jak_u32 nvalues;

        jak_carbon_column_it_values_info(&column_type, &nvalues, it);

        for (jak_u32 i = 0; i < nvalues; i++) {
                bool is_null = jak_carbon_column_it_value_is_null(it, i);
                bool is_true = false;
                if (column_type == CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN) {
                        is_true = jak_carbon_column_it_boolean_values(NULL, it)[i];
                }
                entry_type = carbon_field_type_column_entry_to_regular_type(column_type, is_null, is_true);
                jak_offset_t sub_elem_off = jak_carbon_column_it_tell(it, i);

                struct path_index_node *node = path_index_node_add_column_elem(parent, i, sub_elem_off);
                path_index_node_set_field_type(node, entry_type);
        }
}

static void object_traverse(struct path_index_node *parent, struct jak_carbon_object_it *it)
{
        while (carbon_object_it_next(it)) {
                jak_u64 prop_name_len = 0;
                jak_offset_t key_off, value_off;
                carbon_object_it_tell(&key_off, &value_off, it);
                const char *prop_name = carbon_object_it_prop_name(&prop_name_len, it);
                struct path_index_node *elem_node = path_index_node_add_key_elem(parent, key_off,
                                                                                 prop_name, prop_name_len, value_off);
                object_build_index(elem_node, it);
        }
}

static void object_build_index(struct path_index_node *parent, struct jak_carbon_object_it *elem_it)
{
        carbon_field_type_e field_type;
        carbon_object_it_prop_type(&field_type, elem_it);
        path_index_node_set_field_type(parent, field_type);

        switch (field_type) {
                case CARBON_JAK_FIELD_TYPE_NULL:
                case CARBON_JAK_FIELD_TYPE_TRUE:
                case CARBON_JAK_FIELD_TYPE_FALSE:
                case CARBON_JAK_FIELD_TYPE_STRING:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_FLOAT:
                case CARBON_JAK_FIELD_TYPE_BINARY:
                case CARBON_JAK_FIELD_TYPE_BINARY_CUSTOM:
                        /* path ends here */
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT:
                case CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U8:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U16:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U32:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U64:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I8:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I16:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I32:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I64: {
                        jak_carbon_column_it *it = carbon_object_it_column_value(elem_it);
                        column_traverse(parent, it);

                }
                        break;
                case CARBON_JAK_FIELD_TYPE_ARRAY: {
                        jak_carbon_array_it *it = carbon_object_it_array_value(elem_it);
                        array_traverse(parent, it);
                        jak_carbon_array_it_drop(it);
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_OBJECT: {
                        struct jak_carbon_object_it *it = carbon_object_it_object_value(elem_it);
                        object_traverse(parent, it);
                        carbon_object_it_drop(it);
                }
                        break;
                default: error(&elem_it->err, JAK_ERR_INTERNALERR);
        }
}

static void array_build_index(struct path_index_node *parent, jak_carbon_array_it *elem_it)
{
        carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, elem_it);
        path_index_node_set_field_type(parent, field_type);

        switch (field_type) {
                case CARBON_JAK_FIELD_TYPE_NULL:
                case CARBON_JAK_FIELD_TYPE_TRUE:
                case CARBON_JAK_FIELD_TYPE_FALSE:
                case CARBON_JAK_FIELD_TYPE_STRING:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_FLOAT:
                case CARBON_JAK_FIELD_TYPE_BINARY:
                case CARBON_JAK_FIELD_TYPE_BINARY_CUSTOM:
                        /* path ends here */
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT:
                case CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U8:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U16:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U32:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U64:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I8:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I16:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I32:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I64: {
                        jak_carbon_column_it *it = jak_carbon_array_it_column_value(elem_it);
                        column_traverse(parent, it);

                }
                        break;
                case CARBON_JAK_FIELD_TYPE_ARRAY: {
                        jak_carbon_array_it *it = jak_carbon_array_it_array_value(elem_it);
                        array_traverse(parent, it);
                        jak_carbon_array_it_drop(it);
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_OBJECT: {
                        struct jak_carbon_object_it *it = jak_carbon_array_it_object_value(elem_it);
                        object_traverse(parent, it);
                        carbon_object_it_drop(it);
                }
                        break;
                default: error(&elem_it->err, JAK_ERR_INTERNALERR);
        }
}

static void field_ref_write(struct jak_memfile *file, struct path_index_node *node)
{
        memfile_write_byte(file, node->field_type);
        if (node->field_type != CARBON_JAK_FIELD_TYPE_NULL && node->field_type != CARBON_JAK_FIELD_TYPE_TRUE &&
            node->field_type != CARBON_JAK_FIELD_TYPE_FALSE) {
                /* only in case of field type that is not null, true, or false, there is more information behind
                 * the field offset */
                memfile_write_uintvar_stream(NULL, file, node->field_offset);
        }
}

static void container_contents_flat(struct jak_memfile *file, struct path_index_node *node)
{
        memfile_write_uintvar_stream(NULL, file, node->sub_entries.num_elems);

        /* write position offsets */
        jak_offset_t position_off_latest = memfile_tell(file);
        for (jak_u32 i = 0; i < node->sub_entries.num_elems; i++) {
                memfile_write_uintvar_stream(NULL, file, 0);
        }

        for (jak_u32 i = 0; i < node->sub_entries.num_elems; i++) {
                jak_offset_t node_off = memfile_tell(file);
                struct path_index_node *sub = vec_get(&node->sub_entries, i, struct path_index_node);
                node_flat(file, sub);
                memfile_save_position(file);
                memfile_seek(file, position_off_latest);
                signed_offset_t shift = memfile_update_uintvar_stream(file, node_off);
                position_off_latest = memfile_tell(file);
                memfile_restore_position(file);
                memfile_seek_from_here(file, shift);
        }
}

static void container_field_flat(struct jak_memfile *file, struct path_index_node *node)
{
        switch (node->field_type) {
                case CARBON_JAK_FIELD_TYPE_NULL:
                case CARBON_JAK_FIELD_TYPE_TRUE:
                case CARBON_JAK_FIELD_TYPE_FALSE:
                case CARBON_JAK_FIELD_TYPE_STRING:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_FLOAT:
                case CARBON_JAK_FIELD_TYPE_BINARY:
                case CARBON_JAK_FIELD_TYPE_BINARY_CUSTOM:
                        /* any path will end with this kind of field, and therefore no subsequent elements exists */
                        JAK_ASSERT(node->sub_entries.num_elems == 0);
                        break;
                case CARBON_JAK_FIELD_TYPE_OBJECT:
                case CARBON_JAK_FIELD_TYPE_ARRAY:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U8:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U16:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U32:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U64:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I8:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I16:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I32:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I64:
                case CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT:
                case CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN:
                        /* each of these field types allows for further path traversals, and therefore at least one
                         * subsequent path element must exist */
                        container_contents_flat(file, node);
                        break;
                default: error(&file->err, JAK_ERR_INTERNALERR);
        }
}

static void prop_flat(struct jak_memfile *file, struct path_index_node *node)
{
        memfile_write_byte(file, PATH_MARKER_PROP_NODE);
        field_ref_write(file, node);
        memfile_write_uintvar_stream(NULL, file, node->entry.key.offset);
        container_field_flat(file, node);
}

static void array_flat(struct jak_memfile *file, struct path_index_node *node)
{
        memfile_write_byte(file, PATH_MARKER_ARRAY_NODE);
        field_ref_write(file, node);
        if (JAK_UNLIKELY(node->type == PATH_ROOT)) {
                container_contents_flat(file, node);
        } else {
                container_field_flat(file, node);
        }
}

JAK_FUNC_UNUSED
static void node_into_carbon(jak_carbon_insert *ins, struct jak_carbon_path_index *index)
{
        jak_u8 next = memfile_peek_byte(&index->memfile);
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
                default: error(&index->err, JAK_ERR_CORRUPTED)
        }
}

static void node_to_str(struct jak_string *str, struct jak_carbon_path_index *index, unsigned intent_level)
{
        jak_u8 next = memfile_peek_byte(&index->memfile);
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
                default: error(&index->err, JAK_ERR_CORRUPTED)
        }
}

static jak_u8 field_ref_into_carbon(jak_carbon_insert *ins, struct jak_carbon_path_index *index, bool is_root)
{
        jak_u8 field_type = memfile_read_byte(&index->memfile);

        if (is_root) {
                carbon_insert_prop_null(ins, "container");
        } else {
                carbon_insert_prop_string(ins, "container", carbon_field_type_str(NULL, field_type));
        }


        if (field_type != CARBON_JAK_FIELD_TYPE_NULL && field_type != CARBON_JAK_FIELD_TYPE_TRUE &&
            field_type != CARBON_JAK_FIELD_TYPE_FALSE) {
                /* only in case of field type that is not null, true, or false, there is more information behind
                 * the field offset */
                jak_u64 field_offset = memfile_read_uintvar_stream(NULL, &index->memfile);
                if (is_root) {
                        carbon_insert_prop_null(ins, "offset");
                } else {
                        struct jak_string str;
                        string_create(&str);
                        string_add_u64_as_hex_0x_prefix_compact(&str, field_offset);
                        carbon_insert_prop_string(ins, "offset", string_cstr(&str));
                        string_drop(&str);
                }
        } else {
                carbon_insert_prop_null(ins, "offset");
        }
        return field_type;
}

static jak_u8 field_ref_to_str(struct jak_string *str, struct jak_carbon_path_index *index)
{
        jak_u8 field_type = memfile_read_byte(&index->memfile);

        string_add_char(str, '[');
        string_add_char(str, field_type);
        string_add_char(str, ']');

        if (field_type != CARBON_JAK_FIELD_TYPE_NULL && field_type != CARBON_JAK_FIELD_TYPE_TRUE &&
            field_type != CARBON_JAK_FIELD_TYPE_FALSE) {
                /* only in case of field type that is not null, true, or false, there is more information behind
                 * the field offset */
                jak_u64 field_offset = memfile_read_uintvar_stream(NULL, &index->memfile);
                string_add_char(str, '(');
                string_add_u64_as_hex_0x_prefix_compact(str, field_offset);
                string_add_char(str, ')');
        }

        return field_type;
}

static void column_to_str(struct jak_string *str, struct jak_carbon_path_index *index, unsigned intent_level)
{
        intent(str, intent_level);
        jak_u8 marker = memfile_read_byte(&index->memfile);
        string_add_char(str, '[');
        string_add_char(str, marker);
        string_add_char(str, ']');

        field_ref_to_str(str, index);
}

static jak_u8 _insert_field_ref(jak_carbon_insert *ins, struct jak_carbon_path_index *index, bool is_root)
{
        struct jak_carbon_insert_object_state object;
        jak_carbon_insert *oins = carbon_insert_prop_object_begin(&object, ins, "record-reference", 1024);
        jak_u8 ret = field_ref_into_carbon(oins, index, is_root);
        carbon_insert_prop_object_end(&object);
        return ret;
}

JAK_FUNC_UNUSED
static void column_into_carbon(jak_carbon_insert *ins, struct jak_carbon_path_index *index)
{
        memfile_skip_byte(&index->memfile);
        carbon_insert_prop_string(ins, "type", "column");
        _insert_field_ref(ins, index, false);
}

static void container_contents_into_carbon(jak_carbon_insert *ins, struct jak_carbon_path_index *index)
{
        jak_u64 num_elems = memfile_read_uintvar_stream(NULL, &index->memfile);
        carbon_insert_prop_unsigned(ins, "element-count", num_elems);

        struct jak_carbon_insert_array_state array;
        jak_carbon_insert *ains = carbon_insert_prop_array_begin(&array, ins, "element-offsets", 1024);

        struct jak_string str;
        string_create(&str);
        for (jak_u32 i = 0; i < num_elems; i++) {
                jak_u64 pos_offs = memfile_read_uintvar_stream(NULL, &index->memfile);
                string_clear(&str);
                string_add_u64_as_hex_0x_prefix_compact(&str, pos_offs);
                carbon_insert_string(ains, string_cstr(&str));
        }
        string_drop(&str);

        carbon_insert_prop_array_end(&array);

        ains = carbon_insert_prop_array_begin(&array, ins, "elements", 1024);
        JAK_UNUSED(ains)
        for (jak_u32 i = 0; i < num_elems; i++) {
                struct jak_carbon_insert_object_state node_obj;
                jak_carbon_insert *node_obj_ins = carbon_insert_object_begin(&node_obj, ains, 1024);
                node_into_carbon(node_obj_ins, index);
                carbon_insert_object_end(&node_obj);
        }
        carbon_insert_prop_array_end(&array);

}

static void
container_contents_to_str(struct jak_string *str, struct jak_carbon_path_index *index, unsigned intent_level)
{
        jak_u64 num_elems = memfile_read_uintvar_stream(NULL, &index->memfile);
        string_add_char(str, '(');
        string_add_u64(str, num_elems);
        string_add_char(str, ')');

        for (jak_u32 i = 0; i < num_elems; i++) {
                jak_u64 pos_offs = memfile_read_uintvar_stream(NULL, &index->memfile);
                string_add_char(str, '(');
                string_add_u64_as_hex_0x_prefix_compact(str, pos_offs);
                string_add_char(str, ')');
        }

        for (jak_u32 i = 0; i < num_elems; i++) {
                node_to_str(str, index, intent_level);
        }
}

static void
container_to_str(struct jak_string *str, struct jak_carbon_path_index *index, jak_u8 field_type, unsigned intent_level)
{
        switch (field_type) {
                case CARBON_JAK_FIELD_TYPE_NULL:
                case CARBON_JAK_FIELD_TYPE_TRUE:
                case CARBON_JAK_FIELD_TYPE_FALSE:
                case CARBON_JAK_FIELD_TYPE_STRING:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_FLOAT:
                case CARBON_JAK_FIELD_TYPE_BINARY:
                case CARBON_JAK_FIELD_TYPE_BINARY_CUSTOM:
                        /* nothing to do */
                        break;
                case CARBON_JAK_FIELD_TYPE_OBJECT:
                case CARBON_JAK_FIELD_TYPE_ARRAY:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U8:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U16:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U32:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U64:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I8:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I16:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I32:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I64:
                case CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT:
                case CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN: {
                        /* subsequent path elements to be printed */
                        container_contents_to_str(str, index, ++intent_level);
                }
                        break;
                default: error(&index->err, JAK_ERR_INTERNALERR);
        }
}

static void container_into_carbon(jak_carbon_insert *ins, struct jak_carbon_path_index *index, jak_u8 field_type)
{
        switch (field_type) {
                case CARBON_JAK_FIELD_TYPE_NULL:
                case CARBON_JAK_FIELD_TYPE_TRUE:
                case CARBON_JAK_FIELD_TYPE_FALSE:
                case CARBON_JAK_FIELD_TYPE_STRING:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_FLOAT:
                case CARBON_JAK_FIELD_TYPE_BINARY:
                case CARBON_JAK_FIELD_TYPE_BINARY_CUSTOM:
                        /* nothing to do */
                        break;
                case CARBON_JAK_FIELD_TYPE_OBJECT:
                case CARBON_JAK_FIELD_TYPE_ARRAY:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U8:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U16:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U32:
                case CARBON_JAK_FIELD_TYPE_COLUMN_U64:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I8:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I16:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I32:
                case CARBON_JAK_FIELD_TYPE_COLUMN_I64:
                case CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT:
                case CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN: {
                        /* subsequent path elements to be printed */
                        container_contents_into_carbon(ins, index);
                }
                        break;
                default: error(&index->err, JAK_ERR_INTERNALERR);
        }
}

static void prop_to_str(struct jak_string *str, struct jak_carbon_path_index *index, unsigned intent_level)
{
        intent(str, intent_level++);

        jak_u8 marker = memfile_read_byte(&index->memfile);
        string_add_char(str, '[');
        string_add_char(str, marker);
        string_add_char(str, ']');

        jak_u8 field_type = field_ref_to_str(str, index);

        jak_u64 key_offset = memfile_read_uintvar_stream(NULL, &index->memfile);

        string_add_char(str, '(');
        string_add_u64_as_hex_0x_prefix_compact(str, key_offset);
        string_add_char(str, ')');

        container_to_str(str, index, field_type, intent_level);
}

JAK_FUNC_UNUSED
static void prop_into_carbon(jak_carbon_insert *ins, struct jak_carbon_path_index *index)
{
        memfile_skip_byte(&index->memfile);
        carbon_insert_prop_string(ins, "type", "key");
        jak_u8 field_type = _insert_field_ref(ins, index, false);

        struct jak_string str;
        string_create(&str);

        jak_u64 key_offset = memfile_read_uintvar_stream(NULL, &index->memfile);
        string_add_u64_as_hex_0x_prefix_compact(&str, key_offset);
        carbon_insert_prop_string(ins, "key", string_cstr(&str));
        string_drop(&str);

        container_into_carbon(ins, index, field_type);
}

static void array_into_carbon(jak_carbon_insert *ins, struct jak_carbon_path_index *index, bool is_root)
{
        memfile_skip_byte(&index->memfile);
        jak_u8 field_type;

        carbon_insert_prop_string(ins, "parent", is_root ? "record" : "array");
        field_type = _insert_field_ref(ins, index, is_root);

        struct jak_carbon_insert_object_state object;
        jak_carbon_insert *oins = carbon_insert_prop_object_begin(&object, ins, "nodes", 1024);
        if (JAK_UNLIKELY(is_root)) {
                container_contents_into_carbon(oins, index);
        } else {
                container_into_carbon(oins, index, field_type);
        }
        carbon_insert_prop_object_end(&object);
}

static void
array_to_str(struct jak_string *str, struct jak_carbon_path_index *index, bool is_root, unsigned intent_level)
{
        intent(str, intent_level++);

        jak_u8 marker = memfile_read_byte(&index->memfile);
        string_add_char(str, '[');
        string_add_char(str, marker);
        string_add_char(str, ']');

        jak_u8 field_type = field_ref_to_str(str, index);

        if (JAK_UNLIKELY(is_root)) {
                container_contents_to_str(str, index, intent_level);
        } else {
                container_to_str(str, index, field_type, intent_level);
        }
}

static void column_flat(struct jak_memfile *file, struct path_index_node *node)
{
        memfile_write_byte(file, PATH_MARKER_COLUMN_NODE);
        field_ref_write(file, node);
        JAK_ASSERT(node->sub_entries.num_elems == 0);
}

static void node_flat(struct jak_memfile *file, struct path_index_node *node)
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
                default: error(&file->err, JAK_ERR_INTERNALERR);
                        return;
        }
}

static void index_flat(struct jak_memfile *file, struct path_index_node *root_array)
{
        array_flat(file, root_array);
}

static void index_build(struct jak_memfile *file, jak_carbon *doc)
{
        struct path_index_node root_array;

        /* init */
        path_index_node_init(&root_array);

        jak_carbon_array_it it;
        jak_u64 array_pos = 0;
        jak_carbon_iterator_open(&it, doc);

        /* build index as tree structure */
        while (jak_carbon_array_it_next(&it)) {
                jak_offset_t entry_offset = jak_carbon_array_it_tell(&it);
                struct path_index_node *node = path_index_node_add_array_elem(&root_array, array_pos, entry_offset);
                array_build_index(node, &it);
                array_pos++;
        }
        jak_carbon_iterator_close(&it);

        /* for debug */
        path_index_node_print_level(stdout, &root_array, 0); // TODO: Debug remove

        index_flat(file, &root_array);
        memfile_shrink(file);

        /* cleanup */
        path_index_node_drop(&root_array);
}

static void record_ref_to_str(struct jak_string *str, struct jak_carbon_path_index *index)
{
        jak_u8 key_type = memfile_read_byte(&index->memfile);
        string_add_char(str, '[');
        string_add_char(str, key_type);
        string_add_char(str, ']');

        switch (key_type) {
                case JAK_CARBON_KEY_NOKEY:
                        /* nothing to do */
                        break;
                case JAK_CARBON_KEY_AUTOKEY:
                case JAK_CARBON_KEY_UKEY: {
                        jak_u64 key = memfile_read_u64(&index->memfile);
                        string_add_char(str, '[');
                        string_add_u64(str, key);
                        string_add_char(str, ']');
                }
                        break;
                case JAK_CARBON_KEY_IKEY: {
                        jak_i64 key = memfile_read_i64(&index->memfile);
                        string_add_char(str, '[');;
                        string_add_i64(str, key);
                        string_add_char(str, ']');
                }
                        break;
                case JAK_CARBON_KEY_SKEY: {
                        jak_u64 key_len;
                        const char *key = carbon_string_read(&key_len, &index->memfile);
                        string_add_char(str, '(');
                        string_add_nchar(str, key, key_len);
                        string_add_char(str, ')');
                }
                        break;
                default: error(&index->err, JAK_ERR_INTERNALERR);
        }
        jak_u64 commit_hash = memfile_read_u64(&index->memfile);
        string_add_char(str, '[');
        string_add_u64(str, commit_hash);
        string_add_char(str, ']');
}

static void record_ref_to_carbon(jak_carbon_insert *roins, struct jak_carbon_path_index *index)
{
        char key_type = memfile_read_byte(&index->memfile);
        carbon_insert_prop_string(roins, "key-type", carbon_key_type_str(key_type));

        switch (key_type) {
                case JAK_CARBON_KEY_NOKEY:
                        /* nothing to do */
                        break;
                case JAK_CARBON_KEY_AUTOKEY:
                case JAK_CARBON_KEY_UKEY: {
                        jak_u64 key = memfile_read_u64(&index->memfile);
                        carbon_insert_prop_unsigned(roins, "key-value", key);
                }
                        break;
                case JAK_CARBON_KEY_IKEY: {
                        jak_i64 key = memfile_read_i64(&index->memfile);
                        carbon_insert_prop_signed(roins, "key-value", key);
                }
                        break;
                case JAK_CARBON_KEY_SKEY: {
                        jak_u64 key_len;
                        const char *key = carbon_string_read(&key_len, &index->memfile);
                        carbon_insert_prop_nchar(roins, "key-value", key, key_len);
                }
                        break;
                default: error(&index->err, JAK_ERR_INTERNALERR);
        }
        jak_u64 commit_hash = memfile_read_u64(&index->memfile);
        struct jak_string str;
        string_create(&str);
        jak_carbon_commit_hash_to_str(&str, commit_hash);
        carbon_insert_prop_string(roins, "commit-hash", string_cstr(&str));
        string_drop(&str);
}

// ---------------------------------------------------------------------------------------------------------------------
//  construction and deconstruction
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_create(struct jak_carbon_path_index *index, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(index);
        JAK_ERROR_IF_NULL(doc);
        memblock_create(&index->memblock, PATH_INDEX_CAPACITY);
        memfile_open(&index->memfile, index->memblock, READ_WRITE);
        error_init(&index->err);
        record_ref_create(&index->memfile, doc);
        index_build(&index->memfile, doc);
        return true;
}

bool carbon_path_index_drop(struct jak_carbon_path_index *index)
{
        JAK_UNUSED(index)
        return false;
}

// ---------------------------------------------------------------------------------------------------------------------
//  index data access and meta information
// ---------------------------------------------------------------------------------------------------------------------

const void *carbon_path_index_raw_data(jak_u64 *size, struct jak_carbon_path_index *index)
{
        if (size && index) {
                const char *raw = memblock_raw_data(index->memfile.memblock);
                memblock_size(size, index->memfile.memblock);
                return raw;
        } else {
                return NULL;
        }
}

bool carbon_path_index_commit_hash(jak_u64 *commit_hash, struct jak_carbon_path_index *index)
{
        JAK_ERROR_IF_NULL(commit_hash)
        JAK_ERROR_IF_NULL(index)
        record_ref_read(NULL, NULL, commit_hash, &index->memfile);
        return true;
}

bool carbon_path_index_key_type(jak_carbon_key_e *key_type, struct jak_carbon_path_index *index)
{
        JAK_ERROR_IF_NULL(key_type)
        JAK_ERROR_IF_NULL(index)
        record_ref_read(key_type, NULL, NULL, &index->memfile);
        return true;
}

bool carbon_path_index_key_unsigned_value(jak_u64 *key, struct jak_carbon_path_index *index)
{
        JAK_ERROR_IF_NULL(key)
        JAK_ERROR_IF_NULL(index)
        jak_carbon_key_e key_type;
        jak_u64 ret = *(jak_u64 *) record_ref_read(&key_type, NULL, NULL, &index->memfile);
        error_if(key_type != JAK_CARBON_KEY_AUTOKEY && key_type != JAK_CARBON_KEY_UKEY, &index->err, JAK_ERR_TYPEMISMATCH);
        *key = ret;
        return true;
}

bool carbon_path_index_key_signed_value(jak_i64 *key, struct jak_carbon_path_index *index)
{
        JAK_ERROR_IF_NULL(key)
        JAK_ERROR_IF_NULL(index)
        jak_carbon_key_e key_type;
        jak_i64 ret = *(jak_i64 *) record_ref_read(&key_type, NULL, NULL, &index->memfile);
        error_if(key_type != JAK_CARBON_KEY_IKEY, &index->err, JAK_ERR_TYPEMISMATCH);
        *key = ret;
        return true;
}

const char *carbon_path_index_key_string_value(jak_u64 *str_len, struct jak_carbon_path_index *index)
{
        if (str_len && index) {
                jak_carbon_key_e key_type;
                const char *ret = (const char *) record_ref_read(&key_type, str_len, NULL, &index->memfile);
                error_if(key_type != JAK_CARBON_KEY_SKEY, &index->err, JAK_ERR_TYPEMISMATCH);
                return ret;
        } else {
                error(&index->err, JAK_ERR_NULLPTR);
                return NULL;
        }
}

bool carbon_path_index_indexes_doc(struct jak_carbon_path_index *index, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(doc);

        jak_u64 index_hash = 0, doc_hash = 0;
        carbon_path_index_commit_hash(&index_hash, index);
        jak_carbon_commit_hash(&doc_hash, doc);
        if (JAK_LIKELY(index_hash == doc_hash)) {
                jak_carbon_key_e index_key_type, doc_key_type;
                carbon_path_index_key_type(&index_key_type, index);
                jak_carbon_key_type(&doc_key_type, doc);
                if (JAK_LIKELY(index_key_type == doc_key_type)) {
                        switch (index_key_type) {
                                case JAK_CARBON_KEY_NOKEY:
                                        return true;
                                case JAK_CARBON_KEY_AUTOKEY:
                                case JAK_CARBON_KEY_UKEY: {
                                        jak_u64 index_key, doc_key;
                                        carbon_path_index_key_unsigned_value(&index_key, index);
                                        jak_carbon_key_unsigned_value(&doc_key, doc);
                                        return index_key == doc_key;
                                }
                                case JAK_CARBON_KEY_IKEY: {
                                        jak_i64 index_key, doc_key;
                                        carbon_path_index_key_signed_value(&index_key, index);
                                        jak_carbon_key_signed_value(&doc_key, doc);
                                        return index_key == doc_key;
                                }
                                case JAK_CARBON_KEY_SKEY: {
                                        jak_u64 index_key_len, doc_key_len;
                                        const char *index_key = carbon_path_index_key_string_value(&index_key_len,
                                                                                                   index);
                                        const char *doc_key = jak_carbon_key_string_value(&doc_key_len, doc);
                                        return (index_key_len == doc_key_len) && (strcmp(index_key, doc_key) == 0);
                                }
                                default: error(&doc->err, JAK_ERR_TYPEMISMATCH)
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

bool carbon_path_index_it_open(struct jak_carbon_path_index_it *it, struct jak_carbon_path_index *index,
                               jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(it)
        JAK_ERROR_IF_NULL(index)
        JAK_ERROR_IF_NULL(doc)
        if (carbon_path_index_indexes_doc(index, doc)) {
                JAK_zero_memory(it, sizeof(struct jak_carbon_path_index_it));
                error_init(&it->err);
                memfile_open(&it->memfile, index->memfile.memblock, READ_ONLY);
                it->doc = doc;
                it->container_type = JAK_CARBON_ARRAY;
                return true;
        } else {
                error(&index->err, JAK_ERR_NOTINDEXED)
                return false;
        }
}

//bool carbon_path_index_it_type(jak_carbon_container_e *type, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//// ---------------------------------------------------------------------------------------------------------------------
////  array and column container functions
//// ---------------------------------------------------------------------------------------------------------------------
//
//bool carbon_path_index_it_list_length(jak_u64 *key_len, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_list_goto(jak_u64 pos, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_list_pos(jak_u64 *pos, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_list_can_enter(struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_list_enter(struct jak_carbon_path_index_it *it)
//{
//
//}
//
//// ---------------------------------------------------------------------------------------------------------------------
////  object container functions
//// ---------------------------------------------------------------------------------------------------------------------
//
//bool carbon_path_index_it_obj_num_props(jak_u64 *num_props, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_obj_goto(const char *key_name, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//const char *carbon_path_index_it_key_name(jak_u64 *name_len, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_obj_can_enter(struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_obj_enter(struct jak_carbon_path_index_it *it)
//{
//
//}
//
//// ---------------------------------------------------------------------------------------------------------------------
////  field access
//// ---------------------------------------------------------------------------------------------------------------------
//
//bool carbon_path_index_it_field_type(carbon_field_type_e *type, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_u8_value(jak_u8 *value, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_u16_value(jak_u16 *value, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_u32_value(jak_u32 *value, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_u64_value(jak_u64 *value, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_i8_value(jak_i8 *value, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_i16_value(jak_i16 *value, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_i32_value(jak_i32 *value, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_i64_value(jak_i64 *value, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_float_value(bool *is_null_in, float *value, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_signed_value(bool *is_null_in, jak_i64 *value, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_unsigned_value(bool *is_null_in, jak_u64 *value, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//const char *carbon_path_index_it_field_string_value(jak_u64 *strlen, struct jak_carbon_path_index_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_binary_value(struct jak_carbon_binary *out, jak_carbon_array_it *it)
//{
//
//}
//
//bool carbon_path_index_it_field_array_value(jak_carbon_array_it *it_out, struct jak_carbon_path_index_it *it_in)
//{
//
//}
//
//bool carbon_path_index_it_field_object_value(struct jak_carbon_object_it *it_out, struct jak_carbon_path_index_it *it_in)
//{
//
//}
//
//bool carbon_path_index_it_field_column_value(jak_carbon_column_it *it_out, struct jak_carbon_path_index_it *it_in)
//{
//
//}

// ---------------------------------------------------------------------------------------------------------------------
//  diagnostics
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_hexdump(FILE *file, struct jak_carbon_path_index *index)
{
        return memfile_hexdump_printf(file, &index->memfile);
}

bool carbon_path_index_to_carbon(jak_carbon *doc, struct jak_carbon_path_index *index)
{
        jak_carbon_new context;
        struct jak_carbon_insert_object_state object;

        memfile_seek_to_start(&index->memfile);

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);
        jak_carbon_insert *oins = carbon_insert_object_begin(&object, ins, 1024);

        {
                struct jak_carbon_insert_object_state ref_object;
                jak_carbon_insert *roins = carbon_insert_prop_object_begin(&ref_object, oins,
                                                                                  "record-association", 1024);
                record_ref_to_carbon(roins, index);
                carbon_insert_prop_object_end(&ref_object);
        }
        {
                struct jak_carbon_insert_object_state root_object;
                jak_carbon_insert *roins = carbon_insert_prop_object_begin(&root_object, oins, "index", 1024);
                array_into_carbon(roins, index, true);
                carbon_insert_prop_object_end(&root_object);
        }

        carbon_insert_object_end(&object);
        jak_carbon_create_end(&context);
        return true;
}

const char *carbon_path_index_to_str(struct jak_string *str, struct jak_carbon_path_index *index)
{
        memfile_seek_to_start(&index->memfile);
        record_ref_to_str(str, index);
        array_to_str(str, index, true, 0);
        return string_cstr(str);
}

bool carbon_path_index_print(FILE *file, struct jak_carbon_path_index *index)
{
        struct jak_string str;
        string_create(&str);
        memfile_save_position(&index->memfile);
        memfile_seek_to_start(&index->memfile);
        fprintf(file, "%s", carbon_path_index_to_str(&str, index));
        memfile_restore_position(&index->memfile);
        string_drop(&str);
        return true;
}
