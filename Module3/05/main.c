#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
volatile sig_atomic_t count = 0;
volatile sig_atomic_t sig_count=0;
FILE *file = NULL;
void signalHandler(int sig){

    if(sig==SIGINT || sig==SIGQUIT){
        sig_count++;
        fprintf(file, "Сигнал %s\n", sig == SIGINT ? "SIGINT" : "SIGQUIT");
        fflush(file);
    }
    if(sig_count==3){
        if (file)fclose(file);
        exit(EXIT_SUCCESS);
    }
}
int main(){
    file = fopen("count.txt", "w");
    if (file == NULL) {
        perror("fopen file");
        exit(EXIT_FAILURE);
    }
    signal(SIGINT,signalHandler);
    signal(SIGQUIT,signalHandler);
    while(1){
        fprintf(file, "%d\n", count);
        fflush(file);
        count++;
        sleep(1);
    }
    fclose(file);

}