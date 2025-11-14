#define _CRT_SECURE_NO_WARNINGS
#include "contact_book.h"
#include "double_list.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int isPIDUnique(DoubleLinkList* list, const char* pid) {
    Node* current = list->head;
    while (current != NULL) {
        if (strcmp(current->contact.PID, pid) == 0) {
            return 0;  
        }
        current = current->next;
    }
    return 1; 
}

void freeContact(Contact* contact) {
    if (contact == NULL) return;

    if (contact->phones != NULL) {
        for (int i = 0; i < contact->phone_count; i++) {
            if (contact->phones[i] != NULL) {
                free(contact->phones[i]);
            }
        }
        free(contact->phones);
        contact->phones = NULL;
    }
    contact->phone_count = 0;
}

int addContact(DoubleLinkList* list) {
    Contact newContact;
    memset(&newContact, 0, sizeof(Contact));
    newContact.phones = NULL;
    newContact.phone_count = 0;

    printf("--------Добавление нового контакта--------\n");

    printf("Фамилия: ");
    if (fgets(newContact.surname, sizeof(newContact.surname), stdin) == NULL) {
        printf("Ошибка ввода фамилии!\n");
        return INVALID_INPUT;
    }
    newContact.surname[strcspn(newContact.surname, "\n")] = 0;

    printf("Имя: ");
    if (fgets(newContact.name, sizeof(newContact.name), stdin) == NULL) {
        printf("Ошибка ввода имени!\n");
        freeContact(&newContact);
        return INVALID_INPUT;
    }
    newContact.name[strcspn(newContact.name, "\n")] = 0;
    do {
        printf("PID:");
        if (fgets(newContact.PID, MAX_LENGTH, stdin) == NULL) {
            freeContact(&newContact);
            return INVALID_INPUT;
        }
        newContact.PID[strcspn(newContact.PID, "\n")] = 0;
        if (!isPIDUnique(list, newContact.PID)) {
            printf("Ошибка: PID '%s' уже существует! Введите другой PID.\n", newContact.PID);
        }
    } while (!isPIDUnique(list, newContact.PID));

    // Проверка обязательных полей
    if (strlen(newContact.surname) == 0 || strlen(newContact.name) == 0 || strlen(newContact.PID) == 0) {
        printf("Фамилия, имя и PID не должны быть пустыми!\n");
        freeContact(&newContact);
        return INVALID_INPUT;
    }

    printf("Отчество: ");
    if (fgets(newContact.lastname, sizeof(newContact.lastname), stdin) == NULL) {
        printf("Ошибка ввода отчества!\n");
        freeContact(&newContact);
        return INVALID_INPUT;
    }
    newContact.lastname[strcspn(newContact.lastname, "\n")] = 0;

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
            // Выделяем память для нового массива телефонов
            char** temp = realloc(newContact.phones, (phone_index + 1) * sizeof(char*));
            if (temp == NULL) {
                printf("Ошибка выделения памяти для телефонов!\n");
                freeContact(&newContact);
                return MEMORY_ERROR;
            }
            newContact.phones = temp;

            // Выделяем память для нового телефона
            newContact.phones[phone_index] = malloc(MAX_LENGTH * sizeof(char));
            if (newContact.phones[phone_index] == NULL) {
                printf("Ошибка выделения памяти для номера телефона!\n");
                freeContact(&newContact);
                return MEMORY_ERROR;
            }

            strcpy(newContact.phones[phone_index], phone_input);
            phone_index++;
        }
    } while (strlen(phone_input) > 0 && phone_index < MAX_PHONES);

    newContact.phone_count = phone_index;

    int result = addNode(list, newContact);

    freeContact(&newContact);

    if (result == SUCCESS) {
        return SUCCESS;
    }
    else {
        return ERROR;
    }
}

Node* findContact(DoubleLinkList* list, const char* PID) {
    Node* current = list->head;
    while (current != NULL) {
        if (strcmp(current->contact.PID, PID) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void displayContact(const Contact* contact) {
    printf("Фамилия: %s\n", contact->surname);
    printf("Имя: %s\n", contact->name);
    printf("Отчество: %s\n", contact->lastname);
    printf("PID: %s\n", contact->PID);

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

int deleteContact(DoubleLinkList* list) {
    if (list == NULL || list->count == 0) {
        return EMPTY_BOOK;
    }
    char PID[MAX_LENGTH];
    printf("Введите PID контакта для удаления: ");
    if (fgets(PID, MAX_LENGTH, stdin) == NULL) {
        return INVALID_INPUT;
    }
    PID[strcspn(PID, "\n")] = 0;
    Node* current = findContact(list, PID);
    if (current == NULL) {
        return CONTACT_NOT_FOUND;
    }
    return deleteNode(list, current);
}

int editContact(DoubleLinkList* list) {
    if (list == NULL || list->count == 0) {
        printf("Книга контактов пуста!\n");
        return EMPTY_BOOK;
    }

    char PID[MAX_LENGTH];
    printf("Введите PID контакта для изменения: ");
    if (fgets(PID, MAX_LENGTH, stdin) == NULL) {
        printf("Ошибка ввода!\n");
        return INVALID_INPUT;
    }
    PID[strcspn(PID, "\n")] = 0;

    Node* current = findContact(list, PID);
    if (current == NULL) {
        printf("Контакт с PID '%s' не найден!\n", PID);
        return CONTACT_NOT_FOUND;
    }

    printf("Текущие данные контакта\n");
    displayContact(&current->contact);

    Contact newContact;
    memset(&newContact, 0, sizeof(Contact));

    // Копируем основные поля
    strcpy(newContact.surname, current->contact.surname);
    strcpy(newContact.name, current->contact.name);
    strcpy(newContact.lastname, current->contact.lastname);
    strcpy(newContact.PID, current->contact.PID);

    // Копируем телефоны
    newContact.phone_count = current->contact.phone_count;
    if (newContact.phone_count > 0) {
        newContact.phones = malloc(newContact.phone_count * sizeof(char*));
        if (newContact.phones == NULL) {
            printf("Ошибка выделения памяти!\n");
            return MEMORY_ERROR;
        }

        for (int i = 0; i < newContact.phone_count; i++) {
            newContact.phones[i] = malloc(MAX_LENGTH * sizeof(char));
            if (newContact.phones[i] == NULL) {
                printf("Ошибка выделения памяти!\n");
                // Освобождаем уже выделенную память
                for (int j = 0; j < i; j++) {
                    free(newContact.phones[j]);
                }
                free(newContact.phones);
                return MEMORY_ERROR;
            }
            strcpy(newContact.phones[i], current->contact.phones[i]);
        }
    }
    else {
        newContact.phones = NULL;
    }

    char temp[MAX_LENGTH];

    printf("Фамилия [%s]: ", newContact.surname);
    fgets(temp, MAX_LENGTH, stdin);
    temp[strcspn(temp, "\n")] = 0;
    if (strlen(temp) > 0) {
        strcpy(newContact.surname, temp);
    }

    printf("Имя [%s]: ", newContact.name);
    fgets(temp, MAX_LENGTH, stdin);
    temp[strcspn(temp, "\n")] = 0;
    if (strlen(temp) > 0) {
        strcpy(newContact.name, temp);
    }

    printf("Отчество [%s]: ", newContact.lastname);
    fgets(temp, MAX_LENGTH, stdin);
    temp[strcspn(temp, "\n")] = 0;
    if (strlen(temp) > 0) {
        strcpy(newContact.lastname, temp);
    }

    // Редактирование телефонов
    printf("\nРедактирование телефонов:\n");
    printf("Текущие телефоны:\n");
    if (newContact.phone_count == 0) {
        printf("  Нет телефонов\n");
    }
    else {
        for (int i = 0; i < newContact.phone_count; i++) {
            printf("  %d. %s\n", i + 1, newContact.phones[i]);
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
                if (newContact.phone_count == 0) {
                    printf("Нет телефонов для изменения!\n");
                    break;
                }

                int phone_index;
                printf("Введите номер телефона для изменения (1-%d): ", newContact.phone_count);
                fgets(phone_input, sizeof(phone_input), stdin);
                phone_index = atoi(phone_input) - 1;

                if (phone_index >= 0 && phone_index < newContact.phone_count) {
                    printf("Текущий телефон: %s\n", newContact.phones[phone_index]);
                    printf("Новый телефон: ");
                    fgets(temp, MAX_LENGTH, stdin);
                    temp[strcspn(temp, "\n")] = 0;

                    if (strlen(temp) > 0) {
                        strcpy(newContact.phones[phone_index], temp);
                        printf("Телефон изменен!\n");
                    }
                }
                else {
                    printf("Неверный номер телефона!\n");
                }
                break;
            }

            case 2: {
                if (newContact.phone_count >= MAX_PHONES) {
                    printf("Достигнуто максимальное количество телефонов (%d)!\n", MAX_PHONES);
                    break;
                }

                printf("Новый телефон: ");
                fgets(temp, MAX_LENGTH, stdin);
                temp[strcspn(temp, "\n")] = 0;

                if (strlen(temp) > 0) {
                    // Выделяем память для нового массива
                    char** temp_phones = realloc(newContact.phones,
                        (newContact.phone_count + 1) * sizeof(char*));
                    if (temp_phones == NULL) {
                        printf("Ошибка выделения памяти!\n");
                        break;
                    }
                    newContact.phones = temp_phones;

                    // Выделяем память для нового телефона
                    newContact.phones[newContact.phone_count] = malloc(MAX_LENGTH * sizeof(char));
                    if (newContact.phones[newContact.phone_count] == NULL) {
                        printf("Ошибка выделения памяти!\n");
                        break;
                    }

                    strcpy(newContact.phones[newContact.phone_count], temp);
                    newContact.phone_count++;
                    printf("Телефон добавлен!\n");
                }
                break;
            }

            case 3: {
                if (newContact.phone_count == 0) {
                    printf("Нет телефонов для удаления!\n");
                    break;
                }

                int phone_index;
                printf("Введите номер телефона для удаления (1-%d): ", newContact.phone_count);
                fgets(phone_input, sizeof(phone_input), stdin);
                phone_index = atoi(phone_input) - 1;

                if (phone_index >= 0 && phone_index < newContact.phone_count) {
                    // Освобождаем память удаляемого телефона
                    free(newContact.phones[phone_index]);

                    // Сдвигаем оставшиеся телефоны
                    for (int i = phone_index; i < newContact.phone_count - 1; i++) {
                        newContact.phones[i] = newContact.phones[i + 1];
                    }

                    newContact.phone_count--;

                    if (newContact.phone_count > 0) {
                        char** temp_phones = realloc(newContact.phones,
                            newContact.phone_count * sizeof(char*));
                        if (temp_phones != NULL) {
                            newContact.phones = temp_phones;
                        }

                    }
                    else {
                        free(newContact.phones);
                        newContact.phones = NULL;
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
                if (newContact.phone_count == 0) {
                    printf("  Нет телефонов\n");
                }
                else {
                    for (int i = 0; i < newContact.phone_count; i++) {
                        printf("  %d. %s\n", i + 1, newContact.phones[i]);
                    }
                }
            }

        } while (phone_choice != 4);
    }

    int result = editNode(list, current, newContact);
    freeContact(&newContact);

    return result;
}