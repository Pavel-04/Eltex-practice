#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

union semun {
    int val; 
    struct semid_ds *buf; 
    unsigned short *array;  
    struct seminfo *__buf;  
};

void find_min_max(char* line) {
    line[strcspn(line, "\n")] = '\0';
    int max = 0;
    int min = 256;
    char* tmp = strtok(line, " ");
    
    while(tmp != NULL) {
        int number = atoi(tmp);
        if(number > max) max = number;
        if(number < min) min = number;
        tmp = strtok(NULL, " ");
    }
    printf("Consumer PID %d: %s, Max %d, Min %d\n", getpid(), line, max, min);
}

int main(int argc, char* argv[]){
    if (argc != 2) {
        printf("Использование: %s <имя_файла>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    char* filename = argv[1];
    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.temp", filename);
    
    FILE* temp = fopen(filename, "a");
    if (temp == NULL) {
        perror("fopen temp file");
        exit(EXIT_FAILURE);
    }
    fclose(temp);
    
    char line[256];
    key_t key = ftok(filename, 1);
    
    if(key == -1) {
        perror("key failed");
        exit(EXIT_FAILURE);
    }

    int semid = semget(key, 1, 0666);
    if(semid == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }
    
    while(1) {
        struct sembuf p = {0, -1, 0};
        semop(semid, &p, 1);
        
        FILE *f = fopen(filename, "r");
        if (f == NULL) {
            perror("fopen file");
            exit(EXIT_FAILURE);
        }
        
        FILE *temp_f = fopen(temp_filename, "w");
        if (temp_f == NULL) {
            perror("fopen temp file");
            fclose(f);
            exit(EXIT_FAILURE);
        }
        
        int processed = 0;
        while(fgets(line, sizeof(line), f) != NULL) {
            if (line[0] == '#') {
                fputs(line, temp_f);
                continue;
            }
            
            if (!processed) {
                char original_line[256];
                strcpy(original_line, line);
                find_min_max(original_line);
                processed = 1;
                
                fputs("# ", temp_f); 
                fputs(line, temp_f); 
            } else {
                fputs(line, temp_f);
            }
        }
        
        fclose(f);
        fclose(temp_f);
        rename(temp_filename, filename);
        
        struct sembuf v = {0, 1, 0};
        semop(semid, &v, 1);
        if (!processed) {
            sleep(2);
        } else {
            sleep(1);
        }
    }
    
    return 0;
}