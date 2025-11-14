#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "contact_book.h"
#include "double_list.h"

void displayMenu() {
    printf("\n=== Меню управления контактами ===\n");
    printf("1. Добавить контакт\n");
    printf("2. Показать все контакты\n");
    printf("3. Редактировать контакт\n");
    printf("4. Удалить контакт\n");
    printf("0. Выход\n");
    printf("Выберите действие: ");
}


int handleAddContact(DoubleLinkList* list) {
    int result = addContact(list);
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

int handleShowAllContacts(DoubleLinkList* list) {
    if (list == NULL || list->count == 0) {
        printf("Книга контактов пуста.\n");
        return EMPTY_BOOK;
    }

    printf("\n=== Все контакты (%d) ===\n", list->count);
    printList(list);
    return SUCCESS;
}

int handleEditContact(DoubleLinkList* list) {
    if (list == NULL || list->count == 0) {
        printf("Книга контактов пуста.\n");
        return EMPTY_BOOK;
    }

    int result = editContact(list);
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

int handleDeleteContact(DoubleLinkList* list) {
    if (list == NULL || list->count == 0) {
        printf("Книга контактов пуста.\n");
        return EMPTY_BOOK;
    }

    int result = deleteContact(list);
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

    DoubleLinkList* list = initList();
    if (list == NULL) {
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
            handleAddContact(list);
            break;
        case 2:
            handleShowAllContacts(list);
            break;
        case 3:
            handleEditContact(list);
            break;
        case 4:
            handleDeleteContact(list);
            break;
        case 0:
            printf("Выход из программы...\n");
            break;
        default:
            printf("Неверный выбор! Попробуйте снова.\n");
            break;
        }

    } while (choice != 0);

    // Освобождение памяти
    if (list != NULL) {
        freeList(list);
        list = NULL;
    }

    return 0;
}