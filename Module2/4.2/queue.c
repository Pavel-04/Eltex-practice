
#define _CRT_SECURE_NO_WARNINGS
#include "queue.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Queue* initQueue(){
    Queue* queue = malloc(sizeof(Queue));
    if(queue == NULL){
        return NULL;
    }
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

Node* initNode(char* person, int priority){
    Node* newNode = malloc(sizeof(Node));
    if(newNode == NULL) {
        return NULL;
    }
    newNode->next = NULL;
    strncpy(newNode->person, person, 19);
    newNode->person[19] = '\0';
    newNode->priority = priority;
    return newNode;
}

int addNode(Queue* queue, char* person, int priority){
    if(priority < 0 || priority > 255) {
        return QUEUE_INVALID_PRIORITY;
    }
    
    Node* newNode = initNode(person, priority);
    if(newNode == NULL) {
        return QUEUE_MEMORY_ERROR;
    }

    if(queue->head == NULL){
        queue->head = newNode;
        queue->tail = newNode;
    } else {
        Node* current = queue->head;
        Node* prev = NULL;

        while (current != NULL && (current->priority <= newNode->priority)) {
            prev = current;
            current = current->next;
        }

        if (prev == NULL) {
            newNode->next = queue->head;
            queue->head = newNode;
        } else if (current == NULL) {
            queue->tail->next = newNode;
            queue->tail = newNode;
        } else {
            prev->next = newNode;
            newNode->next = current;
        }
    }
    return QUEUE_SUCCESS;
}

void printQueue(const Queue* queue){
    if(queue == NULL || queue->head == NULL){
        printf("Очередь пустая!\n");
        return;
    }

    Node* current = queue->head;
    while(current != NULL){
        printf("Person: %s, Priority: %d\n", current->person, current->priority);
        current = current->next;
    }
}

int deleteFirst(Queue* queue, char* person, int* priority){
    if(queue == NULL || queue->head == NULL){
        return QUEUE_EMPTY;
    }
    
    Node* tmp = queue->head;
    queue->head = tmp->next;
    
    if(person != NULL) {
        strcpy(person, tmp->person);
    }
    if(priority != NULL) {
        *priority = tmp->priority;
    }
    
    if(tmp == queue->tail) {
        queue->tail = NULL;
    }
    
    free(tmp);
    return QUEUE_SUCCESS;
}

int deleteByPriority(Queue* queue, int priority, char* person, int* deletedPriority){
    if(queue == NULL || queue->head == NULL){
        return QUEUE_EMPTY;
    }
    
    Node* current = queue->head;
    Node* prev = NULL;
    
    while(current != NULL && current->priority != priority){
        prev = current;
        current = current->next;
    }
    
    if(current == NULL){
        return QUEUE_NOT_FOUND;
    }
    
    if(prev == NULL){
        queue->head = current->next;
    } else {
        prev->next = current->next;
    }
    
    if(current == queue->tail){
        queue->tail = prev;
    }
    
    if(person != NULL) {
        strcpy(person, current->person);
    }
    if(deletedPriority != NULL) {
        *deletedPriority = current->priority;
    }
    
    free(current);
    return QUEUE_SUCCESS;
}

int deleteHigherPriority(Queue* queue, int priority, char* person, int* deletedPriority){
    if(queue == NULL || queue->head == NULL){
        return QUEUE_EMPTY;
    }

    Node* current = queue->head;
    Node* prev = NULL;
    
    while(current != NULL && priority >= current->priority){
        prev = current;
        current = current->next;
    }
    
    if(current == NULL){
        return QUEUE_NOT_FOUND;
    }
    
    if(prev == NULL){
        queue->head = current->next;
    } else {
        prev->next = current->next;
    }
    
    if(current == queue->tail){
        queue->tail = prev;
    }
    
    if(person != NULL) {
        strcpy(person, current->person);
    }
    if(deletedPriority != NULL) {
        *deletedPriority = current->priority;
    }
    
    free(current);
    return QUEUE_SUCCESS;
}

void freeQueue(Queue* queue){
    if(queue == NULL) return;
    
    Node* current = queue->head;
    while(current != NULL){
        Node* next = current->next;
        free(current);
        current = next;
    }
    free(queue);
}