#include "common.h"

int sockfd;
static volatile int is_busy = 0;
static time_t busy_until = 0; 
void send_msg(Message *msg){
    if(write(sockfd,msg,sizeof(Message))!=sizeof(Message)){
        perror("write to server");

    }
}
void timer_handler(int sig){
    if(sig==SIGALRM){
        is_busy=0;
        busy_until=0;
        Message status;
        memset(&status,0,sizeof(Message));
        strcpy(status.type,"RESPONSE");
        status.pid=getpid();
        status.task_timer=0;
        strcpy(status.status,"Available");
        strcpy(status.msg,"Available");
        send_msg(&status);
    }
}

void handle_commnad(Message * msg){
    Message response;
    memset(&response,0,sizeof(Message));
    response.pid=getpid();

    if(strcmp(msg->type,"TASK")==0){
        if(is_busy){
            strcpy(response.type,"ERROR");
            response.task_timer=(int)(busy_until-time(NULL));
            sprintf(response.msg,"Ошибка: Driver [%d] занят еще на %d секунд",getpid(),response.task_timer);

        }
        else{
            strcpy(response.type,"RESPONSE");
            is_busy=1;
            busy_until=time(NULL)+msg->task_timer;
            alarm(msg->task_timer);
            signal(SIGALRM,timer_handler);
            strcpy(response.status, "Busy");
            response.task_timer = msg->task_timer;
            sprintf(response.msg,"принял задание занят на %d секунд",response.task_timer);
            
        }
    }else if(strcmp(msg->type,"STATUS")==0){
        strcpy(response.type,"STATUS");
        if(is_busy){
            response.task_timer=(int)(busy_until-time(NULL));
            sprintf(response.msg,"статус Busy занят на %d секунд",response.task_timer);

        }
        else{
            sprintf(response.msg,"статус Available");
        }
    }else if(strcmp(msg->type,"EXIT")==0){
        printf("Driver [%d] отключаюсь...\n",getpid());
        close(sockfd);
        exit(0);
    }
    send_msg(&response);
}
int main(){

    Message msg;
    struct sockaddr_un addr;
    sockfd=socket(AF_UNIX,SOCK_STREAM,0);
    if(sockfd==-1){
        perror("socket create failed");
        exit(EXIT_FAILURE);
    }

    memset(&addr,0,sizeof(addr));
    addr.sun_family=AF_UNIX;
    strncpy(addr.sun_path,SOCKET_PATH,sizeof(addr.sun_path)-1);

    if(connect(sockfd,(struct sockaddr*)&addr,sizeof(addr))==-1){
        perror("connect failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    Message hello;
    memset(&hello,0,sizeof(hello));

    strcpy(hello.type,"CREATE");
    hello.pid=getpid();
    strcpy(hello.status,"Available");
    send_msg(&hello);
    while(1){

        int bytes_read=read(sockfd,&msg,sizeof(Message));
        
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                printf("Driver %d: Сервер отключился\n", getpid());
            }else {
                perror("read");
            }
        }
            handle_commnad(&msg);
    }
    close(sockfd);
}