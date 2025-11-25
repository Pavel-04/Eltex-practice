#include <sys/mman.h>
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

#define SHARED_MEMORY "/shared_memory"
#define SEMOPHORE "/semophore"


volatile sig_atomic_t sig_exit=0;

typedef struct {
    char number_str[256];
    int max; 
    int min;
} shared_data_t;


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

void find_min_max(char* line, int* max, int* min) {
    line[strcspn(line, "\n")] = '\0';
    char line_copy[256];
    strcpy(line_copy,line);
    *max = 0;
    *min = 256;
    char* tmp = strtok(line_copy, " ");
    
    while(tmp != NULL) {
        int number = atoi(tmp);
        if(number > *max) *max = number;
        if(number < *min) *min = number;
        tmp = strtok(NULL, " ");
    }
}
void signal_handler(int sig){
    sig_exit=1;

}
void sem_lock(sem_t * sem){
    if (sem_wait(sem) == -1) {
        perror("sem_wait failed");
        exit(EXIT_FAILURE);
    }
}

void sem_unlock(sem_t * sem){
    if (sem_post(sem) == -1) {
        perror("sem_post failed");
        exit(EXIT_FAILURE);
    }
}

int main(){

    srand(time(NULL));
    int count=0;
    signal(SIGINT,signal_handler);

    sem_t* sem = sem_open(SEMOPHORE, O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    int shm=shm_open(SHARED_MEMORY, O_CREAT | O_RDWR, 0644);
    if(shm==-1){
        perror("shm_open failed");
        sem_close(sem);
        sem_unlink(SEMOPHORE);
        exit(EXIT_FAILURE);
    }

    if(ftruncate(shm,sizeof(shared_data_t))==-1){
        perror("truncate failed");
        sem_close(sem);
        sem_unlink(SEMOPHORE);
        shm_unlink(SHARED_MEMORY);
        exit(EXIT_FAILURE);

    }

    shared_data_t* shared_data=mmap(NULL, sizeof(shared_data_t),PROT_READ|PROT_WRITE,MAP_SHARED,shm,0);

    if(shared_data==MAP_FAILED){
        perror("mmap failed");
        sem_close(sem);
        sem_unlink(SEMOPHORE);
        shm_unlink(SHARED_MEMORY);
        exit(EXIT_FAILURE);
    }

    pid_t pid;

    while(sig_exit!=1){
        sem_lock(sem);

        randStr(shared_data->number_str,sizeof(shared_data->number_str));

        sem_unlock(sem);

        switch(pid=fork()){
            case(-1):
                perror("fork failed");
                close(shm);
                sem_close(sem);
                sem_unlink(SEMOPHORE);
                shm_unlink(SHARED_MEMORY);
                exit(EXIT_FAILURE);
            case(0):
                sem_lock(sem);

                find_min_max(shared_data->number_str,&shared_data->max,&shared_data->min);

                sem_unlock(sem);

                exit(EXIT_SUCCESS);

            default:

                wait(NULL);

                sem_lock(sem);

                printf("line %s, max: %d, min %d\n",shared_data->number_str,shared_data->max,shared_data->min);

                count++;

                sem_unlock(sem);
            
                sleep(1);

        }
    }

    printf("\nОбработано данных: %d\n",count);

    munmap(shared_data,sizeof(shared_data_t));
    close(shm);
    sem_close(sem);
    sem_unlink(SEMOPHORE);
    shm_unlink(SHARED_MEMORY);


}
