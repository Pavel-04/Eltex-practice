#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "handler.h"
#include <ctype.h>
#include <unistd.h>
void argcHandler(int start,int end, char* argv[]){
    for(int i=start;i<=end;i++){
        printf("PID - %d ", getpid());
        int isNum=checkNumber(argv[i]);
        if(isNum){
            char* endptr;
            double number = strtod(argv[i],&endptr);
            printf("%s %f\n",argv[i],number*2);
        }
        else{
            printf("%s\n",argv[i]);
        }
    }


}
int checkNumber(char* str){
    if (str == NULL || *str == '\0') {
        return 0;
    }
    
    int has_digit = 0;
    int has_dot = 0;
    int i = 0;
    if(str[0]=='.')return 0;
    while (str[i] != '\0') {
        if (isdigit(str[i])) {
            has_digit = 1;
        } 
        else if (str[i] == '.') {
            if (has_dot || str[i + 1] == '\0') {
                return 0;
            }
            has_dot = 1;
        } else {
            return 0;
        }
        i++;
    }
    
    return has_digit ? 1 : 0;
}
