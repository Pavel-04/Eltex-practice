#define _CRT_SECURE_NO_WARNINGS
#include "double_list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void freeList(DoubleLinkList* list) {
    if (list == NULL) return;

    Node* current = list->head;
    while (current != NULL) {
        Node* next = current->next;
        freeContact(&current->contact);
        free(current);
        current = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

Node* createNode(const Contact* contact) {
    Node* newNode = malloc(sizeof(Node));
    if (newNode == NULL) {
        return NULL;
    }

    // Инициализируем указатели
    newNode->next = NULL;
    newNode->prev = NULL;

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

DoubleLinkList* initList() {
    DoubleLinkList* list = malloc(sizeof(DoubleLinkList));
    if (list == NULL) {
        return NULL;
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    return list;
}

int addNode(DoubleLinkList* list, Contact newContact) {
    if (list == NULL) return ERROR;

    // Создаем новый узел
    Node* newNode = createNode(&newContact);
    if (newNode == NULL) {
        printf("Ошибка создания узла!\n");
        return MEMORY_ERROR;
    }

    // Если список пустой
    if (list->head == NULL) {
        list->head = newNode;
        list->tail = newNode;
    }
    else {
        // Поиск места для вставки с сортировкой по фамилии
        Node* current = list->head;
        Node* prev = NULL;

        while (current != NULL && strcmp(newContact.surname, current->contact.surname) > 0) {
            prev = current;
            current = current->next;
        }

        if (prev == NULL) {
            // Вставка в начало
            newNode->next = list->head;
            list->head->prev = newNode;
            list->head = newNode;
        }
        else if (current == NULL) {
            // Вставка в конец
            list->tail->next = newNode;
            newNode->prev = list->tail;
            list->tail = newNode;
        }
        else {
            // Вставка в середину
            prev->next = newNode;
            newNode->prev = prev;
            newNode->next = current;
            current->prev = newNode;
        }
    }

    list->count++;
    return SUCCESS;
}

int deleteNode(DoubleLinkList* list, Node* nodeToDelete) {
    if (list == NULL || nodeToDelete == NULL) {
        return ERROR;
    }
    if (nodeToDelete->prev != NULL) {
        nodeToDelete->prev->next = nodeToDelete->next;
    }
    else {
        list->head = nodeToDelete->next;
    }

    if (nodeToDelete->next != NULL) {
        nodeToDelete->next->prev = nodeToDelete->prev;
    }
    else {
        list->tail = nodeToDelete->prev;
    }
    freeContact(&nodeToDelete->contact);
    free(nodeToDelete);

    list->count--;

    return SUCCESS;
}

int editNode(DoubleLinkList* list, Node* nodeToDelete, Contact newContact) {
    if (list == NULL || list->head == NULL) {
        return EMPTY_BOOK;
    }

    int deleteResult = deleteNode(list, nodeToDelete);
    if (deleteResult != SUCCESS) {
        return deleteResult;
    }

    int addResult = addNode(list, newContact);
    return addResult;
}

void printList(const DoubleLinkList* list) {
    if (list == NULL || list->head == NULL) {
        printf("Список пуст.\n");
        return;
    }

    Node* current = list->head;
    int contactNum = 1;

    while (current != NULL) {
        printf("=== Контакт %d ===\n", contactNum++);
        printf("Фамилия: %s\n", current->contact.surname);
        printf("Имя: %s\n", current->contact.name);
        printf("Отчество: %s\n", current->contact.lastname);
        printf("PID: %s\n", current->contact.PID);

        if (current->contact.phone_count > 0) {
            printf("Телефоны:\n");
            for (int i = 0; i < current->contact.phone_count; i++) {
                printf("    %d. %s\n", i + 1, current->contact.phones[i]);
            }
        }
        else {
            printf("Телефоны: нет\n");
        }
        printf("\n");

        current = current->next;
    }
}