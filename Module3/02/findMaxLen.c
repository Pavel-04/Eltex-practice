#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
    if(argc<2){
        printf("enter more argc\n");
        return 0;}
    int i=0;
    int max=0;
    
    while(argv[i]!=NULL){
        int len=strlen(argv[i]);
        if(len>max)max=len;
        i++;
    }
    printf("%d\n", max);


}