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
#include <signal.h>

volatile sig_atomic_t stop = 0;

void handle_signal(int sig) {
    stop = 1;
}

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

void find_min_max(const char* line) {
    char line_copy[256];
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    
    char* newline = strchr(line_copy, '\n');
    if (newline) *newline = '\0';
    
    int max = 0;
    int min = 256;
    char* tmp = strtok(line_copy, " ");
    
    while(tmp != NULL) {
        int number = atoi(tmp);
        if(number > max) max = number;
        if(number < min) min = number;
        tmp = strtok(NULL, " ");
    }
    printf("Consumer: Numbers %s, Max %d, Min %d\n", line_copy, max, min);
}

int main(int argc, char* argv[]){
    if (argc != 2) {
        printf("Использование: %s <имя_файла>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    srand(time(NULL));
    char* filename = argv[1];
    char temp_filename[256];
    char line[256];
    
    snprintf(temp_filename, sizeof(temp_filename), "%s.temp", filename);
    
    sem_t *sem = sem_open(filename, O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        while(!stop) {
            if (sem_wait(sem) == -1) {
                if (errno == EINTR) continue;
                perror("sem_wait consumer");
                break;
            }
            
            FILE *f = fopen(filename, "r");
            if (f == NULL) {
                perror("fopen file consumer");
                sem_post(sem);
                break;
            }
            
            FILE *temp_f = fopen(temp_filename, "w");
            if (temp_f == NULL) {
                perror("fopen temp file");
                fclose(f);
                sem_post(sem);
                break;
            }
            
            int processed = 0;
            while(fgets(line, sizeof(line), f) != NULL) {
                if (line[0] == '#') {
                    fputs(line, temp_f);
                    continue;
                }
                
                if (!processed) {
                    find_min_max(line);
                    processed = 1;
                    fputs("# ", temp_f); 
                    fputs(line, temp_f); 
                } else {
                    fputs(line, temp_f);
                }
            }
            
            fclose(f);
            fclose(temp_f);
            
            if (rename(temp_filename, filename) == -1) {
                perror("rename");
            }
            
            sem_post(sem);
            
            if (!processed) {
                sleep(2);
            } else {
                sleep(1);
            }
        }
    } else {
        while(!stop) {
            char data[256];
            randStr(data, sizeof(data));
            printf("Producer: %s\n", data);
            
            if (sem_wait(sem) == -1) {
                if (errno == EINTR) continue;
                perror("sem_wait producer");
                break;
            }
            
            FILE* f = fopen(filename, "a");
            if (f == NULL) {
                perror("fopen file producer");
                sem_post(sem);
                break;
            }
            fputs(data, f);
            fputs("\n", f);
            fclose(f);
            
            sem_post(sem);
            sleep(1);
        }
        
        wait(NULL);
    }
    
    sem_close(sem);
    if (pid != 0) {
        sem_unlink(filename);
    }
    
    return 0;
}
