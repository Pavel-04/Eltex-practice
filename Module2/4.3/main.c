#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "contact_book.h"
#include "tree.h"

void displayMenu() {
    printf("\n=== Меню управления контактами ===\n");
    printf("1. Добавить контакт\n");
    printf("2. Показать дерево контактов\n");
    printf("3. Показать все контакты списком\n");
    printf("4. Редактировать контакт\n");
    printf("5. Удалить контакт\n");
    printf("0. Выход\n");
    printf("Выберите действие: ");
}


int handleAddContact(Btree* bree) {
    int result = addContact(bree);
    switch (result) {
    case SUCCESS:
        printf("Контакт успешно добавлен!\n");
        break;
    case MEMORY_ERROR:
        printf("Ошибка выделения памяти!\n");
        break;
    case INVALID_INPUT:
        printf("Ошибка ввода данных!\n");
        break;
    case ERROR:
    default:
        printf("Ошибка при добавлении контакта!\n");
        break;
    }
    return result;
}


int handleShowAllContacts(Btree* tree) {
    if (tree == NULL || tree->root == NULL) {
        printf("Книга контактов пуста.\n");
        return EMPTY_BOOK;
    }
    
    printAllContacts(tree);
    return SUCCESS;
}

int handleEditContact(Btree* btree) {
    if (btree == NULL) {
        printf("Книга контактов пуста.\n");
        return EMPTY_BOOK;
    }

    int result = editContact(btree);
    switch (result) {
    case SUCCESS:
        printf("Контакт успешно отредактирован!\n");
        break;
    case EMPTY_BOOK:
        printf("Книга контактов пуста!\n");
        break;
    case CONTACT_NOT_FOUND:
        printf("Контакт не найден!\n");
        break;
    case MEMORY_ERROR:
        printf("Ошибка выделения памяти!\n");
        break;
    case INVALID_INPUT:
        printf("Ошибка ввода данных!\n");
        break;
    case ERROR:
    default:
        printf("Ошибка при редактировании контакта!\n");
        break;
    }
    return result;
}

int handleDeleteContact(Btree* btree) {
    if (btree == NULL) {
        printf("Книга контактов пуста.\n");
        return EMPTY_BOOK;
    }

    int result = deleteContact(btree);
    switch (result) {
    case SUCCESS:
        printf("Контакт успешно удален!\n");
        break;
    case EMPTY_BOOK:
        printf("Книга контактов пуста!\n");
        break;
    case CONTACT_NOT_FOUND:
        printf("Контакт не найден!\n");
        break;
    case ERROR:
    default:
        printf("Ошибка при удалении контакта!\n");
        break;
    }
    return result;
}
int main() {
    setlocale(LC_ALL, "Russian");

    Btree *btree= initBtree();
    if (btree == NULL) {
        printf("Ошибка инициализации списка!\n");
        return -1;
    }

    int choice;
    char input[10];

    do {
        displayMenu();

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Ошибка ввода!\n");
            continue;
        }

        choice = atoi(input);

        switch (choice) {
        case 1:
            handleAddContact(btree);
            break;
        case 2:
            printTreeVertical(btree);
            break;
        case 3:
            handleShowAllContacts(btree);
            break;
        case 4:
            handleEditContact(btree);
            break;
        case 5:
            handleDeleteContact(btree);
            break;
        case 0:
            printf("Выход из программы...\n");
            break;
        default:
            printf("Неверный выбор! Попробуйте снова.\n");
            break;
        }

    } while (choice != 0);
    if (btree != NULL) {
        freeBtree(btree);
        btree = NULL;
    }

    return 0;
}