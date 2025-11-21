#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include "shell.h"

void cleanupCommands(Commands* command) {
    for(int i = 0; i < command->count; i++) {
        free(command->commands[i].command);
        free(command->commands[i].inputFile);
        free(command->commands[i].outputFile);
        
        for(int j = 0; j < MAX_ARGS && command->commands[i].args[j] != NULL; j++) {
            free(command->commands[i].args[j]);
        }
    }
}
void initCommands(Commands* command) {
    for(int i = 0; i < MAX_COMMAND; i++) {
        memset(&command->commands[i], 0, sizeof(Command));
        command->commands[i].command = NULL;
        for(int j = 0; j < MAX_ARGS; j++) {
            command->commands[i].args[j] = NULL;
        }
        command->commands[i].inputFile = NULL;
        command->commands[i].outputFile = NULL;
    }
    command->count = 0;
}
int strParse(char* input, Commands* command){
    int count=0;
    char* argv[10];
    char* tmp = strtok(input,"|");
    
    while(tmp!=NULL && count<MAX_COMMAND){
        while(*tmp == ' ') tmp++;
        char* end = tmp + strlen(tmp) - 1;
        while(end > tmp && *end == ' ') end--;
        *(end + 1) = '\0';
        argv[count] = strdup(tmp);
        count++;
        tmp=strtok(NULL,"|");
    }
    command->count=count;
    
    for(int i=0; i<count;i++){
        char* tmp=strtok(argv[i]," ");
        int arg_count=0;
        
        while(tmp!=NULL && arg_count<MAX_ARGS){
            char* input_redirect = strchr(tmp, '<');
            char* double_output = strstr(tmp, ">>");
            char* single_output = strchr(tmp, '>');
            int processed = 0;
            
            if(input_redirect != NULL){
                command->commands[i].input_flag=1;
                if(strlen(input_redirect)>1){
                    command->commands[i].inputFile=strdup(input_redirect+1);
                } else {
                    tmp=strtok(NULL, " ");
                    if(tmp!=NULL)
                        command->commands[i].inputFile=strdup(tmp);
                }
                processed = 1;
            }
            else if (double_output != NULL){
                command->commands[i].output_flag=1;
                if(strlen(double_output)>2){
                    command->commands[i].outputFile=strdup(double_output+2);
                } else {
                    tmp=strtok(NULL," ");
                    if(tmp!=NULL)
                        command->commands[i].outputFile=strdup(tmp);
                }
                processed = 1;
            }
            else if(single_output != NULL && double_output == NULL){
                command->commands[i].outputCreate_flag=1;
                if(strlen(single_output)>1){
                    command->commands[i].outputFile=strdup(single_output+1);
                } else {
                    tmp=strtok(NULL," ");
                    if(tmp!=NULL)
                        command->commands[i].outputFile=strdup(tmp);
                }
                processed = 1; 
            }
            else if(arg_count == 0){
                command->commands[i].command=strdup(tmp);
                arg_count++;
            }
            else{
                if(arg_count < MAX_ARGS){
                    command->commands[i].args[arg_count-1]=strdup(tmp);
                    arg_count++;
                }
                processed = 1;
            }
            
            if (processed) {
                tmp = strtok(NULL, " ");
            }
        }
        command->commands[i].args[arg_count] = NULL;
        free(argv[i]);
    }
    return 0;
}

int shell(Commands* command,int count){
    if(count<0)return 0;
    int pipes[count-1][2];
    pid_t pid[count];
    for( int i=0;i<count-1;i++){
        if (pipe(pipes[i]) == -1) {
        perror("pipe");
        return 1;
        }
    }
    for(int i=0;i<count;i++){
        pid[i]=fork();
        if(pid[i]==-1){
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if(pid[i]==0){
            if (i==0 && command->commands[i].input_flag==1 && command->commands[i].inputFile != NULL ){
                int input_fd=open(command->commands[i].inputFile, O_RDONLY);
                if(input_fd==-1){
                    perror("open output file");
                    exit(EXIT_FAILURE);
                }
                dup2(input_fd,STDIN_FILENO);
                close(input_fd);
            }
            if (i==count-1 && (command->commands[i].output_flag || command->commands[i].outputCreate_flag) && command->commands[i].outputFile != NULL ){
                int flags = O_WRONLY | O_CREAT;
                if(command->commands[i].output_flag)
                    flags |=O_APPEND;
                else 
                    flags |= O_TRUNC;
                int output_fd=open(command->commands[i].outputFile, flags,0644);
                if(output_fd==-1){
                    perror("open output file");
                    exit(EXIT_FAILURE);
                }
                dup2(output_fd,STDOUT_FILENO);
                close(output_fd);
            }
            if(i>0){
                dup2(pipes[i-1][0], STDIN_FILENO);}
            if (i<count-1){
                dup2(pipes[i][1],STDOUT_FILENO);
            }
            for(int j=0;j<count-1;j++){
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            execvp(command->commands[i].command,command->commands[i].args );
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
    }
    for(int i=0;i<count-1;i++){
                close(pipes[i][0]);
                close(pipes[i][1]);
    }
    for(int i = 0; i < count; i++){
        waitpid(pid[i], NULL, 0);
    }
}