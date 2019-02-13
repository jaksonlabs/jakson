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
#include "carbon/carbon-huffman.h"
#include "carbon/carbon-bitmap.h"

#define DIAG_HUFFMAN_ENABLE_DEBUG

typedef struct carbon_huffman
{
    carbon_vec_t ofType(carbon_huffman_entry_t) table;
    carbon_err_t err;
} carbon_huffman_t;

typedef struct huff_node huff_node_t;

typedef struct huff_node
{
    huff_node_t *prev, *next, *left, *right;
    uint64_t freq;
    unsigned char letter;
} huff_node_t;

static void huff_tree_create(carbon_vec_t ofType(carbon_huffman_entry_t) *table, const carbon_vec_t ofType(uint32_t) *frequencies);

bool carbon_huffman_create(carbon_huffman_t **out, const carbon_string_ref_vec *strings)
{
    CARBON_NON_NULL_OR_ERROR(out);
    CARBON_NON_NULL_OR_ERROR(strings);

    carbon_vec_t ofType(uint32_t) frequencies;
    carbon_vec_create(&frequencies, NULL, sizeof(uint32_t), UCHAR_MAX);
    carbon_vec_enlarge_size_to_capacity(&frequencies);

    uint32_t *freq_data = CARBON_VECTOR_ALL(&frequencies, uint32_t);
    CARBON_ZERO_MEMORY(freq_data, UCHAR_MAX * sizeof(uint32_t));

    for (size_t i = 0; i < strings->num_elems; i++) {
        const char *string = *CARBON_VECTOR_GET(strings, i, const char *);
        size_t string_length = strlen(string);
        for (size_t k = 0; k < string_length; k++) {
            size_t c = (unsigned char) string[k];
            freq_data[c]++;
        }
    }

    carbon_huffman_t *dic = malloc(sizeof(carbon_huffman_t));
    carbon_vec_create(&dic->table, NULL, sizeof(carbon_huffman_entry_t), UCHAR_MAX / 4);
    huff_tree_create(&dic->table, &frequencies);
    carbon_error_init(&dic->err);

    carbon_vec_drop(&frequencies);
    *out = dic;

    return true;
}

CARBON_EXPORT(bool)
carbon_huffman_get_error(carbon_err_t *err, const carbon_huffman_t *dic)
{
    CARBON_NON_NULL_OR_ERROR(err)
    CARBON_NON_NULL_OR_ERROR(dic)
    carbon_error_cpy(err, &dic->err);
    return true;
}

bool carbon_huffman_drop(carbon_huffman_t *dic)
{
    CARBON_NON_NULL_OR_ERROR(dic);

    for (size_t i = 0; i < dic->table.num_elems; i++) {
        carbon_huffman_entry_t *entry = CARBON_VECTOR_GET(&dic->table, i, carbon_huffman_entry_t);
        free(entry->blocks);
    }

    carbon_vec_drop(&dic->table);

    free(dic);

    return true;
}

bool carbon_huffman_serialize_dic(carbon_memfile_t *file, const carbon_huffman_t *dic, char marker_symbol)
{
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(dic)

    for (size_t i = 0; i < dic->table.num_elems; i++) {
        carbon_huffman_entry_t *entry = CARBON_VECTOR_GET(&dic->table, i, carbon_huffman_entry_t);
        carbon_memfile_write(file, &marker_symbol, sizeof(char));
        carbon_memfile_write(file, &entry->letter, sizeof(unsigned char));

        /** block one is the block that holds the significant part of the prefix code */
        carbon_off_t offset_meta, offset_continue;
        carbon_memfile_tell(&offset_meta, file);
        /** this will be the number of bytes used to encode the significant part of the prefix code */
        carbon_memfile_skip(file, sizeof(uint8_t));

        carbon_memfile_begin_bit_mode(file);
        bool first_bit_found = false;
        for (int i = 31; entry->blocks && i >= 0; i--) {
            uint32_t mask = 1 << i;
            uint32_t k = entry->blocks[0] & mask;
            bool bit_state = k != 0;
            first_bit_found |= bit_state;

            if (first_bit_found) {
                carbon_memfile_write_bit(file, bit_state);
            }
        }
        size_t num_bytes_written;
        carbon_memfile_end_bit_mode(&num_bytes_written, file);
        carbon_memfile_tell(&offset_continue, file);
        carbon_memfile_seek(file, offset_meta);
        uint8_t num_bytes_written_uint8 = (uint8_t) num_bytes_written;
        carbon_memfile_write(file, &num_bytes_written_uint8, sizeof(uint8_t));

        carbon_memfile_seek(file, offset_continue);
    }

    return true;
}

static carbon_huffman_entry_t *find_dic_entry(carbon_huffman_t *dic, unsigned char c)
{
    for (size_t i = 0; i < dic->table.num_elems; i++) {
        carbon_huffman_entry_t *entry = CARBON_VECTOR_GET(&dic->table, i, carbon_huffman_entry_t);
        if (entry->letter == c) {
            return entry;
        }
    }
    CARBON_ERROR(&dic->err, CARBON_ERR_HUFFERR)
    return NULL;
}

static size_t encodeString(carbon_memfile_t *file, carbon_huffman_t *dic, const char *string)
{
    carbon_memfile_begin_bit_mode(file);

    for (const char *c = string; *c != '\0'; c++) {
        carbon_huffman_entry_t *entry = find_dic_entry(dic, (unsigned char) *c);
        if (!entry) {
            return 0;
        }

        if (!entry->blocks) {
            carbon_memfile_write_bit(file, false);
        } else {
            for (size_t j = 0; j < entry->nblocks; j++) {
                uint32_t block = entry->blocks[j];

                bool first_bit_found = false;
                for (int i = 31; i >= 0; i--) {
                    uint32_t mask = 1 << i;
                    uint32_t k = block & mask;
                    bool bit_state = k != 0;
                    first_bit_found |= bit_state;

                    if (first_bit_found) {
                        carbon_memfile_write_bit(file, bit_state);
                    }
                }
            }
        }
    }

    size_t num_written_bytes;
    carbon_memfile_end_bit_mode(&num_written_bytes, file);
    return num_written_bytes;
}

bool carbon_huffman_encode(carbon_memfile_t *file,
                           carbon_huffman_t *dic,
                           char marker_symbol,
                           const carbon_vec_t ofType(carbon_string_id_t) *string_ids,
                           const carbon_string_ref_vec *strings)
{
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(dic)
    CARBON_NON_NULL_OR_ERROR(strings)

    assert(string_ids->num_elems == strings->num_elems);

    for (size_t i = 0; i < strings->num_elems; i++) {
        const char *string = *CARBON_VECTOR_GET(strings, i, const char *);
        carbon_string_id_t string_id = *CARBON_VECTOR_GET(string_ids, i, carbon_string_id_t);
        carbon_off_t offset, offset_continue;
        uint32_t string_length = (uint32_t) strlen(string);
        uint32_t num_bytes_encoded = 0;
        carbon_memfile_write(file, &marker_symbol, sizeof(char));
        carbon_memfile_write(file, &string_id, sizeof(carbon_string_id_t));
        carbon_memfile_write(file, &string_length, sizeof(uint32_t));
        carbon_memfile_tell(&offset, file);
        carbon_memfile_skip(file, sizeof(uint32_t));
        if ((num_bytes_encoded = (uint32_t) encodeString(file, dic, string)) == 0) {
            return false;
        }
        carbon_memfile_tell(&offset_continue, file);
        carbon_memfile_seek(file, offset);
        carbon_memfile_write(file, &num_bytes_encoded, sizeof(uint32_t));
        carbon_memfile_seek(file, offset_continue);
    }

    return true;
}

bool carbon_huffman_read_string(carbon_huffman_encoded_str_info_t *info, carbon_memfile_t *file, char marker_symbol)
{
    char marker = *CARBON_MEMFILE_PEEK(file, char);
    if (marker == marker_symbol) {
        carbon_memfile_skip(file, sizeof(char));
        info->string_id = *CARBON_MEMFILE_READ_TYPE(file, carbon_string_id_t);
        info->str_length = *CARBON_MEMFILE_READ_TYPE(file, uint32_t);
        info->nbytes_encoded = *CARBON_MEMFILE_READ_TYPE(file, uint32_t);
        info->encoded_bytes = CARBON_MEMFILE_READ(file, info->nbytes_encoded);
        return true;
    } else {
        return false;
    }
}

bool carbon_huffman_read_dic_entry(carbon_huffman_entry_info_t *info, carbon_memfile_t *file, char marker_symbol)
{
    char marker = *CARBON_MEMFILE_PEEK(file, char);
    if (marker == marker_symbol) {
        carbon_memfile_skip(file, sizeof(char));
        info->letter = *CARBON_MEMFILE_READ_TYPE(file, unsigned char);
        info->nbytes_prefix = *CARBON_MEMFILE_READ_TYPE(file, uint8_t);
        info->prefix_code = CARBON_MEMFILE_PEEK(file, char);

        carbon_memfile_skip(file, info->nbytes_prefix);

        return true;
    } else {
        return false;
    }
}

static const uint32_t *get_num_used_blocks(uint16_t *numUsedBlocks, carbon_huffman_entry_t *entry, uint16_t num_blocks,
                                        const uint32_t *blocks)
{
    for (entry->nblocks = 0; entry->nblocks < num_blocks; entry->nblocks++) {
        const uint32_t *block = blocks + entry->nblocks;
        if (*block != 0) {
            *numUsedBlocks = (num_blocks - entry->nblocks);
            return block;
        }
    }
    return NULL;
}

static void import_into_entry(carbon_huffman_entry_t *entry, const huff_node_t *node, const carbon_bitmap_t *carbon_bitmap_t)
{
    entry->letter = node->letter;
    uint32_t *blocks, num_blocks;
    const uint32_t *used_blocks;
    carbon_bitmap_blocks(&blocks, &num_blocks, carbon_bitmap_t);
    used_blocks = get_num_used_blocks(&entry->nblocks, entry, num_blocks, blocks);
    entry->blocks = malloc(entry->nblocks * sizeof(uint32_t));
    if (num_blocks > 0) {
        memcpy(entry->blocks, used_blocks, entry->nblocks * sizeof(uint32_t));
    } else {
        entry->blocks = NULL;
    }
    free(blocks);
}

static huff_node_t *seek_to_begin(huff_node_t *handle) {
    for (; handle->prev != NULL; handle = handle->prev)
        ;
    return handle;
}

static huff_node_t *seek_to_end(huff_node_t *handle) {
    for (; handle->next != NULL; handle = handle->next)
        ;
    return handle;
}

CARBON_FUNC_UNUSED
static void __diag_print_insight(huff_node_t *n)
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
    printf(": %zu",n->freq);
}

CARBON_FUNC_UNUSED
static void __diag_dump_remaining_candidates(huff_node_t *n)
{
    huff_node_t *it = seek_to_begin(n);
    while (it->next != NULL) {
        __diag_print_insight(it);
        printf(" | ");
        it = it->next;
    }
}

static huff_node_t *find_smallest(huff_node_t *begin, uint64_t lowerBound, huff_node_t *skip)
{
    uint64_t smallest = UINT64_MAX;
    huff_node_t *result = NULL;
    for (huff_node_t *it = begin; it != NULL; it = it->next) {
        if (it != skip && it->freq >= lowerBound && it->freq <= smallest) {
            smallest = it->freq;
            result = it;
        }
    }
    return result;
}


static void assign_code(huff_node_t *node, const carbon_bitmap_t *path, carbon_vec_t ofType(carbon_huffman_entry_t) *table)
{
    if (!node->left && !node->right) {
            carbon_huffman_entry_t *entry = VECTOR_NEW_AND_GET(table, carbon_huffman_entry_t);
            import_into_entry(entry, node, path);
    } else {
        if (node->left) {
            carbon_bitmap_t left;
            carbon_bitmap_cpy(&left, path);
            carbon_bitmap_lshift(&left);
            carbon_bitmap_set(&left, 0, false);
            assign_code(node->left, &left, table);
            carbon_bitmap_drop(&left);
        }
        if (node->right) {
            carbon_bitmap_t right;
            carbon_bitmap_cpy(&right, path);
            carbon_bitmap_lshift(&right);
            carbon_bitmap_set(&right, 0, true);
            assign_code(node->right, &right, table);
            carbon_bitmap_drop(&right);
        }
    }
}

static huff_node_t *trim_and_begin(carbon_vec_t ofType(HuffNode) *candidates)
{
    huff_node_t *begin = NULL;
    for (huff_node_t *it = CARBON_VECTOR_GET(candidates, 0, huff_node_t); ; it++) {
        if (it->freq == 0) {
            --candidates->num_elems;
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

static void huff_tree_create(carbon_vec_t ofType(carbon_huffman_entry_t) *table, const carbon_vec_t ofType(uint32_t) *frequencies)
{
    assert(UCHAR_MAX == frequencies->num_elems);

    carbon_vec_t ofType(HuffNode) candidates;
    carbon_vec_create(&candidates, NULL, sizeof(huff_node_t), UCHAR_MAX * UCHAR_MAX);
    size_t appender_idx = UCHAR_MAX;

    for (unsigned char i = 0; i < UCHAR_MAX; i++) {
        huff_node_t *node = VECTOR_NEW_AND_GET(&candidates, huff_node_t);
        node->letter = i;
        node->freq = *CARBON_VECTOR_GET(frequencies, i, uint32_t);
    }

    for (unsigned char i = 0; i < UCHAR_MAX; i++) {
        huff_node_t *node = CARBON_VECTOR_GET(&candidates, i, huff_node_t);
        huff_node_t *prev = i > 0 ? CARBON_VECTOR_GET(&candidates, i - 1, huff_node_t) : NULL;
        huff_node_t *next = i + 1 < UCHAR_MAX ? CARBON_VECTOR_GET(&candidates, i + 1, huff_node_t) : NULL;
        node->next = next;
        node->prev = prev;
        node->left = node->right = NULL;
    }


    huff_node_t *smallest, *small;
    huff_node_t *handle = trim_and_begin(&candidates);
    huff_node_t *new_node;


    while (handle->next != NULL) {
        smallest = find_smallest(handle, 0, NULL);
        small = find_smallest(handle, smallest->freq, smallest);

        appender_idx++;
        new_node = VECTOR_NEW_AND_GET(&candidates, huff_node_t);
        new_node->freq = small->freq + smallest->freq;
        new_node->letter = '\0';
        new_node->left = small;
        new_node->right = smallest;

        if((small->prev == NULL && smallest->next == NULL) && small->next == smallest) {
            break;
        }

        if((smallest->prev == NULL && small->next == NULL) && smallest->next == small) {
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
            CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR);
        }

        assert (!handle->prev);
        huff_node_t *end = seek_to_end(handle);
        assert(!end->next);
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
    if(handle->next) {
        huff_node_t *finalNode = VECTOR_NEW_AND_GET(&candidates, huff_node_t);
        finalNode->freq = small->freq + smallest->freq;
        finalNode->letter = '\0';
        if (handle->freq < handle->next->freq) {
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

    carbon_bitmap_t root_path;
    carbon_bitmap_create(&root_path, UCHAR_MAX);
    carbon_bitmap_set(&root_path, 0, true);
    assign_code(new_node, &root_path, table);
    carbon_bitmap_drop(&root_path);

    carbon_vec_drop(&candidates);
}