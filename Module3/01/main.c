#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "handler.h"

int main(int argc, char* argv[]){
    if(argc<2){
        printf("enter more argc\n");
        return 0;}
    pid_t pid;
    int midl=argc/2;
    switch (pid=fork()){
        case(-1):
            perror("fork");
            exit(EXIT_FAILURE);
        case(0):
            argcHandler(1,midl,argv);
            _exit(EXIT_SUCCESS);
        default:
            argcHandler(midl+1,argc-1,argv);
            wait(NULL);
            exit(EXIT_SUCCESS);
    }

}