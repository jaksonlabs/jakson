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

#include <limits.h>
#include <inttypes.h>

#include <jak_huffman.h>
#include <jak_bitmap.h>

struct huff_node {
        struct huff_node *prev, *next, *left, *right;
        jak_u64 freq;
        unsigned char letter;
};

static void huff_tree_create(jak_vector ofType(jak_pack_huffman_entry) *table,
                             const jak_vector ofType(jak_u32) *frequencies);

bool jak_coding_huffman_create(jak_huffman *dic)
{
        JAK_ERROR_IF_NULL(dic);

        jak_vector_create(&dic->table, NULL, sizeof(jak_pack_huffman_entry), UCHAR_MAX / 4);
        jak_error_init(&dic->err);

        return true;
}

bool jak_coding_huffman_cpy(jak_huffman *dst, jak_huffman *src)
{
        JAK_ERROR_IF_NULL(dst);
        JAK_ERROR_IF_NULL(src);
        if (!jak_vector_cpy(&dst->table, &src->table)) {
                JAK_ERROR(&src->err, JAK_ERR_HARDCOPYFAILED);
                return false;
        } else {
                return jak_error_cpy(&dst->err, &src->err);
        }
}

bool jak_coding_huffman_build(jak_huffman *encoder, const jak_string_jak_vector_t *strings)
{
        JAK_ERROR_IF_NULL(encoder);
        JAK_ERROR_IF_NULL(strings);

        jak_vector ofType(jak_u32) frequencies;
        jak_vector_create(&frequencies, NULL, sizeof(jak_u32), UCHAR_MAX);
        jak_vector_enlarge_size_to_capacity(&frequencies);

        jak_u32 *freq_data = JAK_VECTOR_ALL(&frequencies, jak_u32);
        JAK_ZERO_MEMORY(freq_data, UCHAR_MAX * sizeof(jak_u32));

        for (size_t i = 0; i < strings->num_elems; i++) {
                const char *string = *JAK_VECTOR_GET(strings, i, const char *);
                size_t jak_string_length = strlen(string);
                for (size_t k = 0; k < jak_string_length; k++) {
                        size_t c = (unsigned char) string[k];
                        freq_data[c]++;
                }
        }

        huff_tree_create(&encoder->table, &frequencies);
        jak_vector_drop(&frequencies);

        return true;
}

bool jak_coding_huffman_get_error(jak_error *err, const jak_huffman *dic)
{
        JAK_ERROR_IF_NULL(err)
        JAK_ERROR_IF_NULL(dic)
        jak_error_cpy(err, &dic->err);
        return true;
}

bool jak_coding_huffman_drop(jak_huffman *dic)
{
        JAK_ERROR_IF_NULL(dic);

        for (size_t i = 0; i < dic->table.num_elems; i++) {
                jak_pack_huffman_entry *entry = JAK_VECTOR_GET(&dic->table, i, jak_pack_huffman_entry);
                free(entry->blocks);
        }

        jak_vector_drop(&dic->table);

        free(dic);

        return true;
}

bool jak_coding_huffman_serialize(jak_memfile *file, const jak_huffman *dic, char marker_symbol)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(dic)

        for (size_t i = 0; i < dic->table.num_elems; i++) {
                jak_pack_huffman_entry *entry = JAK_VECTOR_GET(&dic->table, i, jak_pack_huffman_entry);
                jak_memfile_write(file, &marker_symbol, sizeof(char));
                jak_memfile_write(file, &entry->letter, sizeof(unsigned char));

                /** block one is the block that holds the significant part of the prefix code */
                jak_offset_t offset_meta, offset_continue;
                jak_memfile_get_offset(&offset_meta, file);
                /** this will be the number of bytes used to encode the significant part of the prefix code */
                jak_memfile_skip(file, sizeof(jak_u8));

                jak_memfile_begin_bit_mode(file);
                bool first_bit_found = false;
                for (int i = 31; entry->blocks && i >= 0; i--) {
                        jak_u32 mask = 1 << i;
                        jak_u32 k = entry->blocks[0] & mask;
                        bool bit_state = k != 0;
                        first_bit_found |= bit_state;

                        if (first_bit_found) {
                                jak_memfile_write_bit(file, bit_state);
                        }
                }
                size_t num_bytes_written;
                jak_memfile_end_bit_mode(&num_bytes_written, file);
                jak_memfile_get_offset(&offset_continue, file);
                jak_memfile_seek(file, offset_meta);
                jak_u8 num_bytes_written_uint8 = (jak_u8) num_bytes_written;
                jak_memfile_write(file, &num_bytes_written_uint8, sizeof(jak_u8));

                jak_memfile_seek(file, offset_continue);
        }

        return true;
}

static jak_pack_huffman_entry *find_dic_entry(jak_huffman *dic, unsigned char c)
{
        for (size_t i = 0; i < dic->table.num_elems; i++) {
                jak_pack_huffman_entry *entry = JAK_VECTOR_GET(&dic->table, i, jak_pack_huffman_entry);
                if (entry->letter == c) {
                        return entry;
                }
        }
        JAK_ERROR(&dic->err, JAK_ERR_HUFFERR)
        return NULL;
}

static size_t encodeString(jak_memfile *file, jak_huffman *dic, const char *string)
{
        jak_memfile_begin_bit_mode(file);

        for (const char *c = string; *c != '\0'; c++) {
                jak_pack_huffman_entry *entry = find_dic_entry(dic, (unsigned char) *c);
                if (!entry) {
                        return 0;
                }

                if (!entry->blocks) {
                        jak_memfile_write_bit(file, false);
                } else {
                        for (size_t j = 0; j < entry->nblocks; j++) {
                                jak_u32 block = entry->blocks[j];

                                bool first_bit_found = false;
                                for (int i = 31; i >= 0; i--) {
                                        jak_u32 mask = 1 << i;
                                        jak_u32 k = block & mask;
                                        bool bit_state = k != 0;
                                        first_bit_found |= bit_state;

                                        if (first_bit_found) {
                                                jak_memfile_write_bit(file, bit_state);
                                        }
                                }
                        }
                }
        }

        size_t num_written_bytes;
        jak_memfile_end_bit_mode(&num_written_bytes, file);
        return num_written_bytes;
}

bool jak_coding_huffman_encode(jak_memfile *file, jak_huffman *dic, const char *string)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(dic)
        JAK_ERROR_IF_NULL(string)

        jak_u32 num_bytes_encoded = 0;

        jak_offset_t num_bytes_encoded_off = jak_memfile_tell(file);
        jak_memfile_skip(file, sizeof(jak_u32));

        if ((num_bytes_encoded = (jak_u32) encodeString(file, dic, string)) == 0) {
                return false;
        }

        jak_offset_t continue_off = jak_memfile_tell(file);
        jak_memfile_seek(file, num_bytes_encoded_off);
        jak_memfile_write(file, &num_bytes_encoded, sizeof(jak_u32));
        jak_memfile_seek(file, continue_off);

        return true;
}

bool jak_coding_huffman_read_string(jak_pack_huffman_str_info *info, jak_memfile *src)
{
        info->nbytes_encoded = *JAK_MEMFILE_READ_TYPE(src, jak_u32);
        info->encoded_bytes = JAK_MEMFILE_READ(src, info->nbytes_encoded);
        return true;
}

bool jak_coding_huffman_read_entry(jak_pack_huffman_info *info, jak_memfile *file, char marker_symbol)
{
        char marker = *JAK_MEMFILE_PEEK(file, char);
        if (marker == marker_symbol) {
                jak_memfile_skip(file, sizeof(char));
                info->letter = *JAK_MEMFILE_READ_TYPE(file, unsigned char);
                info->nbytes_prefix = *JAK_MEMFILE_READ_TYPE(file, jak_u8);
                info->prefix_code = JAK_MEMFILE_PEEK(file, char);

                jak_memfile_skip(file, info->nbytes_prefix);

                return true;
        } else {
                return false;
        }
}

static const jak_u32 *get_num_used_blocks(jak_u16 *numUsedBlocks, jak_pack_huffman_entry *entry, jak_u16 num_blocks,
                                          const jak_u32 *blocks)
{
        for (entry->nblocks = 0; entry->nblocks < num_blocks; entry->nblocks++) {
                const jak_u32 *block = blocks + entry->nblocks;
                if (*block != 0) {
                        *numUsedBlocks = (num_blocks - entry->nblocks);
                        return block;
                }
        }
        return NULL;
}

static void
import_into_entry(jak_pack_huffman_entry *entry, const struct huff_node *node, const jak_bitmap *map)
{
        entry->letter = node->letter;
        jak_u32 *blocks, num_blocks;
        const jak_u32 *used_blocks;
        jak_bitmap_blocks(&blocks, &num_blocks, map);
        used_blocks = get_num_used_blocks(&entry->nblocks, entry, num_blocks, blocks);
        entry->blocks = JAK_MALLOC(entry->nblocks * sizeof(jak_u32));
        if (num_blocks > 0) {
                memcpy(entry->blocks, used_blocks, entry->nblocks * sizeof(jak_u32));
        } else {
                entry->blocks = NULL;
        }
        free(blocks);
}

static struct huff_node *seek_to_begin(struct huff_node *handle)
{
        for (; handle->prev != NULL; handle = handle->prev) {}
        return handle;
}

static struct huff_node *seek_to_end(struct huff_node *handle)
{
        for (; handle->next != NULL; handle = handle->next) {}
        return handle;
}

JAK_FUNC_UNUSED
static void __diag_print_insight(struct huff_node *n)
{
        printf("(");
        if (!n->left && !n->right) {
                printf("%c", n->letter);
        } else {
                if (n->left) {
                        __diag_print_insight(n->left);
                }
                printf(",");
                if (n->right) {
                        __diag_print_insight(n->right);
                }
        }
        printf(")");
        printf(": %"PRIu64"", n->freq);
}

JAK_FUNC_UNUSED
static void __diag_dump_remaining_candidates(struct huff_node *n)
{
        struct huff_node *it = seek_to_begin(n);
        while (it->next != NULL) {
                __diag_print_insight(it);
                printf(" | ");
                it = it->next;
        }
}

static struct huff_node *find_smallest(struct huff_node *begin, jak_u64 lowerBound, struct huff_node *skip)
{
        jak_u64 smallest = UINT64_MAX;
        struct huff_node *result = NULL;
        for (struct huff_node *it = begin; it != NULL; it = it->next) {
                if (it != skip && it->freq >= lowerBound && it->freq <= smallest) {
                        smallest = it->freq;
                        result = it;
                }
        }
        return result;
}

static void assign_code(struct huff_node *node, const jak_bitmap *path,
                        jak_vector ofType(jak_pack_huffman_entry) *table)
{
        if (!node->left && !node->right) {
                jak_pack_huffman_entry *entry = JAK_VECTOR_NEW_AND_GET(table, jak_pack_huffman_entry);
                import_into_entry(entry, node, path);
        } else {
                if (node->left) {
                        jak_bitmap left;
                        jak_bitmap_cpy(&left, path);
                        jak_bitmap_lshift(&left);
                        jak_bitmap_set(&left, 0, false);
                        assign_code(node->left, &left, table);
                        jak_bitmap_drop(&left);
                }
                if (node->right) {
                        jak_bitmap right;
                        jak_bitmap_cpy(&right, path);
                        jak_bitmap_lshift(&right);
                        jak_bitmap_set(&right, 0, true);
                        assign_code(node->right, &right, table);
                        jak_bitmap_drop(&right);
                }
        }
}

static struct huff_node *trim_and_begin(jak_vector ofType(HuffNode) *candidates)
{
        struct huff_node *begin = NULL;
        for (struct huff_node *it = JAK_VECTOR_GET(candidates, 0, struct huff_node);; it++) {
                if (it->freq == 0) {
                        if (it->prev) {
                                it->prev->next = it->next;
                        }
                        if (it->next) {
                                it->next->prev = it->prev;
                        }
                } else {
                        if (!begin) {
                                begin = it;
                        }
                }
                if (!it->next) {
                        break;
                }
        }
        return begin;
}

static void huff_tree_create(jak_vector ofType(jak_pack_huffman_entry) *table,
                             const jak_vector ofType(jak_u32) *frequencies)
{
        JAK_ASSERT(UCHAR_MAX == frequencies->num_elems);

        jak_vector ofType(HuffNode) candidates;
        jak_vector_create(&candidates, NULL, sizeof(struct huff_node), UCHAR_MAX * UCHAR_MAX);
        size_t appender_idx = UCHAR_MAX;

        for (unsigned char i = 0; i < UCHAR_MAX; i++) {
                struct huff_node *node = JAK_VECTOR_NEW_AND_GET(&candidates, struct huff_node);
                node->letter = i;
                node->freq = *JAK_VECTOR_GET(frequencies, i, jak_u32);
        }

        for (unsigned char i = 0; i < UCHAR_MAX; i++) {
                struct huff_node *node = JAK_VECTOR_GET(&candidates, i, struct huff_node);
                struct huff_node *prev = i > 0 ? JAK_VECTOR_GET(&candidates, i - 1, struct huff_node) : NULL;
                struct huff_node *next = i + 1 < UCHAR_MAX ? JAK_VECTOR_GET(&candidates, i + 1, struct huff_node) : NULL;
                node->next = next;
                node->prev = prev;
                node->left = node->right = NULL;
        }

        struct huff_node *smallest, *small;
        struct huff_node *handle = trim_and_begin(&candidates);
        struct huff_node *new_node = NULL;

        while (handle->next != NULL) {
                smallest = find_smallest(handle, 0, NULL);
                small = find_smallest(handle, smallest->freq, smallest);

                appender_idx++;
                new_node = JAK_VECTOR_NEW_AND_GET(&candidates, struct huff_node);
                new_node->freq = small->freq + smallest->freq;
                new_node->letter = '\0';
                new_node->left = small;
                new_node->right = smallest;

                if ((small->prev == NULL && smallest->next == NULL) && small->next == smallest) {
                        break;
                }

                if ((smallest->prev == NULL && small->next == NULL) && smallest->next == small) {
                        break;
                }
                if (smallest->prev) {
                        smallest->prev->next = smallest->next;
                }
                if (smallest->next) {
                        smallest->next->prev = smallest->prev;
                }
                if (small->prev) {
                        small->prev->next = small->next;
                }
                if (small->next) {
                        small->next->prev = small->prev;
                }

                if (small->prev) {
                        handle = seek_to_begin(small->prev);
                } else if (smallest->prev) {
                        handle = seek_to_begin(smallest->prev);
                } else if (small->next) {
                        handle = seek_to_begin(small->next);
                } else if (smallest->next) {
                        handle = seek_to_begin(smallest->next);
                } else {
                        JAK_ERROR_PRINT_AND_DIE(JAK_ERR_INTERNALERR);
                }

                JAK_ASSERT (!handle->prev);
                struct huff_node *end = seek_to_end(handle);
                JAK_ASSERT(!end->next);
                end->next = new_node;
                new_node->prev = end;
                new_node->next = NULL;

#ifdef DIAG_HUFFMAN_ENABLE_DEBUG
                printf("in-memory huff-tree: ");
                __diag_print_insight(new_node);
                printf("\n");
                printf("remaining candidates: ");
                __diag_dump_remaining_candidates(handle);
                printf("\n");
#endif
        }

        seek_to_begin(handle);
        if (handle->next) {
                struct huff_node *finalNode = JAK_VECTOR_NEW_AND_GET(&candidates, struct huff_node);
                finalNode->freq = small->freq + smallest->freq;
                finalNode->letter = '\0';
                if (handle->freq > handle->next->freq) {
                        finalNode->left = handle;
                        finalNode->right = handle->next;
                } else {
                        finalNode->left = handle->next;
                        finalNode->right = handle;
                }
                new_node = finalNode;
        }

#ifdef DIAG_HUFFMAN_ENABLE_DEBUG
        printf("final in-memory huff-tree: ");
        __diag_print_insight(new_node);
        printf("\n");
#endif

        jak_bitmap root_path;
        jak_bitmap_create(&root_path, UCHAR_MAX);
        jak_bitmap_set(&root_path, 0, true);
        assign_code(new_node, &root_path, table);
        jak_bitmap_drop(&root_path);

        jak_vector_drop(&candidates);
}