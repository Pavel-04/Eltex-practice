#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <stdint.h>

#include <sys/types.h>  
#include <sys/stat.h>
typedef struct {
    uint16_t changes;
    uint16_t categories;
    char operation;
} PermissionChange;

PermissionChange parseSymbolicModeEx(const char* str);
uint16_t parseNumericMode(const char* str);
int checkstat(mode_t mode);
void printBinaryRepresentation(uint16_t mask);
uint16_t mychmod(uint16_t current_mode, uint16_t changes, uint16_t categories, char operation);

#endif