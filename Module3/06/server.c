#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#define SERVER_ID 10
#define MAX_SIZE 10

int msqid;
int users[MAX_SIZE] = {0};
int client_count = 0;

typedef struct {
    long mtype;
    char mtext[100];
    int client_PID;
    int temp_client_PID;
} msgbuf;
void cleanup_server(int sig) {
    printf("\nСервер завершает работу...\n");
    msgbuf shutdown_msg;
    shutdown_msg.client_PID = SERVER_ID;
    strcpy(shutdown_msg.mtext, "shutdown");
    
    for(int i = 0; i < MAX_SIZE; i++) {
        if(users[i] != 0) {
            printf("Уведомляем клиента [%d] о завершении работы\n", users[i]);
            shutdown_msg.mtype = users[i];  
            if(msgsnd(msqid, &shutdown_msg, sizeof(msgbuf)-sizeof(long), 0) == -1) {
                perror("msgsnd shutdown failed");
            }
        }
    }
    
    sleep(1);

    if(msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl IPC_RMID failed");
    } else {
        printf("Очередь сообщений удалена\n");
    }
    
    exit(EXIT_SUCCESS);
}
int registerClient(int client_pid){
    for(int i=0;i<MAX_SIZE;i++){
        if(users[i]==client_pid){
            return 0;
        }
    }
    for(int i=0;i<MAX_SIZE;i++){
        if(users[i]==0){
            users[i]=client_pid;
            client_count++;
            return 1;
        }
    }
    
    return -1;
}
int unregisterClient(int client_pid){
    for(int i=0;i<MAX_SIZE;i++){
        if(users[i]==client_pid){
            users[i]=0;
            break;
        }
    }
}
int sendMessage(int client_pid, char msg[100]){
    for(int i=0;i<MAX_SIZE;i++){
        if(users[i]!=0 && users[i]!=client_pid){
            msgbuf message;
            message.mtype = users[i];
            message.client_PID = client_pid;
            char temp[100];
            snprintf(temp,sizeof(temp), "Клиент [%d] отправил сообщение: %s", client_pid, msg);
            strcpy(message.mtext, temp);
            if(msgsnd(msqid, &message, sizeof(msgbuf)-sizeof(long), 0) == -1) {
                perror("msgsnd failed");
            }
        }
    }
    return 0;
}

int main(){
    int statusRegister;
    signal(SIGINT, cleanup_server);
    signal(SIGTERM, cleanup_server); 
    
    msgbuf ms;
    key_t key = ftok("/tmp", 1);
    
    if(key == -1) {
        perror("key failed");
        exit(EXIT_FAILURE);
    }
    
    msqid = msgget(key, 0666 | IPC_CREAT);
    if(msqid == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Сервер запущен. Ожидание сообщений...\n");

    while(1){
        if(msgrcv(msqid, &ms, sizeof(msgbuf) - sizeof(long), SERVER_ID, 0) == -1) {
            perror("msgrcv in receiver failed");
            continue;
        }
        
        if (strcmp(ms.mtext, "register") == 0){
            switch(statusRegister = registerClient(ms.client_PID)){
                case 1:
                {
                    msgbuf registerClient;
                    registerClient.mtype = ms.temp_client_PID;
                    registerClient.client_PID = SERVER_ID;
                    strcpy(registerClient.mtext, "Success: Успешная регистрация");
                    printf("Клиент %d успешно зарегистрировался\n", ms.client_PID);
                    printf("Текущее количество клиентов: %d\n", client_count);
                    if(msgsnd(msqid, &registerClient, sizeof(msgbuf) - sizeof(long), 0) == -1) {
                        perror("msgsnd failed");
                    }
                    break;
                }
                case 0:
                    msgbuf clientExist;
                    clientExist.mtype = ms.temp_client_PID;
                    clientExist.client_PID = SERVER_ID;
                    strcpy(clientExist.mtext, "Error: клиент с таким приоритетом существует");
                    printf("Клиент с %d приоритетом уже существует\n", ms.client_PID);
                    if(msgsnd(msqid, &clientExist, sizeof(msgbuf) - sizeof(long), 0) == -1) {
                        perror("msgsnd failed");
                    }
                    break;
                case -1:
                    msgbuf clientError;
                    clientError.mtype = ms.temp_client_PID;
                    clientError.client_PID = SERVER_ID;
                    strcpy(clientError.mtext, "Error: место закончилось");
                    printf("Ошибка: клиента %d, место закончилось\n", ms.client_PID);
                    if(msgsnd(msqid, &clientError, sizeof(msgbuf) - sizeof(long), 0) == -1) {
                        perror("msgsnd failed");
                    }
                    break;
            }
        }
        else{
            printf("Получено сообщение '%s' от клиента [%d]\n", ms.mtext, ms.client_PID);
            if(strcmp(ms.mtext,"shutdown")==0){
                unregisterClient(ms.client_PID);
                printf("Отключение клиента [%d]\n",ms.client_PID);
                msgbuf unregister;
                unregister.mtype=ms.client_PID;
                unregister.client_PID=SERVER_ID;
                strcpy(unregister.mtext,"shutdown");
                client_count--;
                printf("Текущее количество клиентов: %d\n", client_count);
                if(msgsnd(msqid, &unregister, sizeof(msgbuf) - sizeof(long), 0) == -1) {
                        perror("msgsnd failed");
                    }
            }else{
                sendMessage(ms.client_PID, ms.mtext);
            }
        }
    }

    msgctl(msqid, IPC_RMID, NULL);
}