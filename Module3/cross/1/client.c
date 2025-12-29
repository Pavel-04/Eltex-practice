#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>

#define BUFFER_SIZE 1024
#define EXIT_MESSAGE "__EXIT__"

static int sock_send = -1;
static int sock_recv = -1; 
static struct sockaddr_in serv_addr;
static struct in_addr client_ip, server_ip;
static uint16_t source_port;
static pid_t child_pid = 0;  // Сохраняем PID ребёнка

unsigned short checksum(unsigned short *buf, int nwords) {
    unsigned long sum = 0;
    while (nwords > 0) {
        sum += *buf++;
        nwords--;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int send_raw_udp_packet(int sock, 
                       struct sockaddr_in *dest_addr,
                       struct in_addr source_ip,
                       struct in_addr dest_ip,
                       uint16_t source_port,
                       uint16_t dest_port,
                       const char *data,
                       int data_len) {
    
    char packet[BUFFER_SIZE];
    memset(packet, 0, sizeof(packet));
    
    struct iphdr *ip_header = (struct iphdr*)packet;
    ip_header->ihl = 5;
    ip_header->version = 4;
    ip_header->tos = 0;
    ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + data_len);
    ip_header->id = htons(getpid() & 0xFFFF);
    ip_header->frag_off = 0;
    ip_header->ttl = 64;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->check = 0;
    ip_header->saddr = source_ip.s_addr;
    ip_header->daddr = dest_ip.s_addr;
    
    ip_header->check = checksum((unsigned short*)ip_header, ip_header->ihl * 2);
    
    struct udphdr *udp_header = (struct udphdr*)(packet + sizeof(struct iphdr));
    udp_header->source = htons(source_port);
    udp_header->dest = htons(dest_port);
    udp_header->len = htons(sizeof(struct udphdr) + data_len);
    udp_header->check = 0;
    
    char *udp_data = (char*)(packet + sizeof(struct iphdr) + sizeof(struct udphdr));
    memcpy(udp_data, data, data_len);
    
    int packet_len = sizeof(struct iphdr) + sizeof(struct udphdr) + data_len;
    ssize_t sent = sendto(sock, packet, packet_len, 0,
                         (const struct sockaddr*)dest_addr, sizeof(*dest_addr));
    
    return sent;
}

void send_exit_message(void) {
    if (sock_send < 0) return;
    
    printf("\nОтправляю сообщение о выходе серверу...\n");
    send_raw_udp_packet(sock_send, &serv_addr,
                       client_ip, server_ip,
                       source_port, ntohs(serv_addr.sin_port),
                       EXIT_MESSAGE, strlen(EXIT_MESSAGE));
}

void parent_signal_handler(int sig) {
    printf("\nПолучен сигнал %d. Завершаем работу...\n", sig);
    send_exit_message();
    
    if (child_pid > 0) {
        kill(child_pid, SIGTERM);
        waitpid(child_pid, NULL, 0);
    }
    if (sock_send >= 0) close(sock_send);
    exit(0);
}

void child_signal_handler(int sig) {
    if (sock_recv >= 0) close(sock_recv);
    exit(0);
}

int main(int argc, char* argv[]) {
    struct hostent *server;
    int port;
    
    if (argc < 3) {
        printf("Использование: %s <server_ip> <server_port>\n", argv[0]);
        printf("Пример: %s 127.0.0.1 1111\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        printf("ERROR, no such host: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    port = atoi(argv[2]);

    sock_send = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock_send < 0) {
        perror("socket creation failed (send)");
        exit(EXIT_FAILURE);
    }

    int one = 1;
    if (setsockopt(sock_send, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    sock_recv = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock_recv < 0) {
        perror("socket creation failed (receive)");
        close(sock_send);
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    memcpy(&server_ip, server->h_addr_list[0], sizeof(server_ip));
    client_ip.s_addr = inet_addr("127.0.0.1");
    source_port = getpid() & 0xFFFF;

    printf("Клиент IP: %s:%d\n", inet_ntoa(client_ip), source_port);
    printf("Сервер IP: %s:%d\n", inet_ntoa(server_ip), port);
    printf("Для выхода введите 'exit' или нажмите Ctrl+C\n");
    printf("Вводите сообщения:\n");

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(sock_send);
        sock_send = -1;

        signal(SIGINT, child_signal_handler);
        signal(SIGTERM, child_signal_handler);
        signal(SIGQUIT, child_signal_handler);

        char buffer[BUFFER_SIZE];

        while (1) {
            struct sockaddr_in from;
            socklen_t fromlen = sizeof(from);

            int recv_len = recvfrom(sock_recv, buffer, BUFFER_SIZE, 0,
                                   (struct sockaddr*)&from, &fromlen);

            if (recv_len < 0) {
                if (errno == EINTR) continue;
                break;
            }

            struct iphdr *ip = (struct iphdr*)buffer;
            if (ip->protocol != IPPROTO_UDP) continue;

            unsigned int ip_hlen = ip->ihl * 4;
            if (recv_len < ip_hlen + sizeof(struct udphdr)) continue;

            struct udphdr *udp = (struct udphdr*)(buffer + ip_hlen);

            if (ip->saddr != server_ip.s_addr) continue;
            if (ntohs(udp->dest) != source_port) continue;

            char *data = buffer + ip_hlen + sizeof(struct udphdr);
            int data_len = recv_len - ip_hlen - sizeof(struct udphdr);
            if (data_len <= 0) continue;

            data[data_len] = '\0';

            if (strcmp(data, EXIT_MESSAGE) == 0) {
                printf("\n[Сервер завершает работу]\n");
                printf("Клиент завершает работу\n");
                break;
            }

            printf("\n[Ответ от сервера] %s\n", data);
            printf("Вводите сообщения:\n");
            fflush(stdout);
        }

        close(sock_recv);
        if (getppid() != 1) {
            kill(getppid(), SIGTERM);
        }
        exit(0);
    }

    child_pid = pid;
    close(sock_recv);
    sock_recv = -1;

    signal(SIGINT, parent_signal_handler);
    signal(SIGTERM, parent_signal_handler);
    signal(SIGQUIT, parent_signal_handler);

    while (1) {
        char data[BUFFER_SIZE - sizeof(struct iphdr) - sizeof(struct udphdr)];
        if (fgets(data, sizeof(data), stdin) == NULL) {
            send_exit_message();
            break;
        }

        data[strcspn(data, "\n")] = 0;

        if (strcmp(data, "exit") == 0) {
            send_exit_message();
            break;
        }

        int data_len = strlen(data);
        if (data_len == 0) continue;

        ssize_t sent = send_raw_udp_packet(sock_send, &serv_addr,
                                          client_ip, server_ip,
                                          source_port, port,
                                          data, data_len);

        if (sent < 0) {
            perror("sendto failed");
            break;
        }

    }
    kill(child_pid, SIGTERM);
    waitpid(child_pid, NULL, 0);

    if (sock_send >= 0) close(sock_send);

    printf("Клиент завершает работу\n");
    return 0;
}