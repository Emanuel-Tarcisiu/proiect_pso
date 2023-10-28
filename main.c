#include<stdio.h>

int main(int argc, char* argv[]){
    printf("Hello there\n\n");

    if(argc>1){
        for(int i=0;i<argc;i++)
            printf("Arg[%i]: %s\n",i,argv[i]);

        printf("\n");
    }
    
    return 0;
}