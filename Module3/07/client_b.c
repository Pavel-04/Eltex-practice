#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define QUEUE_A_TO_B "/chat_a_to_b"
#define QUEUE_B_TO_A "/chat_b_to_a"
#define EXIT_PRIORITY 255
#define MAX_MSG_SIZE 256
#define MAX_MSGS 10

void cleanup_queues(){
    mq_unlink(QUEUE_A_TO_B);
    mq_unlink(QUEUE_B_TO_A);
}

int main(){


    atexit(cleanup_queues);
    struct mq_attr attr;
    int priority;
    char buffer[MAX_MSG_SIZE];
    ssize_t bytes_read;
    mqd_t mq_send_fd, mq_receive_fd;
    if ((mq_send_fd = mq_open(QUEUE_B_TO_A,O_CREAT| O_WRONLY, 0600,&attr)) == (mqd_t)-1){
        perror("Creating queue error");
        return -1;
    }
    if ((mq_receive_fd = mq_open(QUEUE_A_TO_B,O_CREAT | O_RDONLY, 0600,&attr)) == (mqd_t)-1){
        perror("Creating queue error");
        return -1;
    }
    attr.mq_flags =0;
    attr.mq_maxmsg=MAX_MSGS;
    attr.mq_msgsize=MAX_MSG_SIZE;
    attr.mq_curmsgs=0;

    while(1){
        
        printf("B: ");
        fgets(buffer,sizeof(buffer),stdin);
        buffer[strcspn(buffer,"\n")]=0;

        if(strcmp(buffer,"exit")==0){
            if(mq_send(mq_send_fd,buffer,strlen(buffer),EXIT_PRIORITY)==-1){
                perror("mq_send exit error");
                
            }
            printf("Завершение...\n");
            break;
        }
        if(mq_send(mq_send_fd,buffer,strlen(buffer),1)==-1){
            perror("mq_send error");
            break;
        }
        
        printf("Ожидание от клиента A\n");
        bytes_read = mq_receive(mq_receive_fd,buffer,MAX_MSG_SIZE,&priority);
        if (bytes_read == -1){
            perror("mq_receive failes");
            break;
        }
        if(priority==EXIT_PRIORITY){
            printf("Клиент А отключился...\n");
            break;
        }
        buffer[bytes_read] = '\0';
        printf("A: %s\n", buffer);

        




    }

    mq_close(mq_send_fd);
    mq_close(mq_receive_fd);
    return 0;

}

