#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <unistd.h>

union semun {
 int val; 
 struct semid_ds *buf; 
 unsigned short *array;  
                           
 struct seminfo *__buf;  
};


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

int main(int argc, char*argv[]) {
    if (argc != 2) {
        printf("Использование: %s <имя_файла>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));
    union semun arg;
    char* filename=argv[1];
    FILE* temp = fopen(filename, "a");
    if (temp == NULL) {
        perror("fopen temp file");
        exit(EXIT_FAILURE);
    }
    fclose(temp);
    key_t key=ftok(filename,1);

    if(key == -1) {
        perror("key failed");
        exit(EXIT_FAILURE);
    }

    int semid=semget(key,1,0666 | IPC_CREAT);

    if(semid == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }
    arg.val = 1; 
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl init failed");
        exit(EXIT_FAILURE);
    }
    while(1){
        char data[256];
        randStr(data, sizeof(data));
        printf("%s\n", data);
        struct sembuf p = {0,-1, 0};
        semop(semid,&p,1);

        FILE* f=fopen(filename,"a");
        if (f == NULL) {
            perror("fopen file");
            exit(EXIT_FAILURE);
        }
        fputs(data, f);
        fputs("\n", f);
        fclose(f);

        struct sembuf v = {0,1,0};
        semop(semid,&v,1);
        sleep(1);

    }

    return 0;
}