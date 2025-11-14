#ifndef CONTACT_BOOK_H
#define CONTACT_BOOK_H

#define MAX_LENGTH 50
#define MAX_PHONES 10

// Return codes - made more clear
#define SUCCESS 1
#define ERROR -1
#define EMPTY_BOOK -2
#define CONTACT_NOT_FOUND -3
#define MEMORY_ERROR -4
#define INVALID_INPUT -5
#define BOOK_FULL -6
#define DUPLICATE_PID -7

typedef struct Contact {
    char surname[MAX_LENGTH];
    char name[MAX_LENGTH];
    char lastname[MAX_LENGTH];
    char PID[MAX_LENGTH];
    char** phones;
    int phone_count;
} Contact;

typedef struct Node Node;
typedef struct DoubleLinkList DoubleLinkList;

int addContact(DoubleLinkList* list);
void freeContact(Contact* contact);
Node* findContact(DoubleLinkList* list, const char* PID);
int editContact(DoubleLinkList* list);  
int deleteContact(DoubleLinkList* list);  

#endif