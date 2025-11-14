#include "tree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <locale.h>

void freeTreeR(Node* node) {
    if (node == NULL) {
        return;
    }
    freeTreeR(node->left);
    freeTreeR(node->right);

    freeContact(&node->contact);
    free(node);
}
void freeBtree(Btree* btree) {
    if (btree == NULL) {
        return;
    }
    freeTreeR(btree->root);
    free(btree);
}
Btree* initBtree(){

    Btree* tree = malloc(sizeof(Btree));
    if (tree != NULL) {
        tree->root = NULL;
    }
    return tree;
}

Node* createNode(const Contact* contact) {
    Node* newNode = malloc(sizeof(Node));
    if (newNode == NULL) {
        return NULL;
    }

    // Инициализируем указатели
    newNode->right = NULL;
    newNode->left = NULL;

    // Копируем основные поля
    strcpy(newNode->contact.surname, contact->surname);
    strcpy(newNode->contact.name, contact->name);
    strcpy(newNode->contact.lastname, contact->lastname);
    strcpy(newNode->contact.PID, contact->PID);
    newNode->contact.phone_count = contact->phone_count;

    // Выделяем память для телефонов
    if (contact->phone_count > 0) {
        newNode->contact.phones = malloc(contact->phone_count * sizeof(char*));
        if (newNode->contact.phones == NULL) {
            free(newNode);
            return NULL;
        }

        // Копируем каждый телефон
        for (int i = 0; i < contact->phone_count; i++) {
            newNode->contact.phones[i] = malloc(MAX_LENGTH * sizeof(char));
            if (newNode->contact.phones[i] == NULL) {
                // Освобождаем уже выделенную память
                for (int j = 0; j < i; j++) {
                    free(newNode->contact.phones[j]);
                }
                free(newNode->contact.phones);
                free(newNode);
                return NULL;
            }
            strcpy(newNode->contact.phones[i], contact->phones[i]);
        }
    }
    else {
        newNode->contact.phones = NULL;
    }

    return newNode;
}

Node* addNodeR(Node* node, Contact newContact){
    if(node==NULL){
        return createNode(&newContact);
    }
    int cmp = strcmp(newContact.surname, node->contact.surname);
    if(cmp <= 0){
        node->left = addNodeR(node->left, newContact);
    }
    else{
        node->right = addNodeR(node->right, newContact);
    }
    
    return node;
}

int addNode(Btree *btree, Contact newContact)
{
    if (btree == NULL)
        return ERROR; 
    
    btree->root = addNodeR(btree->root, newContact);
    return SUCCESS;
}
Node* findMinNode(Node* node) {
    while (node != NULL && node->left != NULL) {
        node = node->left;
    }
    return node;
}

Node* deleteNodeR(Node* node, Node* nodeToDelete) {
    if (node == NULL) {
        return NULL;
    }

    if (node == nodeToDelete) {
        if (node->left == NULL) {
            Node* temp = node->right;
            freeContact(&node->contact);
            free(node);
            return temp;
        } else if (node->right == NULL) {
            Node* temp = node->left;
            freeContact(&node->contact);
            free(node);
            return temp;
        }

        Node* temp = findMinNode(node->right);
        
        freeContact(&node->contact);
        
        strcpy(node->contact.surname, temp->contact.surname);
        strcpy(node->contact.name, temp->contact.name);
        strcpy(node->contact.lastname, temp->contact.lastname);
        strcpy(node->contact.PID, temp->contact.PID);
        node->contact.phone_count = temp->contact.phone_count;
        
        if (temp->contact.phone_count > 0) {
            node->contact.phones = malloc(temp->contact.phone_count * sizeof(char*));
            if (node->contact.phones != NULL) {
                for (int i = 0; i < temp->contact.phone_count; i++) {
                    node->contact.phones[i] = malloc(MAX_LENGTH * sizeof(char));
                    if (node->contact.phones[i] != NULL) {
                        strcpy(node->contact.phones[i], temp->contact.phones[i]);
                    }
                }
            }
        } else {
            node->contact.phones = NULL;
        }
        
        node->right = deleteNodeR(node->right, temp);
        
    } else {
        int cmp = strcmp(nodeToDelete->contact.surname, node->contact.surname);
        if (cmp <= 0) {
            node->left = deleteNodeR(node->left, nodeToDelete);
        } else {
            node->right = deleteNodeR(node->right, nodeToDelete);
        }
    }
    
    return node;
}
int deleteNode(Btree* btree, Node* nodeToDelete){
    if (btree == NULL || btree->root == NULL || nodeToDelete == NULL) {
        return ERROR;
    }
    btree->root = deleteNodeR(btree->root, nodeToDelete);
    return SUCCESS;
}
int editNode(Btree *btree,Node* nodeToDelete, Contact newContact){
    if (btree == NULL || btree->root == NULL) {
        return EMPTY_BOOK;
    }

    int deleteResult = deleteNode(btree, nodeToDelete);
    if (deleteResult != SUCCESS) {
        return deleteResult;
    }

    int addResult = addNode(btree, newContact);
    return addResult;
}
void inOrderTraversalR(Node *node) {
    if (node == NULL) return;
    
    inOrderTraversalR(node->left);
    displayContact(&node->contact);
    inOrderTraversalR(node->right);
}
void printAllContacts(Btree *tree) {
    if (tree == NULL || tree->root == NULL) {
        printf("Книга контактов пуста.\n");
        return;
    }
    
    printf("\n=== Все контакты ===\n");
    inOrderTraversalR(tree->root);
}
void printTreeVerticalR(Node* node, const char* prefix, int isLeft, int isRoot) {
    if (node == NULL) return;
    
    if (isRoot) {
        printf("%s [PID: %s]\n", node->contact.surname, node->contact.PID);
    } else {
        printf("%s%s%s [PID: %s]\n", prefix, isLeft ? "├── " : "└── ", node->contact.surname, node->contact.PID);
    }
    
    char newPrefix[256];
    if (isRoot) {
        strcpy(newPrefix, "");
    } else {
        strcpy(newPrefix, prefix);
        strcat(newPrefix, isLeft ? "│   " : "    ");
    }
    
    if (node->left != NULL || node->right != NULL) {
        if (node->left != NULL) {
            printTreeVerticalR(node->left, newPrefix, 1, 0); 
        }
        if (node->right != NULL) {
            printTreeVerticalR(node->right, newPrefix, 0, 0);
        }
    }
}

void printTreeVertical(Btree* tree) {
    if (tree == NULL || tree->root == NULL) {
        printf("Дерево пусто.\n");
        return;
    }
    
    printf("\n=== Дерево контактов ===\n");
    printTreeVerticalR(tree->root, "", 0, 1);
}