#ifndef TREE_H
#define TREE_H

#include "contact_book.h"

typedef struct Node
{
    Contact contact;
    struct Node *left, *right;
} Node;

typedef struct Btree
{
    Node* root;
} Btree;
void freeBtree(Btree* btree);
void freeTreeR(Node* node);
Btree* initBtree();
Node* createNode(const Contact* contact);
int addNode(Btree* btree, Contact newContact);
Node* addNodeR(Node* node, Contact newContact);
void inOrderTraversalR(Node *node);
void printAllContacts(Btree *tree);
void printTreeVertical(Btree *tree);
int editNode(Btree *btree,Node* nodeToDelete, Contact newContact);
int deleteNode(Btree* btree, Node* nodeToDelete);
Node* deleteNodeR(Node* node, Node* nodeToDelete);
Node* findMinNode(Node* node);


#endif