/*
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
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include <limits.h>

#include "carbon/carbon-huffman.h"
#include "carbon/carbon-bitmap.h"

// ---------------------------------------------------------------------------------------------------------------------
//
//  D E B U G C O N F I G
//
// ---------------------------------------------------------------------------------------------------------------------

// #define DIAG_HUFFMAN_ENABLE_DEBUG

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

typedef struct carbon_huffman
{
    carbon_vec_t ofType(carbon_huffman_entry_t) encodingTable;
    carbon_err_t err;
} carbon_huffman_t;

CARBON_FORWARD_STRUCT_DECL(HuffNode)
CARBON_FORWARD_STRUCT_DECL(HuffTree)

struct HuffNode
{
    struct HuffNode *prev, *next, *left, *right;
    uint64_t freq;
    unsigned char letter;
};

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

//static int compareNodesByFrequency(const void *lhs, const void *rhs);
static void createHuffmanTree(carbon_vec_t ofType(carbon_huffman_entry_t) *encodingTable, const carbon_vec_t ofType(uint32_t) *frequencies);

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_huffman_create(carbon_huffman_t **out, const StringRefVector *strings)
{
    CARBON_NON_NULL_OR_ERROR(out);
    CARBON_NON_NULL_OR_ERROR(strings);

    carbon_vec_t ofType(uint32_t) frequencies;
    VectorCreate(&frequencies, NULL, sizeof(uint32_t), UCHAR_MAX);
    VectorEnlargeSizeToCapacity(&frequencies);

    uint32_t *freqData = VECTOR_ALL(&frequencies, uint32_t);
    CARBON_ZERO_MEMORY(freqData, UCHAR_MAX * sizeof(uint32_t));

    for (size_t i = 0; i < strings->numElems; i++) {
        const char *string = *VECTOR_GET(strings, i, const char *);
        size_t stringLength = strlen(string);
        for (size_t k = 0; k < stringLength; k++) {
            size_t c = (unsigned char) string[k];
            freqData[c]++;
        }
    }

    carbon_huffman_t *dic = malloc(sizeof(carbon_huffman_t));
    VectorCreate(&dic->encodingTable, NULL, sizeof(carbon_huffman_entry_t), UCHAR_MAX / 4);
    createHuffmanTree(&dic->encodingTable, &frequencies);
    carbon_error_init(&dic->err);

    VectorDrop(&frequencies);
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

    for (size_t i = 0; i < dic->encodingTable.numElems; i++) {
        carbon_huffman_entry_t *entry = VECTOR_GET(&dic->encodingTable, i, carbon_huffman_entry_t);
        free(entry->blocks);
    }

    VectorDrop(&dic->encodingTable);

    free(dic);

    return true;
}

bool carbon_huffman_serialize_dic(carbon_memfile_t *file, const carbon_huffman_t *dic, char markerSymbol)
{
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(dic)

    for (size_t i = 0; i < dic->encodingTable.numElems; i++) {
        carbon_huffman_entry_t *entry = VECTOR_GET(&dic->encodingTable, i, carbon_huffman_entry_t);
        carbon_memfile_write(file, &markerSymbol, sizeof(char));
        carbon_memfile_write(file, &entry->letter, sizeof(unsigned char));

        /* block one is the block that holds the significant part of the prefix code */
        carbon_off_t offsetMeta, offsetContinue;
        carbon_memfile_tell(&offsetMeta, file);
        /* this will be the number of bytes used to encode the significant part of the prefix code */
        carbon_memfile_skip(file, sizeof(uint8_t));

        carbon_memfile_begin_bit_mode(file);
        bool firstBitFound = false;
        for (int i = 31; entry->blocks && i >= 0; i--) {
            uint32_t mask = 1 << i;
            uint32_t k = entry->blocks[0] & mask;
            bool bitState = k != 0;
            firstBitFound |= bitState;

            if (firstBitFound) {
                carbon_memfile_write_bit(file, bitState);
            }
        }
        size_t numBytesWritten;
        carbon_memfile_end_bit_mode(&numBytesWritten, file);
        carbon_memfile_tell(&offsetContinue, file);
        carbon_memfile_seek(file, offsetMeta);
        uint8_t numBytesWrittenUInt8 = (uint8_t) numBytesWritten;
        carbon_memfile_write(file, &numBytesWrittenUInt8, sizeof(uint8_t));

        carbon_memfile_seek(file, offsetContinue);
    }

    return true;
}

static carbon_huffman_entry_t *findDicEntry(carbon_huffman_t *dic, unsigned char c)
{
    for (size_t i = 0; i < dic->encodingTable.numElems; i++) {
        carbon_huffman_entry_t *entry = VECTOR_GET(&dic->encodingTable, i, carbon_huffman_entry_t);
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
        carbon_huffman_entry_t *entry = findDicEntry(dic, (unsigned char) *c);
        if (!entry) {
            return 0;
        }

        if (!entry->blocks) {
            carbon_memfile_write_bit(file, false);
        } else {
            for (size_t j = 0; j < entry->nblocks; j++) {
                uint32_t block = entry->blocks[j];

                bool firstBitFound = false;
                for (int i = 31; i >= 0; i--) {
                    uint32_t mask = 1 << i;
                    uint32_t k = block & mask;
                    bool bitState = k != 0;
                    firstBitFound |= bitState;

                    if (firstBitFound) {
                        carbon_memfile_write_bit(file, bitState);
                    }
                }
            }
        }
    }

    size_t numWrittenBytes;
    carbon_memfile_end_bit_mode(&numWrittenBytes, file);
    return numWrittenBytes;
}

bool carbon_huffman_encode(carbon_memfile_t *file,
                           carbon_huffman_t *dic,
                           char markerSymbol,
                           const carbon_vec_t ofType(carbon_string_id_t) *carbon_string_id_ts,
                           const StringRefVector *strings)
{
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(dic)
    CARBON_NON_NULL_OR_ERROR(strings)

    assert(carbon_string_id_ts->numElems == strings->numElems);

    for (size_t i = 0; i < strings->numElems; i++) {
        const char *string = *VECTOR_GET(strings, i, const char *);
        carbon_string_id_t string_id = *VECTOR_GET(carbon_string_id_ts, i, carbon_string_id_t);
        carbon_off_t offset, offsetContinue;
        uint32_t stringLength = (uint32_t) strlen(string);
        uint32_t numBytesEncoded = 0;
        carbon_memfile_write(file, &markerSymbol, sizeof(char));
        carbon_memfile_write(file, &string_id, sizeof(carbon_string_id_t));
        carbon_memfile_write(file, &stringLength, sizeof(uint32_t));
        carbon_memfile_tell(&offset, file);
        carbon_memfile_skip(file, sizeof(uint32_t));
        if ((numBytesEncoded = (uint32_t) encodeString(file, dic, string)) == 0) {
            return false;
        }
        carbon_memfile_tell(&offsetContinue, file);
        carbon_memfile_seek(file, offset);
        carbon_memfile_write(file, &numBytesEncoded, sizeof(uint32_t));
        carbon_memfile_seek(file, offsetContinue);
    }

    return true;
}

bool carbon_huffman_read_string(carbon_huffman_encoded_str_info_t *info, carbon_memfile_t *file, char markerSymbol)
{
    char marker = *CARBON_MEMFILE_PEEK(file, char);
    if (marker == markerSymbol) {
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

bool carbon_huffman_read_dic_entry(carbon_huffman_entry_info_t *info, carbon_memfile_t *file, char markerSymbol)
{
    char marker = *CARBON_MEMFILE_PEEK(file, char);
    if (marker == markerSymbol) {
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

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

static const uint32_t *getNumUsedBlocks(uint16_t *numUsedBlocks, carbon_huffman_entry_t *entry, uint16_t numBlocks,
                                        const uint32_t *blocks)
{
    for (entry->nblocks = 0; entry->nblocks < numBlocks; entry->nblocks++) {
        const uint32_t *block = blocks + entry->nblocks;
        if (*block != 0) {
            *numUsedBlocks = (numBlocks - entry->nblocks);
            return block;
        }
    }
    return NULL;
}

static void importIntoEntry(carbon_huffman_entry_t *entry, const struct HuffNode *node, const carbon_bitmap_t *carbon_bitmap_t)
{
    entry->letter = node->letter;
    uint32_t *blocks, numBlocks;
    const uint32_t *usedBlocks;
    carbon_bitmap_blocks(&blocks, &numBlocks, carbon_bitmap_t);
    usedBlocks = getNumUsedBlocks(&entry->nblocks, entry, numBlocks, blocks);
    entry->blocks = malloc(entry->nblocks * sizeof(uint32_t));
    if (numBlocks > 0) {
        memcpy(entry->blocks, usedBlocks, entry->nblocks * sizeof(uint32_t));
    } else {
        entry->blocks = NULL;
    }
    free(blocks);
}

static struct HuffNode *seekToBegin(struct HuffNode *handle) {
    for (; handle->prev != NULL; handle = handle->prev)
        ;
    return handle;
}

static struct HuffNode *seekToEnd(struct HuffNode *handle) {
    for (; handle->next != NULL; handle = handle->next)
        ;
    return handle;
}

CARBON_FUNC_UNUSED
static void __diagPrintInsight(struct HuffNode *n)
{
    printf("(");
    if (!n->left && !n->right) {
        printf("%c", n->letter);
    } else {
        if (n->left) {
            __diagPrintInsight(n->left);
        }
        printf(",");
        if (n->right) {
            __diagPrintInsight(n->right);
        }
    }
    printf(")");
}

CARBON_FUNC_UNUSED
static void __diagDumpRemainingCandidates(struct HuffNode *n)
{
    struct HuffNode *it = seekToBegin(n);
    while (it->next != NULL) {
        __diagPrintInsight(it);
        printf(" | ");
        it = it->next;
    }
}

static struct HuffNode *findSmallest(struct HuffNode *begin, uint64_t lowerBound, struct HuffNode *skip)
{
    uint64_t smallest = UINT64_MAX;
    struct HuffNode *result = NULL;
    for (struct HuffNode *it = begin; it != NULL; it = it->next) {
        if (it != skip && it->freq >= lowerBound && it->freq <= smallest) {
            smallest = it->freq;
            result = it;
        }
    }
    return result;
}


static void assignCode(struct HuffNode *node, const carbon_bitmap_t *path, carbon_vec_t ofType(carbon_huffman_entry_t) *encodingTable)
{
    if (!node->left && !node->right) {
            carbon_huffman_entry_t *entry = VECTOR_NEW_AND_GET(encodingTable, carbon_huffman_entry_t);
            importIntoEntry(entry, node, path);
    } else {
        if (node->left) {
            carbon_bitmap_t left;
            carbon_bitmap_cpy(&left, path);
            carbon_bitmap_lshift(&left);
            carbon_bitmap_set(&left, 0, false);
            assignCode(node->left, &left, encodingTable);
            carbon_bitmap_drop(&left);
        }
        if (node->right) {
            carbon_bitmap_t right;
            carbon_bitmap_cpy(&right, path);
            carbon_bitmap_lshift(&right);
            carbon_bitmap_set(&right, 0, true);
            assignCode(node->right, &right, encodingTable);
            carbon_bitmap_drop(&right);
        }
    }
}

static struct HuffNode *trimAndBegin(carbon_vec_t ofType(HuffNode) *candidates)
{
    struct HuffNode *begin = NULL;
    for (struct HuffNode *it = VECTOR_GET(candidates, 0, struct HuffNode); ; it++) {
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

#define DIAG_HUFFMAN_ENABLE_DEBUG

static void createHuffmanTree(carbon_vec_t ofType(carbon_huffman_entry_t) *encodingTable, const carbon_vec_t ofType(uint32_t) *frequencies)
{
    assert(UCHAR_MAX == frequencies->numElems);

    carbon_vec_t ofType(HuffNode) candidates;
    VectorCreate(&candidates, NULL, sizeof(struct HuffNode), UCHAR_MAX * UCHAR_MAX);
    size_t appenderIdx = UCHAR_MAX;

    for (unsigned char i = 0; i < UCHAR_MAX; i++) {
        struct HuffNode *node = VECTOR_NEW_AND_GET(&candidates, struct HuffNode);
        node->letter = i;
        node->freq = *VECTOR_GET(frequencies, i, uint32_t);
    }

    for (unsigned char i = 0; i < UCHAR_MAX; i++) {
        struct HuffNode *node = VECTOR_GET(&candidates, i, struct HuffNode);
        struct HuffNode *prev = i > 0 ? VECTOR_GET(&candidates, i - 1, struct HuffNode) : NULL;
        struct HuffNode *next = i + 1 < UCHAR_MAX ? VECTOR_GET(&candidates, i + 1, struct HuffNode) : NULL;
        node->next = next;
        node->prev = prev;
        node->left = node->right = NULL;
    }


    struct HuffNode *smallest, *small;
    struct HuffNode *handle = trimAndBegin(&candidates);
    struct HuffNode *newNode;

    while (handle->next != NULL) {
        smallest = findSmallest(handle, 0, NULL);
        small = findSmallest(handle, smallest->freq, smallest);

        appenderIdx++;
        newNode = VECTOR_NEW_AND_GET(&candidates, struct HuffNode);
        newNode->freq = small->freq + smallest->freq;
        newNode->letter = '\0';
        newNode->left = small;
        newNode->right = smallest;

        if(smallest->prev == NULL && small->next == NULL) {
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
            handle = seekToBegin(small->prev);
        } else if (small->next) {
            handle = seekToBegin(small->next);
        } else if (smallest->prev) {
            handle = seekToBegin(smallest->prev);
        } else if (smallest->next) {
            handle = seekToBegin(smallest->next);
        } else {
            CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR);
        }

        assert (!handle->prev);
        struct HuffNode *end = seekToEnd(handle);
        assert(!end->next);
        end->next = newNode;
        newNode->prev = end;
        newNode->next = NULL;

#ifdef DIAG_HUFFMAN_ENABLE_DEBUG
        printf("in-memory huff-tree: ");
        __diagPrintInsight(newNode);
        printf("\n");
        printf("remaining candidates: ");
        __diagDumpRemainingCandidates(handle);
        printf("\n");
#endif
    }

    seekToBegin(handle);
    if(handle->next) {
        struct HuffNode *finalNode = VECTOR_NEW_AND_GET(&candidates, struct HuffNode);
        finalNode->freq = small->freq + smallest->freq;
        finalNode->letter = '\0';
        finalNode->left = handle->next;
        finalNode->right = newNode;
        newNode = finalNode;
    }

#ifdef DIAG_HUFFMAN_ENABLE_DEBUG
    printf("final in-memory huff-tree: ");
    __diagPrintInsight(newNode);
    printf("\n");
#endif

    carbon_bitmap_t rootPath;
    carbon_bitmap_create(&rootPath, UCHAR_MAX);
    carbon_bitmap_set(&rootPath, 0, true);
    assignCode(newNode, &rootPath, encodingTable);
    carbon_bitmap_drop(&rootPath);

    VectorDrop(&candidates);
}