#ifndef SHELL_H
#define SHELL_H

#define MAX_COMMAND 10
#define MAX_ARGS 6

typedef struct {
    char* command;
    char* args[MAX_ARGS];
    int input_flag;
    int output_flag;
    int outputCreate_flag;
    char* inputFile;
    char* outputFile;
}Command;

typedef struct{
    Command commands[MAX_COMMAND];
    int count;
}Commands;

void initCommands(Commands* command);
int strParse(char* input, Commands* command);
int shell(Commands* command,int count);
void cleanupCommands(Commands* command);




#endif