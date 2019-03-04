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
    Hash i, j, min_idx = 0;

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
                swapHashes(&mapping[min_idx], &mapping[i]);



    }
}

size_t removeDuplicates2(Hash keyHashColumn[], size_t arraySize, size_t duplicates[], Hash keyTargetColumn[], Hash mapping[])
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
            mapping[targetIndex] = i;
            duplicates[targetIndex] = 0;

        }
    }

    return elemNumber;
}

void slice_linearizeImpl(Hash compressedColumn[], Hash targetColumn[], size_t low, size_t high, Hash mapping[], Hash duplicates[], size_t factor,
    size_t level, size_t index, Hash newMapping[], Hash newDuplicates[]) {
    // Call recursive if enough elements available
    if((high - low) >= 23) {
        // TO BE DONE
        UNUSED(level);
        UNUSED(index);

        // Fill up current node
        size_t i = 0;

        // Black magic
        size_t startIndex = level == 0 ? 0 : (((size_t)pow(factor, level) - 1) + index * (factor - 1));
        int sourceIndex = low - 1;
        size_t targetIndex;
        for(i = 0; i < factor - 1; i++) {
            sourceIndex += ((high - low) / factor) + 1;
            targetIndex = startIndex + i;
           //  sourceIndex = compressedColumn[sourceIndex];
            Hash val = compressedColumn[sourceIndex];
            UNUSED(val);

            targetColumn[targetIndex] = compressedColumn[sourceIndex];

            // Reorder duplicates  and mapping
            //swapHashes(&mapping[sourceIndex], &mapping[targetIndex]);
            newMapping[targetIndex] = mapping[sourceIndex];
            if (duplicates[sourceIndex] > 0) {
                size_t j;
                for(j = 1; j < duplicates[sourceIndex]; j++) {
                    // swapHashes(&mapping[sourceIndex + j], &mapping[targetIndex + j]);
                    newMapping[targetIndex + j] = mapping[sourceIndex + j];
                }
                // swapHashes(&duplicates[sourceIndex], &duplicates[targetIndex]);
                newDuplicates[targetIndex] = duplicates[sourceIndex];
            }

        }
        size_t begin = low;
        size_t end = begin + ((high - low) / factor) - 1;
        for(i = 0; i < factor; i++) {
            // begin = low + i * (factor - 1);
            // end = i + (i + 1) * (factor - 1) > high ? high : i + (i + 1) * (factor - 1);
            slice_linearizeImpl(compressedColumn, targetColumn, begin, end,
                    mapping, duplicates, factor, (level + 1), (index * (factor)) + i, newMapping, newDuplicates);

            begin += ((high - low)  / factor) + 1;
            end += ((high - low)  / factor) + 1;
        }
    }
    else {
        size_t i = 0;

        // Black magic
        // size_t startIndex = level == 0 ? 0 : 4 + ((size_t)pow(factor, level - 1) + (factor - 1) * index) - 1; //(factor - 1) + ((factor - 1) * (size_t)pow(factor, level - 1)) + (index * factor);
        size_t startIndex = (((size_t)pow(factor, level) - 1)) + (factor - 1) * index;
        size_t sourceIndex, targetIndex;

        for(i = 0; i <= (high-low); i++) {
            targetIndex = startIndex + i;
            sourceIndex = low + i;
            targetColumn[targetIndex] = compressedColumn[sourceIndex];

            // Reorder duplicates  and mapping
            // swapHashes(&mapping[sourceIndex], &mapping[targetIndex]);
            newMapping[targetIndex] = mapping[sourceIndex];
            if (duplicates[sourceIndex] > 0) {
                size_t j;
                for(j = 1; j < duplicates[sourceIndex]; j++) {
                    newMapping[targetIndex + j] = mapping[sourceIndex + j];
                }
                // swapHashes(&duplicates[sourceIndex], &duplicates[targetIndex]);
                newDuplicates[targetIndex] = duplicates[sourceIndex];
            }
        }
    }
}

void slice_linearize(Hash compressedColumn[], Hash targetColumn[], size_t low, size_t high, Hash mapping[], Hash duplicates[], size_t factor, Hash newMapping[], Hash newDuplicates[]) {
    slice_linearizeImpl(compressedColumn, targetColumn, low, high, mapping, duplicates, factor, 0, 0, newMapping, newDuplicates);
}

#define numElems 24

int main()
{
    // size_t mapping[24] = {0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    // size_t compressedHashes[24] = {0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    // size_t duplicates[24] = {0,0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0};
    // size_t newHashes[24] = {0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0};
    size_t data2[32] = { 14,15,16,17,18,19,20,21,22,23,24,1,2,2,3,3,3,4,5,6,7,8,8,8,9,10,10,11,12,12,12,13};
    UNUSED(data2);

    // size_t data[numElems];
    size_t duplicates[numElems];
    size_t newDuplicates[numElems];
    size_t compressedHashes[numElems];
    size_t newHashes[numElems];

    const char* keyColumn[32] = {"A1","A2","A3","A4","A5","A6","A7","A8","A9","A10","A11","A12","A13","A14","A15","A16","A17","A18","A19","A20","A21","A22","A23","A24", "A25", "A26", "A27", "A28", "A29", "A30", "A31", "A32"};
    // const char* stringIdColumn[] = {"A", "B", "C", "D", "E", "F"};
    size_t i = 0;
    for(i = 0; i < 32; i++) {
        // data[i] = numElems - i;
        duplicates[i] = 0;
        newDuplicates[i] = 0;
        // mapping[i] = i;
        compressedHashes[i] = 0;
    }
    size_t mapping[32] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    size_t data[32] = { 14,15,16,17,18,19,20,21,22,23,24,1,2,2,3,3,3,4,5,6,7,8,8,8,9,10,10,11,12,12,12,13};
    Hash val = mapping[5];
    UNUSED(val);

    selectionSort2(data, 32, mapping);

    for(i = 0; i < 7; i++) {
        Hash value = data[i];
        const char* key = keyColumn[mapping[i]];

        assert(value);
        assert(key);
    }

    size_t newMapping[32];
    memcpy(&newMapping, &mapping, sizeof(mapping));

    size_t result = removeDuplicates2(data, 32, duplicates, newHashes, newMapping);
    UNUSED(result);

    size_t newMapping2[32];
    memcpy(&newMapping2, &mapping, sizeof(mapping));

    slice_linearize(newHashes,compressedHashes, 0, result, newMapping, duplicates, 5, newMapping2, newDuplicates);
    int a = -5;
    UNUSED(a);
    for(i = 0; i < 7; i++) {
        Hash value = compressedHashes[i];
        const char* key = keyColumn[mapping[newMapping2[i]]];

        assert(value);
        assert(key);
    }

    return 0;
}