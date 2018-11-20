
#include <stdlib.h>
#include <stdio.h>
#include "Array.h"
#include "log.h"
#include <string.h>

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

int arrayGetElementAtIndex(struct Array *arr, int i)
{
    return arr->elem[i];
}

int arrayGetElementIndex(struct Array *arr, int e)
{
    for (int i = 0; i < arr->size; i++)
        if (arr->elem[i] == e)
            return i;
    return -1;
}

int arrayContainsElement(struct Array *arr, int e)
{
    return arrayGetElementIndex(arr, e) != -1;
}

void arrayDisplay(struct Array *arr)
{
    printf("size=%d capacity=%d\n", arr->size, arr->capacity);
    for (int i = 0; i < arr->capacity; i++)
        printf("%-4d", i);
    printf("\n");
    for (int i = 0; i < arr->capacity; i++) 
        if (i < arr->size) 
            printf("%-4d", arr->elem[i]);
        else
            printf("%-4c", '?');
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
    bzero(arr->elem, sizeof(int) * arr->capacity);

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
    if (a->elem != NULL) {
        free(a->elem);
        a->elem = NULL;
    }
    free(a);
    *arr = NULL;
}

void arrayAddElement(struct Array *arr, int e)
{
    if (arrayIsFull(arr)) 
        arrayResize(arr, arr->size * 2);
    arr->elem[arr->size++] = e;
}

int arrayRemoveLastElement(struct Array *arr)
{
    return arrayRemoveElementAtIndex(arr, arr->size - 1);
}

int arrayRemoveElementAtIndex(struct Array *arr, int i)
{
    if (arrayIsEmpty(arr)) {
        LOG("arr is empty!");
        return -1;
    }
    int toRemove = arr->elem[i];
    for (int j = i + 1; j < arr->size; j++) 
        arr->elem[j - 1] = arr->elem[j];

    arr->size--;
    if (arr->size > 1 && arr->size == arr->capacity / 4)
        arrayResize(arr, arr->capacity / 2);

    return toRemove;
}