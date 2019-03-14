// file slicesort.c

/**
 *  Copyright (C) 2018 Marten Wallewein, Marcus Pinnecke
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "slicesort.h"

// ---------------------------------------------------------------------------------------------------------------------
//
//  C O N S T A N T S
//
// ---------------------------------------------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R
//
// ---------------------------------------------------------------------------------------------------------------------

inline static void swapHashes(Hash* a, Hash* b)
{
    Hash t = *a;
    *a = *b;
    *b = t;
}

inline static void swapKeys(const char ** a, const char ** b)
{
    const char *  t = *a;
    *a = *b;
    *b = t;
}


/*
inline static int partition (Hash arr[], int low, int high, const char * keyArr[], const char * stringIdArr[])
{
    Hash pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high- 1; j++)
    {
        if (arr[j] <= pivot)
        {
            if (arr[i] == 2701 && i == 0) {
                Hash value = arr[i];
                const char* key = keyArr[i];

                assert(value);
                assert(key);
            }
            i++;
            if(i != j){
                swapHashes(&arr[i], &arr[j]);
                swapKeys(&keyArr[i], &keyArr[j]);
                swapKeys(&stringIdArr[i], &stringIdArr[j]);
            }


            Hash value = arr[j];
            const char* key = keyArr[j];

            assert(value);
            assert(key);

            value = arr[i];
            key = keyArr[i];

            assert(value);
            assert(key);
        }
    }

    swapHashes(&arr[i + 1], &arr[high]);
    swapKeys(&keyArr[i + 1], &keyArr[high]);
    swapKeys(&stringIdArr[i + 1], &stringIdArr[high]);
    return (i + 1);
}
*/
//inline static int partition (Hash arr[], int low, int high, const char * keyArr[], const char * stringIdArr[])
//{


   /*
    *   Hash pivot = arr[high];

    int left= low -1;
    int right = high;
    * while(leftPointer < rightPointer) {
        while(arr[leftPointer] < pivot && leftPointer < rightPointer) {
            leftPointer++;
        }

        while(rightPointer > leftPointer && arr[rightPointer] >= pivot) {
            rightPointer--;
        }

        if (arr[leftPointer] > pivot) {
            swapHashes(&arr[leftPointer], &arr[rightPointer]);
            swapKeys(&keyArr[leftPointer], &keyArr[rightPointer]);
            swapKeys(&stringIdArr[leftPointer], &stringIdArr[rightPointer]);
        }
    }

    // swapHashes(&arr[leftPointer], &arr[rightPointer]);
    // swapKeys(&keyArr[leftPointer], &keyArr[rightPointer]);
    // swapKeys(&stringIdArr[leftPointer], &stringIdArr[rightPointer]);

    return leftPointer; */





    // return left;
//}

/*
inline static void quickSort(Hash arr[], int low, int high, const char * keyArr[],  const char * stringIdArr[])
{
    if (low < high)
    {
        int pi = partition(arr, low, high, keyArr, stringIdArr);
        quickSort(arr, low, pi - 1, keyArr, stringIdArr);
        quickSort(arr, pi + 1, high, keyArr, stringIdArr);
    }

    if (low < high) {
        Hash pivot = arr[high];

        int left= low -1;
        int right = high;

        while (left <= right) {
            while (arr[left] < pivot) {
                left++;
            }
            while (arr[right] > pivot) {
                right++;
            }

            if (left <= right) {
                swapHashes(&arr[left], &arr[right]);
                swapKeys(&keyArr[left], &keyArr[right]);
                swapKeys(&stringIdArr[left], &stringIdArr[right]);

                left = left + 1;
                right = right - 1;
            }
        }
        quickSort(arr, low, right, keyArr, stringIdArr);
        quickSort(arr, left + 1, high, keyArr, stringIdArr);
    }


}
*/
void bubblesort(Hash arr[], int length, const char * keyArr[],  const char * stringIdArr[])
{
    int i, j;

    for (i = 1; i < length ; i++)
    {
        for (j = 0; j < length - i ; j++)
        {
            if (arr[j] > arr[j+1])
            {
                swapHashes(&arr[j], &arr[j + 1]);
                swapKeys(&keyArr[j], &keyArr[j + 1]);
                swapKeys(&stringIdArr[j], &stringIdArr[j + 1]);
            }
        }
    }
}

void selectionSort(Hash arr[], size_t length, Hash mapping[])
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


// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

void slicesort2(Hash keyHashColumn[], const char * keyColumn[], const char * stringIdColumn[], size_t arraySize) {
    // quickSort(keyHashColumn, 0, arraySize, keyColumn, stringIdColumn);
    bubblesort(keyHashColumn, arraySize, keyColumn, stringIdColumn);
}

void slicesort3(Hash keyHashColumn[], size_t length, Hash mapping[]) {
    // quickSort(keyHashColumn, 0, arraySize, keyColumn, stringIdColumn);
    selectionSort(keyHashColumn, length, mapping);
}

/*void slicesort3(Hash keyHashColumn[], size_t length, Hash mapping[]) {
    // quickSort(keyHashColumn, 0, arraySize, keyColumn, stringIdColumn);
    selectionSort(keyHashColumn, length, mapping);
}*/


void selectionSort2(Hash arr[], size_t length, Hash mapping[]) {
    Hash i, j, min_idx = 0;

    // One by one move boundary of unsorted subarray
    for (i = 0; i < length - 1; i++) {
        // Find the minimum element in unsorted array
        min_idx = i;
        for (j = i + 1; j < length; j++)
            if (arr[j] < arr[min_idx])
                min_idx = j;

        // Swap the found minimum element with the first element
        swapHashes(&arr[min_idx], &arr[i]);
        swapHashes(&mapping[min_idx], &mapping[i]);


    }
}

size_t
removeDuplicates2(Hash keyHashColumn[], size_t arraySize, size_t duplicates[], Hash keyTargetColumn[], Hash mapping[]) {
    size_t elemNumber = 0;
    size_t i = 0;
    int targetIndex = -1;
    for (i = 0; i < arraySize; i++) {
        if (targetIndex >= 0 && keyHashColumn[i] == keyTargetColumn[targetIndex])
            duplicates[targetIndex]++;
        else {
            elemNumber++;
            keyTargetColumn[++targetIndex] = keyHashColumn[i];
            mapping[targetIndex] = i;
            duplicates[targetIndex] = 0;

        }
    }

    return elemNumber;
}

void slice_linearizeImpl(Hash compressedColumn[], Hash targetColumn[], size_t low, size_t high, Hash mapping[],
                         Hash duplicates[], size_t factor,
                         size_t level, size_t index, Hash newMapping[], Hash newDuplicates[]) {
    // Call recursive if enough elements available
    if ((high - low) >= 23) {
        // TO BE DONE
        UNUSED(level);
        UNUSED(index);

        // Fill up current node
        size_t i = 0;

        // Black magic
        size_t startIndex = level == 0 ? 0 : (((size_t) pow(factor, level) - 1) + index * (factor - 1));
        int sourceIndex = low - 1;
        size_t targetIndex;
        for (i = 0; i < factor - 1; i++) {
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
                for (j = 1; j < duplicates[sourceIndex]; j++) {
                    // swapHashes(&mapping[sourceIndex + j], &mapping[targetIndex + j]);
                    newMapping[targetIndex + j] = mapping[sourceIndex + j];
                }
                // swapHashes(&duplicates[sourceIndex], &duplicates[targetIndex]);
                newDuplicates[targetIndex] = duplicates[sourceIndex];
            }

        }
        size_t begin = low;
        size_t end = begin + ((high - low) / factor) - 1;
        for (i = 0; i < factor; i++) {
            // begin = low + i * (factor - 1);
            // end = i + (i + 1) * (factor - 1) > high ? high : i + (i + 1) * (factor - 1);
            slice_linearizeImpl(compressedColumn, targetColumn, begin, end,
                                mapping, duplicates, factor, (level + 1), (index * (factor)) + i, newMapping,
                                newDuplicates);

            begin += ((high - low) / factor) + 1;
            end += ((high - low) / factor) + 1;
        }
    } else {
        size_t i = 0;

        // Black magic
        // size_t startIndex = level == 0 ? 0 : 4 + ((size_t)pow(factor, level - 1) + (factor - 1) * index) - 1; //(factor - 1) + ((factor - 1) * (size_t)pow(factor, level - 1)) + (index * factor);
        size_t startIndex = (((size_t) pow(factor, level) - 1)) + (factor - 1) * index;
        size_t sourceIndex, targetIndex;

        for (i = 0; i <= (high - low); i++) {
            targetIndex = startIndex + i;
            sourceIndex = low + i;
            targetColumn[targetIndex] = compressedColumn[sourceIndex];

            // Reorder duplicates  and mapping
            // swapHashes(&mapping[sourceIndex], &mapping[targetIndex]);
            newMapping[targetIndex] = mapping[sourceIndex];
            if (duplicates[sourceIndex] > 0) {
                size_t j;
                for (j = 1; j < duplicates[sourceIndex]; j++) {
                    newMapping[targetIndex + j] = mapping[sourceIndex + j];
                }
                // swapHashes(&duplicates[sourceIndex], &duplicates[targetIndex]);
                newDuplicates[targetIndex] = duplicates[sourceIndex];
            }
        }
    }
}

void slice_linearize(Hash compressedColumn[], Hash targetColumn[], size_t low, size_t high, Hash mapping[],
                     Hash duplicates[], size_t factor, Hash newMapping[], Hash newDuplicates[]) {
    slice_linearizeImpl(compressedColumn, targetColumn, low, high, mapping, duplicates, factor, 0, 0, newMapping,
                        newDuplicates);
}

void fillUpCompressedArray(Hash compressedHashes[], size_t currentLength, size_t maxLength) {
    size_t i;
    size_t maxIndexSub = 0;
    for (i = currentLength; i < maxLength; ++i) {
        compressedHashes[i] = LONG_MAX - i;
        ++maxIndexSub;
    }
}


