#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#define PORT 12345
#define BUFFER_SIZE 1024
#define CLIENT_COUNT 2


int compare_address(struct sockaddr_in *a, struct sockaddr_in *b) {
    return (a->sin_addr.s_addr == b->sin_addr.s_addr && 
            a->sin_port == b->sin_port);
}
int main(){

    int server_fd;
    struct sockaddr_in server_addr,client_addr;
    struct sockaddr_in clients[CLIENT_COUNT];
    socklen_t client_addr_len=sizeof(client_addr);
    int client_count=0;
    char buffer[BUFFER_SIZE];
    if((server_fd=socket(AF_INET,SOCK_DGRAM,0))<0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr,0,sizeof(server_addr));

    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_port=htons(PORT);

    if(bind(server_fd,(const struct sockaddr *)&server_addr,sizeof(server_addr))<0){
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("UDP Server Started on port %d\n",PORT);

    while(1){

        memset(buffer,0,BUFFER_SIZE);
        int recv_len=recvfrom(server_fd,buffer,BUFFER_SIZE,0,(struct sockaddr*)&client_addr,&client_addr_len);

        if(recv_len<0){
            perror("recfrom failed");
            continue;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET,&(client_addr.sin_addr),client_ip,client_addr_len);
        int client_port=ntohs(client_addr.sin_port);
        if(strcmp(buffer,"shutdown")==0){
            for(int i=0;i<client_count;i++){
                if(compare_address(&clients[i],&client_addr)){
                    clients[i] = clients[client_count - 1];
                    memset(&clients[client_count - 1], 0, sizeof(clients[0]));
                    printf("Клиент %s:%d отключился\n",client_ip,client_port);
                    client_count--;
                    break;
                }
            }
            
            continue;
        }
        printf("Получен пакет от %s:%d %s\n",client_ip,client_port,buffer);
        
        int client_found = 0;
        for(int i = 0; i < client_count; i++){
            if(compare_address(&clients[i], &client_addr)){
                client_found = 1;
                break;
            }
        }

        if(client_found){
            if(client_count > 1){
                for(int i = 0; i < client_count; i++){
                    if(!compare_address(&clients[i], &client_addr)){
                        if(sendto(server_fd, buffer, recv_len, 0,(const struct sockaddr *)&clients[i], sizeof(clients[i])) < 0){
                            perror("sendto failed");
                        }
                    }
                }
            }
            else{
                if(client_count==1){
                    char temp[100];
                    strcpy(temp,"Сообщение не доставлено, вы единственный клиент.");
                    if(sendto(server_fd, temp, sizeof(temp), 0,(const struct sockaddr *)&client_addr, sizeof(client_addr)) < 0){
                        perror("sendto failed");
                    }
                    
                }
            }
        }
        else if(client_count < CLIENT_COUNT){
            clients[client_count] = client_addr;
            client_count++;
            printf("Новый клиент подключен! Всего: %d\n", client_count);
        }
        else {
            char error_msg[] = "Ошибка: превышен лимит клиентов!";
            if(sendto(server_fd, error_msg, strlen(error_msg), 0,(const struct sockaddr *)&client_addr, sizeof(client_addr))<0){
                perror("sendto failed");
            }
            printf("Отклонено подключение - лимит %d клиентов\n", CLIENT_COUNT);
}
    }

    close(server_fd);


}


