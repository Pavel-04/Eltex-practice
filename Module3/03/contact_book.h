#ifndef CONTACT_BOOK_H
#define CONTACT_BOOK_H

#define MAX_CONTACTS 100
#define MAX_LENGTH 50
#define MAX_PHONES 10
#define FILENAME "contacts.dat"

#define SUCCESS 1
#define ERROR -1
#define EMPTY_BOOK -2
#define CONTACT_NOT_FOUND -3
#define MEMORY_ERROR -4
#define INVALID_INPUT -5
#define BOOK_FULL -6
#define FILE_ERROR -7

typedef struct {
    char surname[MAX_LENGTH];
    char name[MAX_LENGTH];
    char lastname[MAX_LENGTH];
    char PID[MAX_LENGTH];
    char workplace[MAX_LENGTH];
    char post[MAX_LENGTH];
    char phones[MAX_PHONES][MAX_LENGTH];
    int phone_count;
} Contact;

typedef struct {
    Contact contacts[MAX_CONTACTS];
    int count;
} ContactBook;

void initContactBook(ContactBook* book);
int addContact(ContactBook* book);
int deleteContact(ContactBook* book);
void displayContact(const Contact* contact);
int displayContacts(const ContactBook* book);
int editContact(ContactBook* book);
int findIndexContact(const ContactBook* book, const char* PID);
int isPIDUnique(const ContactBook* book, const char* PID);
void freeContact(Contact* contact);
int safeToFile(const ContactBook* book);
int loadFromFile(ContactBook* book);
#endif