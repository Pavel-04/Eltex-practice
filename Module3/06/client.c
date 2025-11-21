#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#define SERVER_ID 10

typedef struct {
    long mtype;
    char mtext[100];
    int client_PID;
    int temp_client_PID;
} msgbuf;

int getMessage(int msqid, int client_pid, int temp_client_pid){
    msgbuf ms;
    while(1){
        if(msgrcv(msqid, &ms, sizeof(msgbuf) - sizeof(long), temp_client_pid, 0) == -1) {
            perror("msgrcv in receiver failed");
            break;
        }
        
        if(ms.client_PID == SERVER_ID){
            if(strstr(ms.mtext, "Success") != NULL){
                printf("Регистрация успешна!\n");
                temp_client_pid = client_pid; 
            }
            else if(strstr(ms.mtext,"Error")!=NULL){
                printf("Ошибка!\n");
                kill(getppid(),SIGTERM);
                exit(EXIT_FAILURE);
            }else if(strcmp(ms.mtext,"shutdown")==0){
                printf("Отключение...\n");
                kill(getppid(),SIGTERM);
                exit(EXIT_SUCCESS);
            }
        } else {
            printf("%s\n", ms.mtext);
        }
    }
    return 0;
}

int main(int argc, char* argv[]){
    if(argc != 2) {
        printf("Использование: %s <client_pid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    char buffer[100];
    msgbuf ms;
    key_t key = ftok("/tmp", 1);
    if(key == -1){
        perror("key failed");
        exit(EXIT_FAILURE);
    }
    
    int msqid = msgget(key, 0666);
    if(msqid == -1){
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }
    
    int client_pid = atoi(argv[1]);
    int temp_client_pid = getpid();
    
    pid_t pid;
    switch(pid = fork()){
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            getMessage(msqid, client_pid, temp_client_pid);
            exit(EXIT_SUCCESS);
        default:
            ms.mtype = SERVER_ID;
            ms.client_PID = client_pid;
            ms.temp_client_PID = temp_client_pid;
            strcpy(ms.mtext, "register");
            
            if(msgsnd(msqid, &ms, sizeof(msgbuf)-sizeof(long), 0) == -1) {
                perror("msgsnd failed");
            }
            while(1){
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                ms.mtype = SERVER_ID;
                ms.client_PID = client_pid;
                strcpy(ms.mtext, buffer);
                
                if(msgsnd(msqid, &ms, sizeof(msgbuf)-sizeof(long), 0) == -1) {
                    perror("msgsnd failed");
                }
            }
            
            int status;
            waitpid(pid, &status, 0);
    }
}