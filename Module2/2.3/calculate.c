#define _CRT_SECURE_NO_WARNINGS
#include <locale.h> 
#include "calculate.h"
#include <stdlib.h>
#include <string.h>

double add(double a, double b) {
    return a + b;
}

double sub(double a, double b) {
    return a - b;
}

double mul(double a, double b) {
    return a * b;
}

double divide(double a, double b) {
    if (b == 0) {
        return 0;
    }
    return a / b;
}

Command commands[] = {
    {"add","Сложение",add},
    { "sub", "Вычитание", sub },
    { "mul", "Умножение", mul },
    { "div", "Деление", divide },
    { "exit", "Выход", NULL }
};

int commands_count = sizeof(commands) / sizeof(commands[0]);

void print_menu(void) {
    printf("-----Калькулятор-----\n");
    printf("Доступные операции\n");
    for (int i = 0; i < commands_count; i++) {
        printf("%d. %s - %s\n", i + 1, commands[i].name, commands[i].description);
    }
}

Command* find_command(const char* name) {
    for (int i = 0; i < commands_count; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

int calculator() {
    char input[100];
    double a, b, result;

    printf("Добро пожаловать в калькулятор!\n");

    while (1) {
        print_menu();
        printf("Введите команду: ");

        if (scanf("%99s", input) != 1) {
            while (getchar() != '\n');
            return CALC_INVALID_INPUT;
        }

        if (strcmp(input, "exit") == 0) {
            printf("До свидания!\n");
            return CALC_EXIT;
        }

        Command* cmd = find_command(input);
        if (cmd == NULL) {
            return CALC_INVALID_COMMAND;
        }

        printf("Введите первое число: ");
        if (scanf("%lf", &a) != 1) {
            while (getchar() != '\n');
            return CALC_INVALID_INPUT;
        }

        printf("Введите второе число: ");
        if (scanf("%lf", &b) != 1) {
            while (getchar() != '\n');
            return CALC_INVALID_INPUT;
        }

        if (cmd->func != NULL) {
            if (strcmp(cmd->name, "div") == 0 && b == 0) {
                return CALC_DIVISION_BY_ZERO;
            }

            result = cmd->func(a, b);
            printf("Результат: %.2lf\n", result);
        }

        while (getchar() != '\n');
        return CALC_SUCCESS; 
    }

    return CALC_SUCCESS;
}