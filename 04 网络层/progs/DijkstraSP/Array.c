
#include <stdlib.h>
#include <stdio.h>
#include "Array.h"
#include "log.h"
#include <string.h>

static void arrayResize(struct Array *arr, int newSize) 
{
    int *old = arr->elem;
    if ((arr->elem = malloc(sizeof(int) * newSize)) == NULL) {
        LOG("arrayResize fail : malloc memory error");
        exit(1);
    }
    bzero(arr->elem, sizeof(int) * newSize);

    for (int i = 0; i < arr->size; i++) 
        arr->elem[i] = old[i];

    arr->capacity = newSize;
    free(old);
}

int arrayGetElementAtIndex(struct Array *arr, int i)
{
    if (arr == NULL) {
        LOG("arrayGetElementAtIndex fail: arr == NULL");
        return 0;
    }
    if (i < 0 || i >= arr->size) {
        LOG("arrayGetElementAtIndex fail: index=%d invalid", i);
        return 0;
    }
    return arr->elem[i];
}

int arrayGetIndexOfElement(struct Array *arr, int e)
{
    if (arr == NULL) {
        LOG("arrayGetIndexOfElement fail: arr == NULL");
        return 0;
    }
    for (int i = 0; i < arr->size; i++)
        if (arr->elem[i] == e)
            return i;
    return -1;
}

int arrayContainsElement(struct Array *arr, int e)
{
    if (arr == NULL) {
        LOG("arrayContainsElement fail: arr == NULL");
        return 0;
    }
    return arrayGetIndexOfElement(arr, e) != -1;
}

void arrayDisplay(struct Array *arr)
{
    if (arr == NULL) {
        LOG("arrayDisplay fail: arr == NULL");
        return;
    }
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
    if ((arr = malloc(sizeof(struct Array))) == NULL) {
        LOG("arrayInit fail : malloc error");
        goto err;
    }

    arr->size = 0;
    arr->capacity = 8;
    if ((arr->elem = malloc(sizeof(int) * arr->capacity)) == NULL) {
        LOG("arrayInit fail : malloc error");
        goto err_1;
    }
    bzero(arr->elem, sizeof(int) * arr->capacity);

    return arr;
err_1:
    free(arr);
err:
    return NULL;
}

int arrayGetSize(struct Array *arr)
{
    if (arr == NULL) {
        LOG("arrayGetSize fail: arr == NULL");
        return 0;
    }
    return arr->size;
}

int arrayIsFull(struct Array *arr)
{
    if (arr == NULL) {
        LOG("arrayIsFull fail: arr == NULL");
        return 0;
    }
    return arr->size == arr->capacity;
}

int arrayIsEmpty(struct Array *arr) 
{
    if (arr == NULL) {
        LOG("arrayIsEmpty fail: arr == NULL");
        return 0;
    }
    return arr->size == 0;
}

void arrayRelease(struct Array **arr)
{
    if (arr == NULL || *arr == NULL) {
        LOG("arrayRelease fail: arr == NULL");
        return;
    }
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
    if (arr == NULL) {
        LOG("arrayAddElement fail: arr == NULL");
        return;
    }
    if (arrayIsFull(arr)) 
        arrayResize(arr, arr->size * 2);
    arr->elem[arr->size++] = e;
}

int arrayRemoveLastElement(struct Array *arr)
{
    if (arr == NULL) {
        LOG("arrayRemoveLastElement fail: arr == NULL");
        return 0;
    }
    return arrayRemoveElementAtIndex(arr, arr->size - 1);
}

int arrayRemoveElementAtIndex(struct Array *arr, int i)
{
    if (arr == NULL) {
        LOG("arrayRemoveElementAtIndex fail: arr == NULL");
        return 0;
    }
    if (i < 0 || i >= arr->size) {
        LOG("arrayRemoveElementAtIndex fail: index=%d invalid", i);
        return 0;
    }
    if (arrayIsEmpty(arr)) {
        LOG("arrayRemoveElementAtIndex fail : arr is empty!");
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