#ifndef CONTACT_BOOK_H
#define CONTACT_BOOK_H

#define MAX_LENGTH 50
#define MAX_PHONES 10

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
typedef struct Btree Btree;

int addContact(Btree* btree);
int isPIDUniqueR(Node* node, const char* pid);
int isPIDUnique(Btree* tree, const char* pid);
void freeContact(Contact* contact);
void displayContact(const Contact* contact);
int editContact(Btree* btree);
int deleteContact(Btree* btree);
Node* findContact(Btree* btree, const char* PID);
Node* findContactR(Node* node, const char* PID);
 

#endif