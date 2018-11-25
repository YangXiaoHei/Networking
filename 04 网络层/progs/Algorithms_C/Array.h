
#ifndef _DYNAMICARRAY_H_
#define _DYNAMICARRAY_H_

struct Array {
    int capacity;
    int size;
    int *elem;
};

struct Array *  arrayInit();
int             arrayIsFull(struct Array *arr);
int             arrayIsEmpty(struct Array *arr);
int             arrayGetSize(struct Array *arr);
void            arrayRelease(struct Array **arr);
void            arrayDisplay(struct Array *arr);
int             arrayContainsElement(struct Array *arr, int e);
int             arrayGetIndexOfElement(struct Array *arr, int e);
int             arrayGetElementAtIndex(struct Array *arr, int i);
void            arrayAddElement(struct Array *arr, int e);
int             arrayRemoveLastElement(struct Array *arr);
int             arrayRemoveFirstElement(struct Array *arr);
int             arrayRemoveElementAtIndex(struct Array *arr, int i);

#endif