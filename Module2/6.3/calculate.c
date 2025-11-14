#define _GNU_SOURCE
#include <locale.h> 
#include <stdio.h>
#include "calculate.h"
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

Command commands[] = {
    {"add", "сложение", NULL, NULL},
    {"sub", "вычитание", NULL, NULL},
    {"mul", "умножение", NULL, NULL},
    {"div", "деление", NULL, NULL},
    {"exit", "выход", NULL, NULL}
};

int commands_count = sizeof(commands) / sizeof(commands[0]);

void print_menu(void) {
    printf("-----Калькулятор-----\n");
    printf("Доступные операции:\n");
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

int load_function_library(Command* cmd) {
    if (cmd->func != NULL) {
        return CALC_SUCCESS; 
    }

    char lib_name[50];
    snprintf(lib_name, sizeof(lib_name), "lib%s.so", cmd->name);
    
    // Загружаем библиотеку
    void* handle = dlopen(lib_name, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Ошибка загрузки библиотеки %s: %s\n", lib_name, dlerror());
        return CALC_LIBRARY_ERROR;
    }
    
    // Загружаем функцию
    char func_name[50];
    snprintf(func_name, sizeof(func_name), "%s_function", cmd->name);
    double (*func_ptr)(double, double) = dlsym(handle, func_name);
    
    char* error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "Ошибка загрузки функции %s: %s\n", func_name, error);
        dlclose(handle);
        return CALC_LIBRARY_ERROR;
    }
    
    cmd->func = func_ptr;
    cmd->library_handle = handle;
    
    return CALC_SUCCESS;
}

void cleanup_libraries(void) {
    for (int i = 0; i < commands_count; i++) {
        if (commands[i].library_handle != NULL) {
            dlclose(commands[i].library_handle);
            commands[i].library_handle = NULL;
            commands[i].func = NULL;
        }
    }
}

int calculator() {
    setlocale(LC_ALL, "Russian");
    
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
            cleanup_libraries();
            return CALC_EXIT;
        }

        Command* cmd = find_command(input);
        if (cmd == NULL) {
            return CALC_INVALID_COMMAND;
        }

        // Загружаем библиотеку с функцией
        int load_result = load_function_library(cmd);
        if (load_result != CALC_SUCCESS) {
            return load_result;
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
            // Проверка деления на ноль
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