#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <locale.h> 
#include <stdlib.h>
#include "contact_book.h"

void initContactBook(ContactBook* book) {
    book->count = 0;
    return;
}

int isPIDUnique(const ContactBook* book, const char* pid) {
    for (int i = 0; i < book->count; i++) {
        if (strcmp(book->contacts[i].PID, pid) == 0) {
            return 0;
        }
    }
    return 1;
}

void freeContact(Contact* contact) {
    for (int i = 0; i < contact->phone_count; i++) {
        free(contact->phones[i]);
    }
    if (contact->phones != NULL) {
        free(contact->phones);
        contact->phones = NULL;
    }
    contact->phone_count = 0;
}

int addContact(ContactBook* book) {
    if (book->count >= MAX_CONTACTS) {
        printf("Телефонная книга заполнена!\n");
        return ERROR;
    }

    Contact* newContact = &book->contacts[book->count];
    newContact->phones = NULL;
    newContact->phone_count = 0;

    printf("--------Добавление нового контакта--------\n");

    printf("Фамилия:");
    if (fgets(newContact->surname, MAX_LENGTH, stdin) == NULL) {
        return ERROR;
    }
    newContact->surname[strcspn(newContact->surname, "\n")] = 0;

    printf("Имя:");
    if (fgets(newContact->name, MAX_LENGTH, stdin) == NULL) {
        return ERROR;
    }
    newContact->name[strcspn(newContact->name, "\n")] = 0;

    do {
        printf("PID:");
        if (fgets(newContact->PID, MAX_LENGTH, stdin) == NULL) {
            return ERROR;
        }
        newContact->PID[strcspn(newContact->PID, "\n")] = 0;
        if (!isPIDUnique(book, newContact->PID)) {
            printf("Ошибка: PID '%s' уже существует! Введите другой PID.\n", newContact->PID);
        }
    } while (!isPIDUnique(book, newContact->PID));

    if (strlen(newContact->surname) == 0 || strlen(newContact->name) == 0 || strlen(newContact->PID) == 0) {
        return ERROR;
    }

    printf("Отчество:");
    if (fgets(newContact->lastname, MAX_LENGTH, stdin) == NULL) {
        return ERROR;
    }
    newContact->lastname[strcspn(newContact->lastname, "\n")] = 0;

    printf("Место работы:");
    if (fgets(newContact->workplace, MAX_LENGTH, stdin) == NULL) {
        return ERROR;
    }
    newContact->workplace[strcspn(newContact->workplace, "\n")] = 0;

    printf("Должность:");
    if (fgets(newContact->post, MAX_LENGTH, stdin) == NULL) {
        return ERROR;
    }
    newContact->post[strcspn(newContact->post, "\n")] = 0;
    printf("Добавление телефонов (для завершения введите пустую строку):\n");
    char phone_input[MAX_LENGTH];
    int phone_index = 0;

    do {
        printf("Телефон %d: ", phone_index + 1);
        if (fgets(phone_input, MAX_LENGTH, stdin) == NULL) {
            printf("Ошибка ввода телефона!\n");
            break;
        }
        phone_input[strcspn(phone_input, "\n")] = 0;

        if (strlen(phone_input) > 0) {
            char** temp = realloc(newContact->phones, (phone_index + 1) * sizeof(char*));
            if (temp == NULL) {
                for (int i = 0; i < phone_index; i++) {
                    free(newContact->phones[i]);
                }
                free(newContact->phones);
                return MEMORY_ERROR;
            }
            newContact->phones = temp;

            newContact->phones[phone_index] = malloc(MAX_LENGTH * sizeof(char));
            if (newContact->phones[phone_index] == NULL) {
                for (int i = 0; i < phone_index; i++) {
                    free(newContact->phones[i]);
                }
                free(newContact->phones);
                return MEMORY_ERROR;
            }

            strcpy(newContact->phones[phone_index], phone_input);
            phone_index++;
        }
    } while (strlen(phone_input) > 0 && phone_index < MAX_PHONES);

    newContact->phone_count = phone_index;
    book->count++;
    return SUCCESS;
}

void displayContact(const Contact* contact) {
    printf("Фамилия: %s\n", contact->surname);
    printf("Имя: %s\n", contact->name);
    printf("Отчество: %s\n", contact->lastname);
    printf("PID: %s\n", contact->PID);
    printf("Место работы: %s\n", contact->workplace);
    printf("Должность: %s\n", contact->post);

    printf("Номера телефонов:\n");
    if (contact->phone_count == 0) {
        printf("  Нет телефонов\n");
    }
    else {
        for (int i = 0; i < contact->phone_count; i++) {
            printf("  %d. %s\n", i + 1, contact->phones[i]);
        }
    }
    printf("\n");
}

int displayContacts(const ContactBook* book) {
    if (book->count == 0) {
        return EMPTY_BOOK;
    }
    for (int i = 0; i < book->count; i++) {
        printf("Контакт #%d\n", i + 1);
        displayContact(&book->contacts[i]);
    }
    return SUCCESS;
}

int findIndexContact(const ContactBook* book, const char* PID) {
    for (int i = 0; i < book->count; i++) {
        if (strcmp(book->contacts[i].PID, PID) == 0) {
            return i;
        }
    }
    return -1;
}

int deleteContact(ContactBook* book) {
    if (book->count == 0) {
        return EMPTY_BOOK;
    }
    char PID[MAX_LENGTH];
    printf("Введите PID контакта для удаления: ");
    if (fgets(PID, MAX_LENGTH, stdin) == NULL) {
        return ERROR;
    }
    PID[strcspn(PID, "\n")] = 0;

    int index = findIndexContact(book, PID);
    if (index == -1) {
        printf("Контакт не найден!\n");
        return CONTACT_NOT_FOUND;
    }

    freeContact(&book->contacts[index]);

    for (int i = index; i < book->count - 1; i++) {
        book->contacts[i] = book->contacts[i + 1];
    }

    book->count--;
    return SUCCESS;
}

int editContact(ContactBook* book) {
    if (book->count == 0) {
        return EMPTY_BOOK;
    }
    char PID[MAX_LENGTH];
    printf("Введите PID контакта для изменения: ");
    if (fgets(PID, MAX_LENGTH, stdin) == NULL) {
        return ERROR;
    }
    PID[strcspn(PID, "\n")] = 0;

    int index = findIndexContact(book, PID);
    if (index == -1) {
        return CONTACT_NOT_FOUND;
    }

    printf("Текущие данные контакта\n");
    displayContact(&book->contacts[index]);

    char temp[MAX_LENGTH];

    printf("Фамилия [%s]: ", book->contacts[index].surname);
    fgets(temp, MAX_LENGTH, stdin);
    temp[strcspn(temp, "\n")] = 0;
    if (strlen(temp) > 0) {
        strcpy(book->contacts[index].surname, temp);
    }

    printf("Имя [%s]: ", book->contacts[index].name);
    fgets(temp, MAX_LENGTH, stdin);
    temp[strcspn(temp, "\n")] = 0;
    if (strlen(temp) > 0) {
        strcpy(book->contacts[index].name, temp);
    }

    do {
        printf("PID [%s]: ", book->contacts[index].PID);
        fgets(temp, MAX_LENGTH, stdin);
        temp[strcspn(temp, "\n")] = 0;
        if (strlen(temp) > 0 && !isPIDUnique(book, temp)) {
            printf("Ошибка: PID '%s' уже существует! Введите другой PID.\n", temp);
        }
        else {
            if (strlen(temp) > 0) {
                strcpy(book->contacts[index].PID, temp);
                break;
            }
            else {
                break;
            }
        }
    } while (strlen(temp) > 0);

    printf("Отчество [%s]: ", book->contacts[index].lastname);
    fgets(temp, MAX_LENGTH, stdin);
    temp[strcspn(temp, "\n")] = 0;
    if (strlen(temp) > 0) {
        strcpy(book->contacts[index].lastname, temp);
    }

    printf("Место работы [%s]: ", book->contacts[index].workplace);
    fgets(temp, MAX_LENGTH, stdin);
    temp[strcspn(temp, "\n")] = 0;
    if (strlen(temp) > 0) {
        strcpy(book->contacts[index].workplace, temp);
    }

    printf("Должность [%s]: ", book->contacts[index].post);
    fgets(temp, MAX_LENGTH, stdin);
    temp[strcspn(temp, "\n")] = 0;
    if (strlen(temp) > 0) {
        strcpy(book->contacts[index].post, temp);
    }

    // Редактирование телефонов
    printf("\nРедактирование телефонов:\n");
    printf("Текущие телефоны:\n");
    if (book->contacts[index].phone_count == 0) {
        printf("  Нет телефонов\n");
    }
    else {
        for (int i = 0; i < book->contacts[index].phone_count; i++) {
            printf("  %d. %s\n", i + 1, book->contacts[index].phones[i]);
        }
    }

    printf("Хотите изменить телефоны? (y/n): ");
    fgets(temp, MAX_LENGTH, stdin);
    temp[strcspn(temp, "\n")] = 0;

    if (strlen(temp) > 0 && (temp[0] == 'y' || temp[0] == 'Y')) {
        int phone_choice;
        char phone_input[10];

        do {
            printf("\n=== РЕДАКТИРОВАНИЕ ТЕЛЕФОНОВ ===\n");
            printf("1. Изменить конкретный телефон\n");
            printf("2. Добавить новый телефон\n");
            printf("3. Удалить телефон\n");
            printf("4. Завершить редактирование телефонов\n");
            printf("Выберите действие: ");

            fgets(phone_input, sizeof(phone_input), stdin);
            phone_choice = atoi(phone_input);

            switch (phone_choice) {
            case 1: {
                // Изменить конкретный телефон
                if (book->contacts[index].phone_count == 0) {
                    printf("Нет телефонов для изменения!\n");
                    break;
                }

                int phone_index;
                printf("Введите номер телефона для изменения (1-%d): ", book->contacts[index].phone_count);
                fgets(phone_input, sizeof(phone_input), stdin);
                phone_index = atoi(phone_input) - 1;

                if (phone_index >= 0 && phone_index < book->contacts[index].phone_count) {
                    printf("Текущий телефон: %s\n", book->contacts[index].phones[phone_index]);
                    printf("Новый телефон: ");
                    fgets(temp, MAX_LENGTH, stdin);
                    temp[strcspn(temp, "\n")] = 0;

                    if (strlen(temp) > 0) {
                        free(book->contacts[index].phones[phone_index]);
                        book->contacts[index].phones[phone_index] = malloc(MAX_LENGTH * sizeof(char));
                        if (book->contacts[index].phones[phone_index] == NULL) {
                            printf("Ошибка выделения памяти!\n");
                            return MEMORY_ERROR;
                        }
                        strcpy(book->contacts[index].phones[phone_index], temp);
                        printf("Телефон изменен!\n");
                    }
                }
                else {
                    printf("Неверный номер телефона!\n");
                }
                break;
            }

            case 2: {
                // Добавить новый телефон
                if (book->contacts[index].phone_count >= MAX_PHONES) {
                    printf("Достигнуто максимальное количество телефонов (%d)!\n", MAX_PHONES);
                    break;
                }

                printf("Новый телефон: ");
                fgets(temp, MAX_LENGTH, stdin);
                temp[strcspn(temp, "\n")] = 0;

                if (strlen(temp) > 0) {
                    char** temp_phones = realloc(book->contacts[index].phones,
                        (book->contacts[index].phone_count + 1) * sizeof(char*));
                    if (temp_phones == NULL) {
                        printf("Ошибка выделения памяти!\n");
                        return MEMORY_ERROR;
                    }
                    book->contacts[index].phones = temp_phones;

                    book->contacts[index].phones[book->contacts[index].phone_count] = malloc(MAX_LENGTH * sizeof(char));
                    if (book->contacts[index].phones[book->contacts[index].phone_count] == NULL) {
                        printf("Ошибка выделения памяти!\n");
                        return MEMORY_ERROR;
                    }

                    strcpy(book->contacts[index].phones[book->contacts[index].phone_count], temp);
                    book->contacts[index].phone_count++;
                    printf("Телефон добавлен!\n");
                }
                break;
            }

            case 3: {
                // Удалить телефон
                if (book->contacts[index].phone_count == 0) {
                    printf("Нет телефонов для удаления!\n");
                    break;
                }

                int phone_index;
                printf("Введите номер телефона для удаления (1-%d): ", book->contacts[index].phone_count);
                fgets(phone_input, sizeof(phone_input), stdin);
                phone_index = atoi(phone_input) - 1;

                if (phone_index >= 0 && phone_index < book->contacts[index].phone_count) {
                    free(book->contacts[index].phones[phone_index]);
                    for (int i = phone_index; i < book->contacts[index].phone_count - 1; i++) {
                        book->contacts[index].phones[i] = book->contacts[index].phones[i + 1];
                    }

                    book->contacts[index].phone_count--;
                    if (book->contacts[index].phone_count > 0) {
                        char** temp_phones = realloc(book->contacts[index].phones,
                            book->contacts[index].phone_count * sizeof(char*));
                        if (temp_phones != NULL) {
                            book->contacts[index].phones = temp_phones;
                        }
                    }
                    else {
                        free(book->contacts[index].phones);
                        book->contacts[index].phones = NULL;
                    }

                    printf("Телефон удален!\n");
                }
                else {
                    printf("Неверный номер телефона!\n");
                }
                break;
            }

            case 4:
                printf("Завершение редактирования телефонов.\n");
                break;

            default:
                printf("Неверный выбор! Попробуйте снова.\n");
            }

            if (phone_choice != 4) {
                printf("\nТекущие телефоны:\n");
                if (book->contacts[index].phone_count == 0) {
                    printf("  Нет телефонов\n");
                }
                else {
                    for (int i = 0; i < book->contacts[index].phone_count; i++) {
                        printf("  %d. %s\n", i + 1, book->contacts[index].phones[i]);
                    }
                }
            }

        } while (phone_choice != 4);
    }

    return SUCCESS;
}