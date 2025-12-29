#include "common.h"

#define MAX_EVENT 10
volatile sig_atomic_t running = 1;
int epoll_fd;
int server_fd;
Driver drivers[MAX_EVENT];
int driver_count=0;

Driver* find_driver_pid(pid_t pid){
    for(int i=0;i<driver_count;i++){
        if(drivers[i].pid==pid){
            return &drivers[i];
        }
    }
    return NULL;
}
int send_to_driver(pid_t pid, Message* msg){
    Driver* d = find_driver_pid(pid);
    if(!d){
        printf("Водитель с таким PID:%d не существует\n",pid);
        return -1;
    }
    if(write(d->fd,msg,sizeof(Message))!=sizeof(Message)){
        perror("write driver");
        return -1;
    }
    return 0;
}
void create_driver(){
    pid_t pid=fork();

    if(pid==0){
        char* argv[]={"./driver",NULL};
        if(execv("./driver",argv)==-1){
            perror("execv failed");
            exit(EXIT_FAILURE);
        }
    }
    else if(pid>0){
        printf("Зарегистрирован водитель PID:%d\n", pid);
    }
    else{
        perror("fork");
    }



}
void get_drivers(){

    printf("------Список водителей------\n");
    printf("Всего водителей %d\n",driver_count);
    for(int i=0;i<driver_count;i++){
        Driver *d=&drivers[i];
        printf("%d. PID=%d FD=%d status=%s\n",i+1,d->pid,d->fd,d->status);
    }
}

void send_task(pid_t pid, int timer){
    Message msg;
    memset(&msg,0,sizeof(Message));
    msg.pid=pid;
    msg.task_timer=timer;
    strcpy(msg.type,"TASK");
    if (send_to_driver(pid, &msg) == -1) {
       printf("Ошибка отправки\n");
    }

}
void get_status(pid_t pid){
    Message msg;
    memset(&msg,0,sizeof(Message));
    msg.pid=pid;
    strcpy(msg.type,"STATUS");
    if ((send_to_driver(pid,&msg))==-1)
    {
        printf("Ошибка отправки\n");
    }
    

}
void exit_taxi(){
    printf("Отключаю водителей...\n");
    Message msg;
    memset(&msg,0,sizeof(Message));
    strcpy(msg.type,"EXIT");
    for(int i=0;i<driver_count;i++){
        if ((send_to_driver(drivers[i].pid,&msg))==-1)
        {
            printf("Ошибка отправки\n");
        }
        usleep(10000);
    }



}
void handle_new_connection(){
    struct sockaddr_un addr;
    socklen_t addr_len = sizeof(addr);
    
    int client_fd = accept(server_fd, (struct sockaddr*)&addr, &addr_len);
    if (client_fd == -1) {
        perror("accept");
        return;
    }


    struct epoll_event ev;
    ev.events=EPOLLIN;
    ev.data.fd=client_fd;

    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_fd,&ev)==-1){
        perror("epoll_ctl");
        close(client_fd);
        return;
    }
}
void handle_driver_message(int fd){
    Message msg;
    
    ssize_t bytes_read=read(fd,&msg,sizeof(Message));

    if (bytes_read == -1) {
        perror("read");
        close(fd);
        return;
    }
    if (bytes_read == 0) {
        printf("Водитель отключился (fd=%d)\n", fd);
        
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
        
        close(fd);
        
        for (int i = 0; i < driver_count; i++) {
            if (drivers[i].fd == fd) {
                printf("Удаляем водителя PID=%d из списка\n", drivers[i].pid);
                for (int j = i; j < driver_count - 1; j++) {
                    drivers[j] = drivers[j + 1];
                }
                driver_count--;
                break;
            }
        }
        return;
    }
    if (bytes_read != sizeof(Message)) {
        printf("Неполное сообщение: %zd из %lu байт\n", bytes_read, sizeof(Message));
        return;
    }

    if(strcmp(msg.type,"CREATE")==0){
        if(driver_count<MAX_EVENT){
            drivers[driver_count].fd=fd;
            drivers[driver_count].pid=msg.pid;
            drivers[driver_count].is_busy=0;
            strcpy(drivers[driver_count].status,msg.status);
            driver_count++;
                
        }
        else{
            printf("Нет места!\n");
            close(fd);
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
            }
    } else if(strcmp(msg.type,"RESPONSE")==0){
        printf("Ответ от Driver [%d] %s\n",msg.pid,msg.msg);
        Driver * d=find_driver_pid(msg.pid);
        strcpy(d->status,msg.status);
    }else if(strcmp(msg.type,"STATUS")==0){
        printf("Ответ от Driver [%d]: %s\n",msg.pid,msg.msg);
    }else if(strcmp(msg.type,"ERROR")==0){
        printf("Ответ от Driver [%d]: %s\n",msg.pid,msg.msg);
    }
}
void print_help() {
    printf("\n-----Taxi menu-----\n");
    printf("1. create_driver\n");
    printf("2. send_task <pid> <sec>\n");
    printf("3. get_status <pid> \n");
    printf("4. get_drivers\n");
    printf("5. help\n");
    printf("6. exit\n");
}
void cli_menu(){

    pid_t pid;
    int timer;
    char command[64];

    fgets(command,sizeof(command),stdin);
    command[strcspn(command,"\n")]=0;

    if (strcmp(command, "create_driver") == 0) {
        create_driver();
    } else if (strcmp(command, "get_drivers") == 0) {
        get_drivers();
    } else if (strcmp(command, "exit") == 0) {
        exit_taxi();
        running=0;
    } else if (sscanf(command, "send_task %d %d", &pid, &timer) == 2) {
        send_task(pid, timer);
    }else if (strcmp(command, "help") == 0) {
        print_help();
    }else if (sscanf(command, "get_status %d", &pid) == 1) {
        get_status(pid);
    } else {
        printf("Неизвестная комманда %s\n", command);
        printf("Используйте: create_driver, send_task, get_status, get_drivers, exit\n");
    }
    
}
void cleanup(){
    for(int i = 0; i < driver_count; i++) {
        if(drivers[i].fd > 0) {
            close(drivers[i].fd);
        }
    }
    if(epoll_fd > 0) {
        close(epoll_fd);
    }
    if(server_fd > 0) {
        close(server_fd);
    }
    unlink(SOCKET_PATH);
    printf("Выход...\n");
}
void init_server(){

    struct sockaddr_un addr;
    struct epoll_event ev;

    server_fd=socket(AF_UNIX,SOCK_STREAM,0);
    if(server_fd==-1){
        perror("socket create failed");
        exit(EXIT_FAILURE);
    }
    unlink(SOCKET_PATH);

    memset(&addr,0,sizeof(addr));
    addr.sun_family=AF_UNIX;
    strncpy(addr.sun_path,SOCKET_PATH,sizeof(addr.sun_path)-1);

    if(bind(server_fd,(struct sockaddr*)&addr,sizeof(addr))==-1){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd,5)==-1){
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listen on %s\n",SOCKET_PATH);

    epoll_fd=epoll_create1(0);
    if(epoll_fd==-1){
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events=EPOLLIN;
    ev.data.fd=server_fd;

    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_fd,&ev)==-1){
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

}
int main(){

    init_server();

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {
        perror("epoll_ctl stdin");
    }
    struct epoll_event events[MAX_EVENTS];
    print_help();

    while(running){
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
            if (nfds == -1) {
                perror("epoll_wait");
                break;
            }

        for(int i=0;i<nfds;i++){

            if(events[i].data.fd==STDIN_FILENO){
                cli_menu();

            }
            else if(events[i].data.fd==server_fd){
                handle_new_connection();
            }
            else{
                handle_driver_message(events[i].data.fd);
            }
        }

    }
    cleanup();



    close(server_fd);
    unlink(SOCKET_PATH);
    
    return 0;

        
}
    