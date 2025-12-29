#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#define SOCKET_PATH "/tmp/taxi_system.sock"
#define MAX_EVENTS 10
#define BUFFER_SIZE 256


typedef struct {
    pid_t pid;  
    int task_timer;  
    char status[32];  
    char msg[124];
    char type[64];
} Message;


typedef struct {
    pid_t pid;
    int fd;
    int is_busy;
    time_t busy_until;
    char status[32];
} Driver;

#endif