#include "contact_book.h"
#ifndef DOUBLE_LIST_H
#define DOUBLE_LIST_H

#define MAX_LENGTH 50
#define MAX_PHONES 10

// Коды возврата
#define SUCCESS 1
#define ERROR -1
#define EMPTY_BOOK -2
#define CONTACT_NOT_FOUND -3
#define MEMORY_ERROR -4
#define INVALID_INPUT -5
#define BOOK_FULL -6
#define DUPLICATE_PID -7

typedef struct Node {
    Contact contact;
    struct Node* next;
    struct Node* prev;
} Node;

typedef struct DoubleLinkList {
    Node* head;
    Node* tail;
    int count;
} DoubleLinkList;

DoubleLinkList* initList();
void freeList(DoubleLinkList* list);
Node* createNode(const Contact* contact);
int addNode(DoubleLinkList* list, Contact newContact);
int deleteNode(DoubleLinkList* list, Node* nodeToDelete);
int editNode(DoubleLinkList* list, Node* nodeToDelete, Contact newContact);
void printList(const DoubleLinkList* list);
void freeContact(Contact* contact);

#endif