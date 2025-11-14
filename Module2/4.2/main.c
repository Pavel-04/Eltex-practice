#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include "queue.h"

void generateRandomQueue(Queue* queue, int n) {
    char* names[] = {"Alexey", "Sasha", "Igor", "Kirill", "Danil", "Maria", 
                    "Anna", "Dmitry", "Sergey", "Olga", "Vladimir", "Elena",
                    "Maxim", "Natalia", "Pavel", "Tatiana", "Andrey", "Julia"};
    int namesCount = sizeof(names) / sizeof(names[0]);
    
    Node* current = queue->head;
    while(current != NULL) {
        Node* next = current->next;
        free(current);
        current = next;
    }
    queue->head = NULL;
    queue->tail = NULL;
    
    for(int i = 0; i < n; i++) {
        char* randomName = names[rand() % namesCount];
        int randomPriority = rand() % 256;
        addNode(queue, randomName, randomPriority);
    }
    
    printf("Сгенерирована очередь из %d элементов\n", n);
}

int main() {
    setlocale(LC_ALL, "Russian");
    srand(time(NULL));
    
    Queue* queue = initQueue();
    if(queue == NULL) {
        printf("Ошибка создания очереди!\n");
        return 1;
    }
    
    int choice, priority, n, result;
    char person[20];
    char deletedPerson[20];
    int deletedPriority;
    
    do {
        printf("\n=== МЕНЮ ОЧЕРЕДИ С ПРИОРИТЕТОМ ===\n");
        printf("1. Добавить элемент\n");
        printf("2. Показать очередь\n");
        printf("3. Удалить первый элемент\n");
        printf("4. Удалить элемент по приоритету\n");
        printf("5. Удалить элемент с приоритетом не ниже заданного\n");
        printf("6. Сгенерировать случайную очередь\n");
        printf("7. Очистить очередь\n");
        printf("0. Выход\n");
        printf("Выберите действие: ");
        scanf("%d", &choice);
        
        switch(choice) {
            case 1:
                printf("Введите имя (до 19 символов): ");
                scanf("%19s", person);
                printf("Введите приоритет (0-255): ");
                scanf("%d", &priority);
                result = addNode(queue, person, priority);
                if(result == QUEUE_SUCCESS) {
                    printf("Элемент добавлен\n");
                } else if(result == QUEUE_INVALID_PRIORITY) {
                    printf("Ошибка: приоритет должен быть от 0 до 255\n");
                } else {
                    printf("Ошибка памяти\n");
                }
                break;
                
            case 2:
                printf("\nТекущая очередь:\n");
                printQueue(queue);
                break;
                
            case 3:
                result = deleteFirst(queue, deletedPerson, &deletedPriority);
                if(result == QUEUE_SUCCESS) {
                    printf("Удален: Person: %s, Priority: %d\n", deletedPerson, deletedPriority);
                } else if(result == QUEUE_EMPTY) {
                    printf("Очередь пустая!\n");
                }
                break;
                
            case 4:
                printf("Введите приоритет для удаления: ");
                scanf("%d", &priority);
                result = deleteByPriority(queue, priority, deletedPerson, &deletedPriority);
                if(result == QUEUE_SUCCESS) {
                    printf("Удален: Person: %s, Priority: %d\n", deletedPerson, deletedPriority);
                } else if(result == QUEUE_EMPTY) {
                    printf("Очередь пустая!\n");
                } else if(result == QUEUE_NOT_FOUND) {
                    printf("Элемент с приоритетом %d не найден\n", priority);
                }
                break;
                
            case 5:
                printf("Введите приоритет: ");
                scanf("%d", &priority);
                result = deleteHigherPriority(queue, priority, deletedPerson, &deletedPriority);
                if(result == QUEUE_SUCCESS) {
                    printf("Удален: Person: %s, Priority: %d\n", deletedPerson, deletedPriority);
                } else if(result == QUEUE_EMPTY) {
                    printf("Очередь пустая!\n");
                } else if(result == QUEUE_NOT_FOUND) {
                    printf("Элемент с приоритетом выше %d не найден\n", priority);
                }
                break;
                
            case 6:
                printf("Введите количество элементов для генерации: ");
                scanf("%d", &n);
                if(n > 0) {
                    generateRandomQueue(queue, n);
                } else {
                    printf("Ошибка: количество должно быть положительным\n");
                }
                break;
                
            case 7:
                freeQueue(queue);
                queue = initQueue();
                printf("Очередь очищена\n");
                break;
                
            case 0:
                printf("Выход...\n");
                break;
                
            default:
                printf("Неверный выбор! Попробуйте снова.\n");
        }
        
    } while(choice != 0);
    
    freeQueue(queue);
    return 0;
}