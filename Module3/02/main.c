#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(){
    char command[15];
    char* args[10];
    char buffer[30];
    pid_t pid;
    while(1){
        int count = 0;
        for (int i = 0; i < 10; i++) {
            args[i] = NULL;
        }
        printf("Enter command name ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; 
        printf("\n");
        
        do{
            printf("Enter arg ");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            
            if (strlen(buffer) == 0) {
                break;
            }
            args[count] = malloc(strlen(buffer) + 1);
            strcpy(args[count], buffer);
            count++;
        } while (strlen(buffer) > 0 && count < 9);
        args[count] = NULL;
        char path[100];
        snprintf(path, sizeof(path), "./%s", command);
        pid=fork();
        if(pid==0){
            execvp(command,args);
            execv(path, args);
            perror("execv failed");
            _exit(EXIT_FAILURE);}
        else if(pid>0){
            int status;
            waitpid(pid, &status,0);
            printf("Child process finished\n");
        }
        else{
            perror("fork");
            exit(EXIT_FAILURE);

        }
        
        for (int i = 0; i < count; i++) {
            free(args[i]);
            args[i] = NULL;
        }
        printf("Try again? (1 - yes, 0 - no): ");
        fgets(buffer, sizeof(buffer), stdin);
        if (atoi(buffer) == 0) {
            break;
        }
    }
    
    return 0;
}