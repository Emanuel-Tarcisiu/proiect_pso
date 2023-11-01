#include<stdio.h>
#include"unistd.h"

int main(int argc, char* argv[]){
    write(STDOUT_FILENO,"Inceput main.c:\n",17);

    if(argc > 1){
        for(int i=0;i<argc;i++)
            printf("Arg[%i]: %s\n",i,argv[i]);

        printf("\n");
    }

    write(STDOUT_FILENO,"Sfarsit\n",9);
    return 0;
}