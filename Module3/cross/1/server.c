#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFFER_SIZE 4096
#define EXIT_MESSAGE "__EXIT__"
#define MAX_CLIENTS 100

typedef struct {
    uint32_t ip;
    uint16_t port;
    int message_count;
} ClientInfo;

static ClientInfo clients[MAX_CLIENTS];
static int client_count = 0;
static int sock = -1;
static int sock_send = -1;
static int server_port = 0;
static int running = 1;

ClientInfo* find_client(uint32_t ip, uint16_t port) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].ip == ip && clients[i].port == port) {
            return &clients[i];
        }
    }
    return NULL;
}

ClientInfo* add_client(uint32_t ip, uint16_t port) {
    if (client_count >= MAX_CLIENTS) {
        return NULL;
    }
    ClientInfo *c = &clients[client_count];
    c->ip = ip;
    c->port = port;
    c->message_count = 0;
    client_count++;
    printf("Новый клиент: %s:%d\n", inet_ntoa(*(struct in_addr*)&ip), port);
    return c;
}

void remove_client(uint32_t ip, uint16_t port) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].ip == ip && clients[i].port == port) {
            printf("Клиент %s:%d отключился. Сообщений: %d\n",
                   inet_ntoa(*(struct in_addr*)&clients[i].ip),
                   clients[i].port,
                   clients[i].message_count);
            if (i < client_count - 1) {
                memmove(&clients[i], &clients[i+1], (client_count - i - 1) * sizeof(ClientInfo));
            }
            client_count--;
            break;
        }
    }
}

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

int send_raw_udp_packet(int sock_fd,
                       uint32_t source_ip,
                       uint32_t dest_ip,
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
    ip_header->saddr = source_ip;
    ip_header->daddr = dest_ip;
    
    ip_header->check = checksum((unsigned short*)ip_header, ip_header->ihl * 2);
    
    struct udphdr *udp_header = (struct udphdr*)(packet + sizeof(struct iphdr));
    udp_header->source = htons(source_port);
    udp_header->dest = htons(dest_port);
    udp_header->len = htons(sizeof(struct udphdr) + data_len);
    udp_header->check = 0;
    
    char *udp_data = (char*)(packet + sizeof(struct iphdr) + sizeof(struct udphdr));
    memcpy(udp_data, data, data_len);
    
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = dest_ip;
    dest_addr.sin_port = htons(dest_port);
    
    int packet_len = sizeof(struct iphdr) + sizeof(struct udphdr) + data_len;
    ssize_t sent = sendto(sock_fd, packet, packet_len, 0,
                         (const struct sockaddr*)&dest_addr, sizeof(dest_addr));
    
    return (int)sent;
}

void disconnect_all_clients() {
    if (client_count == 0) return;
    
    printf("Отправляю сообщение о завершении всем %d клиентам...\n", client_count);
    
    for (int i = 0; i < client_count; i++) {
        uint32_t any_server_ip = clients[0].ip ? clients[0].ip : htonl(INADDR_LOOPBACK);
        int sent = send_raw_udp_packet(sock_send,any_server_ip,clients[i].ip,server_port,clients[i].port,EXIT_MESSAGE,strlen(EXIT_MESSAGE));
        if (sent > 0) {
            printf("  Отправлено отключение клиенту %s:%d\n",
                   inet_ntoa(*(struct in_addr*)&clients[i].ip), clients[i].port);
        }
    }
}

void server_signal_handler(int sig) {
    printf("\nСервер получил сигнал %d, завершаю работу...\n", sig);
    running = 0;
}

int main(int argc, char* argv[]) {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t clilen = sizeof(client_addr);
    
    if (argc < 2) {
        printf("Использование: %s <port>\n", argv[0]);
        printf("Пример: %s 1111\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    server_port = atoi(argv[1]);
    
    memset(clients, 0, sizeof(clients));
    client_count = 0;
    
    signal(SIGINT, server_signal_handler);
    signal(SIGTERM, server_signal_handler);
    signal(SIGQUIT, server_signal_handler);
    
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        perror("socket creation failed (receive)");
        exit(EXIT_FAILURE);
    }
    
    if ((sock_send = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("socket creation failed (send)");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    int one = 1;
    if (setsockopt(sock_send, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt failed (send)");
        close(sock);
        close(sock_send);
        exit(EXIT_FAILURE);
    }
    
    printf("Сервер запущен на порту %d (PID: %d)\n", server_port, getpid());
    printf("Ожидание сообщений...\n");
    
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        
        int recv_len = recvfrom(sock, buffer, BUFFER_SIZE, 0,
                               (struct sockaddr *) &client_addr, &clilen);
        if (recv_len < 0) {
            if (!running) break;
            perror("recv failed");
            continue;
        }
        
        struct iphdr *ip_header = (struct iphdr*)buffer;
        unsigned int ip_header_len = ip_header->ihl * 4;
        
        if (ip_header->protocol != IPPROTO_UDP) continue;
        if (recv_len < ip_header_len + sizeof(struct udphdr)) continue;
        
        struct udphdr *udp_header = (struct udphdr*)(buffer + ip_header_len);
        if (ntohs(udp_header->dest) != server_port) continue;
        
        char *data = (char*)(buffer + ip_header_len + sizeof(struct udphdr));
        int data_length = recv_len - ip_header_len - sizeof(struct udphdr);
        if (data_length <= 0) continue;
        
        data[data_length] = '\0';
        
        uint32_t client_ip = ip_header->saddr;
        uint16_t client_port = ntohs(udp_header->source);
        uint32_t server_ip = ip_header->daddr;
        
        if (strcmp(data, EXIT_MESSAGE) == 0) {
            printf("Клиент %s:%d отключился\n",
                   inet_ntoa(*(struct in_addr*)&client_ip), client_port);
            remove_client(client_ip, client_port);
            continue;
        }
        
        ClientInfo *client = find_client(client_ip, client_port);
        if (!client) {
            client = add_client(client_ip, client_port);
            if (!client) {
                printf("Достигнут лимит клиентов (%d)\n", MAX_CLIENTS);
                continue;
            }
        }
        
        client->message_count++;
        
        printf("Клиент %s:%d [%d]: %s\n",
               inet_ntoa(*(struct in_addr*)&client_ip),
               client_port,
               client->message_count,
               data);
        
        char response[BUFFER_SIZE];
        int response_len = snprintf(response, sizeof(response), "%s %d", data, client->message_count);
        
        int sent = send_raw_udp_packet(sock_send,server_ip,client_ip,server_port,client_port,response,response_len);
        if (sent > 0) {
            printf("  Ответ отправлен: %s %d (%d байт)\n", data, client->message_count, sent);
        } else {
            printf("  Ошибка отправки ответа\n");
        }
    }
    
    disconnect_all_clients();
    
    printf("Сервер завершает работу...\n");
    printf("\nСтатистика сервера:\n");
    for (int i = 0; i < client_count; i++) {
        printf("Клиент %s:%d - сообщений: %d\n",
               inet_ntoa(*(struct in_addr*)&clients[i].ip),
               clients[i].port,
               clients[i].message_count);
    }
    printf("Всего было подключено клиентов: %d\n", client_count);
    
    if (sock >= 0) close(sock);
    if (sock_send >= 0) close(sock_send);
    
    return 0;
}