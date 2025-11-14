typedef struct Node{
    struct Node* next;
    char person[20];
    int priority;
} Node;

typedef struct Queue {
    Node* head;
    Node* tail;
} Queue;

#define QUEUE_SUCCESS 0
#define QUEUE_EMPTY 1
#define QUEUE_NOT_FOUND 2
#define QUEUE_INVALID_PRIORITY 3
#define QUEUE_MEMORY_ERROR 4

Node* initNode(char* person, int priority);
Queue* initQueue();
int addNode(Queue* queue, char* person, int priority);
void printQueue(const Queue* queue);
void freeQueue(Queue* queue);
int deleteFirst(Queue* queue, char* person, int* priority);
int deleteByPriority(Queue* queue, int priority, char* person, int* deletedPriority);
int deleteHigherPriority(Queue* queue, int priority, char* person, int* deletedPriority);