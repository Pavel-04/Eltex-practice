#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/wait.h>
#include <fcntl.h>




int main(){


    int sock;
    char buffer[1024];
    if((sock=socket(AF_INET,SOCK_RAW,IPPROTO_UDP))<0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    while(1){


        int packet_size=recvfrom(sock,buffer,sizeof(buffer),0,NULL,NULL);
        if(packet_size<0){
            perror("recfrom failed");
            continue;
        }
        struct iphdr *ip_header = (struct iphdr*)buffer;
        struct udphdr *udp_header = (struct udphdr*)(buffer + sizeof(struct iphdr));
        
        if(ntohs(udp_header->dest) == 12345) {
            int fd=open("dump.dat", O_WRONLY | O_CREAT | O_APPEND,0644);
            if(fd==-1){
                perror("file open");
            }
            else {
                write(fd, buffer, packet_size); 
                close(fd);
            }
            char *data = (char*)(buffer + sizeof(struct iphdr) + sizeof(struct udphdr));
            int data_length = packet_size - sizeof(struct iphdr) - sizeof(struct udphdr);
            char src_ip[INET_ADDRSTRLEN];
            char dest_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(ip_header->saddr), src_ip, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(ip_header->daddr), dest_ip, INET_ADDRSTRLEN);
            printf("IP отправителя: %s\n", src_ip);
            printf("IP получателя: %s\n", dest_ip);
            printf("Порт отправителя: %d\n", ntohs(udp_header->source));
            printf("Порт получателя: %d\n", ntohs(udp_header->dest));
            
            printf("Перехвачено сообщение: %.*s\n", data_length, data);
        }
    }
    
        close(sock);
    }



