#ifndef CALCULATE_H
#define CALCULATE_H

#include <stdio.h>

#define CALC_SUCCESS 1
#define CALC_ERROR -1
#define CALC_EXIT -2
#define CALC_INVALID_COMMAND -3
#define CALC_INVALID_INPUT -4
#define CALC_DIVISION_BY_ZERO -5

typedef struct {
    char name[20];
    char description[30];
    double(*func)(double, double);
} Command;

double add(double a, double b);
double sub(double a, double b);
double mul(double a, double b);
double divide(double a, double b);

void print_menu(void);
Command* find_command(const char* name);
int calculator(void); 

extern Command commands[];
extern int commands_count;
#endif