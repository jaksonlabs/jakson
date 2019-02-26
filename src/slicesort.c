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

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

void slicesort2(Hash keyHashColumn[], const char * keyColumn[], const char * stringIdColumn[], size_t arraySize) {
    // quickSort(keyHashColumn, 0, arraySize, keyColumn, stringIdColumn);
    bubblesort(keyHashColumn, arraySize, keyColumn, stringIdColumn);
}


