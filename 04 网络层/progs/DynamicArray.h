
#ifndef _DYNAMICARRAY_H_
#define _DYNAMICARRAY_H_

struct Array {
    int capacity;
    int size;
    int *elem;
};

struct Array *arrayInit();
int arrayIsFull(struct Array *arr);
int arrayIsEmpty(struct Array *arr);
int arrayGetSize(struct Array *arr);
void arrayRelease(struct Array **arr);
void arrayDisplay(struct Array *arr);
void arrayAddElement(struct Array *arr, int e);
void arrayRemoveElementAtIndex(struct Array *arr, int i);

#endif