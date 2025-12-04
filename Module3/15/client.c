#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

void error(const char *msg) {
    perror(msg);
    exit(0);
}

void write_file(int sockfd, const char* filename) {
    int fd;
    ssize_t n;
    char buffer[1024];
    int file_started = 0;

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Error creating file");
        return;
    }
    
    while ((n = read(sockfd, buffer, sizeof(buffer))) > 0) {
        if (strstr(buffer, "FILE_END") != NULL) {
            break;
        }
        
        if (strstr(buffer, "FILE_START") != NULL) {
            file_started = 1;
            continue;
        }
        
        if (file_started) {
            if (write(fd, buffer, n) == -1) {
                perror("Error writing to file");
                break;
            }
        }
    }
    
    close(fd);
}

void send_response(int sockfd, const char* response) {
    if (send(sockfd, response, strlen(response), 0) < 0) {
        error("ERROR writing to socket");
    }
}

void display_file_content(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file for display");
        return;
    }
    
    char line[256];
    printf("\n=== Calculation Result ===\n");
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }
    fclose(file);
}

int main(int argc, char *argv[])
{
    int my_sock, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buff[1024];
    
    printf("TCP CLIENT\n");
    
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    
    portno = atoi(argv[2]);
    
    // Создание сокета
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0) error("ERROR opening socket");
    
    // Получение информации о сервере
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    
    // Настройка адреса сервера
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    
    // Подключение к серверу
    if (connect(my_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
    printf("Connected to server successfully!\n");
    
    while (1) {
        bzero(buff, sizeof(buff));
        n = recv(my_sock, buff, sizeof(buff)-1, 0);
        if (n < 0) error("ERROR reading from socket");
        if (n == 0) {
            printf("Server closed connection\n");
            break;
        }
        buff[n] = '\0';
        
        if (strstr(buff, "FILE_START") != NULL) {
            printf("Receiving calculation result from server...\n");
            write_file(my_sock, "result.txt");
            
            bzero(buff, sizeof(buff));
            n = recv(my_sock, buff, sizeof(buff)-1, 0);
            if (n > 0) {
                buff[n] = '\0';
            }
            
            display_file_content("result.txt");
            break;
        }
        
        printf("Server: %s", buff);
        printf("You: ");
        bzero(buff, sizeof(buff));
        fgets(buff, sizeof(buff)-1, stdin);

        if (strlen(buff) > 0 && buff[strlen(buff)-1] == '\n')
            buff[strlen(buff)-1] = '\0';
        
        if (strncmp(buff, "quit", 4) == 0) {
            printf("Exit...\n");
            break;
        }

        n = send(my_sock, buff, strlen(buff), 0);
        if (n < 0) error("ERROR writing to socket");
    }
    
    close(my_sock);
    printf("Connection closed\n");
    return 0;
}