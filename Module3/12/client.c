#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT 12345
#define BUFFER_SIZE 1024


int main(){

    int client_fd;

    if((client_fd=socket(AF_INET,SOCK_DGRAM,0))<0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);


    char buffer[BUFFER_SIZE];

    memset(&server_addr,0,sizeof(server_addr));

    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(PORT);
    inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr);


    pid_t pid;
    switch(pid=fork()){
        case(-1):
            perror("fork failed");
            exit(EXIT_FAILURE);
            close(client_fd);
        case(0):
            while(1){
                memset(buffer, 0, BUFFER_SIZE); 
                int recv_len=recvfrom(client_fd,buffer,BUFFER_SIZE,0,(struct sockaddr*)&sender_addr,&sender_len);
                if(recv_len<0){
                    perror("recvfrom failed");
                    continue;
                }
                buffer[recv_len] = '\0'; 
                char sender_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET,&(sender_addr.sin_addr),sender_ip,INET_ADDRSTRLEN);
                int sender_port=ntohs(sender_addr.sin_port);
                printf("Получено сообщение %s от %s:%d\n",buffer,sender_ip,sender_port);
                if(strstr(buffer,"Ошибка")!=NULL){
                    close(client_fd);
                    kill(getppid(),SIGTERM);
                    exit(EXIT_FAILURE);
                }
            }
        default:
            while(1){
                fgets(buffer,BUFFER_SIZE,stdin);
                buffer[strcspn(buffer,"\n")]=0;
                if(strcmp(buffer,"shutdown")==0){
                    printf("Отключение...\n");
                    if(sendto(client_fd,buffer,strlen(buffer),0,(const struct sockaddr*)&server_addr,sizeof(server_addr))<0){
                        perror("send failed");
                        close(client_fd);
                    }
                    close(client_fd);
                    kill(pid,SIGTERM);
                    wait(NULL);
                    exit(EXIT_SUCCESS);
                }
                if(sendto(client_fd,buffer,strlen(buffer),0,(const struct sockaddr*)&server_addr,sizeof(server_addr))<0){
                    perror("send failed");
                    close(client_fd);
                }
            }


    }
    
    
    

    close(client_fd);



}