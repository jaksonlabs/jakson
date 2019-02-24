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



inline static int partition (Hash arr[], int low, int high, const char * keyArr[], const char * stringIdArr[])
{
    Hash pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high- 1; j++)
    {
        if (arr[j] <= pivot)
        {
            i++;
            swapHashes(&arr[i], &arr[j]);
            swapKeys(&keyArr[i], &keyArr[j]);
            swapKeys(&stringIdArr[i], &stringIdArr[j]);
        }
    }

    swapHashes(&arr[i + 1], &arr[high]);
    swapKeys(&keyArr[i + 1], &keyArr[high]);
    swapKeys(&stringIdArr[i + 1], &stringIdArr[high]);
    return (i + 1);
}

/*
 * inline static int partition (Hash arr[], int low, int high, const char * keyArr[], const char * stringIdArr[])
{
    Hash pivot = arr[high];

    int leftPointer = low -1;
    int rightPointer = high;

    while(true) {
        while(arr[++leftPointer] < pivot) {
            //do nothing
        }

        while(rightPointer > 0 && arr[--rightPointer] > pivot) {
            //do nothing
        }

        if(leftPointer >= rightPointer) {
            break;
        } else {
            swapHashes(&arr[leftPointer], &arr[rightPointer]);
            swapKeys(&keyArr[leftPointer], &keyArr[rightPointer]);
            swapKeys(&stringIdArr[leftPointer], &stringIdArr[rightPointer]);
        }
    }

    swapHashes(&arr[leftPointer], &arr[rightPointer]);
    swapKeys(&keyArr[leftPointer], &keyArr[rightPointer]);
    swapKeys(&stringIdArr[leftPointer], &stringIdArr[rightPointer]);

    return leftPointer;
}
 */

inline static void quickSort(Hash arr[], int low, int high, const char * keyArr[],  const char * stringIdArr[])
{
    if (low < high)
    {
        int pi = partition(arr, low, high, keyArr, stringIdArr);
        quickSort(arr, low, pi - 1, keyArr, stringIdArr);
        quickSort(arr, pi + 1, high, keyArr, stringIdArr);
    }
}


// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

void slicesort2(Hash keyHashColumn[], const char * keyColumn[], const char * stringIdColumn[], size_t arraySize) {
    quickSort(keyHashColumn, 0, arraySize, keyColumn, stringIdColumn);
}
