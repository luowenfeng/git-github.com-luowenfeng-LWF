#ifndef __CIRULAR_LIST_H__
#define __CIRULAR_LIST_H__

typedef struct stCircularList       CircularList;
typedef struct stCircularListNode   CircularListNode;

struct stCircularListNode {
    void *data;
    CircularListNode *prev;
    CircularListNode *next;
    unsigned int index;
};

struct stCircularList {
    CircularListNode *nodes;
    void *pool;
    unsigned int count;
    unsigned int element_size;
};

CircularList* CircularList_Create(unsigned int count, unsigned int element_size);
void CircularList_Destroy(CircularList *list);
void CircularList_Reset(CircularList *list);

#endif // __CIRULAR_LIST_H__
