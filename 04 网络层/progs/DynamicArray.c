
#include "DynamicArray.h"
#include "log.h"
#include <stdlib.h>
#include <stdio.h>

static void arrayResize(struct Array *arr, int newSize) 
{
    int *old = arr->elem;
    if ((arr->elem = malloc(sizeof(int) * newSize)) == NULL)
        return;

    for (int i = 0; i < arr->size; i++) 
        arr->elem[i] = old[i];

    arr->capacity = newSize;
    free(old);
}

void arrayDisplay(struct Array *arr)
{
    for (int i = 0; i < arr->capacity; i++)
        printf("%-4d", i);
    printf("\n");
    for (int i = 0; i < arr->capacity; i++) 
        if (i < arr->size) 
            printf("%-4d", arr->elem[i]);
        else
            printf("%-5c", '?');
    printf("\n");
}

struct Array *arrayInit()
{
    struct Array *arr = NULL;
    if ((arr = malloc(sizeof(struct Array))) == NULL)
        goto err;

    arr->size = 0;
    arr->capacity = 8;
    if ((arr->elem = malloc(sizeof(int) * arr->capacity)) == NULL)
        goto err_1;

    return arr;
err_1:
    free(arr);
err:
    return NULL;
}

int arrayGetSize(struct Array *arr)
{
    return arr->size;
}

int arrayIsFull(struct Array *arr)
{
    return arr->size == arr->capacity;
}

int arrayIsEmpty(struct Array *arr) 
{
    return arr->size == 0;
}

void arrayRelease(struct Array **arr)
{
    struct Array *a = *arr;
    if (a->elem != NULL)
        free(a->elem);
    free(a);
    *arr = NULL;
}

void arrayAddElement(struct Array *arr, int e)
{
    if (arrayIsFull(arr)) 
        arrayResize(arr, arr->size * 2);
    arr->elem[arr->size++] = e;
}

void arrayRemoveElementAtIndex(struct Array *arr, int i)
{
    if (arrayIsEmpty(arr)) {
        LOG("arr is empty!");
        return;
    }

    for (int j = i + 1; j < arr->size; j++) 
        arr->elem[j - 1] = arr->elem[j];

    arr->size--;
    if (arr->size == arr->capacity / 4)
        arrayResize(arr, arr->capacity / 2);
}