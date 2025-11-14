#include <stdio.h>
#include <stdlib.h>
#include "calculate.h"
#include <locale.h>

void handleCalcResult(int result) {
    switch (result) {
    case CALC_SUCCESS:
        printf("Операция выполнена успешно!\n");
        break;
    case CALC_EXIT:
        printf("Программа завершена.\n");
        break;
    case CALC_INVALID_COMMAND:
        printf("Ошибка: неизвестная команда!\n");
        break;
    case CALC_INVALID_INPUT:
        printf("Ошибка: неверный ввод данных!\n");
        break;
    case CALC_DIVISION_BY_ZERO:
        printf("Ошибка: деление на ноль!\n");
        break;
    case CALC_LIBRARY_ERROR:
        printf("Ошибка загрузки библиотеки!\n");
        break;
    default:
        printf("Неизвестный код ошибки: %d\n", result);
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    
    int result;
    do {
        result = calculator();
        handleCalcResult(result); 
    } while (result != CALC_EXIT);

    return 0;
}