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
    exit(1);
}

int nclients = 0;

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

int commands_count = sizeof(commands) / sizeof(commands[0]);

Command* find_command(const char* name) {
    for (int i = 0; i < commands_count; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

void printusers() {
    if(nclients) {
        printf("%d user on-line\n", nclients);
    }
    else {
        printf("No User on line\n");
    }
}

void receive_file(int sockfd, const char* filename) {
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

void send_file(int sockfd, const char* filename) {
    int fd;
    ssize_t n;
    char buffer[1024];

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file for sending");
        return;
    }
    
    // Отправляем сигнал начала файла
    char *file_signal = "FILE_START\n";
    write(sockfd, file_signal, strlen(file_signal));
    
    // Отправляем содержимое файла
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        if (write(sockfd, buffer, n) == -1) {
            perror("Error sending file");
            break;
        }
    }
    
    // Отправляем сигнал конца файла
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
    
    // Читаем выражение из файла
    if (fgets(line, sizeof(line), file) != NULL) {
        // Парсим выражение: число операция число
        if (sscanf(line, "%lf %s %lf", &a, operation, &b) == 3) {
            Command* cmd = find_command(operation);
            
            if (cmd != NULL && cmd->func != NULL) {
                if (strcmp(cmd->name, "/") == 0 && b == 0) {
                    result = 0; // Деление на ноль
                } else {
                    result = cmd->func(a, b);
                }
                
                // Перемещаем указатель в конец файла и добавляем результат
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

void dostuff(int sock) {
    int bytes_recv;
    double a, b;
    char operation[20];
    char buff[1024];
    
    printf("Processing client in child process...\n");
    
    // Приветствие
    char *welcome = "Welcome to calculator server!\n";
    write(sock, welcome, strlen(welcome));
    
    // Запрашиваем первое число
    char *msg1 = "Enter first number: ";
    write(sock, msg1, strlen(msg1));
    
    // Получаем первое число
    bytes_recv = read(sock, buff, sizeof(buff)-1);
    if (bytes_recv < 0) error("ERROR reading from socket");
    buff[bytes_recv] = '\0';
    a = atof(buff);
    
    // Запрашиваем операцию
    char *msg2 = "Enter operation (+, -, *, /): ";
    write(sock, msg2, strlen(msg2));
    
    // Получаем операцию
    bytes_recv = read(sock, buff, sizeof(buff)-1);
    if (bytes_recv < 0) error("ERROR reading from socket");
    buff[bytes_recv] = '\0';
    strncpy(operation, buff, sizeof(operation)-1);
    operation[sizeof(operation)-1] = '\0';
    
    // Запрашиваем второе число
    char *msg3 = "Enter second number: ";
    write(sock, msg3, strlen(msg3));
    
    // Получаем второе число
    bytes_recv = read(sock, buff, sizeof(buff)-1);
    if (bytes_recv < 0) error("ERROR reading from socket");
    buff[bytes_recv] = '\0';
    b = atof(buff);
    
    printf("Received expression: %.2f %s %.2f\n", a, operation, b);
    
    // Создаем файл с выражением
    create_expression_file("expression.txt", a, operation, b);
    printf("Expression file created\n");
    
    // Обрабатываем выражение и добавляем результат в файл
    process_expression_file("expression.txt");
    printf("Expression processed and result added to file\n");
    
    // Отправляем обработанный файл обратно клиенту
    printf("Sending result file back to client...\n");
    send_file(sock, "expression.txt");
    printf("Result file sent successfully\n");
    
    nclients--;
    printf("- Client disconnected\n");
    printusers();
    close(sock);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd;
    int portno;
    int pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    printf("TCP SERVER\n");
    
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    
    bzero((char*) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    
    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");
        nclients++;
        
        printf("+ New connection from [%s]\n", inet_ntoa(cli_addr.sin_addr));
        printusers();
        
        pid = fork();
        if (pid < 0) error("ERROR on fork");
        if (pid == 0) {
            close(sockfd);
            dostuff(newsockfd);
            exit(0);
        }
        else close(newsockfd);
    }
    close(sockfd);
    return 0;
}