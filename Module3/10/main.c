#include <fcntl.h> 
#include <sys/stat.h>    
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/shm.h>

volatile sig_atomic_t sig_exit=0;

union semun {
 int val; 
 struct semid_ds *buf; 
 unsigned short *array;  
                           
 struct seminfo *__buf;  
};

typedef struct {
    char number_str[256];
    int max; 
    int min;
} shared_data_t;

struct sembuf lock = {0, -1, 0};
struct sembuf unlock[2] = {{0, 0, 0},{0, 1, 0}};

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
void sem_lock(int semid){
    if(semop(semid,&lock,1)==-1){
            perror("semop lock failed");
            exit(EXIT_FAILURE);
        }
}

void sem_unlock(int semid){
    if(semop(semid,unlock,2)==-1){
            perror("semop unlock failed");
            exit(EXIT_FAILURE);
        }
}

int main(){
    srand(time(NULL));
    union semun arg;
    int count=0;
    signal(SIGINT,signal_handler);
    key_t key=ftok("/tmp",1);

    if(key==-1){
        perror("key failed");
        exit(EXIT_FAILURE);
    }

    int semid=semget(key,1,0644 | IPC_CREAT);

    if(semid==-1){
        perror("semget failed");
        exit(EXIT_FAILURE);
    }
    arg.val=1;
    if(semctl(semid,0,SETVAL,arg)==-1){
        perror("semctl init failed");
        exit(EXIT_FAILURE);
    }

    int shmid=shmget(key,sizeof(shared_data_t),0644 | IPC_CREAT);

    if(shmid==-1){
        perror("shmget failed");
        shmctl(shmid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    shared_data_t* shared_data= (shared_data_t*) shmat(shmid,NULL,0);
    if(shared_data==(void*)-1){
        perror("shmat failed");
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }

    pid_t pid;

    while(sig_exit!=1){
        sem_lock(semid);

        randStr(shared_data->number_str,sizeof(shared_data->number_str));

        sem_unlock(semid);

        switch(pid=fork()){
            case(-1):
                perror("fork failed");
                shmctl(shmid, IPC_RMID, NULL);
                semctl(semid, 0, IPC_RMID);
                exit(EXIT_FAILURE);
            case(0):
                sem_lock(semid);

                find_min_max(shared_data->number_str,&shared_data->max,&shared_data->min);

                sem_unlock(semid);

                exit(EXIT_SUCCESS);

            default:

                wait(NULL);

                sem_lock(semid);

                printf("line %s, max: %d, min %d\n",shared_data->number_str,shared_data->max,shared_data->min);

                count++;

                sem_unlock(semid);
            
                sleep(1);

        }
    }

    printf("\nОбработано данных: %d\n",count);
    shmdt(shared_data);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
}