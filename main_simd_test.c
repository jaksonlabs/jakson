//
// Created by marten on 15.12.18.
//

#include <stdio.h>
#include <math.h>
#include "src/slicelist.h"
#include "src/slicesort.h"
#include "src/common.h"
#include "src/vector.h"
#include "src/bitmap.h"
#include "src/spinlock.h"
#include "src/bloomfilter.h"
#include "src/hash.h"
#include "src/simd.h"

inline static void swapHashes(Hash* a, Hash* b)
{
    Hash t = *a;
    *a = *b;
    *b = t;
}
/*
inline static void swapKeys(const char ** a, const char ** b)
{
    const char *  t = *a;
    *a = *b;
    *b = t;
}
*/

/*size_t removeDuplicatesSelectionSort(Hash keyHashColumn[], const char * keyColumn[],
        const char * stringIdColumn[], size_t arraySize, size_t duplicates[], Hash keyTargetColumn[])
{
    size_t i, j, min_idx, elemNumber = 0;

    // One by one move boundary of unsorted subarray
    for (i = 0; i < arraySize-1; i++)
    {
        if (keyHashColumn[i] == 0)
            continue;

        // Find the minimum element in unsorted array
        min_idx = i;
        for (j = i+1; j < arraySize; j++)
            if(keyHashColumn[j] == keyHashColumn[min_idx]) {
                duplicates[i]++;
            }
            else if (keyHashColumn[j] < keyHashColumn[min_idx])
                min_idx = j;

        if(keyHashColumn[min_idx] == keyHashColumn[i]) {

            keyHashColumn[min_idx] = SIZE_MAX;
        }
        else {
            // Swap the found minimum element with the first element
            swapHashes(&keyHashColumn[min_idx], &keyHashColumn[i]);
            swapKeys(&keyColumn[min_idx], &keyColumn[i]);
            swapKeys(&stringIdColumn[min_idx], &stringIdColumn[i]);
            duplicates[i] = 0;
            elemNumber++;
        }
    }

    return elemNumber;
}
*/

void bubblesort2(Hash arr[], size_t length, Hash mapping[])
{
    size_t i, j;

    for (i = 1; i < length ; i++)
    {
        for (j = 0; j < length - i ; j++)
        {
            if (arr[j] > arr[j+1])
            {
                swapHashes(&arr[j], &arr[j + 1]);
                mapping[j] = j + 1;
                mapping[j + 1] = j;
                // swapKeys(&stringIdArr[j], &stringIdArr[j + 1]);
            }
        }
    }
}

void selectionSort2(Hash arr[], size_t length, Hash mapping[])
{
    Hash i, j, min_idx;

    // One by one move boundary of unsorted subarray
    for (i = 0; i < length-1; i++)
    {
        // Find the minimum element in unsorted array
        min_idx = i;
        for (j = i+1; j < length; j++)
            if (arr[j] < arr[min_idx])
                min_idx = j;

        // Swap the found minimum element with the first element
        swapHashes(&arr[min_idx], &arr[i]);
        mapping[i] = min_idx;
    }
}

size_t removeDuplicates2(Hash keyHashColumn[], size_t arraySize, size_t duplicates[], Hash keyTargetColumn[])
{
    size_t elemNumber = 0;
    size_t i = 0;
    int targetIndex = -1;
    for (i = 0; i < arraySize; i++)
    {
        if (targetIndex >= 0 && keyHashColumn[i] == keyTargetColumn[targetIndex])
            duplicates[targetIndex]++;
        else
        {
            elemNumber++;
            keyTargetColumn[++targetIndex] = keyHashColumn[i];
            duplicates[targetIndex] = 0;

        }
    }

    return elemNumber;
}

void slice_linearizeImpl(Hash compressedColumn[], Hash targetColumn[], size_t low, size_t high, Hash mapping[], Hash duplicates[], size_t factor,
    size_t level, size_t index) {
    // Call recursive if enough elements available
    if(((high - low) / (factor - 1)) >= factor) {
        // TO BE DONE
        UNUSED(level);
        UNUSED(index);

        // Fill up current node
        size_t i = 0;

        // Black magic
        size_t startIndex = level == 0 ? 0 : 4 + ((size_t)pow(factor, level - 1) + (factor - 1) * index) - 1;
        int sourceIndex = low - 1;
        size_t targetIndex;
        for(i = 0; i < factor - 1; i++) {
            sourceIndex += factor;
            targetIndex = startIndex + i;
           //  sourceIndex = compressedColumn[sourceIndex];
            Hash val = compressedColumn[sourceIndex];
            UNUSED(val);

            targetColumn[targetIndex] = compressedColumn[sourceIndex];

            // Reorder duplicates  and mapping
            if (duplicates[sourceIndex] > 0) {
                size_t j;
                for(j = 0; j < duplicates[sourceIndex]; j++) {
                    swapHashes(&mapping[sourceIndex + j], &mapping[targetIndex + j]);
                }
                swapHashes(&duplicates[sourceIndex], &duplicates[targetIndex]);
            }

        }
        size_t begin = low;
        size_t end = begin + factor - 2;
        for(i = 0; i < factor; i++) {
            // begin = low + i * (factor - 1);
            // end = i + (i + 1) * (factor - 1) > high ? high : i + (i + 1) * (factor - 1);
            slice_linearizeImpl(compressedColumn, targetColumn, begin, end,
                    mapping, duplicates, factor, (level + 1), i);

            begin += factor;
            end += factor;
        }
    }
    else {
        size_t i = 0;

        // Black magic
        size_t startIndex = level == 0 ? 0 : 4 + ((size_t)pow(factor, level - 1) + (factor - 1) * index) - 1; //(factor - 1) + ((factor - 1) * (size_t)pow(factor, level - 1)) + (index * factor);
        size_t sourceIndex, targetIndex;

        for(i = 0; i <= (high-low); i++) {
            targetIndex = startIndex + i;
            sourceIndex = low + i;
            targetColumn[targetIndex] = compressedColumn[sourceIndex];

            // Reorder duplicates  and mapping
            if (duplicates[sourceIndex] > 0) {
                size_t j;
                for(j = 0; j < duplicates[sourceIndex]; j++) {
                    swapHashes(&mapping[sourceIndex + j], &mapping[targetIndex + j]);
                }
                swapHashes(&duplicates[sourceIndex], &duplicates[targetIndex]);
            }
        }
    }
}

void slice_linearize(Hash compressedColumn[], Hash targetColumn[], size_t low, size_t high, Hash mapping[], Hash duplicates[], size_t factor) {
    slice_linearizeImpl(compressedColumn, targetColumn, low, high, mapping, duplicates, factor, 0, 0);
}

int main()
{
    size_t mapping[24] = {0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    size_t compressedHashes[24] = {0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    size_t duplicates[24] = {0,0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0};
    size_t newHashes[24] = {0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0};
    size_t data[32] = { 14,15,16,17,18,19,20,21,22,23,24,1,2,2,3,3,3,4,5,6,7,8,8,8,9,10,10,11,12,12,12,13};
    const char* keyColumn[] = {"A1","A2","A3","A4","A5","A6","A7","A8","A9","A10","A11","A12","A13","A14","A15","A16","A17","A18","A19","A20","A21","A22","A23","A24"};
    // const char* stringIdColumn[] = {"A", "B", "C", "D", "E", "F"};
    selectionSort2(data, 32, mapping);
    size_t result = removeDuplicates2(data, 32, duplicates, newHashes);
    UNUSED(result);

    slice_linearize(newHashes,compressedHashes, 0, 24, mapping, duplicates, 5);
    size_t i = 0;
    for(i = 0; i < 7; i++) {
        Hash value = data[i];
        const char* key = keyColumn[mapping[i]];

        assert(value);
        assert(key);
    }


    return 0;
}