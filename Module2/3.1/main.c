#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "permissions.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <file> <permissions>\n", argv[0]);
        printf("Example: %s file.txt ugo+rx\n", argv[0]);
        printf("Example: %s file.txt 755\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    const char* newpermission = argv[2];
    struct stat st;

    uint16_t bitmask;
    uint16_t categories = 0;
    char last_operation = ' ';

    if (isdigit(newpermission[0])) {
        bitmask = parseNumericMode(newpermission);
        last_operation = '=';
        categories = 0; 
    }
    else {
        PermissionChange result = parseSymbolicModeEx(newpermission);
        bitmask = result.changes;
        categories = result.categories;
        last_operation = result.operation;
    }

    if (stat(filename, &st) == -1) {
        printf("Error: %s\n", strerror(errno));
        return 1;
    }

    uint16_t current_mask = checkstat(st.st_mode);

    printf("Current file permissions:\n");
    printBinaryRepresentation(current_mask);
    printf("\n");

    printf("Requested changes (operation '%c'):\n", last_operation);
    printBinaryRepresentation(bitmask);
    if (categories != 0) {
        printf("Categories mask: ");
        for (int i = 8; i >= 0; i--) {
            printf("%d", (categories >> i) & 1);
            if (i % 3 == 0 && i != 0) printf(" ");
        }
        printf("\n");
    }
    printf("\n");

    uint16_t new_mask = mychmod(current_mask, bitmask, categories, last_operation);

    printf("Result after applying changes:\n");
    printBinaryRepresentation(new_mask);

    return 0;
}