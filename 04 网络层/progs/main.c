#include <stdio.h>
#include <stdlib.h>
#include "DynamicArray.h"

int main(int argc, char const *argv[])
{
    struct Array *arr = arrayInit();

    int k = 50;
    while (k--) 
        arrayAddElement(arr, rand() % 300);
    
    arrayDisplay(arr);

    k = 45;
    while (k--)
        arrayRemoveElementAtIndex(arr, rand() % (arrayGetSize(arr)));

    arrayDisplay(arr);
    arrayRelease(&arr);
    
    return 0;
}
