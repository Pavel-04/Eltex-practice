#include <stdio.h>
#include <stdlib.h>
#include "contact_book.h"
#include <locale.h> 
#define _CRT_SECURE_NO_WARNINGS

void displayMenu() {
    printf("\n=== ТЕЛЕФОННАЯ КНИГА ===\n");
    printf("1. Показать все контакты\n");
    printf("2. Добавить контакт\n");
    printf("3. Редактировать контакт\n");
    printf("4. Удалить контакт\n");
    printf("5. Выход\n");
    printf("Выберите действие: ");
}

void handleResult(int result, const char* operation) {
    switch (result) {
    case SUCCESS:
        printf("Операция '%s' выполнена успешно!\n", operation);
        break;
    case EMPTY_BOOK:
        printf("Ошибка: телефонная книга пуста!\n");
        break;
    case CONTACT_NOT_FOUND:
        printf("Ошибка: контакт не найден!\n");
        break;
    case MEMORY_ERROR:
        printf("Ошибка: недостаточно памяти!\n");
        break;
    case BOOK_FULL:
        printf("Ошибка: телефонная книга заполнена!\n");
        break;
    case FILE_ERROR:
        printf("Ошибка работы с файлом!\n");
        break;
    case ERROR:
        printf("Ошибка выполнения операции '%s'!\n", operation);
        break;
    default:
        printf("Неизвестный код ошибки: %d\n", result);
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    int choice;
    char input[10];
    ContactBook book;
    initContactBook(&book);
    int loadResult = loadFromFile(&book);
    if (loadResult != SUCCESS) {
        printf("Предупреждение: не удалось загрузить данные из файла. Будет создана новая телефонная книга.\n");
    } else {
        printf("Данные успешно загружены из файла. Загружено контактов: %d\n", book.count);
    }
    printf("Добро пожаловать в телефонную книгу!\n");

    while (1) {
        displayMenu();
        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

        int result;
        switch (choice) {
        case 1:
            result = displayContacts(&book);
            if (result != SUCCESS) {
                handleResult(result, "показать контакты");
            }
            break;
        case 2:
            result = addContact(&book);
            handleResult(result, "добавить контакт");
            break;
        case 3:
            result = editContact(&book);
            handleResult(result, "редактировать контакт");
            break;
        case 4:
            result = deleteContact(&book);
            handleResult(result, "удалить контакт");
            break;
        case 5:
            result = safeToFile(&book);
            if (result != SUCCESS) {
                handleResult(result, "сохранение контактов");
            }
            else {
                printf("Данные успешно сохранены. Выход из программы.\n");
            }
            return 0;
        default:
            printf("Неверный выбор! Попробуйте снова.\n");
        }
    }
    return 0;
}