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
#include <sys/select.h>
#include <errno.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(1);
}

typedef struct {
    char name[20];
    double(*func)(double, double);
} Command;

double add(double a, double b) {
    return a + b;
}

double sub(double a, double b) {
    return a - b;
}

double mul(double a, double b) {
    return a * b;
}

double divide(double a, double b) {
    if (b == 0) {
        return 0;
    }
    return a / b;
}

Command commands[] = {
    {"+", add},
    {"-", sub},
    {"*", mul},
    {"/", divide}
};

typedef struct {
    int sockfd;
    int state;
    double a;
    char operation[20];
    double b;
    char buffer[BUFFER_SIZE];
} ClientState;

int commands_count = sizeof(commands) / sizeof(commands[0]);

Command* find_command(const char* name) {
    for (int i = 0; i < commands_count; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

void printusers(int nclients) {
    if(nclients) {
        printf("%d user on-line\n", nclients);
    }
    else {
        printf("No User on line\n");
    }
}

void send_file(int sockfd, const char* filename) {
    int fd;
    ssize_t n;
    char buffer[1024];

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file for sending");
        return;
    }
    
    char *file_signal = "FILE_START\n";
    write(sockfd, file_signal, strlen(file_signal));
    
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        if (write(sockfd, buffer, n) == -1) {
            perror("Error sending file");
            break;
        }
    }
    
    char *file_end = "FILE_END\n";
    write(sockfd, file_end, strlen(file_end));
    
    close(fd);
}

void create_expression_file(const char* filename, double a, const char* operation, double b) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error creating expression file");
        return;
    }
    
    fprintf(file, "%.2f %s %.2f", a, operation, b);
    fclose(file);
}

void process_expression_file(const char* filename) {
    FILE *file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Error opening file for processing");
        return;
    }
    
    double a, b, result;
    char operation[20];
    char line[256];
    if (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%lf %s %lf", &a, operation, &b) == 3) {
            Command* cmd = find_command(operation);
            
            if (cmd != NULL && cmd->func != NULL) {
                if (strcmp(cmd->name, "/") == 0 && b == 0) {
                    result = 0;
                } else {
                    result = cmd->func(a, b);
                }
                fseek(file, 0, SEEK_END);
                fprintf(file, " = %.2f\n", result);
            } else {
                fseek(file, 0, SEEK_END);
                fprintf(file, " = Error: Unknown operation '%s'\n", operation);
            }
        } else {
            fseek(file, 0, SEEK_END);
            fprintf(file, " = Error: Invalid expression format\n");
        }
    }
    
    fclose(file);
}

void handle_client(ClientState *client) {
    char response[BUFFER_SIZE];
    
    size_t len = strlen(client->buffer);
    if (len > 0 && client->buffer[len-1] == '\n') {
        client->buffer[len-1] = '\0';
    }
    
    printf("Processing client %d, state=%d, buffer='%s'\n", 
           client->sockfd, client->state, client->buffer);
    
    switch (client->state) {
        case 0:
            client->a = atof(client->buffer);
            printf("Client %d: first number = %.2f\n", client->sockfd, client->a);
            
            strcpy(response, "Enter operation (+, -, *, /): ");
            write(client->sockfd, response, strlen(response));
            client->state = 1;
            break;
            
        case 1:
            strncpy(client->operation, client->buffer, sizeof(client->operation)-1);
            client->operation[sizeof(client->operation)-1] = '\0';
            printf("Client %d: operation = %s\n", client->sockfd, client->operation);
            
            strcpy(response, "Enter second number: ");
            write(client->sockfd, response, strlen(response));
            client->state = 2;
            break;
            
        case 2:
            client->b = atof(client->buffer);
            printf("Client %d: second number = %.2f\n", client->sockfd, client->b);
            
            printf("Client %d: processing %.2f %s %.2f\n", 
                   client->sockfd, client->a, client->operation, client->b);
            
            create_expression_file("expression.txt", client->a, client->operation, client->b);
            
            process_expression_file("expression.txt");
            
            printf("Sending result file to client %d...\n", client->sockfd);
            send_file(client->sockfd, "expression.txt");
            printf("Result file sent to client %d\n", client->sockfd);
            
            close(client->sockfd);
            client->sockfd = -1;
            break;
    }
    
    memset(client->buffer, 0, sizeof(client->buffer));
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd;
    int portno;
    socklen_t clilen;
    fd_set readfds, masterfds;
    int maxfd;
    int client_count = 0;
    struct sockaddr_in serv_addr, cli_addr;

    ClientState clients[MAX_CLIENTS];
    
    printf("TCP SERVER\n");
    
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    for(int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sockfd = -1;
        clients[i].state = 0;
        memset(clients[i].buffer, 0, sizeof(clients[i].buffer));
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    
    bzero((char*) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    listen(sockfd, 5);
    
    FD_ZERO(&masterfds);
    FD_SET(sockfd, &masterfds);
    maxfd = sockfd;
    clilen = sizeof(cli_addr);
    
    printf("Server listening on port %d...\n", portno);
    
    while (1) {
        readfds = masterfds;
        
        int select_result = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        
        if (select_result < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("select failed");
            FD_ZERO(&masterfds);
            FD_SET(sockfd, &masterfds);
            maxfd = sockfd;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].sockfd > 0) {
                    FD_SET(clients[i].sockfd, &masterfds);
                    if (clients[i].sockfd > maxfd) {
                        maxfd = clients[i].sockfd;
                    }
                }
            }
            continue;
        }
        if (FD_ISSET(sockfd, &readfds)) {
            newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
            
            if (newsockfd < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                } else {
                    perror("accept error");
                }
            } else {
                flags = fcntl(newsockfd, F_GETFL, 0);
                fcntl(newsockfd, F_SETFL, flags | O_NONBLOCK);
                int client_index = -1;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].sockfd == -1) {
                        client_index = i;
                        break;
                    }
                }

                if (client_index != -1 && client_count < MAX_CLIENTS) {
                    clients[client_index].sockfd = newsockfd;
                    clients[client_index].state = 0;
                    memset(clients[client_index].buffer, 0, sizeof(clients[client_index].buffer));
                    client_count++;

                    FD_SET(newsockfd, &masterfds);
                    
                    if (newsockfd > maxfd) {
                        maxfd = newsockfd;
                    }
                    
                    printf("+ New connection from [%s], socket %d\n", 
                           inet_ntoa(cli_addr.sin_addr), newsockfd);
                    printusers(client_count);
                    
                    char *welcome = "Welcome to calculator server!\nEnter first number: ";
                    write(newsockfd, welcome, strlen(welcome));
                } else {
                    printf("Max clients reached. Rejecting connection.\n");
                    char *reject = "Server is full. Try again later.\n";
                    write(newsockfd, reject, strlen(reject));
                    close(newsockfd);
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].sockfd > 0 && FD_ISSET(clients[i].sockfd, &readfds)) {
                int bytes_recv = recv(clients[i].sockfd, clients[i].buffer, 
                                     sizeof(clients[i].buffer) - 1, 0);
                
                if (bytes_recv > 0) {
                    clients[i].buffer[bytes_recv] = '\0';
                    printf("Received from client %d: %s\n", 
                           clients[i].sockfd, clients[i].buffer);
                    
                    handle_client(&clients[i]);
                    
                    if (clients[i].sockfd == -1) {
                        FD_CLR(clients[i].sockfd, &masterfds);
                        client_count--;
                        
                        maxfd = sockfd;
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (clients[j].sockfd > maxfd) {
                                maxfd = clients[j].sockfd;
                            }
                        }
                        
                        printf("- Client disconnected, remaining: %d\n", client_count);
                    }
                } 
                else if (bytes_recv == 0) {
                    printf("- Client %d disconnected gracefully\n", clients[i].sockfd);
                    
                    FD_CLR(clients[i].sockfd, &masterfds);
                    close(clients[i].sockfd);
                    clients[i].sockfd = -1;
                    client_count--;
                    
                    maxfd = sockfd;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].sockfd > maxfd) {
                            maxfd = clients[j].sockfd;
                        }
                    }
                    
                    printf("Remaining clients: %d\n", client_count);
                } 
                else {
                    if (errno != EWOULDBLOCK && errno != EAGAIN) {
                        perror("recv error");
                        printf("- Client %d disconnected due to error\n", clients[i].sockfd);
                        
                        FD_CLR(clients[i].sockfd, &masterfds);
                        close(clients[i].sockfd);
                        clients[i].sockfd = -1;
                        client_count--;
                        
                        maxfd = sockfd;
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (clients[j].sockfd > maxfd) {
                                maxfd = clients[j].sockfd;
                            }
                        }
                    }
                }
            }
        }
    }
    
    close(sockfd);
    return 0;
}