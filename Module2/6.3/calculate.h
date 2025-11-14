#ifndef CALCULATE_H
#define CALCULATE_H

#include <stdio.h>

#define CALC_SUCCESS 1
#define CALC_EXIT -2
#define CALC_INVALID_COMMAND -3
#define CALC_INVALID_INPUT -4
#define CALC_DIVISION_BY_ZERO -5
#define CALC_LIBRARY_ERROR -6

typedef struct {
    char name[20];
    char description[30];
    double (*func)(double, double);
    void* library_handle;
} Command;

void print_menu(void);
Command* find_command(const char* name);
int calculator(void);
void cleanup_libraries(void);

extern Command commands[];
extern int commands_count;

#endif