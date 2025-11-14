#include "permissions.h"
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <ctype.h>

PermissionChange parseSymbolicModeEx(const char* str) {
    PermissionChange result = { 0, 0, ' ' };

    if (strlen(str) == 9 && (str[0] == 'r' || str[0] == '-')) {
        for (int i = 0; i < 9; i++) {
            if (str[i] == 'r' && i % 3 == 0) result.changes |= (1 << (8 - i));
            if (str[i] == 'w' && (i - 1) % 3 == 0) result.changes |= (1 << (8 - i));
            if (str[i] == 'x' && (i - 2) % 3 == 0) result.changes |= (1 << (8 - i));
        }
        result.categories = 0b111111111;
        result.operation = '=';
        return result;
    }

    const char* ptr = str;
    while (*ptr) {
        uint16_t category_mask = 0;

        while (*ptr && *ptr != '+' && *ptr != '-' && *ptr != '=') {
            switch (*ptr) {
            case 'u': category_mask |= 0b111000000; break; 
            case 'g': category_mask |= 0b000111000; break; 
            case 'o': category_mask |= 0b000000111; break; 
            case 'a': category_mask |= 0b111111111; break; 
            }
            ptr++;
        }

        if (!*ptr) break;

        char operation = *ptr;
        result.operation = operation;
        ptr++;

        uint16_t perm_mask = 0;
        while (*ptr && *ptr != ',' && *ptr != ' ') {
            switch (*ptr) {
            case 'r': perm_mask |= 0b100100100; break; 
            case 'w': perm_mask |= 0b010010010; break; 
            case 'x': perm_mask |= 0b001001001; break; 
            }
            ptr++;
        }

        switch (operation) {
        case '+':
            result.changes |= (category_mask & perm_mask);
            break;
        case '-':
            result.changes |= (category_mask & perm_mask);
            break;
        case '=':
            result.changes |= (category_mask & perm_mask);
            result.categories |= category_mask;
            break;
        }

        if (*ptr == ',') ptr++;
    }

    return result;
}

uint16_t parseNumericMode(const char* str) {
    if (strlen(str) != 3) {
        return 0;
    }

    for (int i = 0; i < 3; i++) {
        if (str[i] < '0' || str[i] > '7') {
            return 0;
        }
    }

    uint16_t mask = (str[0] - '0') << 6 | 
        (str[1] - '0') << 3 |  
        (str[2] - '0');    
    return mask;
}

int checkstat(mode_t mode) {
    uint16_t mask = 0;

    if (mode & S_IRUSR) mask |= (1 << 8);
    if (mode & S_IWUSR) mask |= (1 << 7);
    if (mode & S_IXUSR) mask |= (1 << 6);

    if (mode & S_IRGRP) mask |= (1 << 5);
    if (mode & S_IWGRP) mask |= (1 << 4);
    if (mode & S_IXGRP) mask |= (1 << 3);

    if (mode & S_IROTH) mask |= (1 << 2);
    if (mode & S_IWOTH) mask |= (1 << 1);
    if (mode & S_IXOTH) mask |= (1 << 0);

    return mask;
}

void printBinaryRepresentation(uint16_t mask) {
    printf("Binary representation ");
    for (int i = 8; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
        if (i % 3 == 0 && i != 0) printf(" ");
    }
    printf("\n");

    printf("Octal representation: %o\n", mask);

    printf("Symbolic representation: ");
    for (int i = 8; i >= 0; i--) {
        int bit = (mask >> i) & 1;
        int pos = 8 - i;

        if (pos % 3 == 0) printf("%c", bit ? 'r' : '-');
        else if ((pos - 1) % 3 == 0) printf("%c", bit ? 'w' : '-');
        else if ((pos - 2) % 3 == 0) printf("%c", bit ? 'x' : '-');
    }
    printf("\n");
}

uint16_t mychmod(uint16_t current_mode, uint16_t changes, uint16_t categories, char operation) {
    uint16_t new_mode = current_mode;

    switch (operation) {
    case '+':
        new_mode |= changes;
        break;
    case '-':

        new_mode &= ~changes;
        break;
    case '=':

        if (categories == 0) {
            categories = 0b111111111;
        }

        new_mode &= ~categories; 
        new_mode |= changes; 
        break;
    }

    return new_mode;
}