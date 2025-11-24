#include <fcntl.h> 
#include <sys/stat.h>    
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

void randStr(char* data, size_t size) {
    data[0] = '\0';
    char str[20];
    int n = rand() % 8 + 5;
    
    for(int i = 0; i < n; i++) {
        int number = rand() % 256;
        snprintf(str, sizeof(str), "%d", number);
        if (i > 0) {
            strncat(data, " ", size - strlen(data) - 1);
        }
        strncat(data, str, size - strlen(data) - 1);
    }
}
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
    printf("Consumer %s, Max %d, Min %d\n", line, max, min);
}

int main(int argc, char*argv[]){
    char data[256];
    if (argc != 2) {
        printf("Использование: %s <имя_файла>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    srand(time(NULL));
    char* filename=argv[1];
    char temp_filename[256];
    char line[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.temp", filename);
    FILE* f=fopen(filename,"a");
    if (f == NULL) {
        perror("fopen file");
        exit(EXIT_FAILURE);
    }
    sem_t *sem = sem_open(filename, O_CREAT, 0644, 1);
    pid_t pid;

    switch (pid=fork()){
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            while(1) {
            sem_wait(sem);
        
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
        
            sem_post(sem);
            if (!processed) {
                sleep(2);
            } else {
                sleep(1);
            }
        }      
        default:
            while(1){
                char data[256];
                randStr(data, sizeof(data));
                printf("%s\n", data);
                sem_wait(sem);

                FILE* f=fopen(filename,"a");
                if (f == NULL) {
                    perror("fopen file");
                    exit(EXIT_FAILURE);
                }
                fputs(data, f);
                fputs("\n", f);
                fclose(f);
                sem_post(sem);
                sleep(1);

    }

    }
    sem_close(sem);
    sem_unlink(filename);
    
    

    



}