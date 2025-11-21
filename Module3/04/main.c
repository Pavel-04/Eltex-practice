#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include "shell.h"
int main() {
    char input[256];
    char buffer[10];
    while(1){
        Commands command;
        initCommands(&command);
        printf("Enter command ");
        fgets(input,sizeof(input),stdin);
        input[strcspn(input,"\n")]=0;
    
        strParse(input,&command);
        /*
    for(int i=0;i<command.count;i++){
        printf("command %s \n",command.commands[i].command);
        int j=0;
        printf("Args");
        while(command.commands[i].args[j]!=NULL){
        printf(" %s",command.commands[i].args[j]);
        j++;
        }
        printf("\n");
        printf("input flag %d\n",command.commands[i].input_flag);
        printf("outputcreate flag %d\n",command.commands[i].outputCreate_flag);
        printf("output flag %d\n",command.commands[i].output_flag);
        printf("input file %s\n",command.commands[i].inputFile);
        printf("output file %s\n", command.commands[i].outputFile);
        printf("\n");
    }*/
        shell(&command,command.count);
        cleanupCommands(&command);
        printf("Try again? (1 - yes, 0 - no): ");
        fgets(buffer, sizeof(buffer), stdin);
        if (atoi(buffer) == 0) {
            break;
        }
    }
    return 0;
}